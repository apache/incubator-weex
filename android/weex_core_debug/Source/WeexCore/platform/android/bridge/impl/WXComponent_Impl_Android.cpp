#include <base/android/jni/android_jni.h>
#include "WXComponent_Impl_Android.h"
#include "WeexCore/platform/android/bridge/WXComponent_jni.h"
#include <WeexCore/platform/android/WeexCore.h>
#include <WeexCore/render/RenderManager.h>
#include <WeexCore/render/RenderPage.h>
#include <WeexCore/render/RenderObject.h>
#include <base/android/string/StringUtils.h>

using namespace WeexCore;

namespace WeexCore {
  bool RegisterJNIWXComponent(JNIEnv *env) {
    return RegisterNativesImpl(env);
  }
}

static void BindMeasurementToWXCore(JNIEnv *env, jobject jcaller,
                                    jstring instanceId,
                                    jstring ref,
                                    jobject contentBoxMeasurement) {
  if (contentBoxMeasurement == nullptr)
    return;
  RenderPage *page = RenderManager::getInstance()->getPage(jString2Str(env, instanceId));

  if (page == nullptr)
    return;
  RenderObject *render = page->getRenderObject(jString2Str(env, ref));

  if (render == nullptr)
    return;
  render->bindMeasureFunc_Impl_Android(contentBoxMeasurement);
}