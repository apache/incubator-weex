#ifndef WEEX_PROJECT_MEASURESIZE_IMPL_ANDROID_H
#define WEEX_PROJECT_MEASURESIZE_IMPL_ANDROID_H

#include <jni.h>

namespace WeexCore {
  bool RegisterJNIMeasureSize(JNIEnv *env);

  jfloat getMeasureHeight(JNIEnv *env, jobject size);

  jfloat getMeasureWidth(JNIEnv *env, jobject size);
}


#endif //WEEX_PROJECT_MEASURESIZE_IMPL_ANDROID_H
