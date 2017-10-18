#ifndef _LOG_UTILS_H_
#define _LOG_UTILS_H_

#include <android/log.h>

#define LOG_TAG "WeexCore"

#define LOGE(...)    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGA(...)    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
//#define DEBUG

#ifdef DEBUG

#define LOGV(...) 	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
#define LOGD(...) 	__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGI(...) 	__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOG_LINE LOGV("%s, %d", __func__, __LINE__)

#else

#define LOGV(...) ((void) 0)
#define LOGD(...) ((void) 0)
#define LOGI(...) ((void) 0)
#define LOGW(...) ((void) 0)
//#define LOGE(...) ((void) 0)
#define LOG_LINE

#endif

#ifndef DISALLOW_COPY_AND_ASSIGN
#define DISALLOW_COPY_AND_ASSIGN(TypeName)  \
  TypeName(const TypeName&) = delete;      \
  void operator=(const TypeName&) = delete
#endif

#endif //_LOG_UTILS_H_
