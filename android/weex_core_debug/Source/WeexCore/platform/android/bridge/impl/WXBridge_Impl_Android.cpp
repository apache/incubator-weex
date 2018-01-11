#include <WeexCore/render/manager/RenderManager.h>
#include <WeexCore/render/page/RenderPage.h>
#include <WeexCore/render/node/RenderObject.h>
#include <base/TimeUtils.h>
#include "WXBridge_Impl_Android.h"

static jmethodID jSetJSFrmVersionMethodId;
static jmethodID jReportExceptionMethodId;
static jmethodID jCallNativeMethodId;
static jmethodID jCallNativeModuleMethodId;
static jmethodID jCallNativeComponentMethodId;

static jmethodID jSetTimeoutNativeMethodId;
static jmethodID jLogMethodId;
static jmethodID jCallUpdateFinishMethodId;

static jmethodID jCallRefreshFinishMethodId;
static jmethodID jCallRemoveElementMethodId;
static jmethodID jCallMoveElementMethodId;
static jmethodID jCallAddEventMethodId;
static jmethodID jCallRemoveEventMethodId;

static jmethodID jMapConstructorMethodId;
static jmethodID jMapPutMethodId;
static jmethodID jSetConstructorMethodId;
static jmethodID jSetAddMethodId;

static jmethodID jCallCreateBodyByWeexCoreMethodId;
static jmethodID jCallAddElementByWeexCoreMethodId;
static jmethodID jCallUpdateStyleByWeexCoreMethodId;
static jmethodID jCallUpdateAttrsByWeexCoreMethodId;
static jmethodID jCallLayoutByWeexCoreMethodId;
static jmethodID jCallCreateFinishByWeexCoreMethodId;
//static jmethodID jCallLogOfFirstScreenMethodId;

namespace WeexCore {

  Bridge_Impl_Android *Bridge_Impl_Android::m_instance = nullptr;

  Bridge_Impl_Android::Bridge_Impl_Android() {
  }

  Bridge_Impl_Android::~Bridge_Impl_Android() {
  }

  void static cpyCMap2JMap(std::map<std::string, std::string> *cMap, jobject &jMap, JNIEnv *env) {
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

  void static
  cpyCVector2JMap(std::vector<std::pair<std::string, std::string> *> *cVector, jobject &jMap,
                  JNIEnv *env) {
    jstring jKey;
    jstring jValue;

    for (int i = 0; i < cVector->size(); ++i) {
      jKey = env->NewStringUTF((*cVector)[i]->first.c_str());
      jValue = env->NewStringUTF((*cVector)[i]->second.c_str());
      env->CallObjectMethod(jMap, jMapPutMethodId, jKey, jValue);
      env->DeleteLocalRef(jKey);
      env->DeleteLocalRef(jValue);
    }
  }

  void static cpyCSet2JSet(std::set<std::string> *cSet, jobject &jSet, JNIEnv *env) {
    jstring jValue;

    std::set<std::string>::const_iterator it = cSet->begin();
    std::set<std::string>::const_iterator end = cSet->end();

    for (; it != end; ++it) {
      jValue = env->NewStringUTF((*it).c_str());
      env->CallBooleanMethod(jSet, jSetAddMethodId, jValue);
      env->DeleteLocalRef(jValue);
    }
  }

  void Bridge_Impl_Android::setJSVersion(jstring &jversion) {
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
  Bridge_Impl_Android::reportException(jstring &jInstanceId, jstring &jFunc, jstring &jExceptionString) {
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

  int Bridge_Impl_Android::callNative(jstring &instanceId, jbyteArray &taskString, jstring &callback) {
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

  jobject Bridge_Impl_Android::callNativeModule(jstring &jInstanceId, jstring &jmodule, jstring &jmethod,
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
  Bridge_Impl_Android::callNativeComponent(jstring &jInstanceId, jstring &jcomponentRef, jstring &jmethod,
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

  void Bridge_Impl_Android::setTimeout(jstring &jCallbackID, jstring &jTime) {
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

  void Bridge_Impl_Android::callNativeLog(jbyteArray &str_msg) {
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

  int Bridge_Impl_Android::callUpdateFinish(jstring &instanceId, jbyteArray &taskString,
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

  int Bridge_Impl_Android::callRefreshFinish(jstring &instanceId, jbyteArray &taskString,
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

  int Bridge_Impl_Android::callAddEvent(std::string &instanceId,
                                        std::string &ref, std::string &event, std::string &callback) {
    JNIEnv *env = getJNIEnv();
    if (jCallAddEventMethodId == NULL) {
      jCallAddEventMethodId = env->GetMethodID(jBridgeClazz,
                                               "callAddEvent",
                                               "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I");
    }
    jstring jPageId = env->NewStringUTF(instanceId.c_str());
    jstring jRef = env->NewStringUTF(ref.c_str());
    jstring jEventId = env->NewStringUTF(event.c_str());
    jstring jCallbackId = env->NewStringUTF(callback.c_str());

    int flag = env->CallIntMethod(jThis, jCallAddEventMethodId, jPageId, jRef, jEventId, jCallbackId);
    if (flag == -1) {
      LOGE("instance destroy JFM must stop callAddEvent");
    }
    env->DeleteLocalRef(jPageId);
    env->DeleteLocalRef(jRef);
    env->DeleteLocalRef(jEventId);
    env->DeleteLocalRef(jCallbackId);
    return flag;
  }

  int Bridge_Impl_Android::callRemoveEvent(std::string &instanceId,
                                           std::string &ref, std::string &event, std::string &callback) {
    JNIEnv *env = getJNIEnv();
    if (jCallRemoveEventMethodId == NULL) {
      jCallRemoveEventMethodId = env->GetMethodID(jBridgeClazz,
                                                  "callRemoveEvent",
                                                  "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I");
    }
    jstring jPageId = env->NewStringUTF(instanceId.c_str());
    jstring jRef = env->NewStringUTF(ref.c_str());
    jstring jEventId = env->NewStringUTF(event.c_str());
    jstring jCallbackId = env->NewStringUTF(callback.c_str());

    int flag = env->CallIntMethod(jThis, jCallRemoveEventMethodId, jPageId, jRef, jEventId, jCallbackId);
    if (flag == -1) {
      LOGE("instance destroy JFM must stop callRemoveElement");
    }
    env->DeleteLocalRef(jPageId);
    env->DeleteLocalRef(jRef);
    env->DeleteLocalRef(jEventId);
    env->DeleteLocalRef(jCallbackId);
    return flag;
  }


  /****************** WeexCore *****************/

  int Bridge_Impl_Android::callCreateBody(std::string &pageId, std::string &componentType,
                                          std::string &ref,
                                          std::map<std::string, std::string> *styles,
                                          std::map<std::string, std::string> *attributes,
                                          std::set<std::string> *events,
                                          std::map<std::string, std::string> *paddings,
                                          std::map<std::string, std::string> *margins,
                                          std::map<std::string, std::string> *borders) {
    JNIEnv *env = getJNIEnv();

    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
    long long startTime = getCurrentTime();

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

    page->CreateJMapJNITime(getCurrentTime() - startTime);

    long long startTimeCallBridge = getCurrentTime();

    if (jCallCreateBodyByWeexCoreMethodId == NULL)
      jCallCreateBodyByWeexCoreMethodId = env->GetMethodID(jBridgeClazz,
                                                           "callCreateBodyByWeexCore",
                                                           "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/util/HashMap;Ljava/util/HashMap;Ljava/util/HashSet;Ljava/util/HashMap;Ljava/util/HashMap;Ljava/util/HashMap;)I");

    jstring jPageId = env->NewStringUTF(pageId.c_str());
    jstring jComponentType = env->NewStringUTF(componentType.c_str());
    jstring jRef = env->NewStringUTF(ref.c_str());

    int flag = 0;
    flag = env->CallIntMethod(jThis, jCallCreateBodyByWeexCoreMethodId, jPageId, jComponentType,
                              jRef, jStyles, jAttributes,
                              jEvents,
                              jPaddings, jMargins, jBorders);

    page->CallBridgeTime(getCurrentTime() - startTimeCallBridge);

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

  int Bridge_Impl_Android::callAddElement(std::string &pageId, std::string &componentType,
                                          std::string &ref, int &index,
                                          std::string &parentRef,
                                          std::map<std::string, std::string> *styles,
                                          std::map<std::string, std::string> *attributes,
                                          std::set<std::string> *events,
                                          std::map<std::string, std::string> *paddings,
                                          std::map<std::string, std::string> *margins,
                                          std::map<std::string, std::string> *borders) {
    JNIEnv *env = getJNIEnv();

    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
    long long startTime = getCurrentTime();

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

    page->CreateJMapJNITime(getCurrentTime() - startTime);

    long long startTimeCallBridge = getCurrentTime();

    if (jCallAddElementByWeexCoreMethodId == NULL)
      jCallAddElementByWeexCoreMethodId = env->GetMethodID(jBridgeClazz,
                                                           "callAddElementByWeexCore",
                                                           "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ILjava/lang/String;Ljava/util/HashMap;Ljava/util/HashMap;Ljava/util/HashSet;Ljava/util/HashMap;Ljava/util/HashMap;Ljava/util/HashMap;)I");

    jstring jPageId = env->NewStringUTF(pageId.c_str());
    jstring jComponentType = env->NewStringUTF(componentType.c_str());
    jstring jRef = env->NewStringUTF(ref.c_str());
    jstring jParentRef = env->NewStringUTF(parentRef.c_str());

    int flag = 0;
    flag = env->CallIntMethod(jThis, jCallAddElementByWeexCoreMethodId, jPageId, jComponentType,
                              jRef, index, jParentRef,
                              jStyles, jAttributes, jEvents, jPaddings, jMargins, jBorders);

    page->CallBridgeTime(getCurrentTime() - startTimeCallBridge);

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

  int Bridge_Impl_Android::callRemoveElement(std::string &pageId, std::string &ref) {
    JNIEnv *env = getJNIEnv();

    if (jCallRemoveElementMethodId == NULL) {
      jCallRemoveElementMethodId = env->GetMethodID(jBridgeClazz,
                                                    "callRemoveElement",
                                                    "(Ljava/lang/String;Ljava/lang/String;)I");
    }

    jstring jPageId = env->NewStringUTF(pageId.c_str());
    jstring jRef = env->NewStringUTF(ref.c_str());

    int flag = env->CallIntMethod(jThis, jCallRemoveElementMethodId, jPageId, jRef);
    if (flag == -1) {
      LOGE("instance destroy JFM must stop callRemoveElement");
    }

    env->DeleteLocalRef(jPageId);
    env->DeleteLocalRef(jRef);
    return 0;
  }

  int Bridge_Impl_Android::callMoveElement(std::string &pageId, std::string &ref, std::string &parentRef,
                                     int index) {
    JNIEnv *env = getJNIEnv();

    if (jCallMoveElementMethodId == NULL) {
      jCallMoveElementMethodId = env->GetMethodID(jBridgeClazz,
                                                  "callMoveElement",
                                                  "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)I");
    }

    jstring jPageId = env->NewStringUTF(pageId.c_str());
    jstring jRef = env->NewStringUTF(ref.c_str());
    jstring jParentRef = env->NewStringUTF(parentRef.c_str());

    int flag = env->CallIntMethod(jThis, jCallMoveElementMethodId, jPageId, jRef, jParentRef,
                                  index);
    if (flag == -1) {
      LOGE("instance destroy JFM must stop callRemoveElement");
    }

    env->DeleteLocalRef(jPageId);
    env->DeleteLocalRef(jRef);
    env->DeleteLocalRef(jParentRef);
    return 0;
  }

  int
  Bridge_Impl_Android::callUpdateStyle(std::string &pageId, std::string &ref,
                                       std::vector<std::pair<std::string, std::string> *> *style,
                                       std::vector<std::pair<std::string, std::string> *> *margin,
                                       std::vector<std::pair<std::string, std::string> *> *padding,
                                       std::vector<std::pair<std::string, std::string> *> *border) {
    JNIEnv *env = getJNIEnv();

    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
    long long startTime = getCurrentTime();

    if (jMapConstructorMethodId == NULL)
      jMapConstructorMethodId = env->GetMethodID(jMapClazz, "<init>", "()V");
    if (jMapPutMethodId == NULL)
      jMapPutMethodId = env->GetMethodID(jMapClazz, "put",
                                         "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

    jobject jStyles = env->NewObject(jMapClazz, jMapConstructorMethodId);
    jobject jMargins = env->NewObject(jMapClazz, jMapConstructorMethodId);
    jobject jPaddings = env->NewObject(jMapClazz, jMapConstructorMethodId);
    jobject jBorders = env->NewObject(jMapClazz, jMapConstructorMethodId);

    if (style != nullptr) {
      cpyCVector2JMap(style, jStyles, env);
    }

    if (margin != nullptr) {
      cpyCVector2JMap(margin, jMargins, env);
    }

    if (padding != nullptr) {
      cpyCVector2JMap(padding, jPaddings, env);
    }

    if (border != nullptr) {
      cpyCVector2JMap(border, jBorders, env);
    }

    page->CreateJMapJNITime(getCurrentTime() - startTime);

    long long startTimeCallBridge = getCurrentTime();

    if (jCallUpdateStyleByWeexCoreMethodId == NULL)
      jCallUpdateStyleByWeexCoreMethodId = env->GetMethodID(jBridgeClazz,
                                                            "callUpdateStyleByWeexCore",
                                                            "(Ljava/lang/String;Ljava/lang/String;Ljava/util/HashMap;Ljava/util/HashMap;Ljava/util/HashMap;Ljava/util/HashMap;)I");

    jstring jPageId = env->NewStringUTF(pageId.c_str());
    jstring jRef = env->NewStringUTF(ref.c_str());

    int flag = 0;
    flag = env->CallIntMethod(jThis, jCallUpdateStyleByWeexCoreMethodId, jPageId, jRef, jStyles,
                              jMargins, jPaddings, jBorders);

    page->CallBridgeTime(getCurrentTime() - startTimeCallBridge);

    if (flag == -1) {
      LOGE("instance destroy JFM must stop callUpdateStyle");
    }

    env->DeleteLocalRef(jPageId);
    env->DeleteLocalRef(jRef);
    env->DeleteLocalRef(jStyles);
    env->DeleteLocalRef(jMargins);
    env->DeleteLocalRef(jPaddings);
    env->DeleteLocalRef(jBorders);
    return flag;
  }

  int Bridge_Impl_Android::callUpdateAttr(std::string &pageId, std::string &ref,
                                          std::vector<std::pair<std::string, std::string> *> *attrs) {
    JNIEnv *env = getJNIEnv();

    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
    long long startTime = getCurrentTime();

    if (jMapConstructorMethodId == NULL)
      jMapConstructorMethodId = env->GetMethodID(jMapClazz, "<init>", "()V");
    if (jMapPutMethodId == NULL)
      jMapPutMethodId = env->GetMethodID(jMapClazz, "put",
                                         "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

    jobject jAttrs = env->NewObject(jMapClazz, jMapConstructorMethodId);

    if (attrs != nullptr) {
      cpyCVector2JMap(attrs, jAttrs, env);
    }

    page->CreateJMapJNITime(getCurrentTime() - startTime);

    long long startTimeCallBridge = getCurrentTime();

    if (jCallUpdateAttrsByWeexCoreMethodId == NULL) {
      jCallUpdateAttrsByWeexCoreMethodId = env->GetMethodID(jBridgeClazz,
                                                            "callUpdateAttrsByWeexCore",
                                                            "(Ljava/lang/String;Ljava/lang/String;Ljava/util/HashMap;)I");
    }

    jstring jPageId = env->NewStringUTF(pageId.c_str());
    jstring jRef = env->NewStringUTF(ref.c_str());

    int flag = 0;
    flag = env->CallIntMethod(jThis, jCallUpdateAttrsByWeexCoreMethodId, jPageId, jRef, jAttrs);

    page->CallBridgeTime(getCurrentTime() - startTimeCallBridge);

    if (flag == -1) {
      LOGE("instance destroy JFM must stop callUpdateStyle");
    }

    env->DeleteLocalRef(jPageId);
    env->DeleteLocalRef(jRef);
    env->DeleteLocalRef(jAttrs);
    return flag;
  }

  int Bridge_Impl_Android::callLayout(std::string &pageId, std::string &ref, int top,
                                      int bottom, int left, int right, int height,
                                      int width) {
    JNIEnv *env = getJNIEnv();

    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
    long long startTimeCallBridge = getCurrentTime();

    if (jCallLayoutByWeexCoreMethodId == NULL)
      jCallLayoutByWeexCoreMethodId = env->GetMethodID(jBridgeClazz,
                                                       "callLayoutByWeexCore",
                                                       "(Ljava/lang/String;Ljava/lang/String;IIIIII)I");

    jstring jPageId = env->NewStringUTF(pageId.c_str());
    jstring jRef = env->NewStringUTF(ref.c_str());

    int flag = 0;
    flag = env->CallIntMethod(jThis, jCallLayoutByWeexCoreMethodId, jPageId,
                              jRef, top, bottom, left, right, height, width);

    page->CallBridgeTime(getCurrentTime() - startTimeCallBridge);

    if (flag == -1) {
      LOGE("instance destroy JFM must stop callLayoutByWeexCore");
    }

    env->DeleteLocalRef(jPageId);
    env->DeleteLocalRef(jRef);
    return flag;
  }

  int Bridge_Impl_Android::callCreateFinish(std::string &pageId) {
    JNIEnv *env = getJNIEnv();

    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
    long long startTimeCallBridge = getCurrentTime();

    if (jCallCreateFinishByWeexCoreMethodId == NULL)
      jCallCreateFinishByWeexCoreMethodId = env->GetMethodID(jBridgeClazz,
                                                             "callCreateFinishByWeexCore",
                                                             "(Ljava/lang/String;)I");

    jstring jPageId = env->NewStringUTF(pageId.c_str());
    int flag = env->CallIntMethod(jThis, jCallCreateFinishByWeexCoreMethodId, jPageId);

    page->CallBridgeTime(getCurrentTime() - startTimeCallBridge);

    if (flag == -1) {
      LOGE("instance destroy JFM must stop callCreateFinish");
    }

    env->DeleteLocalRef(jPageId);
    return flag;
  }

//  void Bridge_Impl_Android::callLogOfFirstScreen(std::string &message) {
//    JNIEnv *env = getJNIEnv();
//
//    if (jCallLogOfFirstScreenMethodId == NULL)
//      jCallLogOfFirstScreenMethodId = env->GetMethodID(jBridgeClazz,
//                                                       "callLogOfFirstScreen",
//                                                       "(Ljava/lang/String;)V");
//
//    jstring jMessageId = env->NewStringUTF(message.c_str());
//    env->CallVoidMethod(jThis, jCallLogOfFirstScreenMethodId, jMessageId);
//
//    env->DeleteLocalRef(jMessageId);
//  }
} //end WeexCore