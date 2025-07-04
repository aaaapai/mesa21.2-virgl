/*
 * Copyright (c) 2008-2024 Broadcom. All Rights Reserved.
 * The term “Broadcom” refers to Broadcom Inc.
 * and/or its subsidiaries.
 * SPDX-License-Identifier: MIT
 */

#ifndef SVGA_STATE_H
#define SVGA_STATE_H


#include "util/compiler.h"
#include "pipe/p_defines.h"

struct svga_context;


void
svga_init_state(struct svga_context *svga);

void
svga_destroy_state(struct svga_context *svga);


struct svga_tracked_state {
   const char *name;
   uint64_t dirty;
   enum pipe_error (*update)(struct svga_context *svga, uint64_t dirty);
};

/* NEED_SWTNL
 */
extern struct svga_tracked_state svga_update_need_swvfetch;
extern struct svga_tracked_state svga_update_need_pipeline;
extern struct svga_tracked_state svga_update_need_swtnl;

/* HW_CLEAR
 */
extern struct svga_tracked_state svga_hw_viewport;
extern struct svga_tracked_state svga_hw_scissor;
extern struct svga_tracked_state svga_hw_framebuffer;

/* HW_DRAW
 */
extern struct svga_tracked_state svga_need_tgsi_transform;
extern struct svga_tracked_state svga_hw_vs;
extern struct svga_tracked_state svga_hw_fs;
extern struct svga_tracked_state svga_hw_gs;
extern struct svga_tracked_state svga_hw_tcs;
extern struct svga_tracked_state svga_hw_tes;
extern struct svga_tracked_state svga_hw_rss;
extern struct svga_tracked_state svga_hw_pstipple;
extern struct svga_tracked_state svga_hw_sampler;
extern struct svga_tracked_state svga_hw_sampler_bindings;
extern struct svga_tracked_state svga_hw_tss;
extern struct svga_tracked_state svga_hw_tss_binding;
extern struct svga_tracked_state svga_hw_clip_planes;
extern struct svga_tracked_state svga_hw_vdecl;
extern struct svga_tracked_state svga_hw_fs_constants;
extern struct svga_tracked_state svga_hw_gs_constants;
extern struct svga_tracked_state svga_hw_vs_constants;
extern struct svga_tracked_state svga_hw_tes_constants;
extern struct svga_tracked_state svga_hw_tcs_constants;
extern struct svga_tracked_state svga_hw_cs_constants;
extern struct svga_tracked_state svga_hw_fs_constbufs;
extern struct svga_tracked_state svga_hw_vs_constbufs;
extern struct svga_tracked_state svga_hw_gs_constbufs;
extern struct svga_tracked_state svga_hw_tcs_constbufs;
extern struct svga_tracked_state svga_hw_tes_constbufs;
extern struct svga_tracked_state svga_hw_cs_constbufs;
extern struct svga_tracked_state svga_hw_uav;

extern struct svga_tracked_state svga_hw_cs;
extern struct svga_tracked_state svga_hw_cs_uav;
extern struct svga_tracked_state svga_hw_cs_sampler;
extern struct svga_tracked_state svga_hw_cs_sampler_bindings;

extern struct svga_tracked_state svga_need_rawbuf_srv;
extern struct svga_tracked_state svga_cs_need_rawbuf_srv;

/* SWTNL_DRAW
 */
extern struct svga_tracked_state svga_update_swtnl_draw;
extern struct svga_tracked_state svga_update_swtnl_vdecl;

/* Bring the hardware fully up-to-date so that we can emit draw
 * commands.
 */
#define SVGA_STATE_NEED_SWTNL        0
#define SVGA_STATE_HW_CLEAR          1
#define SVGA_STATE_HW_DRAW           2
#define SVGA_STATE_SWTNL_DRAW        3
#define SVGA_STATE_MAX               4


enum pipe_error
svga_update_state(struct svga_context *svga, unsigned level);

bool
svga_update_state_retry(struct svga_context *svga, unsigned level);

enum pipe_error
svga_emit_initial_state(struct svga_context *svga);

enum pipe_error
svga_reemit_framebuffer_bindings(struct svga_context *svga);

enum pipe_error
svga_rebind_framebuffer_bindings(struct svga_context *svga);

enum pipe_error
svga_reemit_tss_bindings(struct svga_context *svga);

enum pipe_error
svga_reemit_vs_bindings(struct svga_context *svga);

enum pipe_error
svga_reemit_fs_bindings(struct svga_context *svga);

void
svga_init_tracked_state(struct svga_context *svga);

void *
svga_create_fs_state(struct pipe_context *pipe,
                     const struct pipe_shader_state *templ);

void
svga_bind_fs_state(struct pipe_context *pipe, void *shader);

bool svga_update_compute_state(struct svga_context *svga);

#endif
