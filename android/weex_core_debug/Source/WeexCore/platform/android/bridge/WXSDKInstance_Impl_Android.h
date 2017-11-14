#ifndef WEEX_PROJECT_WXSDKINSTANCE_IMPL_ANDROID_H
#define WEEX_PROJECT_WXSDKINSTANCE_IMPL_ANDROID_H

#include <jni.h>

namespace WeexCore {
  bool RegisterJNISDKInstance(JNIEnv *env);

  jobject getSDKInstanceByID(JNIEnv* env, jstring pageID);
}

#endif //WEEX_PROJECT_WXSDKINSTANCE_IMPL_ANDROID_H
