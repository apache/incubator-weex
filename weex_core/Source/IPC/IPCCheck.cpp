#if !defined(NDEBUG)
#include "IPCCheck.h"
#include <android/log.h>
#define TAG "linzj_IPC"

void reportCheckFailed(const char* msg, const char* file, int line)
{
    __android_log_print(ANDROID_LOG_ERROR, TAG, msg, file, line);
    __builtin_trap();
}

void reportUnreachable(const char* file, int line)
{
    __android_log_print(ANDROID_LOG_ERROR, TAG, "unreachable statement reached %s %d", file, line);
    __builtin_trap();
}
#endif
