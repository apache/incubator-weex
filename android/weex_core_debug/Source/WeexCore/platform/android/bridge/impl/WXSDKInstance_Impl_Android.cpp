#include <WeexCore/platform/android/base/jni/android_jni.h>
#include "WXSDKInstance_Impl_Android.h"
#include <WeexCore/render/manager/RenderManager.h>
#include <WeexCore/render/page/RenderPage.h>
#include <WeexCore/render/node/RenderObject.h>
#include <WeexCore/platform/android/base/string/StringUtils.h>
#include "WeexCore/platform/android/jniprebuild/jniheader/WXSDKInstance_jni.h"

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

  RenderPage *page = RenderManager::GetInstance()->GetPage(jString2StrFast(env, instanceId));
  if (page == nullptr)
    return;

  RenderObject *render = page->GetRenderObject(jString2StrFast(env, ref));
  if (render == nullptr)
    return;

  render->BindComponentImplAndroid(component);
}

static void OnInstanceClose(JNIEnv *env, jobject jcaller,
                            jstring instanceId) {
  RenderManager::GetInstance()->ClosePage(jString2StrFast(env, instanceId));
}

static void SetDefaultHeightAndWidthIntoRootDom(JNIEnv *env, jobject jcaller,
                                                jstring instanceId,
                                                jfloat defaultWidth,
                                                jfloat defaultHeight) {
  RenderPage *page = RenderManager::GetInstance()->GetPage(jString2StrFast(env, instanceId));
  if (page == nullptr)
    return;

#if RENDER_LOG
  LOGD("[JNI] SetDefaultHeightAndWidthIntoRootDom >>>> pageId: %s, defaultWidth: %f, defaultHeight: %f",
       page->PageId().c_str(), defaultWidth,defaultHeight);
#endif

  page->SetDefaultHeightAndWidthIntoRootRender(defaultWidth, defaultHeight);
}

static jint PrintFirstScreenRenderTime(JNIEnv *env, jobject jcaller,
                                       jstring instanceId) {
  RenderPage *page = RenderManager::GetInstance()->GetPage(jString2StrFast(env, instanceId));
  if (page == nullptr)
    return 0;

  return page->PrintFirstScreenLog();
}

static jint PrintRenderFinishTime(JNIEnv *env, jobject jcaller,
                                  jstring instanceId) {
  RenderPage *page = RenderManager::GetInstance()->GetPage(jString2StrFast(env, instanceId));
  if (page == nullptr)
    return 0;

  return page->PrintRenderSuccessLog();
}

//Notice that this method is invoked from main thread.
static jboolean NotifyLayout(JNIEnv* env, jobject jcaller,
                         jstring instanceId) {
  std::string pageId = jString2StrFast(env, instanceId);
  RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
  if (page != nullptr) {

#if RENDER_LOG
    LOGD("[JNI] NotifyLayout >>>> pageId: %s, needForceLayout: %s, dirty: %s", pageId.c_str(),
         page->needLayout.load() ? "true" : "false", page->isDirty() ? "true" : "false");
#endif

    if (!page->needLayout.load()) {
      page->needLayout.store(true);
    }

    bool ret = !page->hasForeLayoutAction.load() && page->isDirty();
    if (ret) {
      page->hasForeLayoutAction.store(true);
    }
    return ret ? JNI_TRUE : JNI_FALSE;
  }
}

//Notice that this method is invoked from JS thread.
static void ForceLayout(JNIEnv *env, jobject jcaller,
                        jstring instanceId) {
  std::string pageId = jString2StrFast(env, instanceId);
  RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
  if (page != nullptr) {

#if RENDER_LOG
    LOGD("[JNI] ForceLayout >>>> pageId: %s, needForceLayout: %s", pageId.c_str(), page->hasForeLayoutAction.load()?"true":"false");
#endif

    page->LayoutImmediately();
    page->hasForeLayoutAction.store(false);
  }
}