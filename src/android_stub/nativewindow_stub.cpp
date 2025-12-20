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

static atomic_int isinitNativeWindowWrapper(0);

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
            ALOGI("Successfully loaded libnativewindow.so");
            break;
        } else {
            sNativeWindowHandle = /*dlopen(libPaths[i], RTLD_LAZY | RTLD_LOCAL);*/ NULL;
            if (sNativeWindowHandle != NULL) {
              ALOGI("Successfully loaded %s", libPaths[i]);
              break;
            } else {
             ALOGW("Failed to load %s: %s", libPaths[i], dlerror());
            }
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

extern "C" {

AHardwareBuffer* ANativeWindowBuffer_getHardwareBuffer(ANativeWindowBuffer* anwb) {
    printf("ANativeWindowBuffer_getHardwareBuffer called with anwb=%p\n", anwb);

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitNativeWindowWrapper, &expected, 1)) {
        initNativeWindowWrapper();
    }

    if (fp_ANativeWindowBuffer_getHardwareBuffer) {
        return fp_ANativeWindowBuffer_getHardwareBuffer(anwb);
    }
    printf("Failed to call fp_ANativeWindowBuffer_getHardwareBuffer, will return NULL.\n");
    return nullptr;
}

void AHardwareBuffer_acquire(AHardwareBuffer* buffer) {
    printf("AHardwareBuffer_acquire called with buffer=%p\n", buffer);

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitNativeWindowWrapper, &expected, 1)) {
        initNativeWindowWrapper();
    }

    if (fp_AHardwareBuffer_acquire) {
        fp_AHardwareBuffer_acquire(buffer);
        return;
    }
    printf("Failed to call fp_AHardwareBuffer_acquire.\n");
}

void AHardwareBuffer_release(AHardwareBuffer* buffer) {
    printf("AHardwareBuffer_release called with buffer=%p\n", buffer);

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitNativeWindowWrapper, &expected, 1)) {
        initNativeWindowWrapper();
    }
   
    if (fp_AHardwareBuffer_release) {
        fp_AHardwareBuffer_release(buffer);
        return;
    }
    printf("Failed to call fp_AHardwareBuffer_release.\n");
}

void AHardwareBuffer_describe(const AHardwareBuffer* buffer, AHardwareBuffer_Desc* outDesc) {
    printf("AHardwareBuffer_describe called with buffer=%p, outDesc=%p\n", buffer, outDesc);

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitNativeWindowWrapper, &expected, 1)) {
        initNativeWindowWrapper();
    }
   
    if (fp_AHardwareBuffer_describe) {
        fp_AHardwareBuffer_describe(buffer, outDesc);
        return;
    }
    printf("Failed to call fp_AHardwareBuffer_describe.\n");
}

int AHardwareBuffer_allocate(const AHardwareBuffer_Desc* desc, AHardwareBuffer** outBuffer) {
    printf("AHardwareBuffer_allocate called with desc=%p, outBuffer=%p\n", desc, outBuffer);

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitNativeWindowWrapper, &expected, 1)) {
        initNativeWindowWrapper();
    }
   
    if (fp_AHardwareBuffer_allocate) {
        return fp_AHardwareBuffer_allocate(desc, outBuffer);
    }
    printf("Failed to call fp_AHardwareBuffer_allocate, will return 0.\n");
    return 0;
}

const native_handle_t* AHardwareBuffer_getNativeHandle(const AHardwareBuffer* buffer) {
    printf("AHardwareBuffer_getNativeHandle called with buffer=%p\n", buffer);

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitNativeWindowWrapper, &expected, 1)) {
        initNativeWindowWrapper();
    }
   
    if (fp_AHardwareBuffer_getNativeHandle) {
        return fp_AHardwareBuffer_getNativeHandle(buffer);
    }
    printf("Failed to call fp_AHardwareBuffer_getNativeHandle, will return NULL.\n");
    return NULL;
}

void ANativeWindow_acquire(ANativeWindow* window) {
    printf("ANativeWindow_acquire called with window=%p\n", window);

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitNativeWindowWrapper, &expected, 1)) {
        initNativeWindowWrapper();
    }
   
    if (fp_ANativeWindow_acquire) {
        fp_ANativeWindow_acquire(window);
        return;
    }
    printf("Failed to call fp_ANativeWindow_acquire.\n");
}

void ANativeWindow_release(ANativeWindow* window) {
    printf("ANativeWindow_release called with window=%p\n", window);

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitNativeWindowWrapper, &expected, 1)) {
        initNativeWindowWrapper();
    }
   
    if (fp_ANativeWindow_release) {
        fp_ANativeWindow_release(window);
        return;
    }
    printf("Failed to call fp_ANativeWindow_release.\n");
}

int32_t ANativeWindow_getFormat(ANativeWindow* window) {
    printf("ANativeWindow_getFormat called with window=%p\n", window);

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitNativeWindowWrapper, &expected, 1)) {
        initNativeWindowWrapper();
    }
   
    if (fp_ANativeWindow_getFormat) {
        return fp_ANativeWindow_getFormat(window);
    }
    printf("Failed to call fp_ANativeWindow_getFormat, will return 0.\n");
    return 0;
}

int ANativeWindow_setSwapInterval(ANativeWindow* window, int interval) {
    printf("ANativeWindow_setSwapInterval called with window=%p, interval=%d\n", window, interval);

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitNativeWindowWrapper, &expected, 1)) {
        initNativeWindowWrapper();
    }
   
    if (fp_ANativeWindow_setSwapInterval) {
        return fp_ANativeWindow_setSwapInterval(window, interval);
    }
    printf("Failed to call fp_ANativeWindow_setSwapInterval, will return 0.\n");
    return 0;
}

int ANativeWindow_query(const ANativeWindow* window, ANativeWindowQuery query, int* value) {
    printf("ANativeWindow_query called with window=%p, query=%d, value=%p\n", window, query, value);

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitNativeWindowWrapper, &expected, 1)) {
        initNativeWindowWrapper();
    }
   
    if (fp_ANativeWindow_query) {
        return fp_ANativeWindow_query(window, query, value);
    }
    printf("Failed to call fp_ANativeWindow_query, will return 0.\n");
    return 0;
}

int ANativeWindow_dequeueBuffer(ANativeWindow* window, ANativeWindowBuffer** buffer, int* fenceFd) {
    printf("ANativeWindow_dequeueBuffer called with window=%p, buffer=%p, fenceFd=%p\n", window, buffer, fenceFd);

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitNativeWindowWrapper, &expected, 1)) {
        initNativeWindowWrapper();
    }
   
    if (fp_ANativeWindow_dequeueBuffer) {
        return fp_ANativeWindow_dequeueBuffer(window, buffer, fenceFd);
    }
    printf("Failed to call fp_ANativeWindow_dequeueBuffer, will return 0.\n");
    return 0;
}

int ANativeWindow_queueBuffer(ANativeWindow* window, ANativeWindowBuffer* buffer, int fenceFd) {
    printf("ANativeWindow_queueBuffer called with window=%p, buffer=%p, fenceFd=%d\n", window, buffer, fenceFd);

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitNativeWindowWrapper, &expected, 1)) {
        initNativeWindowWrapper();
    }

    if (fp_ANativeWindow_queueBuffer) {
        return fp_ANativeWindow_queueBuffer(window, buffer, fenceFd);
    }
    printf("Failed to call fp_ANativeWindow_queueBuffer, will return 0.\n");
    return 0;
}

int ANativeWindow_cancelBuffer(ANativeWindow* window, ANativeWindowBuffer* buffer, int fenceFd) {
    printf("ANativeWindow_cancelBuffer called with window=%p, buffer=%p, fenceFd=%d\n", window, buffer, fenceFd);

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitNativeWindowWrapper, &expected, 1)) {
        initNativeWindowWrapper();
    }
   
    if (fp_ANativeWindow_cancelBuffer) {
        return fp_ANativeWindow_cancelBuffer(window, buffer, fenceFd);
    }
    printf("Failed to call fp_ANativeWindow_cancelBuffer, will return 0.\n");
    return 0;
}

int ANativeWindow_setUsage(ANativeWindow* window, uint64_t usage) {
    printf("ANativeWindow_setUsage called with window=%p, usage=%llu\n", window, (unsigned long long)usage);

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitNativeWindowWrapper, &expected, 1)) {
        initNativeWindowWrapper();
    }
   
    if (fp_ANativeWindow_setUsage) {
        return fp_ANativeWindow_setUsage(window, usage);
    }
    printf("Failed to call fp_ANativeWindow_setUsage, will return 0.\n");
    return 0;
}

int ANativeWindow_setSharedBufferMode(ANativeWindow* window, bool sharedBufferMode) {
    printf("ANativeWindow_setSharedBufferMode called with window=%p, sharedBufferMode=%d\n", window, sharedBufferMode);

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitNativeWindowWrapper, &expected, 1)) {
        initNativeWindowWrapper();
    }
   
    if (fp_ANativeWindow_setSharedBufferMode) {
        return fp_ANativeWindow_setSharedBufferMode(window, sharedBufferMode);
    }
    printf("Failed to call fp_ANativeWindow_setSharedBufferMode, will return 0.\n");
    return 0;
}

int32_t ANativeWindow_getWidth(ANativeWindow* window) {
    printf("ANativeWindow_getWidth called with window=%p\n", window);

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitNativeWindowWrapper, &expected, 1)) {
        initNativeWindowWrapper();
    }
   
    if (fp_ANativeWindow_getWidth) {
        return fp_ANativeWindow_getWidth(window);
    }
    printf("Failed to call fp_ANativeWindow_getWidth, will return 0.\n");
    return 0;
}

int32_t ANativeWindow_getHeight(ANativeWindow* window) {
    printf("ANativeWindow_getHeight called with window=%p\n", window);

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitNativeWindowWrapper, &expected, 1)) {
        initNativeWindowWrapper();
    }
   
    if (fp_ANativeWindow_getHeight) {
        return fp_ANativeWindow_getHeight(window);
    }
    printf("Failed to call fp_ANativeWindow_getHeight, will return 0.\n");
    return 0;
}

}
