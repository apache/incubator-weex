#include <base/android/jni/android_jni.h>
#include "WXSDKInstance_Impl_Android.h"
#include <gen/WXSDKInstance_jni.h>
#include <WeexCore/render/RenderManager.h>
#include <WeexCore/render/RenderPage.h>
#include <WeexCore/render/RenderObject.h>
#include <base/android/string/StringUtils.h>

using namespace WeexCore;

namespace WeexCore {
  bool RegisterJNISDKInstance(JNIEnv *env) {
    return RegisterNativesImpl(env);
  }
}

static void BindComponentToWXCore(JNIEnv* env, jobject jcaller,
                                  jstring instanceId,
                                  jobject component,
                                  jstring ref) {
  RenderPage *page = RenderManager::getInstance()->getPage(jString2Str(env, instanceId));
  RenderObject *render = page->getRenderObject(jString2Str(env, ref));
  render->bindComponent_Impl_Android(component);
}
