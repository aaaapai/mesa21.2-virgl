#ifndef MESA_STUB_AHB_H
#define MESA_STUB_AHB_H

#include "u_gralloc_internal.h"
#include <android/hardware_buffer.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ahb_gralloc_buffer {
    AHardwareBuffer *buffer;
    int width;
    int height;
    int format;
};

struct ahb_gralloc {
    struct u_gralloc base;
    struct ahb_gralloc_buffer *buffer;
};

struct u_gralloc *u_gralloc_ahb_create(void);

#ifdef __cplusplus
}
#endif

#endif /* MESA_STUB_AHB_H */
