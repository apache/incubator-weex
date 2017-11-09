#ifndef WEEX_PROJECT_CONTENTBOXMEASUREMENT_IMPL_ANDROID_H
#define WEEX_PROJECT_CONTENTBOXMEASUREMENT_IMPL_ANDROID_H

#include <jni.h>

namespace WeexCore {
  bool RegisterJNIContentBoxMeasurement(JNIEnv *env);
}

void measure(JNIEnv *env);

#endif //WEEX_PROJECT_CONTENTBOXMEASUREMENT_IMPL_ANDROID_H
