/*
 * Copyright © 2022 Collabora, LTD
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
#ifndef VK_SAMPLER_H
#define VK_SAMPLER_H

#include "vk_object.h"
#include "vk_ycbcr_conversion.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline bool
vk_border_color_is_custom(VkBorderColor color)
{
   return color == VK_BORDER_COLOR_FLOAT_CUSTOM_EXT ||
          color == VK_BORDER_COLOR_INT_CUSTOM_EXT;
}

VkClearColorValue vk_border_color_value(VkBorderColor color);
bool vk_border_color_is_int(VkBorderColor color);

VkClearColorValue
vk_sampler_border_color_value(const VkSamplerCreateInfo *pCreateInfo,
                              VkFormat *format_out);

/* This struct needs to be hashable mem-comparable */
PRAGMA_DIAGNOSTIC_PUSH
PRAGMA_DIAGNOSTIC_ERROR(-Wpadded)
struct vk_sampler_state {
   /* Taken straight from VkSamplerCreateInfo */
   VkSamplerCreateFlags flags;
   VkFilter mag_filter;
   VkFilter min_filter;
   VkSamplerMipmapMode mipmap_mode;
   VkSamplerAddressMode address_mode_u;
   VkSamplerAddressMode address_mode_v;
   VkSamplerAddressMode address_mode_w;
   float mip_lod_bias;
   float max_anisotropy;
   VkCompareOp compare_op;
   float min_lod;
   float max_lod;
   VkBorderColor border_color;

   /* Booleans moved here for better packing */
   bool anisotropy_enable;
   bool compare_enable;
   bool unnormalized_coordinates;

   /** VkSamplerBorderColorComponentMappingCreateInfoEXT::srgb */
   bool image_view_is_srgb;

   bool has_ycbcr_conversion;
   bool _pad[3];

   /** Format of paired image views or VK_FORMAT_UNDEFINED
    *
    * This is taken either from VkSamplerYcbcrConversionCreateInfo::format or
    * VkSamplerCustomBorderColorCreateInfoEXT::format.
    */
   VkFormat format;

   /** Border color value
    *
    * If VkSamplerCreateInfo::borderColor is one of the Vulkan 1.0 enumerated
    * border colors, this will be the VkClearColorValue representation of that
    * value. VkSamplerCreateInfo::borderColor is VK_BORDER_COLOR_*_CUSTOM_EXT,
    * this is VkSamplerCustomBorderColorCreateInfoEXT::customBorderColor.
    */
   VkClearColorValue border_color_value;

   /** VkSamplerBorderColorComponentMappingCreateInfoEXT::components */
   VkComponentMapping border_color_component_mapping;

   /**
    * VkSamplerReductionModeCreateInfo::reductionMode or
    * VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE.
    */
   VkSamplerReductionMode reduction_mode;

   /** VkSamplerYcbcrConversionInfo::conversion or NULL
    *
    * We ensure that this is always NULL whenever vk_sampler::format is not a
    * YCbCr format.  This is important on Android where YCbCr conversion
    * objects are required for all EXTERNAL formats, even if they are not
    * YCbCr formats.
    */
   struct vk_ycbcr_conversion_state ycbcr_conversion;
};
PRAGMA_DIAGNOSTIC_POP

void vk_sampler_state_init(struct vk_sampler_state *state,
                           const VkSamplerCreateInfo *pCreateInfo);

struct vk_sampler {
   struct vk_object_base base;

   /** Format of paired image views or VK_FORMAT_UNDEFINED
    *
    * This is taken either from VkSamplerYcbcrConversionCreateInfo::format or
    * VkSamplerCustomBorderColorCreateInfoEXT::format.
    */
   VkFormat format;

   /** VkSamplerCreateInfo::borderColor */
   VkBorderColor border_color;

   /** Border color value
    *
    * If VkSamplerCreateInfo::borderColor is one of the Vulkan 1.0 enumerated
    * border colors, this will be the VkClearColorValue representation of that
    * value. VkSamplerCreateInfo::borderColor is VK_BORDER_COLOR_*_CUSTOM_EXT,
    * this is VkSamplerCustomBorderColorCreateInfoEXT::customBorderColor.
    */
   VkClearColorValue border_color_value;

   /**
    * VkSamplerReductionModeCreateInfo::reductionMode or
    * VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE.
    */
   VkSamplerReductionMode reduction_mode;

   /** VkSamplerYcbcrConversionInfo::conversion or NULL
    *
    * We ensure that this is always NULL whenever vk_sampler::format is not a
    * YCbCr format.  This is important on Android where YCbCr conversion
    * objects are required for all EXTERNAL formats, even if they are not
    * YCbCr formats.
    */
   struct vk_ycbcr_conversion *ycbcr_conversion;
};
VK_DEFINE_NONDISP_HANDLE_CASTS(vk_sampler, base, VkSampler,
                               VK_OBJECT_TYPE_SAMPLER);

void vk_sampler_init(struct vk_device *device,
                     struct vk_sampler *sampler,
                     const VkSamplerCreateInfo *pCreateInfo);
void vk_sampler_finish(struct vk_sampler *sampler);

void *vk_sampler_create(struct vk_device *device,
                        const VkSamplerCreateInfo *pCreateInfo,
                        const VkAllocationCallbacks *alloc,
                        size_t size);
void vk_sampler_destroy(struct vk_device *device,
                        const VkAllocationCallbacks *alloc,
                        struct vk_sampler *sampler);

#ifdef __cplusplus
}
#endif

#endif /* VK_SAMPLER_H */
