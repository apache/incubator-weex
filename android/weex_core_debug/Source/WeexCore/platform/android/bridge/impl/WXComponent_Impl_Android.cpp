#include <WeexCore/platform/android/base/jni/android_jni.h>
#include "WXComponent_Impl_Android.h"
#include "WeexCore/platform/android/jniprebuild/jniheader/WXComponent_jni.h"
#include <WeexCore/platform/android/bridge/impl/WeexCore_Impl_Android.h>
#include <WeexCore/render/manager/RenderManager.h>
#include <WeexCore/render/page/RenderPage.h>
#include <WeexCore/render/node/RenderObject.h>
#include <WeexCore/platform/android/base/string/StringUtils.h>

using namespace WeexCore;

namespace WeexCore {
  bool RegisterJNIWXComponent(JNIEnv *env) {
    return RegisterNativesImpl(env);
  }
}

static void BindMeasurementToWXCore(JNIEnv *env, jobject jcaller,
                                    jint instanceId,
                                    jstring ref,
                                    jobject contentBoxMeasurement) {
  if (contentBoxMeasurement == nullptr)
    return;

  RenderPage *page = RenderManager::GetInstance()->GetPage(instanceId);
  if (page == nullptr)
    return;

  RenderObject *render = page->GetRenderObject(jString2StrFast(env, ref));
  if (render == nullptr)
    return;

  render->BindMeasureFuncImplAndroid(contentBoxMeasurement);
}