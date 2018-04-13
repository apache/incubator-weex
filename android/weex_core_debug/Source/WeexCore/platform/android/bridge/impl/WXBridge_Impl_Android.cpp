#include <WeexCore/render/manager/RenderManager.h>
#include <WeexCore/render/page/RenderPage.h>
#include <WeexCore/render/node/RenderObject.h>
#include <base/TimeUtils.h>
#include <WeexCore/platform/android/base/string/StringUtils.h>
#include "WXBridge_Impl_Android.h"
#include <WeexCore/layout/WXCoreStyle.h>

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

static jmethodID jCallCreateBodyMethodId;
static jmethodID jCallAddElementMethodId;
static jmethodID jCallUpdateStyleMethodId;
static jmethodID jCallHasTransitionProsMethodId;
static jmethodID jCallUpdateAttrsMethodId;
static jmethodID jCallLayoutMethodId;
static jmethodID jCallCreateFinishMethodId;

namespace WeexCore {

  Bridge_Impl_Android *Bridge_Impl_Android::m_instance = nullptr;

  Bridge_Impl_Android::Bridge_Impl_Android() {}

  Bridge_Impl_Android::~Bridge_Impl_Android() {}

  void static cpyCMap2JMap(std::map<std::string, std::string> *cMap,
                           jobject &jMap, JNIEnv *env) {
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

  void static cpyCVector2JMap(std::vector<std::pair<std::string, std::string>> *cVector,
                              jobject &jMap, JNIEnv *env) {
    jstring jKey;
    jstring jValue;

    for (int i = 0; i < cVector->size(); ++i) {
      jKey = env->NewStringUTF((*cVector)[i].first.c_str());
      jValue = env->NewStringUTF((*cVector)[i].second.c_str());
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

  void Bridge_Impl_Android::setJSVersion(const char* version) {
    JNIEnv *env = getJNIEnv();
    jstring jVersion = env->NewStringUTF(version);

    if (jSetJSFrmVersionMethodId == NULL) {
      jSetJSFrmVersionMethodId = env->GetMethodID(jBridgeClazz,
                                                  "setJSFrmVersion",
                                                  "(Ljava/lang/String;)V");
    }
    env->CallVoidMethod(jThis, jSetJSFrmVersionMethodId, jVersion);

    if (jVersion != nullptr)
      env->DeleteLocalRef(jVersion);
  }

  void Bridge_Impl_Android::reportException(const char *pageId, const char *func, const char *exception_string) {
    JNIEnv *env = getJNIEnv();
    jstring jPageId = env->NewStringUTF(pageId);
    jstring jFunc = env->NewStringUTF(func);
    jstring jExceptionString = env->NewStringUTF(exception_string);

    if (jReportExceptionMethodId == NULL) {
      jReportExceptionMethodId = env->GetMethodID(jBridgeClazz,
                                                  "reportJSException",
                                                  "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
    }
    env->CallVoidMethod(jThis, jReportExceptionMethodId, jPageId, jFunc, jExceptionString);

    if (jPageId != nullptr)
      env->DeleteLocalRef(jPageId);
    if (jFunc != nullptr)
      env->DeleteLocalRef(jFunc);
    if (jExceptionString != nullptr)
      env->DeleteLocalRef(jExceptionString);
  }

  int Bridge_Impl_Android::callNative(const char *pageId, const char *task, const char *callback) {
    JNIEnv *env = getJNIEnv();
    jstring jPageId = env->NewStringUTF(pageId);
    jbyteArray jTask = newJByteArray(env, task);
    jstring jCallback = env->NewStringUTF(callback);

    int flag = -1;

    if (jPageId != nullptr && jTask != nullptr) {
      if (jCallNativeMethodId == NULL) {
        jCallNativeMethodId = env->GetMethodID(jBridgeClazz,
                                               "callNative",
                                               "(Ljava/lang/String;[BLjava/lang/String;)I");
      }

      flag = env->CallIntMethod(jThis, jCallNativeMethodId, jPageId, jTask, jCallback);
    }

    if (flag == -1) {
      LOGE("instance destroy JFM must stop callNative");
    }

    if (jPageId != nullptr)
      env->DeleteLocalRef(jPageId);
    if (jTask != nullptr)
      env->DeleteLocalRef(jTask);
    if (jCallback != nullptr)
      env->DeleteLocalRef(jCallback);
    return flag;
  }

  jobject Bridge_Impl_Android::callNativeModule(const char *pageId, const char *module, const char *method,
                                                const char *argString, const char *optString) {
    JNIEnv *env = getJNIEnv();
    jstring jPageId = env->NewStringUTF(pageId);
    jstring jModule = env->NewStringUTF(module);
    jstring jMethod = env->NewStringUTF(method);
    jbyteArray jArgString = newJByteArray(env, argString);
    jbyteArray jOptString = newJByteArray(env, optString);

    jobject result = nullptr;

    if (jPageId != nullptr && jModule != nullptr && jMethod != nullptr) {
      if (jCallNativeModuleMethodId == NULL) {
        jCallNativeModuleMethodId = env->GetMethodID(jBridgeClazz,
                                                     "callNativeModule",
                                                     "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[B[B)Ljava/lang/Object;");
      }

      result = env->CallObjectMethod(jThis, jCallNativeModuleMethodId, jPageId, jModule,
                                             jMethod, jArgString, jOptString);
    }

    if (jPageId != nullptr)
      env->DeleteLocalRef(jPageId);
    if (jModule != nullptr)
      env->DeleteLocalRef(jModule);
    if (jMethod != nullptr)
      env->DeleteLocalRef(jMethod);
    if (jArgString != nullptr)
      env->DeleteLocalRef(jArgString);
    if (jOptString != nullptr)
      env->DeleteLocalRef(jOptString);
    return result;
  }

  void Bridge_Impl_Android::callNativeComponent(const char *pageId, const char *ref,
                                                const char *method, const char *argString,
                                                const char *optString) {
    JNIEnv *env = getJNIEnv();
    jstring jPageId = env->NewStringUTF(pageId);
    jstring jRef = env->NewStringUTF(ref);
    jstring jMethod = env->NewStringUTF(method);
    jbyteArray jArgString = newJByteArray(env, argString);
    jbyteArray jOptString = newJByteArray(env, optString);

    if (jPageId != nullptr && jRef != nullptr && jMethod != nullptr) {
      if (jCallNativeComponentMethodId == NULL) {
        jCallNativeComponentMethodId = env->GetMethodID(jBridgeClazz,
                                                        "callNativeComponent",
                                                        "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[B[B)V");
      }
      env->CallVoidMethod(jThis, jCallNativeComponentMethodId, jPageId, jRef, jMethod,
                          jArgString, jOptString);
    }

    if (jPageId != nullptr)
      env->DeleteLocalRef(jPageId);
    if (jRef != nullptr)
      env->DeleteLocalRef(jRef);
    if (jMethod != nullptr)
      env->DeleteLocalRef(jMethod);
    if (jArgString != nullptr)
      env->DeleteLocalRef(jArgString);
    if (jOptString != nullptr)
      env->DeleteLocalRef(jOptString);
  }

  void Bridge_Impl_Android::setTimeout(const char* callbackID, const char* time) {
    JNIEnv *env = getJNIEnv();
    jstring jCallbackID = env->NewStringUTF(callbackID);
    jstring jTime = env->NewStringUTF(time);

    if (jCallbackID == nullptr || jTime == nullptr)
      return;

    if (jSetTimeoutNativeMethodId == NULL) {
      jSetTimeoutNativeMethodId = env->GetMethodID(jBridgeClazz,
                                                   "setTimeoutNative",
                                                   "(Ljava/lang/String;Ljava/lang/String;)V");
    }

    env->CallVoidMethod(jThis, jSetTimeoutNativeMethodId, jCallbackID, jTime);

    if (jCallbackID != nullptr)
      env->DeleteLocalRef(jCallbackID);
    if (jTime != nullptr)
      env->DeleteLocalRef(jTime);
  }

  void Bridge_Impl_Android::callNativeLog(const char* str_array) {
    JNIEnv *env = getJNIEnv();
    if (jWXLogUtils != NULL) {
      if (jLogMethodId == NULL) {
        jLogMethodId = env->GetStaticMethodID(jWXLogUtils, "d",
                                              "(Ljava/lang/String;[B)V");
      }
      if (jLogMethodId != NULL) {
        jstring str_tag = env->NewStringUTF("jsLog");
        jbyteArray str_msg = newJByteArray(env, str_array);
        env->CallStaticVoidMethod(jWXLogUtils, jLogMethodId, str_tag, str_msg);
        env->DeleteLocalRef(str_tag);
        env->DeleteLocalRef(str_msg);
      }
    }
  }

  int Bridge_Impl_Android::callUpdateFinish(const char *pageId, const char *task, const char *callback) {
    JNIEnv *env = getJNIEnv();
    jstring jPageId = env->NewStringUTF(pageId);
    jbyteArray jTask = newJByteArray(env, task);
    jstring jCallback = env->NewStringUTF(callback);

    if (jCallUpdateFinishMethodId == NULL) {
      jCallUpdateFinishMethodId = env->GetMethodID(jBridgeClazz,
                                                   "callUpdateFinish",
                                                   "(Ljava/lang/String;[BLjava/lang/String;)I");
    }

    int flag = env->CallIntMethod(jThis, jCallUpdateFinishMethodId, jPageId, jTask, jCallback);

    if (flag == -1) {
      LOGE("instance destroy JFM must stop callUpdateFinish");
    }

    if (jPageId != nullptr)
      env->DeleteLocalRef(jPageId);
    if (jTask != nullptr)
      env->DeleteLocalRef(jTask);
    if (jCallback != nullptr)
      env->DeleteLocalRef(jCallback);
    return flag;
  }

  int Bridge_Impl_Android::callRefreshFinish(const char *pageId, const char *task, const char *callback) {
    JNIEnv *env = getJNIEnv();
    jstring jPageId = env->NewStringUTF(pageId);
    jbyteArray jTask = newJByteArray(env, task);
    jstring jCallback = env->NewStringUTF(callback);

    if (jCallRefreshFinishMethodId == NULL) {
      jCallRefreshFinishMethodId = env->GetMethodID(jBridgeClazz,
                                                    "callRefreshFinish",
                                                    "(Ljava/lang/String;[BLjava/lang/String;)I");
    }
    int flag = env->CallIntMethod(jThis, jCallRefreshFinishMethodId, jPageId, jTask, jCallback);

    if (flag == -1) {
      LOGE("instance destroy JFM must stop callNative");
    }

    if (jPageId != nullptr)
      env->DeleteLocalRef(jPageId);
    if (jTask != nullptr)
      env->DeleteLocalRef(jTask);
    if (jCallback != nullptr)
      env->DeleteLocalRef(jCallback);
    return flag;
  }

  int Bridge_Impl_Android::callCreateBody(int &pageId, const char *componentType, const char *ref,
                                          std::map<std::string, std::string> *styles,
                                          std::map<std::string, std::string> *attributes,
                                          std::set<std::string> *events,
                                          const WXCoreMargin &margins,
                                          const WXCorePadding &paddings,
                                          const WXCoreBorderWidth &borders) {
    JNIEnv *env = getJNIEnv();

    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
    if (page == nullptr)
      return -1;

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

    cpyCMap2JMap(styles, jStyles, env);
    cpyCMap2JMap(attributes, jAttributes, env);
    cpyCSet2JSet(events, jEvents, env);

    page->CreateJMapJNITime(getCurrentTime() - startTime);

    float c_margins[4];
    float c_paddings[4];
    float c_borders[4];
    jfloatArray jMargins = env->NewFloatArray(4);
    jfloatArray jPaddings = env->NewFloatArray(4);
    jfloatArray jBorders = env->NewFloatArray(4);

    c_margins[0] = margins.getMargin(kMarginTop);
    c_margins[1] = margins.getMargin(kMarginBottom);
    c_margins[2] = margins.getMargin(kMarginLeft);
    c_margins[3] = margins.getMargin(kMarginRight);
    env->SetFloatArrayRegion(jMargins, 0, 4, c_margins);

    c_paddings[0] = paddings.getPadding(kPaddingTop);
    c_paddings[1] = paddings.getPadding(kPaddingBottom);
    c_paddings[2] = paddings.getPadding(kPaddingLeft);
    c_paddings[3] = paddings.getPadding(kPaddingRight);
    env->SetFloatArrayRegion(jPaddings, 0, 4, c_paddings);

    c_borders[0] = borders.getBorderWidth(kBorderWidthTop);
    c_borders[1] = borders.getBorderWidth(kBorderWidthBottom);
    c_borders[2] = borders.getBorderWidth(kBorderWidthLeft);
    c_borders[3] = borders.getBorderWidth(kBorderWidthRight);
    env->SetFloatArrayRegion(jBorders, 0, 4, c_borders);

    long long startTimeCallBridge = getCurrentTime();

    if (jCallCreateBodyMethodId == NULL)
      jCallCreateBodyMethodId = env->GetMethodID(jBridgeClazz,
                                                           "callCreateBody",
                                                           "(ILjava/lang/String;Ljava/lang/String;Ljava/util/HashMap;Ljava/util/HashMap;Ljava/util/HashSet;[F[F[F)I");

    jstring jComponentType = env->NewStringUTF(componentType);
    jstring jRef = env->NewStringUTF(ref);

    int flag = 0;
    flag = env->CallIntMethod(jThis, jCallCreateBodyMethodId, pageId, jComponentType,
                              jRef, jStyles, jAttributes,
                              jEvents,
                              jMargins, jPaddings, jBorders);

    page->CallBridgeTime(getCurrentTime() - startTimeCallBridge);

    if (flag == -1) {
      LOGE("instance destroy JFM must stop callCreateBody");
    }

    env->DeleteLocalRef(jComponentType);
    env->DeleteLocalRef(jRef);
    env->DeleteLocalRef(jStyles);
    env->DeleteLocalRef(jAttributes);
    env->DeleteLocalRef(jEvents);
    env->DeleteLocalRef(jMargins);
    env->DeleteLocalRef(jPaddings);
    env->DeleteLocalRef(jBorders);
    return flag;
  }

  int Bridge_Impl_Android::callAddElement(int &pageId, const char *componentType,
                                          const char *ref, int &index,
                                          const char *parentRef,
                                          std::map<std::string, std::string> *styles,
                                          std::map<std::string, std::string> *attributes,
                                          std::set<std::string> *events,
                                          const WXCoreMargin &margins,
                                          const WXCorePadding &paddings,
                                          const WXCoreBorderWidth &borders) {
    JNIEnv *env = getJNIEnv();

    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
    if (page == nullptr)
      return -1;

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

    cpyCMap2JMap(styles, jStyles, env);
    cpyCMap2JMap(attributes, jAttributes, env);
    cpyCSet2JSet(events, jEvents, env);

    page->CreateJMapJNITime(getCurrentTime() - startTime);

    float c_margins[4];
    float c_paddings[4];
    float c_borders[4];
    jfloatArray jMargins = env->NewFloatArray(4);
    jfloatArray jPaddings = env->NewFloatArray(4);
    jfloatArray jBorders = env->NewFloatArray(4);

    c_margins[0] = margins.getMargin(kMarginTop);
    c_margins[1] = margins.getMargin(kMarginBottom);
    c_margins[2] = margins.getMargin(kMarginLeft);
    c_margins[3] = margins.getMargin(kMarginRight);
    env->SetFloatArrayRegion(jMargins, 0, 4, c_margins);

    c_paddings[0] = paddings.getPadding(kPaddingTop);
    c_paddings[1] = paddings.getPadding(kPaddingBottom);
    c_paddings[2] = paddings.getPadding(kPaddingLeft);
    c_paddings[3] = paddings.getPadding(kPaddingRight);
    env->SetFloatArrayRegion(jPaddings, 0, 4, c_paddings);

    c_borders[0] = borders.getBorderWidth(kBorderWidthTop);
    c_borders[1] = borders.getBorderWidth(kBorderWidthBottom);
    c_borders[2] = borders.getBorderWidth(kBorderWidthLeft);
    c_borders[3] = borders.getBorderWidth(kBorderWidthRight);
    env->SetFloatArrayRegion(jBorders, 0, 4, c_borders);

    long long startTimeCallBridge = getCurrentTime();

    if (jCallAddElementMethodId == NULL)
      jCallAddElementMethodId = env->GetMethodID(jBridgeClazz,
                                                           "callAddElement",
                                                           "(ILjava/lang/String;Ljava/lang/String;ILjava/lang/String;Ljava/util/HashMap;Ljava/util/HashMap;Ljava/util/HashSet;[F[F[F)I");

    jstring jComponentType = env->NewStringUTF(componentType);
    jstring jRef = env->NewStringUTF(ref);
    jstring jParentRef = env->NewStringUTF(parentRef);

    int flag = 0;
    flag = env->CallIntMethod(jThis, jCallAddElementMethodId, pageId, jComponentType,
                              jRef, index, jParentRef,
                              jStyles, jAttributes, jEvents, jMargins, jPaddings, jBorders);

    page->CallBridgeTime(getCurrentTime() - startTimeCallBridge);

    if (flag == -1) {
      LOGE("instance destroy JFM must stop callAddElement");
    }

    env->DeleteLocalRef(jComponentType);
    env->DeleteLocalRef(jRef);
    env->DeleteLocalRef(jParentRef);
    env->DeleteLocalRef(jStyles);
    env->DeleteLocalRef(jAttributes);
    env->DeleteLocalRef(jEvents);
    env->DeleteLocalRef(jMargins);
    env->DeleteLocalRef(jPaddings);
    env->DeleteLocalRef(jBorders);
    return flag;
  }

  int Bridge_Impl_Android::callRemoveElement(int &pageId, const char *ref) {
    JNIEnv *env = getJNIEnv();

    if (jCallRemoveElementMethodId == NULL) {
      jCallRemoveElementMethodId = env->GetMethodID(jBridgeClazz,
                                                    "callRemoveElement",
                                                    "(ILjava/lang/String;)I");
    }

    jstring jRef = env->NewStringUTF(ref);

    int flag = env->CallIntMethod(jThis, jCallRemoveElementMethodId, pageId, jRef);
    if (flag == -1) {
      LOGE("instance destroy JFM must stop callRemoveElement");
    }

    env->DeleteLocalRef(jRef);
    return 0;
  }

  int Bridge_Impl_Android::callMoveElement(int &pageId, const char *ref, const char *parentRef,
                                     int index) {
    JNIEnv *env = getJNIEnv();

    if (jCallMoveElementMethodId == NULL) {
      jCallMoveElementMethodId = env->GetMethodID(jBridgeClazz,
                                                  "callMoveElement",
                                                  "(ILjava/lang/String;Ljava/lang/String;I)I");
    }

    jstring jRef = env->NewStringUTF(ref);
    jstring jParentRef = env->NewStringUTF(parentRef);

    int flag = env->CallIntMethod(jThis, jCallMoveElementMethodId, pageId, jRef, jParentRef,
                                  index);
    if (flag == -1) {
      LOGE("instance destroy JFM must stop callRemoveElement");
    }

    env->DeleteLocalRef(jRef);
    env->DeleteLocalRef(jParentRef);
    return 0;
  }

  int Bridge_Impl_Android::callAddEvent(int &pageId, const char *ref, const char *event) {
    JNIEnv *env = getJNIEnv();
    if (jCallAddEventMethodId == NULL) {
      jCallAddEventMethodId = env->GetMethodID(jBridgeClazz,
                                               "callAddEvent",
                                               "(ILjava/lang/String;Ljava/lang/String;)I");
    }
    jstring jRef = env->NewStringUTF(ref);
    jstring jEventId = env->NewStringUTF(event);

    int flag = env->CallIntMethod(jThis, jCallAddEventMethodId, pageId, jRef, jEventId);
    if (flag == -1) {
      LOGE("instance destroy JFM must stop callAddEvent");
    }

    env->DeleteLocalRef(jRef);
    env->DeleteLocalRef(jEventId);
    return flag;
  }

  int Bridge_Impl_Android::callRemoveEvent(int &pageId, const char *ref, const char *event) {
    JNIEnv *env = getJNIEnv();
    if (jCallRemoveEventMethodId == NULL) {
      jCallRemoveEventMethodId = env->GetMethodID(jBridgeClazz,
                                                  "callRemoveEvent",
                                                  "(ILjava/lang/String;Ljava/lang/String;)I");
    }
    jstring jRef = env->NewStringUTF(ref);
    jstring jEventId = env->NewStringUTF(event);

    int flag = env->CallIntMethod(jThis, jCallRemoveEventMethodId, pageId, jRef, jEventId);
    if (flag == -1) {
      LOGE("instance destroy JFM must stop callRemoveElement");
    }

    env->DeleteLocalRef(jRef);
    env->DeleteLocalRef(jEventId);
    return flag;
  }

  int Bridge_Impl_Android::callUpdateStyle(int &pageId, const char *ref,
                                       std::vector<std::pair<std::string, std::string>> *style,
                                       std::vector<std::pair<std::string, std::string>> *margin,
                                       std::vector<std::pair<std::string, std::string>> *padding,
                                       std::vector<std::pair<std::string, std::string>> *border) {
    JNIEnv *env = getJNIEnv();

    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
    if (page == nullptr)
      return -1;

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

    if (jCallUpdateStyleMethodId == NULL)
      jCallUpdateStyleMethodId = env->GetMethodID(jBridgeClazz,
                                                            "callUpdateStyle",
                                                            "(ILjava/lang/String;Ljava/util/HashMap;Ljava/util/HashMap;Ljava/util/HashMap;Ljava/util/HashMap;)I");

    jstring jRef = env->NewStringUTF(ref);

    int flag = 0;
    flag = env->CallIntMethod(jThis, jCallUpdateStyleMethodId, pageId, jRef, jStyles,
                              jMargins, jPaddings, jBorders);

    page->CallBridgeTime(getCurrentTime() - startTimeCallBridge);

    if (flag == -1) {
      LOGE("instance destroy JFM must stop callUpdateStyle");
    }

    env->DeleteLocalRef(jRef);
    env->DeleteLocalRef(jStyles);
    env->DeleteLocalRef(jMargins);
    env->DeleteLocalRef(jPaddings);
    env->DeleteLocalRef(jBorders);
    return flag;
  }

  int Bridge_Impl_Android::callUpdateAttr(int &pageId, const char *ref,
                                          std::vector<std::pair<std::string, std::string>> *attrs) {
    JNIEnv *env = getJNIEnv();

    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
    if (page == nullptr)
      return -1;

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

    if (jCallUpdateAttrsMethodId == NULL) {
      jCallUpdateAttrsMethodId = env->GetMethodID(jBridgeClazz,
                                                            "callUpdateAttrs",
                                                            "(ILjava/lang/String;Ljava/util/HashMap;)I");
    }

    jstring jRef = env->NewStringUTF(ref);

    int flag = 0;
    flag = env->CallIntMethod(jThis, jCallUpdateAttrsMethodId, pageId, jRef, jAttrs);

    page->CallBridgeTime(getCurrentTime() - startTimeCallBridge);

    if (flag == -1) {
      LOGE("instance destroy JFM must stop callUpdateStyle");
    }

    env->DeleteLocalRef(jRef);
    env->DeleteLocalRef(jAttrs);
    return flag;
  }

  int Bridge_Impl_Android::callLayout(int &pageId, const char *ref, int top,
                                      int bottom, int left, int right, int height,
                                      int width) {
    JNIEnv *env = getJNIEnv();

    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
    if (page == nullptr)
      return -1;

    long long startTimeCallBridge = getCurrentTime();

    if (jCallLayoutMethodId == NULL)
      jCallLayoutMethodId = env->GetMethodID(jBridgeClazz,
                                                       "callLayout",
                                                       "(ILjava/lang/String;IIIIII)I");

    jstring jRef = env->NewStringUTF(ref);

    int flag = 0;
    flag = env->CallIntMethod(jThis, jCallLayoutMethodId, pageId,
                              jRef, top, bottom, left, right, height, width);

    page->CallBridgeTime(getCurrentTime() - startTimeCallBridge);

    if (flag == -1) {
      LOGE("instance destroy JFM must stop callLayout");
    }

    env->DeleteLocalRef(jRef);
    return flag;
  }

  int Bridge_Impl_Android::callCreateFinish(int &pageId) {
    JNIEnv *env = getJNIEnv();

    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
    if (page == nullptr)
      return -1;

    long long startTimeCallBridge = getCurrentTime();

    if (jCallCreateFinishMethodId == NULL)
      jCallCreateFinishMethodId = env->GetMethodID(jBridgeClazz,
                                                             "callCreateFinish",
                                                             "(I)I");

    int flag = env->CallIntMethod(jThis, jCallCreateFinishMethodId, pageId);

    page->CallBridgeTime(getCurrentTime() - startTimeCallBridge);

    if (flag == -1) {
      LOGE("instance destroy JFM must stop callCreateFinish");
    }

    return flag;
  }

  int Bridge_Impl_Android::callHasTransitionPros(int &pageId, const std::string &ref,
                                       std::vector<std::pair<std::string, std::string>> *style) {
    JNIEnv *env = getJNIEnv();

    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
    if (page == nullptr)
      return -1;

    long long startTime = getCurrentTime();

    if (jMapConstructorMethodId == NULL)
      jMapConstructorMethodId = env->GetMethodID(jMapClazz, "<init>", "()V");
    if (jMapPutMethodId == NULL)
      jMapPutMethodId = env->GetMethodID(jMapClazz, "put",
                                         "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

    jobject jStyles = env->NewObject(jMapClazz, jMapConstructorMethodId);

    if (style != nullptr) {
      cpyCVector2JMap(style, jStyles, env);
    }

    page->CreateJMapJNITime(getCurrentTime() - startTime);

    long long startTimeCallBridge = getCurrentTime();

    if (jCallHasTransitionProsMethodId == NULL)
        jCallHasTransitionProsMethodId = env->GetMethodID(jBridgeClazz,
                                                            "callHasTransitionPros",
                                                            "(ILjava/lang/String;Ljava/util/HashMap;)I");

    jstring jRef = env->NewStringUTF(ref.c_str());

    int flag = 0;
    flag = env->CallIntMethod(jThis, jCallHasTransitionProsMethodId, pageId, jRef, jStyles);
    page->CallBridgeTime(getCurrentTime() - startTimeCallBridge);

    env->DeleteLocalRef(jRef);
    env->DeleteLocalRef(jStyles);
    return flag;
  }

} //end WeexCore