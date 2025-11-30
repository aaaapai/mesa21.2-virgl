#include <cutils/properties.h>
#include <sync/sync.h>
#include <hardware/hardware.h>
#include <android/log.h>
#include <backtrace/Backtrace.h>
#include <dlfcn.h>
#include <stdatomic.h>

static atomic_int isinitSyncWrapper(0);

// 在文件开头添加以下内容

// ================== sync 相关函数包装 ==================

static void* sSyncHandle = NULL;

// 函数指针类型定义
typedef int (*sync_wait_t)(int, int);
typedef int (*sync_merge_t)(const char*, int, int);
typedef struct sync_file_info* (*sync_file_info_t)(int);
typedef void (*sync_file_info_free_t)(struct sync_file_info*);

// 函数指针变量
static sync_wait_t fp_sync_wait = NULL;
static sync_merge_t fp_sync_merge = NULL;
static sync_file_info_t fp_sync_file_info = NULL;
static sync_file_info_free_t fp_sync_file_info_free = NULL;

static void initSyncWrapper() {
    const char* syncLibPaths[] = {
        "/system/lib64/libsync.so",
        "/system/lib/libsync.so",
        "libsync.so",
        NULL
    };
    
    for (int i = 0; syncLibPaths[i] != NULL; i++) {
        sSyncHandle = dlopen("libsync.so", RTLD_NOLOAD);
        if (sSyncHandle != NULL) {
            printf("Successfully loaded %s (sync)", syncLibPaths[i]);
            break;
        } else {
            sSyncHandle = dlopen(syncLibPaths[i], RTLD_LAZY | RTLD_LOCAL);
            if (sSyncHandle != NULL) {
                printf("Successfully loaded %s (sync)", syncLibPaths[i]);
                break;
            } else {
                printf("Failed to load %s (sync): %s", syncLibPaths[i], dlerror());
            }
        }
    }
    
    if (sSyncHandle == NULL) {
        printf("All sync library paths failed, using stub implementations");
        return;
    }
    
    // 解析 sync 函数符号
    fp_sync_wait = (sync_wait_t)dlsym(sSyncHandle, "sync_wait");
    fp_sync_merge = (sync_merge_t)dlsym(sSyncHandle, "sync_merge");
    fp_sync_file_info = (sync_file_info_t)dlsym(sSyncHandle, "sync_file_info");
    fp_sync_file_info_free = (sync_file_info_free_t)dlsym(sSyncHandle, "sync_file_info_free");
    
    // 检查是否有函数解析失败
    if (!fp_sync_wait || !fp_sync_merge || !fp_sync_file_info || !fp_sync_file_info_free) {
        printf("Some sync functions failed to resolve, library may be incomplete");
    }
}

// 修改原有的 sync 函数实现
int sync_wait(int fd, int timeout) {

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitSyncWrapper, &expected, 1)) {
       initSyncWrapper();
    }
   
    if (fp_sync_wait) {
        return fp_sync_wait(fd, timeout);
    }
    // Stub 实现
    return 0;
}

int sync_merge(const char *name, int fd, int fd2) {

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitSyncWrapper, &expected, 1)) {
       initSyncWrapper();
    }
   
    if (fp_sync_merge) {
        return fp_sync_merge(name, fd, fd2);
    }
    // Stub 实现
    return 0;
}

struct sync_file_info* sync_file_info(int32_t fd) {

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitSyncWrapper, &expected, 1)) {
       initSyncWrapper();
    }

    if (fp_sync_file_info) {
        return fp_sync_file_info(fd);
    }
    // Stub 实现
    return NULL;
}

void sync_file_info_free(struct sync_file_info* info) {

    int expected = 0;
    if (atomic_compare_exchange_strong(&isinitSyncWrapper, &expected, 1)) {
       initSyncWrapper();
    }

    if (fp_sync_file_info_free) {
        fp_sync_file_info_free(info);
    }
    // Stub 实现 - 无操作
}
