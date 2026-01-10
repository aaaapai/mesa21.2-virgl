#include <vndk/window.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <android/log.h>
#include <mutex>
#include <atomic>

#define LOG_TAG "NativeWindowWrapper"
#define ALOGE(...) do { printf("E/%s: ", LOG_TAG); printf(__VA_ARGS__); printf("\n"); } while(0)
#define ALOGW(...) do { printf("W/%s: ", LOG_TAG); printf(__VA_ARGS__); printf("\n"); } while(0)
#define ALOGI(...) do { printf("I/%s: ", LOG_TAG); printf(__VA_ARGS__); printf("\n"); } while(0)

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
static ANativeWindowBuffer_getHardwareBuffer_t fp_ANativeWindowBuffer_getHardwareBuffer = nullptr;
static AHardwareBuffer_acquire_t fp_AHardwareBuffer_acquire = nullptr;
static AHardwareBuffer_release_t fp_AHardwareBuffer_release = nullptr;
static AHardwareBuffer_describe_t fp_AHardwareBuffer_describe = nullptr;
static AHardwareBuffer_allocate_t fp_AHardwareBuffer_allocate = nullptr;
static AHardwareBuffer_getNativeHandle_t fp_AHardwareBuffer_getNativeHandle = nullptr;
static ANativeWindow_acquire_t fp_ANativeWindow_acquire = nullptr;
static ANativeWindow_release_t fp_ANativeWindow_release = nullptr;
static ANativeWindow_getFormat_t fp_ANativeWindow_getFormat = nullptr;
static ANativeWindow_setSwapInterval_t fp_ANativeWindow_setSwapInterval = nullptr;
static ANativeWindow_query_t fp_ANativeWindow_query = nullptr;
static ANativeWindow_dequeueBuffer_t fp_ANativeWindow_dequeueBuffer = nullptr;
static ANativeWindow_queueBuffer_t fp_ANativeWindow_queueBuffer = nullptr;
static ANativeWindow_cancelBuffer_t fp_ANativeWindow_cancelBuffer = nullptr;
static ANativeWindow_setUsage_t fp_ANativeWindow_setUsage = nullptr;
static ANativeWindow_setSharedBufferMode_t fp_ANativeWindow_setSharedBufferMode = nullptr;
static ANativeWindow_getWidth_t fp_ANativeWindow_getWidth = nullptr;
static ANativeWindow_getHeight_t fp_ANativeWindow_getHeight = nullptr;

// 线程安全初始化机制
static std::once_flag sInitFlag;
static std::mutex sLibraryMutex;
static void* sNativeWindowHandle = nullptr;
static std::atomic<bool> sLibraryLoaded{false};

// 内部初始化函数
static void initNativeWindowWrapperImpl() {
    std::lock_guard<std::mutex> lock(sLibraryMutex);
    
    // 如果已经加载成功，直接返回
    if (sLibraryLoaded) {
        return;
    }
    
    // 尝试加载库文件
    const char* libPaths[] = {
        "/system/lib64/libnativewindow.so",
        "/system/lib/libnativewindow.so",
        "libnativewindow.so",
        nullptr
    };
    
    for (int i = 0; libPaths[i] != nullptr; i++) {
        // 先尝试NOLOAD查看是否已加载
        void* handle = dlopen(libPaths[i], RTLD_NOLOAD | RTLD_LOCAL);
        if (!handle) {
            // 未加载，尝试加载
            handle = dlopen(libPaths[i], RTLD_LAZY | RTLD_LOCAL);
        }
        
        if (handle) {
            sNativeWindowHandle = handle;
            ALOGI("Successfully loaded %s", libPaths[i]);
            break;
        }
        ALOGW("Failed to load %s: %s", libPaths[i], dlerror());
    }
    
    if (sNativeWindowHandle == nullptr) {
        ALOGE("All library paths failed, using stub implementations");
        sLibraryLoaded = true; // 标记为已处理，避免重复尝试
        return;
    }
    
    // 解析函数符号
    fp_ANativeWindowBuffer_getHardwareBuffer = reinterpret_cast<ANativeWindowBuffer_getHardwareBuffer_t>(
        dlsym(sNativeWindowHandle, "ANativeWindowBuffer_getHardwareBuffer"));
    fp_AHardwareBuffer_acquire = reinterpret_cast<AHardwareBuffer_acquire_t>(
        dlsym(sNativeWindowHandle, "AHardwareBuffer_acquire"));
    fp_AHardwareBuffer_release = reinterpret_cast<AHardwareBuffer_release_t>(
        dlsym(sNativeWindowHandle, "AHardwareBuffer_release"));
    fp_AHardwareBuffer_describe = reinterpret_cast<AHardwareBuffer_describe_t>(
        dlsym(sNativeWindowHandle, "AHardwareBuffer_describe"));
    fp_AHardwareBuffer_allocate = reinterpret_cast<AHardwareBuffer_allocate_t>(
        dlsym(sNativeWindowHandle, "AHardwareBuffer_allocate"));
    fp_AHardwareBuffer_getNativeHandle = reinterpret_cast<AHardwareBuffer_getNativeHandle_t>(
        dlsym(sNativeWindowHandle, "AHardwareBuffer_getNativeHandle"));
    fp_ANativeWindow_acquire = reinterpret_cast<ANativeWindow_acquire_t>(
        dlsym(sNativeWindowHandle, "ANativeWindow_acquire"));
    fp_ANativeWindow_release = reinterpret_cast<ANativeWindow_release_t>(
        dlsym(sNativeWindowHandle, "ANativeWindow_release"));
    fp_ANativeWindow_getFormat = reinterpret_cast<ANativeWindow_getFormat_t>(
        dlsym(sNativeWindowHandle, "ANativeWindow_getFormat"));
    fp_ANativeWindow_setSwapInterval = reinterpret_cast<ANativeWindow_setSwapInterval_t>(
        dlsym(sNativeWindowHandle, "ANativeWindow_setSwapInterval"));
    fp_ANativeWindow_query = reinterpret_cast<ANativeWindow_query_t>(
        dlsym(sNativeWindowHandle, "ANativeWindow_query"));
    fp_ANativeWindow_dequeueBuffer = reinterpret_cast<ANativeWindow_dequeueBuffer_t>(
        dlsym(sNativeWindowHandle, "ANativeWindow_dequeueBuffer"));
    fp_ANativeWindow_queueBuffer = reinterpret_cast<ANativeWindow_queueBuffer_t>(
        dlsym(sNativeWindowHandle, "ANativeWindow_queueBuffer"));
    fp_ANativeWindow_cancelBuffer = reinterpret_cast<ANativeWindow_cancelBuffer_t>(
        dlsym(sNativeWindowHandle, "ANativeWindow_cancelBuffer"));
    fp_ANativeWindow_setUsage = reinterpret_cast<ANativeWindow_setUsage_t>(
        dlsym(sNativeWindowHandle, "ANativeWindow_setUsage"));
    fp_ANativeWindow_setSharedBufferMode = reinterpret_cast<ANativeWindow_setSharedBufferMode_t>(
        dlsym(sNativeWindowHandle, "ANativeWindow_setSharedBufferMode"));
    fp_ANativeWindow_getWidth = reinterpret_cast<ANativeWindow_getWidth_t>(
        dlsym(sNativeWindowHandle, "ANativeWindow_getWidth"));
    fp_ANativeWindow_getHeight = reinterpret_cast<ANativeWindow_getHeight_t>(
        dlsym(sNativeWindowHandle, "ANativeWindow_getHeight"));
    
    // 检查关键函数是否解析成功
    if (!fp_ANativeWindowBuffer_getHardwareBuffer || !fp_AHardwareBuffer_acquire ||
        !fp_AHardwareBuffer_release || !fp_ANativeWindow_acquire || !fp_ANativeWindow_release) {
        ALOGW("Some critical functions failed to resolve, library may be incomplete");
    }
    
    sLibraryLoaded = true;
}

// 确保初始化的辅助函数
static inline void ensureInitialized() {
    std::call_once(sInitFlag, initNativeWindowWrapperImpl);
}

// 内部函数：安全地获取函数指针
template<typename T>
static T getFunctionPointer(T* funcPtr, const char* funcName) {
    ensureInitialized();
    if (funcPtr == nullptr) {
        ALOGW("Function %s not available, using stub", funcName);
    }
    return funcPtr;
}

// 简化调用的宏
#define SAFE_CALL(func, ...) \
    ensureInitialized(); \
    if (fp_##func) { \
        return fp_##func(__VA_ARGS__); \
    } \
    ALOGW("%s: no implementation available", #func);

#define SAFE_CALL_VOID(func, ...) \
    ensureInitialized(); \
    if (fp_##func) { \
        fp_##func(__VA_ARGS__); \
        return; \
    } \
    ALOGW("%s: no implementation available", #func);

extern "C" {

AHardwareBuffer* ANativeWindowBuffer_getHardwareBuffer(ANativeWindowBuffer* anwb) {
    //printf("ANativeWindowBuffer_getHardwareBuffer called with anwb=%p\n", anwb);
    SAFE_CALL(ANativeWindowBuffer_getHardwareBuffer, anwb);
    return nullptr;
}

void AHardwareBuffer_acquire(AHardwareBuffer* buffer) {
    //printf("AHardwareBuffer_acquire called with buffer=%p\n", buffer);
    SAFE_CALL_VOID(AHardwareBuffer_acquire, buffer);
}

void AHardwareBuffer_release(AHardwareBuffer* buffer) {
    //printf("AHardwareBuffer_release called with buffer=%p\n", buffer);
    SAFE_CALL_VOID(AHardwareBuffer_release, buffer);
}

void AHardwareBuffer_describe(const AHardwareBuffer* buffer, AHardwareBuffer_Desc* outDesc) {
    //printf("AHardwareBuffer_describe called with buffer=%p, outDesc=%p\n", buffer, outDesc);
    SAFE_CALL_VOID(AHardwareBuffer_describe, buffer, outDesc);
}

int AHardwareBuffer_allocate(const AHardwareBuffer_Desc* desc, AHardwareBuffer** outBuffer) {
    //printf("AHardwareBuffer_allocate called with desc=%p, outBuffer=%p\n", desc, outBuffer);
    SAFE_CALL(AHardwareBuffer_allocate, desc, outBuffer);
    return 0;
}

const native_handle_t* AHardwareBuffer_getNativeHandle(const AHardwareBuffer* buffer) {
    //printf("AHardwareBuffer_getNativeHandle called with buffer=%p\n", buffer);
    SAFE_CALL(AHardwareBuffer_getNativeHandle, buffer);
    return nullptr;
}

void ANativeWindow_acquire(ANativeWindow* window) {
    //printf("ANativeWindow_acquire called with window=%p\n", window);
    SAFE_CALL_VOID(ANativeWindow_acquire, window);
}

void ANativeWindow_release(ANativeWindow* window) {
    //printf("ANativeWindow_release called with window=%p\n", window);
    SAFE_CALL_VOID(ANativeWindow_release, window);
}

int32_t ANativeWindow_getFormat(ANativeWindow* window) {
    //printf("ANativeWindow_getFormat called with window=%p\n", window);
    SAFE_CALL(ANativeWindow_getFormat, window);
    return 0;
}

int ANativeWindow_setSwapInterval(ANativeWindow* window, int interval) {
    //printf("ANativeWindow_setSwapInterval called with window=%p, interval=%d\n", window, interval);
    SAFE_CALL(ANativeWindow_setSwapInterval, window, interval);
    return 0;
}

int ANativeWindow_query(const ANativeWindow* window, ANativeWindowQuery query, int* value) {
    //printf("ANativeWindow_query called with window=%p, query=%d, value=%p\n", window, query, value);
    SAFE_CALL(ANativeWindow_query, window, query, value);
    return 0;
}

int ANativeWindow_dequeueBuffer(ANativeWindow* window, ANativeWindowBuffer** buffer, int* fenceFd) {
    //printf("ANativeWindow_dequeueBuffer called with window=%p, buffer=%p, fenceFd=%p\n", window, buffer, fenceFd);
    SAFE_CALL(ANativeWindow_dequeueBuffer, window, buffer, fenceFd);
    return 0;
}

int ANativeWindow_queueBuffer(ANativeWindow* window, ANativeWindowBuffer* buffer, int fenceFd) {
    //printf("ANativeWindow_queueBuffer called with window=%p, buffer=%p, fenceFd=%d\n", window, buffer, fenceFd);
    SAFE_CALL(ANativeWindow_queueBuffer, window, buffer, fenceFd);
    return 0;
}

int ANativeWindow_cancelBuffer(ANativeWindow* window, ANativeWindowBuffer* buffer, int fenceFd) {
    //printf("ANativeWindow_cancelBuffer called with window=%p, buffer=%p, fenceFd=%d\n", window, buffer, fenceFd);
    SAFE_CALL(ANativeWindow_cancelBuffer, window, buffer, fenceFd);
    return 0;
}

int ANativeWindow_setUsage(ANativeWindow* window, uint64_t usage) {
    //printf("ANativeWindow_setUsage called with window=%p, usage=%llu\n", window, (unsigned long long)usage);
    SAFE_CALL(ANativeWindow_setUsage, window, usage);
    return 0;
}

int ANativeWindow_setSharedBufferMode(ANativeWindow* window, bool sharedBufferMode) {
    //printf("ANativeWindow_setSharedBufferMode called with window=%p, sharedBufferMode=%d\n", window, sharedBufferMode);
    SAFE_CALL(ANativeWindow_setSharedBufferMode, window, sharedBufferMode);
    return 0;
}

int32_t ANativeWindow_getWidth(ANativeWindow* window) {
    //printf("ANativeWindow_getWidth called with window=%p\n", window);
    SAFE_CALL(ANativeWindow_getWidth, window);
    return 0;
}

int32_t ANativeWindow_getHeight(ANativeWindow* window) {
    //printf("ANativeWindow_getHeight called with window=%p\n", window);
    SAFE_CALL(ANativeWindow_getHeight, window);
    return 0;
}

} // extern "C"
