/*
 * Copyright © 2016 Rob Clark <robclark@freedesktop.org>
 * Copyright © 2018 Google, Inc.
 * SPDX-License-Identifier: MIT
 *
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#define FD_BO_NO_HARDPIN 1

#include "pipe/p_state.h"
#include "util/u_memory.h"
#include "util/u_string.h"

#include "fd6_context.h"
#include "fd6_pack.h"
#include "fd6_zsa.h"

/* update lza state based on stencil-test func:
 *
 * Conceptually the order of the pipeline is:
 *
 *
 *   FS -> Alpha-Test  ->  Stencil-Test  ->  Depth-Test
 *                              |                |
 *                       if wrmask != 0     if wrmask != 0
 *                              |                |
 *                              v                v
 *                        Stencil-Write      Depth-Write
 *
 * Because Stencil-Test can have side effects (Stencil-Write) prior
 * to depth test, in this case we potentially need to disable early
 * lrz-test.  See:
 *
 * https://www.khronos.org/opengl/wiki/Per-Sample_Processing
 */
static void
update_lrz_stencil(struct fd6_zsa_stateobj *so, enum pipe_compare_func func,
                   bool stencil_write)
{
   switch (func) {
   case PIPE_FUNC_ALWAYS:
      /* nothing to do for LRZ, but for stencil test when stencil-
       * write is enabled, we need to disable lrz-test, since
       * conceptually stencil test and write happens before depth-
       * test:
       */
      if (stencil_write) {
         so->lrz.enable = false;
         so->lrz.test = false;
      }
      break;
   case PIPE_FUNC_NEVER:
      /* fragment never passes, disable lrz_write for this draw: */
      so->lrz.write = false;
      break;
   default:
      /* whether the fragment passes or not depends on result
       * of stencil test, which we cannot know when doing binning
       * pass:
       */
      so->lrz.write = false;
      /* similarly to the PIPE_FUNC_ALWAY case, if there are side-
       * effects from stencil test we need to disable lrz-test.
       */
      if (stencil_write) {
         so->lrz.enable = false;
         so->lrz.test = false;
      }
      break;
   }
}

template <chip CHIP>
void *
fd6_zsa_state_create(struct pipe_context *pctx,
                     const struct pipe_depth_stencil_alpha_state *cso)
{
   struct fd_context *ctx = fd_context(pctx);
   struct fd6_zsa_stateobj *so;

   so = CALLOC_STRUCT(fd6_zsa_stateobj);
   if (!so)
      return NULL;

   so->base = *cso;

   so->writes_zs = util_writes_depth_stencil(cso);
   so->writes_z = util_writes_depth(cso);

   enum adreno_compare_func depth_func =
      (enum adreno_compare_func)cso->depth_func; /* maps 1:1 */

   /* On some GPUs it is necessary to enable z test for depth bounds test
    * when UBWC is enabled. Otherwise, the GPU would hang. FUNC_ALWAYS is
    * required to pass z test. Relevant tests:
    *  dEQP-VK.pipeline.extended_dynamic_state.two_draws_dynamic.depth_bounds_test_disable
    *  dEQP-VK.dynamic_state.ds_state.depth_bounds_1
    */
   if (cso->depth_bounds_test && !cso->depth_enabled &&
       ctx->screen->info->a6xx.depth_bounds_require_depth_test_quirk) {
      so->rb_depth_cntl |= A6XX_RB_DEPTH_CNTL_Z_TEST_ENABLE;
      depth_func = FUNC_ALWAYS;
   }

   so->rb_depth_cntl |= A6XX_RB_DEPTH_CNTL_ZFUNC(depth_func);

   if (cso->depth_enabled) {
      so->rb_depth_cntl |=
         A6XX_RB_DEPTH_CNTL_Z_TEST_ENABLE | A6XX_RB_DEPTH_CNTL_Z_READ_ENABLE;

      so->lrz.test = true;

      if (cso->depth_writemask) {
         so->lrz.write = true;
      }

      switch (cso->depth_func) {
      case PIPE_FUNC_LESS:
      case PIPE_FUNC_LEQUAL:
         so->lrz.enable = true;
         so->lrz.direction = FD_LRZ_LESS;
         break;

      case PIPE_FUNC_GREATER:
      case PIPE_FUNC_GEQUAL:
         so->lrz.enable = true;
         so->lrz.direction = FD_LRZ_GREATER;
         break;

      case PIPE_FUNC_NEVER:
         so->lrz.enable = true;
         so->lrz.write = false;
         so->lrz.direction = FD_LRZ_LESS;
         break;

      case PIPE_FUNC_ALWAYS:
      case PIPE_FUNC_NOTEQUAL:
         if (cso->depth_writemask) {
            perf_debug_ctx(ctx, "Invalidating LRZ due to ALWAYS/NOTEQUAL with depth write");
            so->lrz.write = false;
            so->invalidate_lrz = true;
         } else {
            perf_debug_ctx(ctx, "Skipping LRZ due to ALWAYS/NOTEQUAL");
            so->lrz.enable = false;
            so->lrz.write = false;
         }
         break;

      case PIPE_FUNC_EQUAL:
         so->lrz.enable = false;
         so->lrz.write = false;
         break;
      }
   }

   if (cso->depth_writemask)
      so->rb_depth_cntl |= A6XX_RB_DEPTH_CNTL_Z_WRITE_ENABLE;

   if (cso->stencil[0].enabled) {
      const struct pipe_stencil_state *s = &cso->stencil[0];

      /* stencil test happens before depth test, so without performing
       * stencil test we don't really know what the updates to the
       * depth buffer will be.
       */
      update_lrz_stencil(so, (enum pipe_compare_func)s->func, util_writes_stencil(s));

      so->rb_stencil_control |=
         A6XX_RB_STENCIL_CNTL_STENCIL_READ |
         A6XX_RB_STENCIL_CNTL_STENCIL_ENABLE |
         A6XX_RB_STENCIL_CNTL_FUNC((enum adreno_compare_func)s->func) | /* maps 1:1 */
         A6XX_RB_STENCIL_CNTL_FAIL(fd_stencil_op(s->fail_op)) |
         A6XX_RB_STENCIL_CNTL_ZPASS(fd_stencil_op(s->zpass_op)) |
         A6XX_RB_STENCIL_CNTL_ZFAIL(fd_stencil_op(s->zfail_op));

      so->rb_stencilmask = A6XX_RB_STENCIL_MASK_MASK(s->valuemask);
      so->rb_stencilwrmask = A6XX_RB_STENCIL_WRITE_MASK_WRMASK(s->writemask);

      if (cso->stencil[1].enabled) {
         const struct pipe_stencil_state *bs = &cso->stencil[1];

         update_lrz_stencil(so, (enum pipe_compare_func)bs->func, util_writes_stencil(bs));

         so->rb_stencil_control |=
            A6XX_RB_STENCIL_CNTL_STENCIL_ENABLE_BF |
            A6XX_RB_STENCIL_CNTL_FUNC_BF((enum adreno_compare_func)bs->func) | /* maps 1:1 */
            A6XX_RB_STENCIL_CNTL_FAIL_BF(fd_stencil_op(bs->fail_op)) |
            A6XX_RB_STENCIL_CNTL_ZPASS_BF(fd_stencil_op(bs->zpass_op)) |
            A6XX_RB_STENCIL_CNTL_ZFAIL_BF(fd_stencil_op(bs->zfail_op));

         so->rb_stencilmask |= A6XX_RB_STENCIL_MASK_BFMASK(bs->valuemask);
         so->rb_stencilwrmask |= A6XX_RB_STENCIL_WRITE_MASK_BFWRMASK(bs->writemask);
      }
   }

   if (cso->alpha_enabled) {
      /* Alpha test is functionally a conditional discard, so we can't
       * write LRZ before seeing if we end up discarding or not
       */
      if (cso->alpha_func != PIPE_FUNC_ALWAYS) {
         so->lrz.write = false;
         so->alpha_test = true;
      }

      uint32_t ref = cso->alpha_ref_value * 255.0f;
      so->rb_alpha_control =
         A6XX_RB_ALPHA_TEST_CNTL_ALPHA_TEST |
         A6XX_RB_ALPHA_TEST_CNTL_ALPHA_REF(ref) |
         A6XX_RB_ALPHA_TEST_CNTL_ALPHA_TEST_FUNC(
               (enum adreno_compare_func)cso->alpha_func);
   }

   if (cso->depth_bounds_test) {
      so->rb_depth_cntl |= A6XX_RB_DEPTH_CNTL_Z_BOUNDS_ENABLE |
                           A6XX_RB_DEPTH_CNTL_Z_READ_ENABLE;
      so->lrz.z_bounds_enable = true;
   }

   /* Build the four state permutations (with/without alpha/depth-clamp)*/
   for (int i = 0; i < 4; i++) {
      struct fd_ringbuffer *ring = fd_ringbuffer_new_object(ctx->pipe, 16 * 4);
      bool depth_clamp_enable = (i & FD6_ZSA_DEPTH_CLAMP);

      OUT_PKT4(ring, REG_A6XX_RB_ALPHA_TEST_CNTL, 1);
      OUT_RING(ring,
               (i & FD6_ZSA_NO_ALPHA)
                  ? so->rb_alpha_control & ~A6XX_RB_ALPHA_TEST_CNTL_ALPHA_TEST
                  : so->rb_alpha_control);

      OUT_PKT4(ring, REG_A6XX_RB_STENCIL_CNTL, 1);
      OUT_RING(ring, so->rb_stencil_control);

      OUT_REG(ring, A6XX_GRAS_SU_STENCIL_CNTL(cso->stencil[0].enabled));

      OUT_PKT4(ring, REG_A6XX_RB_DEPTH_CNTL, 1);
      OUT_RING(ring,
               so->rb_depth_cntl | COND(depth_clamp_enable || CHIP >= A7XX,
                                        A6XX_RB_DEPTH_CNTL_Z_CLAMP_ENABLE));

      OUT_REG(ring, A6XX_GRAS_SU_DEPTH_CNTL(cso->depth_enabled));

      OUT_PKT4(ring, REG_A6XX_RB_STENCIL_MASK, 2);
      OUT_RING(ring, so->rb_stencilmask);
      OUT_RING(ring, so->rb_stencilwrmask);

      if (CHIP >= A7XX && !depth_clamp_enable) {
         OUT_REG(ring,
            A6XX_RB_DEPTH_BOUND_MIN(0.0f),
            A6XX_RB_DEPTH_BOUND_MAX(1.0f),
         );
      } else {
         OUT_REG(ring,
            A6XX_RB_DEPTH_BOUND_MIN(cso->depth_bounds_min),
            A6XX_RB_DEPTH_BOUND_MAX(cso->depth_bounds_max),
         );
      }

      so->stateobj[i] = ring;
   }

   return so;
}
FD_GENX(fd6_zsa_state_create);

void
fd6_zsa_state_delete(struct pipe_context *pctx, void *hwcso)
{
   struct fd6_zsa_stateobj *so = (struct fd6_zsa_stateobj *)hwcso;

   for (int i = 0; i < ARRAY_SIZE(so->stateobj); i++)
      fd_ringbuffer_del(so->stateobj[i]);
   FREE(hwcso);
}
