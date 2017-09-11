#include "WeexCore.h"
#include "WeexProxy.h"

using namespace WeexCore;

jclass jBridgeClazz;
jclass jWXJSObject;
jclass jWXLogUtils;

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

static jint native_execJSService(JNIEnv *env,
                                 jobject object,
                                 jstring script) {
  if (script == nullptr)
    return false;
  return WeexProxy::execJSService(env, object, script);
}

static void native_takeHeapSnapshot(JNIEnv *env,
                                    jobject object,
                                    jstring name) {
}

static void native_onVsync(JNIEnv *env,
                           jobject object,
                           jstring jinstanceId) {


  // LOGE("onVsync instanceId:%s", jString2Str(env, jinstanceId).c_str());
}

static jint native_initFramework(JNIEnv *env,
                                 jobject object,
                                 jstring script,
                                 jobject params) {
  jThis = env->NewGlobalRef(object);
  return WeexProxy::doInitFramework(env, jThis, script, params);
}

/**
 * Called to execute JavaScript such as . createInstance(),destroyInstance ext.
 *
 */
static jint native_execJS(JNIEnv *env,
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

static const char *gBridgeClassPathName = "com/taobao/weex/bridge/WXBridge";
static JNINativeMethod gMethods[] = {
    {"initFramework",
        "(Ljava/lang/String;Lcom/taobao/weex/bridge/WXParams;)I",
        (void *) native_initFramework},
    {"execJS",
        "(Ljava/lang/String;Ljava/lang/String;"
            "Ljava/lang/String;[Lcom/taobao/weex/bridge/WXJSObject;)I",
        (void *) native_execJS},
    {"takeHeapSnapshot",
        "(Ljava/lang/String;)V",
        (void *) native_takeHeapSnapshot},
    {"execJSService",
        "(Ljava/lang/String;)I",
        (void *) native_execJSService},
    {"onVsync",
            "(Ljava/lang/String;)V",
            (void*) native_onVsync}
};

static int registerBridgeNativeMethods(JNIEnv *env, JNINativeMethod *methods, int numMethods) {
  if (jBridgeClazz == NULL) {
    LOGE("registerBridgeNativeMethods failed to find bridge class.");
    return JNI_FALSE;
  }
  if ((env)->RegisterNatives(jBridgeClazz, methods, numMethods) < 0) {
    LOGE("registerBridgeNativeMethods failed to register native methods for bridge class.");
    return JNI_FALSE;
  }

  return JNI_TRUE;
}

static bool registerNatives(JNIEnv *env) {
  if (JNI_TRUE !=
      registerBridgeNativeMethods(env, gMethods, sizeof(gMethods) / sizeof(gMethods[0])))
    return false;
  return true;
}

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
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

  env->DeleteLocalRef(tempClass);
  if (!registerNatives(env)) {
    return JNI_FALSE;
  }

  LOGD("end JNI_OnLoad");
  WeexProxy::setCacheDir(env);
  // s_cacheDir = getCacheDir(env);
  return JNI_VERSION_1_4;
}

void JNI_OnUnload(JavaVM *vm, void *reserved) {
  LOGD("beigin JNI_OnUnload");
  JNIEnv *env;

  /* Get environment */
  if ((vm)->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
    return;
  }
  env->DeleteGlobalRef(jBridgeClazz);
  env->DeleteGlobalRef(jWXJSObject);
  env->DeleteGlobalRef(jWXLogUtils);
  if (jThis)
    env->DeleteGlobalRef(jThis);
  // FIXME: move to other place
  // deinitHeapTimer();

  WeexProxy::reset();
  // using base::debug::TraceEvent;
  // TraceEvent::StopATrace(env);
  LOGD(" end JNI_OnUnload");
}
