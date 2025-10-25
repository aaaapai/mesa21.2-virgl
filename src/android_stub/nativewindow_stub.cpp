#include <vndk/window.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <android/log.h>
#include <stdatomic.h>

#define LOG_TAG "NativeWindowWrapper"
#define ALOGE(...) do { printf("E/%s: ", LOG_TAG); printf(__VA_ARGS__); printf("\n"); } while(0)
#define ALOGW(...) do { printf("W/%s: ", LOG_TAG); printf(__VA_ARGS__); printf("\n"); } while(0)
#define ALOGI(...) do { printf("I/%s: ", LOG_TAG); printf(__VA_ARGS__); printf("\n"); } while(0)

static void* sNativeWindowHandle = NULL;

// 函数指针类型定义
typedef AHardwareBuffer* (*ANativeWindowBuffer_getHardwareBuffer_t)(ANativeWindowBuffer*);
typedef void (*AHardwareBuffer_acquire_t)(AHardwareBuffer*);
typedef void (*AHardwareBuffer_release_t)(AHardwareBuffer*);
typedef void (*AHardwareBuffer_describe_t)(const AHardwareBuffer*, AHardwareBuffer_Desc*);
typedef int (*AHardwareBuffer_allocate_t)(const AHardwareBuffer_Desc*, AHardwareBuffer**);
typedef const native_handle_t* (*AHardwareBuffer_getNativeHandle_t)(const AHardwareBuffer*);
typedef void (*ANativeWindow_acquire_t)(ANativeWindow*);
typedef void (*ANativeWindow_release_t)(ANativeWindow*);
typedef int32_t (*ANativeWindow_getFormat_t)(ANativeWindow*);
typedef int (*ANativeWindow_setSwapInterval_t)(ANativeWindow*, int);
typedef int (*ANativeWindow_query_t)(const ANativeWindow*, ANativeWindowQuery, int*);
typedef int (*ANativeWindow_dequeueBuffer_t)(ANativeWindow*, ANativeWindowBuffer**, int*);
typedef int (*ANativeWindow_queueBuffer_t)(ANativeWindow*, ANativeWindowBuffer*, int);
typedef int (*ANativeWindow_cancelBuffer_t)(ANativeWindow*, ANativeWindowBuffer*, int);
typedef int (*ANativeWindow_setUsage_t)(ANativeWindow*, uint64_t);
typedef int (*ANativeWindow_setSharedBufferMode_t)(ANativeWindow*, bool);
typedef int32_t (*ANativeWindow_getWidth_t)(ANativeWindow*);
typedef int32_t (*ANativeWindow_getHeight_t)(ANativeWindow*);

// 函数指针变量
static ANativeWindowBuffer_getHardwareBuffer_t fp_ANativeWindowBuffer_getHardwareBuffer = NULL;
static AHardwareBuffer_acquire_t fp_AHardwareBuffer_acquire = NULL;
static AHardwareBuffer_release_t fp_AHardwareBuffer_release = NULL;
static AHardwareBuffer_describe_t fp_AHardwareBuffer_describe = NULL;
static AHardwareBuffer_allocate_t fp_AHardwareBuffer_allocate = NULL;
static AHardwareBuffer_getNativeHandle_t fp_AHardwareBuffer_getNativeHandle = NULL;
static ANativeWindow_acquire_t fp_ANativeWindow_acquire = NULL;
static ANativeWindow_release_t fp_ANativeWindow_release = NULL;
static ANativeWindow_getFormat_t fp_ANativeWindow_getFormat = NULL;
static ANativeWindow_setSwapInterval_t fp_ANativeWindow_setSwapInterval = NULL;
static ANativeWindow_query_t fp_ANativeWindow_query = NULL;
static ANativeWindow_dequeueBuffer_t fp_ANativeWindow_dequeueBuffer = NULL;
static ANativeWindow_queueBuffer_t fp_ANativeWindow_queueBuffer = NULL;
static ANativeWindow_cancelBuffer_t fp_ANativeWindow_cancelBuffer = NULL;
static ANativeWindow_setUsage_t fp_ANativeWindow_setUsage = NULL;
static ANativeWindow_setSharedBufferMode_t fp_ANativeWindow_setSharedBufferMode = NULL;
static ANativeWindow_getWidth_t fp_ANativeWindow_getWidth = NULL;
static ANativeWindow_getHeight_t fp_ANativeWindow_getHeight = NULL;

static atomic_int isinitNativeWindowWrapper = 0;

static void initNativeWindowWrapper() {
    const char* libPaths[] = {
        "/system/lib64/libnativewindow.so",
        "/system/lib/libnativewindow.so",
        "libnativewindow.so",
        NULL
    };
    
    for (int i = 0; libPaths[i] != NULL; i++) {
       
        sNativeWindowHandle = dlopen("libnativewindow.so", RTLD_NOLOAD);
        if (sNativeWindowHandle != NULL) {
            ALOGI("Successfully loaded %s", libPaths[i]);
            break;
        } else {
            sNativeWindowHandle = dlopen(libPaths[i], RTLD_LAZY | RTLD_LOCAL);
            if (sNativeWindowHandle != NULL) {
              ALOGI("Successfully loaded %s", libPaths[i]);
              break;
            } else {
             ALOGW("Failed to load %s: %s", libPaths[i], dlerror());
            }
        }
    
    if (sNativeWindowHandle == NULL) {
        ALOGE("All library paths failed, using stub implementations");
        return;
    }
    
    // 解析函数符号
    fp_ANativeWindowBuffer_getHardwareBuffer = (ANativeWindowBuffer_getHardwareBuffer_t)
        dlsym(sNativeWindowHandle, "ANativeWindowBuffer_getHardwareBuffer");
    fp_AHardwareBuffer_acquire = (AHardwareBuffer_acquire_t)
        dlsym(sNativeWindowHandle, "AHardwareBuffer_acquire");
    fp_AHardwareBuffer_release = (AHardwareBuffer_release_t)
        dlsym(sNativeWindowHandle, "AHardwareBuffer_release");
    fp_AHardwareBuffer_describe = (AHardwareBuffer_describe_t)
        dlsym(sNativeWindowHandle, "AHardwareBuffer_describe");
    fp_AHardwareBuffer_allocate = (AHardwareBuffer_allocate_t)
        dlsym(sNativeWindowHandle, "AHardwareBuffer_allocate");
    fp_AHardwareBuffer_getNativeHandle = (AHardwareBuffer_getNativeHandle_t)
        dlsym(sNativeWindowHandle, "AHardwareBuffer_getNativeHandle");
    fp_ANativeWindow_acquire = (ANativeWindow_acquire_t)
        dlsym(sNativeWindowHandle, "ANativeWindow_acquire");
    fp_ANativeWindow_release = (ANativeWindow_release_t)
        dlsym(sNativeWindowHandle, "ANativeWindow_release");
    fp_ANativeWindow_getFormat = (ANativeWindow_getFormat_t)
        dlsym(sNativeWindowHandle, "ANativeWindow_getFormat");
    fp_ANativeWindow_setSwapInterval = (ANativeWindow_setSwapInterval_t)
        dlsym(sNativeWindowHandle, "ANativeWindow_setSwapInterval");
    fp_ANativeWindow_query = (ANativeWindow_query_t)
        dlsym(sNativeWindowHandle, "ANativeWindow_query");
    fp_ANativeWindow_dequeueBuffer = (ANativeWindow_dequeueBuffer_t)
        dlsym(sNativeWindowHandle, "ANativeWindow_dequeueBuffer");
    fp_ANativeWindow_queueBuffer = (ANativeWindow_queueBuffer_t)
        dlsym(sNativeWindowHandle, "ANativeWindow_queueBuffer");
    fp_ANativeWindow_cancelBuffer = (ANativeWindow_cancelBuffer_t)
        dlsym(sNativeWindowHandle, "ANativeWindow_cancelBuffer");
    fp_ANativeWindow_setUsage = (ANativeWindow_setUsage_t)
        dlsym(sNativeWindowHandle, "ANativeWindow_setUsage");
    fp_ANativeWindow_setSharedBufferMode = (ANativeWindow_setSharedBufferMode_t)
        dlsym(sNativeWindowHandle, "ANativeWindow_setSharedBufferMode");
    fp_ANativeWindow_getWidth = (ANativeWindow_getWidth_t)
        dlsym(sNativeWindowHandle, "ANativeWindow_getWidth");
    fp_ANativeWindow_getHeight = (ANativeWindow_getHeight_t)
        dlsym(sNativeWindowHandle, "ANativeWindow_getHeight");
    
    // 检查是否有函数解析失败
    if (!fp_ANativeWindowBuffer_getHardwareBuffer || !fp_AHardwareBuffer_acquire ||
        !fp_AHardwareBuffer_release || !fp_ANativeWindow_acquire || !fp_ANativeWindow_release) {
        ALOGW("Some functions failed to resolve, library may be incomplete");
    }
}

}

extern "C" {

AHardwareBuffer* ANativeWindowBuffer_getHardwareBuffer(ANativeWindowBuffer* anwb) {

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitNativeWindowWrapper, &expected, 1)) {
        initNativeWindowWrapper();
    }

    if (fp_ANativeWindowBuffer_getHardwareBuffer) {
        return fp_ANativeWindowBuffer_getHardwareBuffer(anwb);
    }
    return nullptr;
}

void AHardwareBuffer_acquire(AHardwareBuffer* buffer) {

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitNativeWindowWrapper, &expected, 1)) {
        initNativeWindowWrapper();
    }

    if (fp_AHardwareBuffer_acquire) {
        fp_AHardwareBuffer_acquire(buffer);
    }
}

void AHardwareBuffer_release(AHardwareBuffer* buffer) {

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitNativeWindowWrapper, &expected, 1)) {
        initNativeWindowWrapper();
    }
   
    if (fp_AHardwareBuffer_release) {
        fp_AHardwareBuffer_release(buffer);
    }
}

void AHardwareBuffer_describe(const AHardwareBuffer* buffer, AHardwareBuffer_Desc* outDesc) {

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitNativeWindowWrapper, &expected, 1)) {
        initNativeWindowWrapper();
    }
   
    if (fp_AHardwareBuffer_describe) {
        fp_AHardwareBuffer_describe(buffer, outDesc);
    }
}

int AHardwareBuffer_allocate(const AHardwareBuffer_Desc* desc, AHardwareBuffer** outBuffer) {

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitNativeWindowWrapper, &expected, 1)) {
        initNativeWindowWrapper();
    }
   
    if (fp_AHardwareBuffer_allocate) {
        return fp_AHardwareBuffer_allocate(desc, outBuffer);
    }
    return 0;
}

const native_handle_t* AHardwareBuffer_getNativeHandle(const AHardwareBuffer* buffer) {

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitNativeWindowWrapper, &expected, 1)) {
        initNativeWindowWrapper();
    }
   
    if (fp_AHardwareBuffer_getNativeHandle) {
        return fp_AHardwareBuffer_getNativeHandle(buffer);
    }
    return NULL;
}

void ANativeWindow_acquire(ANativeWindow* window) {

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitNativeWindowWrapper, &expected, 1)) {
        initNativeWindowWrapper();
    }
   
    if (fp_ANativeWindow_acquire) {
        fp_ANativeWindow_acquire(window);
    }
}

void ANativeWindow_release(ANativeWindow* window) {

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitNativeWindowWrapper, &expected, 1)) {
        initNativeWindowWrapper();
    }
   
    if (fp_ANativeWindow_release) {
        fp_ANativeWindow_release(window);
    }
}

int32_t ANativeWindow_getFormat(ANativeWindow* window) {

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitNativeWindowWrapper, &expected, 1)) {
        initNativeWindowWrapper();
    }
   
    if (fp_ANativeWindow_getFormat) {
        return fp_ANativeWindow_getFormat(window);
    }
    return 0;
}

int ANativeWindow_setSwapInterval(ANativeWindow* window, int interval) {

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitNativeWindowWrapper, &expected, 1)) {
        initNativeWindowWrapper();
    }
   
    if (fp_ANativeWindow_setSwapInterval) {
        return fp_ANativeWindow_setSwapInterval(window, interval);
    }
    return 0;
}

int ANativeWindow_query(const ANativeWindow* window, ANativeWindowQuery query, int* value) {

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitNativeWindowWrapper, &expected, 1)) {
        initNativeWindowWrapper();
    }
   
    if (fp_ANativeWindow_query) {
        return fp_ANativeWindow_query(window, query, value);
    }
    return 0;
}

int ANativeWindow_dequeueBuffer(ANativeWindow* window, ANativeWindowBuffer** buffer, int* fenceFd) {

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitNativeWindowWrapper, &expected, 1)) {
        initNativeWindowWrapper();
    }
   
    if (fp_ANativeWindow_dequeueBuffer) {
        return fp_ANativeWindow_dequeueBuffer(window, buffer, fenceFd);
    }
    return 0;
}

int ANativeWindow_queueBuffer(ANativeWindow* window, ANativeWindowBuffer* buffer, int fenceFd) {

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitNativeWindowWrapper, &expected, 1)) {
        initNativeWindowWrapper();
    }

    if (fp_ANativeWindow_queueBuffer) {
        return fp_ANativeWindow_queueBuffer(window, buffer, fenceFd);
    }
    return 0;
}

int ANativeWindow_cancelBuffer(ANativeWindow* window, ANativeWindowBuffer* buffer, int fenceFd) {

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitNativeWindowWrapper, &expected, 1)) {
        initNativeWindowWrapper();
    }
   
    if (fp_ANativeWindow_cancelBuffer) {
        return fp_ANativeWindow_cancelBuffer(window, buffer, fenceFd);
    }
    return 0;
}

int ANativeWindow_setUsage(ANativeWindow* window, uint64_t usage) {

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitNativeWindowWrapper, &expected, 1)) {
        initNativeWindowWrapper();
    }
   
    if (fp_ANativeWindow_setUsage) {
        return fp_ANativeWindow_setUsage(window, usage);
    }
    return 0;
}

int ANativeWindow_setSharedBufferMode(ANativeWindow* window, bool sharedBufferMode) {

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitNativeWindowWrapper, &expected, 1)) {
        initNativeWindowWrapper();
    }
   
    if (fp_ANativeWindow_setSharedBufferMode) {
        return fp_ANativeWindow_setSharedBufferMode(window, sharedBufferMode);
    }
    return 0;
}

int32_t ANativeWindow_getWidth(ANativeWindow* window) {

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitNativeWindowWrapper, &expected, 1)) {
        initNativeWindowWrapper();
    }
   
    if (fp_ANativeWindow_getWidth) {
        return fp_ANativeWindow_getWidth(window);
    }
    return 0;
}

int32_t ANativeWindow_getHeight(ANativeWindow* window) {

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitNativeWindowWrapper, &expected, 1)) {
        initNativeWindowWrapper();
    }
   
    if (fp_ANativeWindow_getHeight) {
        return fp_ANativeWindow_getHeight(window);
    }
    return 0;
}

}
