#ifndef WEEX_PROJECT_MEASUREMODE_H
#define WEEX_PROJECT_MEASUREMODE_H

#include <jni.h>

namespace WeexCore {
  bool RegisterJNIMeasureMode(JNIEnv *env);

  jint getEXACTLY(JNIEnv *env);

  jint getUNSPECIFIED(JNIEnv *env);
}


#endif //WEEX_PROJECT_MEASUREMODE_H
