#ifndef WEEX_PROJECT_MEASUREMODE_H
#define WEEX_PROJECT_MEASUREMODE_H

#include <jni.h>

namespace WeexCore {
  bool RegisterJNIMeasureMode(JNIEnv *env);

  jint Exactly(JNIEnv *env);

  jint Unspecified(JNIEnv *env);
}


#endif //WEEX_PROJECT_MEASUREMODE_H
