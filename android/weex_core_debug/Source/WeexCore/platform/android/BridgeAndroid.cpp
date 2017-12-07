#include <WeexCore/render/RenderObject.h>
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

static jmethodID jMapConstructorMethodId;
static jmethodID jMapPutMethodId;
static jmethodID jSetConstructorMethodId;
static jmethodID jSetAddMethodId;

namespace WeexCore {

  BridgeAndroid *BridgeAndroid::m_instance = nullptr;

  BridgeAndroid::BridgeAndroid() {
  }

  BridgeAndroid::~BridgeAndroid() {
  }

  void static cpyCMap2JMap(std::map<std::string, std::string> *cMap, jobject jMap, JNIEnv *env) {
    std::map<std::string, std::string>::const_iterator it = cMap->begin();
    std::map<std::string, std::string>::const_iterator end = cMap->end();
    jstring jKey;
    jstring jValue;

    for (; it != end; ++it) {
      jKey = env->NewStringUTF(it->first.c_str());
      jValue = env->NewStringUTF(it->second.c_str());
      env->CallObjectMethod(jMap, jMapPutMethodId, jKey, jValue);
      env->DeleteLocalRef(jKey);
      env->DeleteLocalRef(jValue);
    }
  }

  void static cpyCSet2JSet(std::set<std::string> *cSet, jobject jSet, JNIEnv *env) {
    jstring jValue;

    std::set<std::string>::const_iterator it = cSet->begin();
    std::set<std::string>::const_iterator end = cSet->end();

    for (; it != end; ++it) {
      jValue = env->NewStringUTF((*it).c_str());
      env->CallBooleanMethod(jSet, jSetAddMethodId, jValue);
      env->DeleteLocalRef(jValue);
    }
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

  void
  BridgeAndroid::reportException(jstring &jInstanceId, jstring &jFunc, jstring &jExceptionString) {
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

  void
  BridgeAndroid::callNativeComponent(jstring &jInstanceId, jstring &jcomponentRef, jstring &jmethod,
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

  void BridgeAndroid::callNativeLog(jbyteArray &str_msg) {
    JNIEnv *env = getJNIEnv();
    if (jWXLogUtils != NULL) {
      if (jLogMethodId == NULL) {
        jLogMethodId = env->GetStaticMethodID(jWXLogUtils, "d",
                                              "(Ljava/lang/String;[B)V");
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


  int
  BridgeAndroid::callCreateBody(jstring &jInstanceId, jbyteArray &jTaskString, jstring &jCallback) {
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

  int BridgeAndroid::callCreateBodyByWeexCore(std::string &pageId, std::string &componentType,
                                              std::string &ref,
                                              std::map<std::string, std::string> *styles,
                                              std::map<std::string, std::string> *attributes,
                                              std::set<std::string> *events,
                                              std::map<std::string, std::string> *paddings,
                                              std::map<std::string, std::string> *margins,
                                              std::map<std::string, std::string> *borders) {
    JNIEnv *env = getJNIEnv();
    jmethodID jCallCreateBodyByWeexCoreMethodId = env->GetMethodID(jBridgeClazz,
                                                                   "callCreateBodyByWeexCore",
                                                                   "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/util/HashMap;Ljava/util/HashMap;Ljava/util/HashSet;Ljava/util/HashMap;Ljava/util/HashMap;Ljava/util/HashMap;)I");

    jstring jPageId = env->NewStringUTF(pageId.c_str());
    jstring jComponentType = env->NewStringUTF(componentType.c_str());
    jstring jRef = env->NewStringUTF(ref.c_str());

    if (jMapConstructorMethodId == NULL)
      jMapConstructorMethodId = env->GetMethodID(jMapClazz, "<init>", "()V");
    if (jMapPutMethodId == NULL)
      jMapPutMethodId = env->GetMethodID(jMapClazz, "put",
                                         "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
    if (jSetConstructorMethodId == NULL)
      jSetConstructorMethodId = env->GetMethodID(jSetClazz, "<init>", "()V");
    if (jSetAddMethodId == NULL)
      jSetAddMethodId = env->GetMethodID(jSetClazz, "add",
                                         "(Ljava/lang/Object;)Z");

    jobject jStyles = env->NewObject(jMapClazz, jMapConstructorMethodId);
    jobject jAttributes = env->NewObject(jMapClazz, jMapConstructorMethodId);
    jobject jEvents = env->NewObject(jSetClazz, jSetConstructorMethodId);
    jobject jPaddings = env->NewObject(jMapClazz, jMapConstructorMethodId);
    jobject jMargins = env->NewObject(jMapClazz, jMapConstructorMethodId);
    jobject jBorders = env->NewObject(jMapClazz, jMapConstructorMethodId);

    cpyCMap2JMap(styles, jStyles, env);
    cpyCMap2JMap(attributes, jAttributes, env);
    cpyCSet2JSet(events, jEvents, env);
    cpyCMap2JMap(paddings, jPaddings, env);
    cpyCMap2JMap(margins, jMargins, env);
    cpyCMap2JMap(borders, jBorders, env);

    int flag = 0;
    flag = env->CallIntMethod(jThis, jCallCreateBodyByWeexCoreMethodId, jPageId, jComponentType,
                              jRef, jStyles, jAttributes,
                              jEvents,
                              jPaddings, jMargins, jBorders);
    if (flag == -1) {
      LOGE("instance destroy JFM must stop callCreateBody");
    }

    env->DeleteLocalRef(jPageId);
    env->DeleteLocalRef(jComponentType);
    env->DeleteLocalRef(jRef);
    env->DeleteLocalRef(jStyles);
    env->DeleteLocalRef(jAttributes);
    env->DeleteLocalRef(jEvents);
    env->DeleteLocalRef(jPaddings);
    env->DeleteLocalRef(jMargins);
    env->DeleteLocalRef(jBorders);
    return flag;
  }

  int BridgeAndroid::callAddElementByWeexCore(std::string &pageId, std::string &componentType,
                                              std::string &ref, int index,
                                              std::string parentRef,
                                              std::map<std::string, std::string> *styles,
                                              std::map<std::string, std::string> *attributes,
                                              std::set<std::string> *events,
                                              std::map<std::string, std::string> *paddings,
                                              std::map<std::string, std::string> *margins,
                                              std::map<std::string, std::string> *borders) {
    JNIEnv *env = getJNIEnv();
    jmethodID jCallAddElementByWeexCoreMethodId = env->GetMethodID(jBridgeClazz,
                                                                   "callAddElementByWeexCore",
                                                                   "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ILjava/lang/String;Ljava/util/HashMap;Ljava/util/HashMap;Ljava/util/HashSet;Ljava/util/HashMap;Ljava/util/HashMap;Ljava/util/HashMap;)I");

    jstring jPageId = env->NewStringUTF(pageId.c_str());
    jstring jComponentType = env->NewStringUTF(componentType.c_str());
    jstring jRef = env->NewStringUTF(ref.c_str());
    jstring jParentRef = env->NewStringUTF(parentRef.c_str());

    if (jMapConstructorMethodId == NULL)
      jMapConstructorMethodId = env->GetMethodID(jMapClazz, "<init>", "()V");
    if (jMapPutMethodId == NULL)
      jMapPutMethodId = env->GetMethodID(jMapClazz, "put",
                                         "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
    if (jSetConstructorMethodId == NULL)
      jSetConstructorMethodId = env->GetMethodID(jSetClazz, "<init>", "()V");
    if (jSetAddMethodId == NULL)
      jSetAddMethodId = env->GetMethodID(jSetClazz, "add",
                                         "(Ljava/lang/Object;)Z");

    jobject jStyles = env->NewObject(jMapClazz, jMapConstructorMethodId);
    jobject jAttributes = env->NewObject(jMapClazz, jMapConstructorMethodId);
    jobject jEvents = env->NewObject(jSetClazz, jSetConstructorMethodId);
    jobject jPaddings = env->NewObject(jMapClazz, jMapConstructorMethodId);
    jobject jMargins = env->NewObject(jMapClazz, jMapConstructorMethodId);
    jobject jBorders = env->NewObject(jMapClazz, jMapConstructorMethodId);

    cpyCMap2JMap(styles, jStyles, env);
    cpyCMap2JMap(attributes, jAttributes, env);
    cpyCSet2JSet(events, jEvents, env);
    cpyCMap2JMap(paddings, jPaddings, env);
    cpyCMap2JMap(margins, jMargins, env);
    cpyCMap2JMap(borders, jBorders, env);

    int flag = 0;
    flag = env->CallIntMethod(jThis, jCallAddElementByWeexCoreMethodId, jPageId, jComponentType,
                              jRef, index, jParentRef,
                              jStyles, jAttributes, jEvents, jPaddings, jMargins, jBorders);
    if (flag == -1) {
      LOGE("instance destroy JFM must stop callAddElement");
    }

    env->DeleteLocalRef(jPageId);
    env->DeleteLocalRef(jComponentType);
    env->DeleteLocalRef(jRef);
    env->DeleteLocalRef(jParentRef);
    env->DeleteLocalRef(jStyles);
    env->DeleteLocalRef(jAttributes);
    env->DeleteLocalRef(jEvents);
    env->DeleteLocalRef(jPaddings);
    env->DeleteLocalRef(jMargins);
    env->DeleteLocalRef(jBorders);
    return flag;
  }

  int
  BridgeAndroid::callUpdateStyleByWeexCore(std::string &pageId, std::string &ref, std::string &key,
                                           std::string &value) {
    JNIEnv *env = getJNIEnv();
    jmethodID jCallUpdateStyleByWeexCoreMethodId = env->GetMethodID(jBridgeClazz,
                                                                    "callUpdateStyleByWeexCore",
                                                                    "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I");

    jstring jPageId = env->NewStringUTF(pageId.c_str());
    jstring jRef = env->NewStringUTF(ref.c_str());
    jstring jKey = env->NewStringUTF(key.c_str());
    jstring jValue = env->NewStringUTF(value.c_str());

    int flag = 0;
    flag = env->CallIntMethod(jThis, jCallUpdateStyleByWeexCoreMethodId, jPageId, jRef, jKey,
                              jValue);
    if (flag == -1) {
      LOGE("instance destroy JFM must stop callUpdateStyle");
    }

    env->DeleteLocalRef(jPageId);
    env->DeleteLocalRef(jRef);
    env->DeleteLocalRef(jKey);
    env->DeleteLocalRef(jValue);
    return flag;
  }

  int BridgeAndroid::callLayoutByWeexCore(std::string &pageId, std::string &ref, int top,
                                          int bottom, int left, int right, int height,
                                          int width) {
    JNIEnv *env = getJNIEnv();
    jmethodID jCallLayoutByWeexCoreMethodId = env->GetMethodID(jBridgeClazz,
                                                               "callLayoutByWeexCore",
                                                               "(Ljava/lang/String;Ljava/lang/String;IIIIII)I");

    jstring jPageId = env->NewStringUTF(pageId.c_str());
    jstring jRef = env->NewStringUTF(ref.c_str());

    int flag = 0;
    flag = env->CallIntMethod(jThis, jCallLayoutByWeexCoreMethodId, jPageId,
                              jRef, top, bottom, left, right, height, width);
    if (flag == -1) {
      LOGE("instance destroy JFM must stop callLayoutByWeexCore");
    }

    env->DeleteLocalRef(jPageId);
    env->DeleteLocalRef(jRef);
    return flag;
  }

  int BridgeAndroid::callCreateFinishByWeexCore(std::string &pageId) {
    JNIEnv *env = getJNIEnv();
    jmethodID jCallCreateFinishByWeexCoreMethodId = env->GetMethodID(jBridgeClazz,
                                                                     "callCreateFinishByWeexCore",
                                                                     "(Ljava/lang/String;)I");

    jstring jPageId = env->NewStringUTF(pageId.c_str());
    int flag = env->CallIntMethod(jThis, jCallCreateFinishByWeexCoreMethodId, jPageId);
    if (flag == -1) {
      LOGE("instance destroy JFM must stop callCreateFinish");
    }

    env->DeleteLocalRef(jPageId);
    return flag;
  }
} //end WeexCore