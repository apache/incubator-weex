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

static void OnInstanceClose(JNIEnv *env, jobject jcaller, jint instanceId) {
  RenderManager::GetInstance()->ClosePage(instanceId);
}

static void SetDefaultHeightAndWidthIntoRootDom(JNIEnv *env, jobject jcaller,
                                                jint instanceId, jfloat defaultWidth, jfloat defaultHeight,
                                                jboolean isWidthWrapContent, jboolean isHeightWrapContent) {
  RenderPage *page = RenderManager::GetInstance()->GetPage(instanceId);
  if (page == nullptr)
    return;

#if RENDER_LOG
  LOGD("[JNI] SetDefaultHeightAndWidthIntoRootDom >>>> pageId: %d, defaultWidth: %f, defaultHeight: %f",
       page->PageId(), defaultWidth,defaultHeight);
#endif

  page->SetDefaultHeightAndWidthIntoRootRender(defaultWidth, defaultHeight, isWidthWrapContent, isHeightWrapContent);
}

static void SetRenderContainerWrapContent(JNIEnv* env, jobject jcaller, jboolean wrap, jint instanceId) {
  RenderPage *page = RenderManager::GetInstance()->GetPage(instanceId);
  if (page == nullptr)
    return;

  page->SetRenderContainerWidthWrapContent(wrap);
}

static jint PrintFirstScreenRenderTime(JNIEnv *env, jobject jcaller, jint instanceId) {
  RenderPage *page = RenderManager::GetInstance()->GetPage(instanceId);
  if (page == nullptr)
    return 0;

  return page->PrintFirstScreenLog();
}

static jint PrintRenderFinishTime(JNIEnv *env, jobject jcaller, jint instanceId) {
  RenderPage *page = RenderManager::GetInstance()->GetPage(instanceId);
  if (page == nullptr)
    return 0;

  return page->PrintRenderSuccessLog();
}

//Notice that this method is invoked from main thread.
static jboolean NotifyLayout(JNIEnv* env, jobject jcaller, jint instanceId) {
  RenderPage *page = RenderManager::GetInstance()->GetPage(instanceId);
  if (page != nullptr) {

#if RENDER_LOG
    LOGD("[JNI] NotifyLayout >>>> pageId: %d, needForceLayout: %s, dirty: %s", instanceId,
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
static void ForceLayout(JNIEnv *env, jobject jcaller, jint instanceId) {
  RenderPage *page = RenderManager::GetInstance()->GetPage(instanceId);
  if (page != nullptr) {

#if RENDER_LOG
    LOGD("[JNI] ForceLayout >>>> pageId: %d, needForceLayout: %s", instanceId, page->hasForeLayoutAction.load()?"true":"false");
#endif

    page->LayoutImmediately();
    page->hasForeLayoutAction.store(false);
  }
}