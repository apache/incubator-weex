#include "WeexCore_Impl_Android.h"
#include <WeexCore/platform/android/jsengine/multiprocess/WeexProxy.h>
#include <WeexCore/platform/android/base/jni/android_jni.h>
#include <WeexCore/platform/android/jniprebuild/jniheader/WXBridge_jni.h>
#include <WeexCore/platform/android/base/string/StringUtils.h>
#include <WeexCore/render/manager/RenderManager.h>

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
  // LOGE("onVsync instanceId:%s", jString2Str(env, jinstanceId).c_str());
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