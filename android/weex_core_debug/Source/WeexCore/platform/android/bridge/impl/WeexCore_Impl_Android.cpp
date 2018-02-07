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

//    std::string mMessage = "SetStyleWidth=" + std::to_string(value) + " ref=" + jString2Str(env, ref);
//    Bridge_Impl_Android::getInstance()->callLogOfFirstScreen(mMessage);

  RenderPage *page = RenderManager::GetInstance()->GetPage(jString2Str(env, instanceId));

  if (page == nullptr)
    return;
  RenderObject *render = page->GetRenderObject(jString2Str(env, ref));
  render->setStyleWidthLevel(CSS_STYLE);
  render->setStyleWidth(value);
}

void static
SetStyleHeight(JNIEnv *env, jobject jcaller,
               jstring instanceId,
               jstring ref,
               jfloat value) {

//    std::string mMessage = "SetStyleHeight=" + std::to_string(value) + " ref=" + jString2Str(env, ref);
//    Bridge_Impl_Android::getInstance()->callLogOfFirstScreen(mMessage);

  RenderPage *page = RenderManager::GetInstance()->GetPage(jString2Str(env, instanceId));

  if (page == nullptr)
    return;
  RenderObject *render = page->GetRenderObject(jString2Str(env, ref));
  render->setStyleHeightLevel(CSS_STYLE);
  render->setStyleHeight(value);
}

static void SetMargin(JNIEnv *env, jobject jcaller,
                      jstring instanceId,
                      jstring ref,
                      jint edge,
                      jfloat value) {
//    std::string mMessage = "SetMargin=" + std::to_string(value);
//    Bridge_Impl_Android::getInstance()->callLogOfFirstScreen(mMessage);

  RenderPage *page = RenderManager::GetInstance()->GetPage(jString2Str(env, instanceId));

  if (page == nullptr)
    return;
  RenderObject *render = page->GetRenderObject(jString2Str(env, ref));
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
}

static void SetPadding(JNIEnv *env, jobject jcaller,
                       jstring instanceId,
                       jstring ref,
                       jint edge,
                       jfloat value) {
//    std::string mMessage = "SetPadding=" + std::to_string(value);
//    Bridge_Impl_Android::getInstance()->callLogOfFirstScreen(mMessage);

  RenderPage *page = RenderManager::GetInstance()->GetPage(jString2Str(env, instanceId));

  if (page == nullptr)
    return;
  RenderObject *render = page->GetRenderObject(jString2Str(env, ref));
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
}


static void SetPosition(JNIEnv *env, jobject jcaller,
                        jstring instanceId,
                        jstring ref,
                        jint edge,
                        jfloat value) {
//    std::string mMessage = "SetPosition=" + std::to_string(value);
//    Bridge_Impl_Android::getInstance()->callLogOfFirstScreen(mMessage);

  RenderPage *page = RenderManager::GetInstance()->GetPage(jString2Str(env, instanceId));

  if (page == nullptr)
    return;
  RenderObject *render = page->GetRenderObject(jString2Str(env, ref));
  if (edge == 0) {
    render->setStylePosition(kPositionEdgeTop, value);
  } else if (edge == 1) {
    render->setStylePosition(kPositionEdgeRight, value);
  } else if (edge == 2) {
    render->setStylePosition(kPositionEdgeBottom, value);
  } else if (edge == 3) {
    render->setStylePosition(kPositionEdgeLeft, value);
  }
}

static void CalculateLayout(JNIEnv *env, jobject jcaller,
                            jstring instanceId,
                            jstring ref) {
//    std::string mMessage = "CalculateLayout=" + jString2Str(env, instanceId);
//    Bridge_Impl_Android::getInstance()->callLogOfFirstScreen(mMessage);
  RenderPage *page = RenderManager::GetInstance()->GetPage(jString2Str(env, instanceId));

  if (page == nullptr)
    return;
  page->CalculateLayout();
}

static void RegisterCoreEnv(JNIEnv *env, jobject jcaller,
                            jstring key,
                            jstring value) {
  LOGE("RegisterCoreEnv，key: %s, value: %s", jString2Str(env, key).c_str(),
       jString2Str(env, value).c_str());
  WXCoreEnvironment::getInstance()->AddOption(jString2Str(env, key), jString2Str(env, value));
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

static void OnVsync(JNIEnv *env,
                    jobject object,
                    jstring jinstanceId) {
  RenderManager::GetInstance()->Batch(jString2Str(env, jinstanceId));
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