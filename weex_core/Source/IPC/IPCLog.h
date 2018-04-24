#ifndef IPCLOG_H
#define IPCLOG_H
#include <android/log.h>
#define TAG "linzj_IPC"
#define IPC_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

#if defined(ENABLE_IPC_DEBUG_LOG) && ENABLE_IPC_DEBUG_LOG
#define IPC_LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#else
#define IPC_LOGD(...)
#endif
#endif /* IPCLOG_H */
