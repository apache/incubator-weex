#include "BridgeAndroid.h"

static jmethodID jSetJSFrmVersionMethodId;
static jmethodID jReportExceptionMethodId;
static jmethodID jCallNativeMethodId;
static jmethodID jCallNativeModuleMethodId;
static jmethodID jCallNativeComponentMethodId;
static jmethodID jCallAddElementMethodId;

static jmethodID jSetTimeoutNativeMethodId;
static jmethodID jLogMethodId;
static jmethodID jCallCreateBodyMethodId;
static jmethodID jCallUpdateFinishMethodId;

static jmethodID jCallCreateFinishMethodId;
static jmethodID jCallRefreshFinishMethodId;
static jmethodID jCallUpdateAttrsMethodId;
static jmethodID jCallUpdateStyleMethodId;
static jmethodID jCallRemoveElementMethodId;
static jmethodID jCallMoveElementMethodId;
static jmethodID jCallAddEventMethodId;
static jmethodID jCallRemoveEventMethodId;

namespace WeexCore {
  BridgeAndroid::BridgeAndroid() {
  }

  BridgeAndroid::~BridgeAndroid() {
  }

  void BridgeAndroid::setJSVersion(jstring &jversion) {
    JNIEnv *env = getJNIEnv();
    if (jSetJSFrmVersionMethodId == NULL) {
      jSetJSFrmVersionMethodId = env->GetMethodID(jBridgeClazz,
                                                  "setJSFrmVersion",
                                                  "(Ljava/lang/String;)V");
    }
    env->CallVoidMethod(jThis, jSetJSFrmVersionMethodId, jversion);
    env->DeleteLocalRef(jversion);
  }

  void BridgeAndroid::reportException(jstring &jInstanceId, jstring &jFunc, jstring &jExceptionString) {
    JNIEnv *env = getJNIEnv();
    if (jReportExceptionMethodId == NULL) {
      jReportExceptionMethodId = env->GetMethodID(jBridgeClazz,
                                                  "reportJSException",
                                                  "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
    }
    env->CallVoidMethod(jThis, jReportExceptionMethodId, jInstanceId, jFunc, jExceptionString);
    env->DeleteLocalRef(jExceptionString);
    env->DeleteLocalRef(jInstanceId);
    env->DeleteLocalRef(jFunc);
  }

  int BridgeAndroid::callNative(jstring &instanceId, jbyteArray &taskString, jstring &callback) {
    JNIEnv *env = getJNIEnv();
    if (jCallNativeMethodId == NULL) {
      jCallNativeMethodId = env->GetMethodID(jBridgeClazz,
                                             "callNative",
                                             "(Ljava/lang/String;[BLjava/lang/String;)I");
    }

    int flag = env->CallIntMethod(jThis, jCallNativeMethodId, instanceId, taskString, callback);
    if (flag == -1) {
      LOGE("instance destroy JFM must stop callNative");
    }
    env->DeleteLocalRef(instanceId);
    env->DeleteLocalRef(taskString);
    env->DeleteLocalRef(callback);
    return flag;
  }

  jobject BridgeAndroid::callNativeModule(jstring &jInstanceId, jstring &jmodule, jstring &jmethod,
                                          jbyteArray &jArgString, jbyteArray &jOptString) {
    JNIEnv *env = getJNIEnv();
    if (jCallNativeModuleMethodId == NULL) {
      jCallNativeModuleMethodId = env->GetMethodID(jBridgeClazz,
                                                   "callNativeModule",
                                                   "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[B[B)Ljava/lang/Object;");
    }

    jobject result = env->CallObjectMethod(jThis, jCallNativeModuleMethodId, jInstanceId, jmodule,
                                           jmethod, jArgString, jOptString);
    env->DeleteLocalRef(jInstanceId);
    env->DeleteLocalRef(jmodule);
    env->DeleteLocalRef(jmethod);
    env->DeleteLocalRef(jArgString);
    env->DeleteLocalRef(jOptString);
    return result;
  }

  void BridgeAndroid::callNativeComponent(jstring &jInstanceId, jstring &jcomponentRef, jstring &jmethod,
                                     jbyteArray &jArgString, jbyteArray &jOptString) {
    JNIEnv *env = getJNIEnv();
    if (jCallNativeComponentMethodId == NULL) {
      jCallNativeComponentMethodId = env->GetMethodID(jBridgeClazz,
                                                      "callNativeComponent",
                                                      "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[B[B)V");
    }
    env->CallVoidMethod(jThis, jCallNativeComponentMethodId, jInstanceId, jcomponentRef, jmethod,
                        jArgString, jOptString);
    env->DeleteLocalRef(jInstanceId);
    env->DeleteLocalRef(jcomponentRef);
    env->DeleteLocalRef(jmethod);
    env->DeleteLocalRef(jArgString);
    env->DeleteLocalRef(jOptString);
  }

  int BridgeAndroid::callAddElement(jstring &jInstanceId, jstring &jref, jbyteArray &jdomString,
                                    jstring &jindex, jstring &jCallback) {
    JNIEnv *env = getJNIEnv();
    if (jCallAddElementMethodId == NULL) {
      jCallAddElementMethodId = env->GetMethodID(jBridgeClazz,
                                                 "callAddElement",
                                                 "(Ljava/lang/String;Ljava/lang/String;[BLjava/lang/String;Ljava/lang/String;)I");
    }

    int flag = env->CallIntMethod(jThis, jCallAddElementMethodId, jInstanceId, jref, jdomString,
                                  jindex,
                                  jCallback);
    if (flag == -1) {
      LOGE("instance destroy JFM must stop callNative");
    }
    env->DeleteLocalRef(jInstanceId);
    env->DeleteLocalRef(jref);
    env->DeleteLocalRef(jdomString);
    env->DeleteLocalRef(jindex);
    env->DeleteLocalRef(jCallback);
    return flag;
  }

  void BridgeAndroid::setTimeout(jstring &jCallbackID, jstring &jTime) {
    JNIEnv *env = getJNIEnv();
    if (jSetTimeoutNativeMethodId == NULL) {
      jSetTimeoutNativeMethodId = env->GetMethodID(jBridgeClazz,
                                                   "setTimeoutNative",
                                                   "(Ljava/lang/String;Ljava/lang/String;)V");
    }
    env->CallVoidMethod(jThis, jSetTimeoutNativeMethodId, jCallbackID, jTime);
    env->DeleteLocalRef(jCallbackID);
    env->DeleteLocalRef(jTime);
  }

  void BridgeAndroid::callNativeLog(jstring &str_msg) {
    JNIEnv *env = getJNIEnv();
    if (jWXLogUtils != NULL) {
      if (jLogMethodId == NULL) {
        jLogMethodId = env->GetStaticMethodID(jWXLogUtils, "d",
                                              "(Ljava/lang/String;Ljava/lang/String;)V");
      }
      if (jLogMethodId != NULL) {
        jstring str_tag = env->NewStringUTF("jsLog");
        // str_msg = env->NewStringUTF(s);
        env->CallStaticVoidMethod(jWXLogUtils, jLogMethodId, str_tag, str_msg);
        env->DeleteLocalRef(str_tag);
      }
    }
    env->DeleteLocalRef(str_msg);
  }


  int BridgeAndroid::callCreateBody(jstring &jInstanceId, jbyteArray &jTaskString, jstring &jCallback) {
    JNIEnv *env = getJNIEnv();
    if (jCallCreateBodyMethodId == NULL) {
      jCallCreateBodyMethodId = env->GetMethodID(jBridgeClazz,
                                                 "callCreateBody",
                                                 "(Ljava/lang/String;[BLjava/lang/String;)I");
    }
    int flag = env->CallIntMethod(jThis, jCallCreateBodyMethodId, jInstanceId, jTaskString,
                                  jCallback);
    if (flag == -1) {
      LOGE("instance destroy JFM must stop callCreateBody");
    }

    env->DeleteLocalRef(jTaskString);
    env->DeleteLocalRef(jInstanceId);
    env->DeleteLocalRef(jCallback);
    return flag;
  }


  int BridgeAndroid::callUpdateFinish(jstring &instanceId, jbyteArray &taskString,
                                      jstring &callback) {
    JNIEnv *env = getJNIEnv();
    if (jCallUpdateFinishMethodId == NULL) {
      jCallUpdateFinishMethodId = env->GetMethodID(jBridgeClazz,
                                                   "callUpdateFinish",
                                                   "(Ljava/lang/String;[BLjava/lang/String;)I");
    }

    int flag = env->CallIntMethod(jThis, jCallUpdateFinishMethodId, instanceId, taskString,
                                  callback);
    if (flag == -1) {
      LOGE("instance destroy JFM must stop callUpdateFinish");
    }
    env->DeleteLocalRef(instanceId);
    env->DeleteLocalRef(taskString);
    env->DeleteLocalRef(callback);
    return flag;
  }

  int BridgeAndroid::callCreateFinish(jstring &instanceId, jbyteArray &taskString,
                                      jstring &callback) {
    JNIEnv *env = getJNIEnv();
    if (jCallCreateFinishMethodId == NULL) {
      jCallCreateFinishMethodId = env->GetMethodID(jBridgeClazz,
                                                   "callCreateFinish",
                                                   "(Ljava/lang/String;[BLjava/lang/String;)I");
    }

    int flag = env->CallIntMethod(jThis, jCallCreateFinishMethodId, instanceId, taskString,
                                  callback);
    if (flag == -1) {
      LOGE("instance destroy JFM must stop callCreateFinish");
    }
    env->DeleteLocalRef(instanceId);
    env->DeleteLocalRef(taskString);
    env->DeleteLocalRef(callback);
    return flag;
  }

  int BridgeAndroid::callRefreshFinish(jstring &instanceId, jbyteArray &taskString,
                                       jstring &callback) {
    JNIEnv *env = getJNIEnv();
    if (jCallRefreshFinishMethodId == NULL) {
      jCallRefreshFinishMethodId = env->GetMethodID(jBridgeClazz,
                                                    "callRefreshFinish",
                                                    "(Ljava/lang/String;[BLjava/lang/String;)I");
    }
    int flag = env->CallIntMethod(jThis, jCallRefreshFinishMethodId, instanceId, taskString,
                                  callback);
    if (flag == -1) {
      LOGE("instance destroy JFM must stop callNative");
    }
    env->DeleteLocalRef(instanceId);
    env->DeleteLocalRef(taskString);
    env->DeleteLocalRef(callback);
    return flag;
  }

  int BridgeAndroid::callUpdateAttrs(jstring &instanceId, jstring &ref,
                                     jbyteArray &taskString, jstring &callback) {
    JNIEnv *env = getJNIEnv();
    if (jCallUpdateAttrsMethodId == NULL) {
      jCallUpdateAttrsMethodId = env->GetMethodID(jBridgeClazz,
                                                  "callUpdateAttrs",
                                                  "(Ljava/lang/String;Ljava/lang/String;[BLjava/lang/String;)I");
    }
    int flag = env->CallIntMethod(jThis, jCallUpdateAttrsMethodId, instanceId, ref, taskString,
                                  callback);
    if (flag == -1) {
      LOGE("instance destroy JFM must stop callUpdateAttrs");
    }
    env->DeleteLocalRef(instanceId);
    env->DeleteLocalRef(ref);
    env->DeleteLocalRef(taskString);
    env->DeleteLocalRef(callback);
    return flag;
  }

  int BridgeAndroid::callUpdateStyle(jstring &instanceId, jstring &ref,
                                     jbyteArray &taskString, jstring &callback) {
    JNIEnv *env = getJNIEnv();
    if (jCallUpdateStyleMethodId == NULL) {
      jCallUpdateStyleMethodId = env->GetMethodID(jBridgeClazz,
                                                  "callUpdateStyle",
                                                  "(Ljava/lang/String;Ljava/lang/String;[BLjava/lang/String;)I");
    }

    int flag = env->CallIntMethod(jThis, jCallUpdateStyleMethodId, instanceId, ref, taskString,
                                  callback);
    if (flag == -1) {
      LOGE("instance destroy JFM must stop callUpdateStyle");
    }
    env->DeleteLocalRef(instanceId);
    env->DeleteLocalRef(ref);
    env->DeleteLocalRef(taskString);
    env->DeleteLocalRef(callback);
    return flag;
  }

  int BridgeAndroid::callRemoveElement(jstring &instanceId, jstring &ref,
                                       jstring &callback) {
    JNIEnv *env = getJNIEnv();
    if (jCallRemoveElementMethodId == NULL) {
      jCallRemoveElementMethodId = env->GetMethodID(jBridgeClazz,
                                                    "callRemoveElement",
                                                    "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I");
    }

    int flag = env->CallIntMethod(jThis, jCallRemoveElementMethodId, instanceId, ref, callback);
    if (flag == -1) {
      LOGE("instance destroy JFM must stop callRemoveElement");
    }
    env->DeleteLocalRef(instanceId);
    env->DeleteLocalRef(ref);
    env->DeleteLocalRef(callback);
    return flag;
  }

  int BridgeAndroid::callMoveElement(jstring &instanceId, jstring &ref, jstring &parentref,
                                     jstring &index, jstring &callback) {
    JNIEnv *env = getJNIEnv();
    if (jCallMoveElementMethodId == NULL) {
      jCallMoveElementMethodId = env->GetMethodID(jBridgeClazz,
                                                  "callMoveElement",
                                                  "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I");
    }

    int flag = env->CallIntMethod(jThis, jCallMoveElementMethodId, instanceId, ref, parentref,
                                  index, callback);
    if (flag == -1) {
      LOGE("instance destroy JFM must stop callRemoveElement");
    }

    env->DeleteLocalRef(instanceId);
    env->DeleteLocalRef(ref);
    env->DeleteLocalRef(parentref);
    env->DeleteLocalRef(index);
    env->DeleteLocalRef(callback);
    return flag;
  }

  int BridgeAndroid::callAddEvent(jstring &instanceId,
                                  jstring &ref, jstring &event, jstring &Callback) {
    JNIEnv *env = getJNIEnv();
    if (jCallAddEventMethodId == NULL) {
      jCallAddEventMethodId = env->GetMethodID(jBridgeClazz,
                                               "callAddEvent",
                                               "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I");
    }

    int flag = env->CallIntMethod(jThis, jCallAddEventMethodId, instanceId, ref, event, Callback);
    if (flag == -1) {
      LOGE("instance destroy JFM must stop callAddEvent");
    }
    env->DeleteLocalRef(instanceId);
    env->DeleteLocalRef(ref);
    env->DeleteLocalRef(event);
    env->DeleteLocalRef(Callback);
    return flag;
  }

  int BridgeAndroid::callRemoveEvent(jstring &instanceId,
                                     jstring &ref, jstring &event, jstring &Callback) {
    JNIEnv *env = getJNIEnv();
    if (jCallRemoveEventMethodId == NULL) {
      jCallRemoveEventMethodId = env->GetMethodID(jBridgeClazz,
                                                  "callRemoveEvent",
                                                  "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I");
    }

    int flag = env->CallIntMethod(jThis, jCallRemoveEventMethodId, instanceId, ref, event,
                                  Callback);
    if (flag == -1) {
      LOGE("instance destroy JFM must stop callRemoveElement");
    }
    env->DeleteLocalRef(instanceId);
    env->DeleteLocalRef(ref);
    env->DeleteLocalRef(event);
    env->DeleteLocalRef(Callback);
    return flag;
  }

} //end WeexCore