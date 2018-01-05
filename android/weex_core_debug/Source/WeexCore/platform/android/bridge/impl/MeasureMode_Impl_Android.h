#ifndef WEEX_PROJECT_MEASUREMODE_H
#define WEEX_PROJECT_MEASUREMODE_H

#include <jni.h>

namespace WeexCore {
  bool RegisterJNIMeasureMode(JNIEnv *env);

  jint getExactly(JNIEnv *env);

  jint getUnspecified(JNIEnv *env);
}


#endif //WEEX_PROJECT_MEASUREMODE_H
