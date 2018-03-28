#include "WeexCore_Impl_Android.h"
#include <WeexCore/platform/android/jsengine/multiprocess/WeexProxy.h>
#include <WeexCore/platform/android/base/jni/android_jni.h>
#include <WeexCore/platform/android/jniprebuild/jniheader/WXBridge_jni.h>
#include <WeexCore/platform/android/base/string/StringUtils.h>
#include <WeexCore/render/manager/RenderManager.h>
#include <WeexCore/render/page/RenderPage.h>
#include <WeexCore/render/node/RenderObject.h>
#include <WeexCore/env/CoreEnvironment.h>

using namespace WeexCore;

jclass jBridgeClazz;
jclass jWXJSObject;
jclass jWXLogUtils;
jclass jMapClazz;
jclass jSetClazz;
jmethodID jDoubleValueMethodId;
// static
jobject jThis;

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

void static
SetStyleWidth(JNIEnv *env, jobject jcaller,
              jstring instanceId,
              jstring ref,
              jfloat value) {

  RenderPage *page = RenderManager::GetInstance()->GetPage(jString2StrFast(env, instanceId));
  if (page == nullptr)
    return;

  RenderObject *render = page->GetRenderObject(jString2StrFast(env, ref));
  if (render == nullptr)
    return;

  render->setStyleWidthLevel(CSS_STYLE);
  render->setStyleWidth(value, true);
  page->updateDirty(true);
}

void static
SetStyleHeight(JNIEnv *env, jobject jcaller,
               jstring instanceId,
               jstring ref,
               jfloat value) {

  RenderPage *page = RenderManager::GetInstance()->GetPage(jString2StrFast(env, instanceId));
  if (page == nullptr)
    return;

  RenderObject *render = page->GetRenderObject(jString2StrFast(env, ref));
  if (render == nullptr)
    return;

  render->setStyleHeightLevel(CSS_STYLE);
  render->setStyleHeight(value);
  page->updateDirty(true);
}

static void SetMargin(JNIEnv *env, jobject jcaller,
                      jstring instanceId,
                      jstring ref,
                      jint edge,
                      jfloat value) {
  RenderPage *page = RenderManager::GetInstance()->GetPage(jString2StrFast(env, instanceId));
  if (page == nullptr)
    return;

  RenderObject *render = page->GetRenderObject(jString2StrFast(env, ref));
  if (render == nullptr)
    return;

  if (edge == 0) {
    render->setMargin(kMarginTop, value);
  } else if (edge == 1) {
    render->setMargin(kMarginRight, value);
  } else if (edge == 2) {
    render->setMargin(kMarginBottom, value);
  } else if (edge == 3) {
    render->setMargin(kMarginLeft, value);
  } else if (edge == 4) {
    render->setMargin(kMarginALL, value);
  }
  page->updateDirty(true);
}

static void SetPadding(JNIEnv *env, jobject jcaller,
                       jstring instanceId,
                       jstring ref,
                       jint edge,
                       jfloat value) {

  RenderPage *page = RenderManager::GetInstance()->GetPage(jString2StrFast(env, instanceId));
  if (page == nullptr)
    return;

  RenderObject *render = page->GetRenderObject(jString2StrFast(env, ref));
  if (render == nullptr)
    return;

  if (edge == 0) {
    render->setPadding(kPaddingTop, value);
  } else if (edge == 1) {
    render->setPadding(kPaddingRight, value);
  } else if (edge == 2) {
    render->setPadding(kPaddingBottom, value);
  } else if (edge == 3) {
    render->setPadding(kPaddingLeft, value);
  } else if (edge == 4) {
    render->setPadding(kPaddingALL, value);
  }
  page->updateDirty(true);
}


static void SetPosition(JNIEnv *env, jobject jcaller,
                        jstring instanceId,
                        jstring ref,
                        jint edge,
                        jfloat value) {

  RenderPage *page = RenderManager::GetInstance()->GetPage(jString2StrFast(env, instanceId));
  if (page == nullptr)
    return;

  RenderObject *render = page->GetRenderObject(jString2StrFast(env, ref));
  if (render == nullptr)
    return;

  if (edge == 0) {
    render->setStylePosition(kPositionEdgeTop, value);
  } else if (edge == 1) {
    render->setStylePosition(kPositionEdgeRight, value);
  } else if (edge == 2) {
    render->setStylePosition(kPositionEdgeBottom, value);
  } else if (edge == 3) {
    render->setStylePosition(kPositionEdgeLeft, value);
  }
  page->updateDirty(true);
}

static void MarkDirty(JNIEnv *env, jobject jcaller,
                            jstring instanceId,
                            jstring ref,
                            jboolean dirty) {
  RenderPage *page = RenderManager::GetInstance()->GetPage(jString2StrFast(env, instanceId));
  if (page == nullptr)
    return;

  if (dirty) {

    RenderObject *render = page->GetRenderObject(jString2StrFast(env, ref));
    if (render == nullptr)
      return;
    render->markDirty();
  }
}

static void RegisterCoreEnv(JNIEnv *env, jobject jcaller,
                            jstring key,
                            jstring value) {
  LOGE("RegisterCoreEnv，key: %s, value: %s", jString2StrFast(env, key).c_str(),
       jString2StrFast(env, value).c_str());
  WXCoreEnvironment::getInstance()->AddOption(jString2StrFast(env, key), jString2StrFast(env, value));
}

static void SetViewPortWidth(JNIEnv *env, jobject jcaller,
                            jstring instanceId,
                            jfloat value) {
  RenderPage *page = RenderManager::GetInstance()->GetPage(jString2StrFast(env, instanceId));
  if (page == nullptr)
    return;

  page->SetViewPortWidth(value);
}


static jint InitFramework(JNIEnv *env,
                          jobject object,
                          jstring script,
                          jobject params) {
  jThis = env->NewGlobalRef(object);
  return WeexProxy::doInitFramework(env, jThis, script, params);
}

static jint ExecJSService(JNIEnv *env,
                          jobject object,
                          jstring script) {
  if (script == nullptr)
    return false;
  return WeexProxy::execJSService(env, object, script);
}

static void TakeHeapSnapshot(JNIEnv *env,
                             jobject object,
                             jstring name) {
}

/**
 * Called to execute JavaScript such as . createInstance(),destroyInstance ext.
 *
 */
static jint ExecJS(JNIEnv *env,
                   jobject jthis,
                   jstring jinstanceid,
                   jstring jnamespace,
                   jstring jfunction,
                   jobjectArray jargs) {
  if (jfunction == NULL || jinstanceid == NULL) {
    LOGE("native_execJS function is NULL");
    return false;
  }

  return WeexProxy::execJS(env, jThis, jinstanceid, jnamespace, jfunction, jargs);
}

static jbyteArray ExecJSWithResult(JNIEnv* env, jobject jcaller,
                                   jstring instanceId,
                                   jstring _namespace,
                                   jstring _function,
                                   jobjectArray args) {
  if (_function == NULL || instanceId == NULL) {
    LOGE("native_execJS function is NULL");
    return NULL;
  }

  return WeexProxy::execJSWithResult(env, jcaller, instanceId, _namespace, _function, args);
}

static void UpdateGlobalConfig(JNIEnv* env, jobject jcaller,
                               jstring config) {
  WeexProxy::updateGlobalConfig(env, jcaller, config);
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
    if (jThis)
      env->DeleteGlobalRef(jThis);
    WeexProxy::reset();
    LOGD(" end JNI_OnUnload");
  }


}