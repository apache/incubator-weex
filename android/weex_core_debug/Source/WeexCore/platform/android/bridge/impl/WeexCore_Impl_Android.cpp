#include "WeexCore_Impl_Android.h"
#include <WeexCore/platform/android/jsengine/multiprocess/WeexProxy.h>
#include <WeexCore/platform/android/base/jni/android_jni.h>
#include <WeexCore/platform/android/jniprebuild/jniheader/WXBridge_jni.h>
#include <WeexCore/platform/android/base/string/StringUtils.h>
#include <WeexCore/render/manager/RenderManager.h>
#include <WeexCore/render/page/RenderPage.h>
#include <WeexCore/render/node/RenderObject.h>
#include <WeexCore/config/CoreEnvironment.h>
#include <map>

using namespace WeexCore;

jclass jBridgeClazz;
jclass jWXJSObject;
jclass jWXLogUtils;
jclass jMapClazz;
jclass jSetClazz;
jmethodID jDoubleValueMethodId;
jobject jThis;
std::map<std::string, jobject> componentTypeCache;

static JavaVM *sVm = NULL;

JNIEnv *getJNIEnv() {
  JNIEnv *env = NULL;
  if ((sVm)->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
    return JNI_FALSE;
  }
  return env;
}

static jint InitFrameworkEnv(JNIEnv *env, jobject jcaller,
                             jstring framework,
                             jobject params,
                             jstring cacheDir,
                             jboolean pieSupport) {
  jThis = env->NewGlobalRef(jcaller);
  return WeexProxy::doInitFramework(env, jThis, framework, params, cacheDir, pieSupport);
}

static void BindMeasurementToWXCore(JNIEnv *env, jobject jcaller, jint instanceId, jint ref, jobject contentBoxMeasurement) {
  if (contentBoxMeasurement == nullptr)
    return;

  RenderPage *page = RenderManager::GetInstance()->GetPage(instanceId);
  if (page == nullptr)
    return;

  RenderObject *render = page->GetRenderObject(ref);
  if (render == nullptr)
    return;

  render->BindMeasureFuncImplAndroid(contentBoxMeasurement);
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

static void SetStyleWidth(JNIEnv *env, jobject jcaller,
                          jint instanceId, jint ref, jfloat value) {

  RenderPage *page = RenderManager::GetInstance()->GetPage(instanceId);
  if (page == nullptr)
    return;

  RenderObject *render = page->GetRenderObject(ref);
  if (render == nullptr)
    return;

  render->setStyleWidthLevel(CSS_STYLE);
  render->setStyleWidth(value, true);
  page->updateDirty(true);
}

static void SetStyleHeight(JNIEnv *env, jobject jcaller,
                           jint instanceId, jint ref, jfloat value) {

  RenderPage *page = RenderManager::GetInstance()->GetPage(instanceId);
  if (page == nullptr)
    return;

  RenderObject *render = page->GetRenderObject(ref);
  if (render == nullptr)
    return;

  render->setStyleHeightLevel(CSS_STYLE);
  render->setStyleHeight(value);
  page->updateDirty(true);
}

static void SetMargin(JNIEnv *env, jobject jcaller,
                      jint instanceId, jint ref, jint edge, jfloat value) {
  RenderPage *page = RenderManager::GetInstance()->GetPage(instanceId);
  if (page == nullptr)
    return;

  RenderObject *render = page->GetRenderObject(ref);
  if (render == nullptr)
    return;

  if (edge == 0) {
    render->setMargin(kMarginTop, value);
  } else if (edge == 1) {
    render->setMargin(kMarginBottom, value);
  } else if (edge == 2) {
    render->setMargin(kMarginLeft, value);
  } else if (edge == 3) {
    render->setMargin(kMarginRight, value);
  } else if (edge == 4) {
    render->setMargin(kMarginALL, value);
  }
  page->updateDirty(true);
}

static void SetPadding(JNIEnv *env, jobject jcaller,
                       jint instanceId, jint ref, jint edge, jfloat value) {

  RenderPage *page = RenderManager::GetInstance()->GetPage(instanceId);
  if (page == nullptr)
    return;

  RenderObject *render = page->GetRenderObject(ref);
  if (render == nullptr)
    return;

  if (edge == 0) {
    render->setPadding(kPaddingTop, value);
  } else if (edge == 1) {
    render->setPadding(kPaddingBottom, value);
  } else if (edge == 2) {
    render->setPadding(kPaddingLeft, value);
  } else if (edge == 3) {
    render->setPadding(kPaddingRight, value);
  } else if (edge == 4) {
    render->setPadding(kPaddingALL, value);
  }
  page->updateDirty(true);
}


static void SetPosition(JNIEnv *env, jobject jcaller,
                        jint instanceId, jint ref, jint edge, jfloat value) {

  RenderPage *page = RenderManager::GetInstance()->GetPage(instanceId);
  if (page == nullptr)
    return;

  RenderObject *render = page->GetRenderObject(ref);
  if (render == nullptr)
    return;

  if (edge == 0) {
    render->setStylePosition(kPositionEdgeTop, value);
  } else if (edge == 1) {
    render->setStylePosition(kPositionEdgeBottom, value);
  } else if (edge == 2) {
    render->setStylePosition(kPositionEdgeLeft, value);
  } else if (edge == 3) {
    render->setStylePosition(kPositionEdgeRight, value);
  }
  page->updateDirty(true);
}

static void MarkDirty(JNIEnv *env, jobject jcaller,
                      jint instanceId, jint ref, jboolean dirty) {
  RenderPage *page = RenderManager::GetInstance()->GetPage(instanceId);
  if (page == nullptr)
    return;

  if (dirty) {

    RenderObject *render = page->GetRenderObject(ref);
    if (render == nullptr)
      return;
    render->markDirty();
  }
}

static void RegisterCoreEnv(JNIEnv *env, jobject jcaller, jstring key, jstring value) {
  LOGE("RegisterCoreEnv，key: %s, value: %s", jString2StrFast(env, key).c_str(),
       jString2StrFast(env, value).c_str());
  WXCoreEnvironment::getInstance()->AddOption(jString2StrFast(env, key), jString2StrFast(env, value));
}

static void SetViewPortWidth(JNIEnv *env, jobject jcaller, jint instanceId, jfloat value) {
  RenderPage *page = RenderManager::GetInstance()->GetPage(instanceId);
  if (page == nullptr)
    return;

  page->SetViewPortWidth(value);
}


static jint InitFramework(JNIEnv *env, jobject object, jstring script, jobject params) {
  jThis = env->NewGlobalRef(object);
  return WeexProxy::doInitFramework(env, jThis, script, params);
}

static jint ExecJSService(JNIEnv *env, jobject object, jstring script) {
  if (script == nullptr)
    return false;
  return WeexProxy::execJSService(env, object, script);
}

static void TakeHeapSnapshot(JNIEnv *env, jobject object, jstring name) {
}

/**
 * Called to execute JavaScript such as . createInstance(),destroyInstance ext.
 *
 */
static jint ExecJS(JNIEnv *env, jobject jthis, jstring jinstanceid, jstring jnamespace, jstring jfunction, jobjectArray jargs) {
  if (jfunction == NULL || jinstanceid == NULL) {
    LOGE("native_execJS function is NULL");
    return false;
  }

  return WeexProxy::execJS(env, jThis, jinstanceid, jnamespace, jfunction, jargs);
}

static jbyteArray ExecJSWithResult(JNIEnv* env, jobject jcaller, jstring instanceId, jstring _namespace, jstring _function, jobjectArray args) {
  if (_function == NULL || instanceId == NULL) {
    LOGE("native_execJS function is NULL");
    return NULL;
  }

  return WeexProxy::execJSWithResult(env, jcaller, instanceId, _namespace, _function, args);
}

static void UpdateGlobalConfig(JNIEnv* env, jobject jcaller, jstring config) {
  WeexProxy::updateGlobalConfig(env, jcaller, config);
}


static jint CreateInstanceContext(JNIEnv* env, jobject jcaller, jstring instanceId, jstring name, jstring function, jobjectArray args) {
  return WeexProxy::createInstanceContext(env, jcaller, instanceId, name, function, args);
}

static jint DestoryInstance(JNIEnv* env, jobject jcaller, jstring instanceId, jstring name, jstring function, jobjectArray args) {
  return WeexProxy::destoryInstance(env, jcaller, instanceId, name, function, args);
}

static jstring ExecJSOnInstance(JNIEnv* env, jobject jcaller, jstring instanceId, jstring script, jint type) {
  return WeexProxy::execJSOnInstance(env, jcaller, instanceId, script, type);
}

jstring getComponentTypeFromCache(const std::string type) {
  std::map<std::string, jobject>::const_iterator iter = componentTypeCache.find(type);
  if (iter != componentTypeCache.end()) {
    return (jstring)(iter->second);
  } else {
    return nullptr;
  }
}

jstring putComponentTypeToCache(const std::string type) {
  JNIEnv *env = getJNIEnv();
  jstring jType = env->NewStringUTF(type.c_str());
  jobject jGlobalType = env->NewGlobalRef(jType);
  componentTypeCache.insert(std::pair<std::string, jobject>(type, jGlobalType));
  env->DeleteLocalRef(jType);
  return (jstring) jGlobalType;
}

namespace WeexCore {
  bool RegisterJNIUtils(JNIEnv *env) {
    return RegisterNativesImpl(env);
  }

  jint OnLoad(JavaVM *vm, void *reserved) {
    LOGD("begin JNI_OnLoad");
    JNIEnv *env;
    /* Get environment */
    if ((vm)->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
      return JNI_FALSE;
    }
    try {
      throw 1;
    } catch (int) {
    }
    sVm = vm;
    jclass tempClass = env->FindClass(
        "com/taobao/weex/bridge/WXBridge");
    jBridgeClazz = (jclass) env->NewGlobalRef(tempClass);

    tempClass = env->FindClass("com/taobao/weex/bridge/WXJSObject");
    jWXJSObject = (jclass) env->NewGlobalRef(tempClass);

    tempClass = env->FindClass("com/taobao/weex/utils/WXLogUtils");
    jWXLogUtils = (jclass) env->NewGlobalRef(tempClass);

    tempClass = env->FindClass("java/util/HashMap");
    jMapClazz = (jclass) env->NewGlobalRef(tempClass);

    tempClass = env->FindClass("java/util/HashSet");
    jSetClazz = (jclass) env->NewGlobalRef(tempClass);

    env->DeleteLocalRef(tempClass);

    LOGD("end JNI_OnLoad");
    WeexProxy::setCacheDir(env);
    return JNI_VERSION_1_4;
  }

  void Unload(JavaVM *vm, void *reserved) {
    LOGD("beigin JNI_OnUnload");
    JNIEnv *env;

    /* Get environment */
    if ((vm)->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
      return;
    }
    env->DeleteGlobalRef(jBridgeClazz);
    env->DeleteGlobalRef(jWXJSObject);
    env->DeleteGlobalRef(jWXLogUtils);
    env->DeleteGlobalRef(jMapClazz);

    for (auto iter = componentTypeCache.begin(); iter != componentTypeCache.end(); iter++) {
      if (iter->second != nullptr) {
        LOGE("DeleteGlobalRef componentTypeCache");
        env->DeleteGlobalRef(iter->second);
        iter->second = nullptr;
      }
    }
    componentTypeCache.clear();

    if (jThis)
      env->DeleteGlobalRef(jThis);
    WeexProxy::reset();
    LOGD(" end JNI_OnUnload");
  }
}