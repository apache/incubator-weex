#include <base/android/jni/android_jni.h>
#include "WXSDKInstance_Impl_Android.h"
#include <WeexCore/render/RenderManager.h>
#include <WeexCore/render/RenderPage.h>
#include <WeexCore/render/RenderObject.h>
#include <base/android/string/StringUtils.h>
#include "WeexCore/platform/android/bridge/WXSDKInstance_jni.h"

using namespace WeexCore;

namespace WeexCore {
  bool RegisterJNISDKInstance(JNIEnv *env) {
    return RegisterNativesImpl(env);
  }
}

static void BindComponentToWXCore(JNIEnv *env, jobject jcaller,
                                  jstring instanceId,
                                  jobject component,
                                  jstring ref) {
  if (component == nullptr)
    return;
  RenderPage *page = RenderManager::getInstance()->getPage(jString2Str(env, instanceId));

  if (page == nullptr)
    return;
  RenderObject *render = page->getRenderObject(jString2Str(env, ref));

  if (render == nullptr)
    return;
  render->bindComponent_Impl_Android(component);
}

static void BindInstanceToWXCore(JNIEnv *env, jobject jcaller,
                                 jstring instanceId,
                                 jobject instance) {
  if (instance == nullptr)
    return;
  RenderPage *page = RenderManager::getInstance()->getPage(jString2Str(env, instanceId));

  if (page == nullptr)
    return;
  page->bindInstance_Impl_Android(instance);
}

static void PrintFirstScreenLog(JNIEnv *env, jobject jcaller,
                                jstring instanceId) {
  RenderPage *page = RenderManager::getInstance()->getPage(jString2Str(env, instanceId));

  if (page == nullptr)
    return;
  page->printFirstScreenLog();
}

static void PrintRenderSuccessLog(JNIEnv *env, jobject jcaller,
                                  jstring instanceId) {
  RenderPage *page = RenderManager::getInstance()->getPage(jString2Str(env, instanceId));

  if (page == nullptr)
    return;
  page->printRenderSuccessLog();
}
