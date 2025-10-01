#include "zink_screen.h"

#include "zink_kopper.h"
#include "zink_compiler.h"
#include "zink_context.h"
#include "zink_descriptors.h"
#include "zink_fence.h"
#include "vk_format.h"
#include "zink_format.h"
#include "zink_program.h"
#include "zink_public.h"
#include "zink_query.h"
#include "zink_resource.h"
#include "zink_state.h"
#include "nir_to_spirv/nir_to_spirv.h" // for SPIRV_VERSION

#include "util/u_debug.h"
#include "util/u_dl.h"
#include "util/os_file.h"
#include "util/u_memory.h"
#include "util/u_screen.h"
#include "util/u_string.h"
#include "util/perf/u_trace.h"
#include "util/u_transfer_helper.h"
#include "util/hex.h"
#include "util/xmlconfig.h"

#include "util/u_cpu_detect.h"

#ifdef HAVE_LIBDRM
#include <xf86drm.h>
#include <fcntl.h>
#include <sys/stat.h>
#ifdef MAJOR_IN_MKDEV
#include <sys/mkdev.h>
#endif
#ifdef MAJOR_IN_SYSMACROS
#include <sys/sysmacros.h>
#endif
#endif

#if DETECT_OS_WINDOWS
#include <io.h>
#define VK_LIBNAME "vulkan-1.dll"
#else
#include <unistd.h>
#if DETECT_OS_APPLE
#define VK_LIBNAME "libvulkan.1.dylib"
#elif DETECT_OS_ANDROID
#define VK_LIBNAME "libvulkan.so"
#else
#define VK_LIBNAME "libvulkan.so.1"
#endif
#endif

#ifdef __APPLE__
#include "MoltenVK/mvk_vulkan.h"
// Source of MVK_VERSION
#include "MoltenVK/mvk_config.h"
#define VK_NO_PROTOTYPES
#include "MoltenVK/mvk_deprecated_api.h"
#include "MoltenVK/mvk_private_api.h"
#endif /* __APPLE__ */

#ifdef HAVE_LIBDRM
#include "drm-uapi/dma-buf.h"
#include <xf86drm.h>
#endif

#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>
#include <cstdint>
#include <pthread>
#include <memory>
#include <cstring>

// 模拟实现宏
#define ZINK_SIMULATE_FUNC(name) zink_simulate_##name

VkDevice get_command_buffer_device(VkCommandBuffer commandBuffer);
VkFormat get_image_view_format(VkImageView imageView);
void get_image_view_size(VkImageView imageView, uint32_t* width, uint32_t* height, uint32_t* layers);

void zink_simulate_vkDestroyImageView(
    VkDevice device,
    VkImageView imageView,
    const VkAllocationCallbacks* pAllocator);

VkResult zink_simulate_vkCreateImageView(
    VkDevice device,
    const VkImageViewCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkImageView* pImageView);

void zink_simulate_vkDestroyImage(
    VkDevice device,
    VkImage image,
    const VkAllocationCallbacks* pAllocator);

VkResult zink_simulate_vkCreateImage(
    VkDevice device,
    const VkImageCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkImage* pImage);

void zink_simulate_vkFreeCommandBuffers(
    VkDevice device,
    VkCommandPool commandPool,
    uint32_t commandBufferCount,
    const VkCommandBuffer* pCommandBuffers);

VkResult zink_simulate_vkAllocateCommandBuffers(
    VkDevice device,
    const VkCommandBufferAllocateInfo* pAllocateInfo,
    VkCommandBuffer* pCommandBuffers);

void zink_simulate_vkDestroyCommandPool(
    VkDevice device,
    VkCommandPool commandPool,
    const VkAllocationCallbacks* pAllocator);

VkResult zink_simulate_vkCreateCommandPool(
    VkDevice device,
    const VkCommandPoolCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkCommandPool* pCommandPool);

VkResult zink_simulate_vkCreateSemaphore(
    VkDevice device,
    const VkSemaphoreCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkSemaphore* pSemaphore);

void zink_simulate_vkDestroySemaphore(
    VkDevice device,
    VkSemaphore semaphore,
    const VkAllocationCallbacks* pAllocator);

VkResult zink_simulate_vkGetSemaphoreCounterValue(
    VkDevice device, 
    VkSemaphore semaphore, 
    uint64_t* pValue);

VkResult zink_simulate_vkWaitSemaphores(
    VkDevice device,
    const VkSemaphoreWaitInfo* pWaitInfo,
    uint64_t timeout);

VkResult zink_simulate_vkSignalSemaphore(
    VkDevice device,
    const VkSemaphoreSignalInfo* pSignalInfo);

VkResult zink_simulate_QueueSubmitWithTimeline(
    VkQueue queue,
    uint32_t submitCount,
    const VkSubmitInfo* pSubmits,
    VkFence fence);

VkResult zink_simulate_vkQueueSubmit2(
    VkQueue queue,
    uint32_t submitCount,
    const VkSubmitInfo2* pSubmits,
    VkFence fence);



void ZINK_SIMULATE_FUNC(vkCmdBeginRendering)(VkCommandBuffer commandBuffer, 
                                            const VkRenderingInfo* pRenderingInfo);

void ZINK_SIMULATE_FUNC(vkCmdEndRendering)(VkCommandBuffer commandBuffer);

VkResult ZINK_SIMULATE_FUNC(vkCreateGraphicsPipelines)(VkDevice device, 
                                                      VkPipelineCache pipelineCache,
                                                      uint32_t createInfoCount,
                                                      const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                      const VkAllocationCallbacks* pAllocator,
                                                      VkPipeline* pPipelines);
    
void ZINK_SIMULATE_FUNC(vkDestroyDevice)(VkDevice device, const VkAllocationCallbacks* pAllocator);

void ZINK_SIMULATE_FUNC(vkCmdPipelineBarrier2)(
    VkCommandBuffer commandBuffer,
    const VkDependencyInfo* pDependencyInfo);

