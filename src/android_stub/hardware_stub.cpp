#include <hardware/hardware.h>
#include <dlfcn.h>
#include <stdatomic.h>
#include <errno.h>
#include <android/log.h>

#define LOG_TAG "HardwareWrapper"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// ================== 声明 libpojavexec.so 中的函数 ==================
// 这些函数在 libpojavexec.so 中已实现，我们只需声明并动态获取
typedef bool (*ns_load_t)(const char* lib_search_path);
typedef void* (*ns_dlopen_t)(const char* name, int flag);

static ns_load_t linker_ns_load = nullptr;
static ns_dlopen_t linker_ns_dlopen = nullptr;

// 尝试加载 libpojavexec.so 并获取函数指针
static bool initNamespaceBypass() {
    void* handle = dlopen("libpojavexec.so", RTLD_LAZY);
    if (!handle) {
        LOGE("Failed to dlopen libpojavexec.so: %s", dlerror());
        return false;
    }
    linker_ns_load = (ns_load_t)dlsym(handle, "linker_ns_load");
    linker_ns_dlopen = (ns_dlopen_t)dlsym(handle, "linker_ns_dlopen");
    if (!linker_ns_load || !linker_ns_dlopen) {
        LOGE("Failed to get symbols from libpojavexec.so");
        dlclose(handle);
        return false;
    }
    // 注意：这里不关闭 handle，因为后续还要调用里面的函数
    return true;
}

// ================== Hardware 函数包装 ==================
static void* sHardwareHandle = nullptr;
static atomic_int isinitHardwareWrapper = 0;

typedef int (*hw_get_module_t)(const char*, const struct hw_module_t**);
static hw_get_module_t fp_hw_get_module = nullptr;

static void initHardwareWrapper() {
    // 1️⃣ 优先使用命名空间绕过方式加载 libhardware.so
    if (initNamespaceBypass()) {
        // 创建命名空间，允许访问 /system/lib64 (或 /system/lib)
        const char* searchPath = nullptr;
#if defined(__aarch64__)
        searchPath = "/system/lib64";
#else
        searchPath = "/system/lib";
#endif
        if (linker_ns_load(searchPath)) {
            LOGD("Namespace created successfully, loading libhardware.so via bypass");
            sHardwareHandle = linker_ns_dlopen("libhardware.so", RTLD_LAZY | RTLD_GLOBAL);
            if (sHardwareHandle) {
                fp_hw_get_module = (hw_get_module_t)dlsym(sHardwareHandle, "hw_get_module");
                if (fp_hw_get_module) {
                    LOGD("Successfully loaded hw_get_module from bypassed libhardware.so");
                    return;
                } else {
                    LOGE("dlsym hw_get_module failed in bypass mode");
                    dlclose(sHardwareHandle);
                    sHardwareHandle = nullptr;
                }
            } else {
                LOGE("linker_ns_dlopen failed for libhardware.so");
            }
        } else {
            LOGE("linker_ns_load failed");
        }
    }

    // 2️⃣ 回退方案：普通 dlopen（可能因命名空间限制失败，但保留作为兼容）
    const char* hardwareLibPaths[] = {
        "/system/lib64/libhardware.so",
        "/system/lib/libhardware.so",
        "libhardware.so",
        nullptr
    };
    
    for (int i = 0; hardwareLibPaths[i] != nullptr; i++) {
        sHardwareHandle = dlopen(hardwareLibPaths[i], RTLD_LAZY | RTLD_GLOBAL);
        if (sHardwareHandle != nullptr) {
            break;
        }
    }
    
    if (sHardwareHandle == nullptr) {
        LOGE("All attempts to dlopen libhardware.so failed");
        return;
    }
    
    fp_hw_get_module = (hw_get_module_t)dlsym(sHardwareHandle, "hw_get_module");
    if (fp_hw_get_module) {
        LOGD("Loaded hw_get_module via fallback dlopen");
    } else {
        LOGE("dlsym hw_get_module failed in fallback mode");
        dlclose(sHardwareHandle);
        sHardwareHandle = nullptr;
    }
}

extern "C" {

int hw_get_module(const char *id, const struct hw_module_t **module) {
    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitHardwareWrapper, &expected, 1)) {
        initHardwareWrapper();
    }
    
    if (fp_hw_get_module) {
        return fp_hw_get_module(id, module);
    }
    
    // Stub 实现
    if (module) {
        *module = nullptr;
    }
    return -ENOENT;
}

} // extern "C"
