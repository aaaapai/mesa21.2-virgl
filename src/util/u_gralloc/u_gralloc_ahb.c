#include "u_gralloc_ahb.h"
#include "util/log.h"
#include "util/u_memory.h"
#include <stdlib.h>
#include <errno.h>

static int ahb_get_buffer_info(struct u_gralloc *base,
                               struct u_gralloc_buffer_handle *hnd,
                               struct u_gralloc_buffer_basic_info *out)
{
    struct ahb_gralloc *gr = (struct ahb_gralloc *)base;
    if (!gr->buffer || !gr->buffer->buffer)
        return -EINVAL;

    out->drm_fourcc = 0;
    out->modifier = 0;
    out->num_planes = 1;
    out->fds[0] = 0;
    out->offsets[0] = 0;
    out->strides[0] = gr->buffer->width;

    return 0;
}

static int ahb_get_front_rendering_usage(struct u_gralloc *base, uint64_t *out_usage)
{
    *out_usage = 0;
    return 0;
}

static int ahb_destroy(struct u_gralloc *base)
{
    struct ahb_gralloc *gr = (struct ahb_gralloc *)base;
    if (gr->buffer) {
        if (gr->buffer->buffer)
            AHardwareBuffer_release(gr->buffer->buffer);
        FREE(gr->buffer);
    }
    FREE(gr);
    return 0;
}

struct u_gralloc *u_gralloc_ahb_create(void)
{
    struct ahb_gralloc *gr = CALLOC_STRUCT(ahb_gralloc);
    if (!gr)
        return NULL;

    gr->buffer = CALLOC_STRUCT(ahb_gralloc_buffer);
    if (!gr->buffer) {
        FREE(gr);
        return NULL;
    }

    gr->buffer->width = 512;
    gr->buffer->height = 512;
    gr->buffer->format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;

    AHardwareBuffer_Desc desc = {};
    desc.width = gr->buffer->width;
    desc.height = gr->buffer->height;
    desc.layers = 1;
    desc.format = gr->buffer->format;
    desc.usage = AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN | AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN;

    if (AHardwareBuffer_allocate(&desc, &gr->buffer->buffer) != 0) {
        mesa_loge("Failed to allocate AHardwareBuffer");
        ahb_destroy(&gr->base);
        return NULL;
    }

    gr->base.ops.get_buffer_basic_info = ahb_get_buffer_info;
    gr->base.ops.get_front_rendering_usage = ahb_get_front_rendering_usage;
    gr->base.ops.destroy = ahb_destroy;

    mesa_logi("Using pure AHardwareBuffer gralloc");

    return &gr->base;
}
