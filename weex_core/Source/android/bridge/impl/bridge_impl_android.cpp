/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#include <core/render/manager/render_manager.h>
#include <core/render/page/render_page.h>
#include <core/render/node/render_object.h>
#include <base/TimeUtils.h>
#include <android/base/string/string_utils.h>
#include "bridge_impl_android.h"
#include <core/layout/style.h>
#include <map>
#include <android/base/string/jstring_cache.h>

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
static jmethodID jCallCreateBodyMethodId;
static jmethodID jCallAddElementMethodId;
static jmethodID jCallUpdateStyleMethodId;
static jmethodID jCallHasTransitionProsMethodId;
static jmethodID jCallUpdateAttrsMethodId;
static jmethodID jCallLayoutMethodId;
static jmethodID jCallCreateFinishMethodId;
static jmethodID jCallAppendTreeCreateFinishMethodId;
static jmethodID jCallGetMeasurementMethodId;

static jmethodID jPostMessage;
static jmethodID jDispatchMeaasge;

namespace WeexCore {

  Bridge_Impl_Android *Bridge_Impl_Android::m_instance = nullptr;

  Bridge_Impl_Android::Bridge_Impl_Android() {}

  Bridge_Impl_Android::~Bridge_Impl_Android() {}

  void Bridge_Impl_Android::setGlobalRef(jobject &jRef) {
    jThis = jRef;
    jSetJSFrmVersionMethodId = NULL;
    jReportExceptionMethodId = NULL;
    jCallNativeMethodId = NULL;
    jCallNativeModuleMethodId = NULL;
    jCallNativeComponentMethodId = NULL;

    jSetTimeoutNativeMethodId = NULL;
    jLogMethodId = NULL;
    jCallUpdateFinishMethodId = NULL;

    jCallRefreshFinishMethodId = NULL;
    jCallRemoveElementMethodId = NULL;
    jCallMoveElementMethodId = NULL;
    jCallAddEventMethodId = NULL;
    jCallRemoveEventMethodId = NULL;

    jCallCreateBodyMethodId = NULL;
    jCallAddElementMethodId = NULL;
    jCallUpdateStyleMethodId = NULL;
    jCallHasTransitionProsMethodId = NULL;
    jCallUpdateAttrsMethodId = NULL;
    jCallLayoutMethodId = NULL;
    jCallAppendTreeCreateFinishMethodId = NULL;
    jCallCreateFinishMethodId = NULL;

    jCallGetMeasurementMethodId = NULL;
  }

  void static cpyCMap2JMap(std::map<std::string, std::string> *cMap, jobject &jMap, JNIEnv *env) {

    std::map<std::string, std::string>::const_iterator it = cMap->begin();
    std::map<std::string, std::string>::const_iterator end = cMap->end();
    jstring jKey;
    jbyteArray jValue;

    for (; it != end; ++it) {
      jKey = getKeyFromCache(env, it->first.c_str());
      jValue = newJByteArray(env, it->second.c_str());
      env->CallObjectMethod(jMap, jMapPutMethodId, jKey, jValue);
      env->DeleteLocalRef(jValue);
    }
  }

  void static cpyCVector2JMap(std::vector<std::pair<std::string, std::string>> *cVector, jobject &jMap, JNIEnv *env) {

    jstring jKey;
    jbyteArray jValue;

    for (int i = 0; i < cVector->size(); ++i) {
      jKey = getKeyFromCache(env, (*cVector)[i].first.c_str());
      jValue = newJByteArray(env, (*cVector)[i].second.c_str());
      env->CallObjectMethod(jMap, jMapPutMethodId, jKey, jValue);
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

  void Bridge_Impl_Android::reportException(const char* pageId, const char *func, const char *exception_string) {

    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
    int64_t startTime = getCurrentTime();

    JNIEnv *env = getJNIEnv();
    jstring jFunc = env->NewStringUTF(func);
    jstring jExceptionString = env->NewStringUTF(exception_string);
    jstring jPageId = getKeyFromCache(env, pageId);

    if (jReportExceptionMethodId == NULL) {
      jReportExceptionMethodId = env->GetMethodID(jBridgeClazz,
                                                  "reportJSException",
                                                  "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
    }
    env->CallVoidMethod(jThis, jReportExceptionMethodId, jPageId, jFunc, jExceptionString);

    if (jFunc != nullptr)
      env->DeleteLocalRef(jFunc);
    if (jExceptionString != nullptr)
      env->DeleteLocalRef(jExceptionString);

    if (page != nullptr)
      page->CallBridgeTime(getCurrentTime() - startTime);
  }

  int Bridge_Impl_Android::callNative(const char* pageId, const char *task, const char *callback) {

    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
    int64_t startTime = getCurrentTime();

    JNIEnv *env = getJNIEnv();
    jbyteArray jTask = newJByteArray(env, task);
    jstring jCallback = env->NewStringUTF(callback);
    jstring jPageId = getKeyFromCache(env, pageId);

    int flag = -1;

    if (jTask != nullptr) {
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

    if (jTask != nullptr)
      env->DeleteLocalRef(jTask);
    if (jCallback != nullptr)
      env->DeleteLocalRef(jCallback);

    if (page != nullptr)
      page->CallBridgeTime(getCurrentTime() - startTime);
    return flag;
  }

  void* Bridge_Impl_Android::callNativeModule(const char* pageId, const char *module, const char *method,
                                                const char *arguments, int argumentsLen,  const char *options, int optionsLen) {
    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
    int64_t startTime = getCurrentTime();

    JNIEnv *env = getJNIEnv();
    jstring jModule = env->NewStringUTF(module);
    jstring jMethod = env->NewStringUTF(method);
    jbyteArray jArgString = newJByteArray(env, arguments, argumentsLen);
    jbyteArray jOptString = newJByteArray(env, options, optionsLen);
    jstring jPageId = getKeyFromCache(env, pageId);
    jobject result = nullptr;

    if (jModule != nullptr && jMethod != nullptr) {
      if (jCallNativeModuleMethodId == NULL) {
        jCallNativeModuleMethodId = env->GetMethodID(jBridgeClazz,
                                                     "callNativeModule",
                                                     "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[B[B)Ljava/lang/Object;");
      }

      result = env->CallObjectMethod(jThis, jCallNativeModuleMethodId, jPageId, jModule, jMethod, jArgString, jOptString);
    }

    if (jModule != nullptr)
      env->DeleteLocalRef(jModule);
    if (jMethod != nullptr)
      env->DeleteLocalRef(jMethod);
    if (jArgString != nullptr)
      env->DeleteLocalRef(jArgString);
    if (jOptString != nullptr)
      env->DeleteLocalRef(jOptString);

    if (page != nullptr)
      page->CallBridgeTime(getCurrentTime() - startTime);
    return result;
  }

  void Bridge_Impl_Android::callNativeComponent(const char* pageId, const char* ref, const char *method,
                                                const char *arguments, int argumentsLength, const char *options, int optionsLength) {
    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
    int64_t startTime = getCurrentTime();

    JNIEnv *env = getJNIEnv();
    jstring jMethod = env->NewStringUTF(method);
    jbyteArray jArgString = newJByteArray(env, arguments, argumentsLength);
    jbyteArray jOptString = newJByteArray(env, options, optionsLength);
    jstring jPageId = getKeyFromCache(env, pageId);
    jstring jRef = getKeyFromCache(env, ref);

    if (jMethod != nullptr) {
      if (jCallNativeComponentMethodId == NULL) {
        jCallNativeComponentMethodId = env->GetMethodID(jBridgeClazz,
                                                        "callNativeComponent",
                                                        "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[B[B)V");
      }
      env->CallVoidMethod(jThis, jCallNativeComponentMethodId, jPageId, jRef, jMethod, jArgString, jOptString);
    }

    if (jMethod != nullptr)
      env->DeleteLocalRef(jMethod);
    if (jArgString != nullptr)
      env->DeleteLocalRef(jArgString);
    if (jOptString != nullptr)
      env->DeleteLocalRef(jOptString);

    if (page != nullptr)
      page->CallBridgeTime(getCurrentTime() - startTime);
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

  int Bridge_Impl_Android::callUpdateFinish(const char* pageId, const char *task, const char *callback) {

    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
    int64_t startTime = getCurrentTime();

    JNIEnv *env = getJNIEnv();
    jbyteArray jTask = newJByteArray(env, task);
    jstring jCallback = env->NewStringUTF(callback);
    jstring jPageId = getKeyFromCache(env, pageId);

    if (jCallUpdateFinishMethodId == NULL) {
      jCallUpdateFinishMethodId = env->GetMethodID(jBridgeClazz,
                                                   "callUpdateFinish",
                                                   "(Ljava/lang/String;[BLjava/lang/String;)I");
    }

    int flag = env->CallIntMethod(jThis, jCallUpdateFinishMethodId, jPageId, jTask, jCallback);

    if (flag == -1) {
      LOGE("instance destroy JFM must stop callUpdateFinish");
    }

    if (jTask != nullptr)
      env->DeleteLocalRef(jTask);
    if (jCallback != nullptr)
      env->DeleteLocalRef(jCallback);

    if (page != nullptr)
      page->CallBridgeTime(getCurrentTime() - startTime);
    return flag;
  }

  int Bridge_Impl_Android::callRefreshFinish(const char* pageId, const char *task, const char *callback) {

    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
    int64_t startTime = getCurrentTime();

    JNIEnv *env = getJNIEnv();
    jbyteArray jTask = newJByteArray(env, task);
    jstring jCallback = env->NewStringUTF(callback);
    jstring jPageId = getKeyFromCache(env, pageId);

    if (jCallRefreshFinishMethodId == NULL) {
      jCallRefreshFinishMethodId = env->GetMethodID(jBridgeClazz,
                                                    "callRefreshFinish",
                                                    "(Ljava/lang/String;[BLjava/lang/String;)I");
    }
    int flag = env->CallIntMethod(jThis, jCallRefreshFinishMethodId, jPageId, jTask, jCallback);

    if (flag == -1) {
      LOGE("instance destroy JFM must stop callNative");
    }

    if (jTask != nullptr)
      env->DeleteLocalRef(jTask);
    if (jCallback != nullptr)
      env->DeleteLocalRef(jCallback);

    if (page != nullptr)
      page->CallBridgeTime(getCurrentTime() - startTime);
    return flag;
  }

  int Bridge_Impl_Android::callCreateBody(const char* pageId, const char *componentType, const char* ref,
                                          std::map<std::string, std::string> *styles,
                                          std::map<std::string, std::string> *attributes,
                                          std::set<std::string> *events,
                                          const WXCoreMargin &margins,
                                          const WXCorePadding &paddings,
                                          const WXCoreBorderWidth &borders) {
    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
    int64_t startTime = getCurrentTime();

    JNIEnv *env = getJNIEnv();

    jstring jPageId = getKeyFromCache(env, pageId);
    jstring jRef = getKeyFromCache(env, ref);

    jobject jStyles = env->NewObject(jMapClazz, jMapConstructorMethodId);
    jobject jAttributes = env->NewObject(jMapClazz, jMapConstructorMethodId);
    jobject jEvents = env->NewObject(jSetClazz, jSetConstructorMethodId);

    cpyCMap2JMap(styles, jStyles, env);
    cpyCMap2JMap(attributes, jAttributes, env);
    cpyCSet2JSet(events, jEvents, env);

    float c_margins[4];
    float c_paddings[4];
    float c_borders[4];

    c_margins[0] = margins.getMargin(kMarginTop);
    c_margins[1] = margins.getMargin(kMarginBottom);
    c_margins[2] = margins.getMargin(kMarginLeft);
    c_margins[3] = margins.getMargin(kMarginRight);

    c_paddings[0] = paddings.getPadding(kPaddingTop);
    c_paddings[1] = paddings.getPadding(kPaddingBottom);
    c_paddings[2] = paddings.getPadding(kPaddingLeft);
    c_paddings[3] = paddings.getPadding(kPaddingRight);

    c_borders[0] = borders.getBorderWidth(kBorderWidthTop);
    c_borders[1] = borders.getBorderWidth(kBorderWidthBottom);
    c_borders[2] = borders.getBorderWidth(kBorderWidthLeft);
    c_borders[3] = borders.getBorderWidth(kBorderWidthRight);

    jfloatArray jMargins = c2jFloatArray(env, c_margins);
    jfloatArray jPaddings = c2jFloatArray(env, c_paddings);
    jfloatArray jBorders = c2jFloatArray(env, c_borders);

    if (jCallCreateBodyMethodId == NULL)
      jCallCreateBodyMethodId = env->GetMethodID(jBridgeClazz,
                                                 "callCreateBody",
                                                 "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/util/HashMap;Ljava/util/HashMap;Ljava/util/HashSet;[F[F[F)I");


    jstring jComponentType = getComponentTypeFromCache(componentType);
    if (jComponentType == nullptr) {
      jComponentType = putComponentTypeToCache(componentType);
    }

    int flag = 0;
    flag = env->CallIntMethod(jThis, jCallCreateBodyMethodId, jPageId,
                              jComponentType, jRef, jStyles, jAttributes,
                              jEvents, jMargins, jPaddings, jBorders);

    if (flag == -1) {
      LOGE("instance destroy JFM must stop callCreateBody");
    }

    env->DeleteLocalRef(jStyles);
    env->DeleteLocalRef(jAttributes);
    env->DeleteLocalRef(jEvents);
    env->DeleteLocalRef(jMargins);
    env->DeleteLocalRef(jPaddings);
    env->DeleteLocalRef(jBorders);

    if (page != nullptr)
      page->CallBridgeTime(getCurrentTime() - startTime);
    return flag;
  }

  int Bridge_Impl_Android::callAddElement(const char* pageId, const char *componentType,
                                          const char* ref, int &index, const char* parentRef,
                                          std::map<std::string, std::string> *styles,
                                          std::map<std::string, std::string> *attributes,
                                          std::set<std::string> *events,
                                          const WXCoreMargin &margins,
                                          const WXCorePadding &paddings,
                                          const WXCoreBorderWidth &borders,
                                          bool willLayout) {
    JNIEnv *env = getJNIEnv();
    jstring jPageId = getKeyFromCache(env, pageId);
    jstring jRef = getKeyFromCache(env, ref);
    jstring jParentRef = getKeyFromCache(env, parentRef);

    jobject jStyles = env->NewObject(jMapClazz, jMapConstructorMethodId);
    jobject jAttributes = env->NewObject(jMapClazz, jMapConstructorMethodId);
    jobject jEvents = env->NewObject(jSetClazz, jSetConstructorMethodId);

    cpyCMap2JMap(styles, jStyles, env);
    cpyCMap2JMap(attributes, jAttributes, env);
    cpyCSet2JSet(events, jEvents, env);

    float c_margins[4];
    float c_paddings[4];
    float c_borders[4];

    c_margins[0] = margins.getMargin(kMarginTop);
    c_margins[1] = margins.getMargin(kMarginBottom);
    c_margins[2] = margins.getMargin(kMarginLeft);
    c_margins[3] = margins.getMargin(kMarginRight);

    c_paddings[0] = paddings.getPadding(kPaddingTop);
    c_paddings[1] = paddings.getPadding(kPaddingBottom);
    c_paddings[2] = paddings.getPadding(kPaddingLeft);
    c_paddings[3] = paddings.getPadding(kPaddingRight);

    c_borders[0] = borders.getBorderWidth(kBorderWidthTop);
    c_borders[1] = borders.getBorderWidth(kBorderWidthBottom);
    c_borders[2] = borders.getBorderWidth(kBorderWidthLeft);
    c_borders[3] = borders.getBorderWidth(kBorderWidthRight);

    jfloatArray jMargins = c2jFloatArray(env, c_margins);
    jfloatArray jPaddings = c2jFloatArray(env, c_paddings);
    jfloatArray jBorders = c2jFloatArray(env, c_borders);

    if (jCallAddElementMethodId == NULL)
      jCallAddElementMethodId = env->GetMethodID(jBridgeClazz,
                                                           "callAddElement",
                                                           "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ILjava/lang/String;Ljava/util/HashMap;Ljava/util/HashMap;Ljava/util/HashSet;[F[F[FZ)I");

    jstring jComponentType = getComponentTypeFromCache(componentType);
    if (jComponentType == nullptr) {
      jComponentType = putComponentTypeToCache(componentType);
    }

    int flag = 0;
    flag = env->CallIntMethod(jThis, jCallAddElementMethodId, jPageId, jComponentType, jRef, index,
                              jParentRef, jStyles, jAttributes, jEvents, jMargins, jPaddings, jBorders, willLayout);

    if (flag == -1) {
      LOGE("instance destroy JFM must stop callAddElement");
    }

    env->DeleteLocalRef(jStyles);
    env->DeleteLocalRef(jAttributes);
    env->DeleteLocalRef(jEvents);
    env->DeleteLocalRef(jMargins);
    env->DeleteLocalRef(jPaddings);
    env->DeleteLocalRef(jBorders);

    return flag;
  }

  int Bridge_Impl_Android::callRemoveElement(const char* pageId, const char* ref) {

    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
    int64_t startTime = getCurrentTime();

    JNIEnv *env = getJNIEnv();

    jstring jPageId = getKeyFromCache(env, pageId);
    jstring jRef = getKeyFromCache(env, ref);

    if (jCallRemoveElementMethodId == NULL) {
      jCallRemoveElementMethodId = env->GetMethodID(jBridgeClazz,
                                                    "callRemoveElement",
                                                    "(Ljava/lang/String;Ljava/lang/String;)I");
    }

    int flag = env->CallIntMethod(jThis, jCallRemoveElementMethodId, jPageId, jRef);
    if (flag == -1) {
      LOGE("instance destroy JFM must stop callRemoveElement");
    }

    if (page != nullptr)
      page->CallBridgeTime(getCurrentTime() - startTime);
    return 0;
  }

  int Bridge_Impl_Android::callMoveElement(const char* pageId, const char* ref, const char* parentRef, int index) {

    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
    int64_t startTime = getCurrentTime();

    JNIEnv *env = getJNIEnv();

    jstring jPageId = getKeyFromCache(env, pageId);
    jstring jRef = getKeyFromCache(env, ref);
    jstring jParentRef = getKeyFromCache(env, parentRef);

    if (jCallMoveElementMethodId == NULL) {
      jCallMoveElementMethodId = env->GetMethodID(jBridgeClazz,
                                                  "callMoveElement",
                                                  "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)I");
    }

    int flag = env->CallIntMethod(jThis, jCallMoveElementMethodId, jPageId, jRef, jParentRef, index);
    if (flag == -1) {
      LOGE("instance destroy JFM must stop callRemoveElement");
    }

    if (page != nullptr)
      page->CallBridgeTime(getCurrentTime() - startTime);
    return 0;
  }

  int Bridge_Impl_Android::callAddEvent(const char* pageId, const char* ref, const char *event) {

    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
    int64_t startTime = getCurrentTime();

    JNIEnv *env = getJNIEnv();
    jstring jPageId = getKeyFromCache(env, pageId);
    jstring jRef = getKeyFromCache(env, ref);

    if (jCallAddEventMethodId == NULL) {
      jCallAddEventMethodId = env->GetMethodID(jBridgeClazz,
                                               "callAddEvent",
                                               "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I");
    }
    jstring jEventId = env->NewStringUTF(event);

    int flag = env->CallIntMethod(jThis, jCallAddEventMethodId, jPageId, jRef, jEventId);
    if (flag == -1) {
      LOGE("instance destroy JFM must stop callAddEvent");
    }

    if (page != nullptr)
      page->CallBridgeTime(getCurrentTime() - startTime);
    env->DeleteLocalRef(jEventId);
    return flag;
  }

  int Bridge_Impl_Android::callRemoveEvent(const char* pageId, const char* ref, const char *event) {

    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
    int64_t startTime = getCurrentTime();

    JNIEnv *env = getJNIEnv();

    jstring jPageId = getKeyFromCache(env, pageId);
    jstring jRef = getKeyFromCache(env, ref);

    if (jCallRemoveEventMethodId == NULL) {
      jCallRemoveEventMethodId = env->GetMethodID(jBridgeClazz,
                                                  "callRemoveEvent",
                                                  "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I");
    }
    jstring jEventId = env->NewStringUTF(event);

    int flag = env->CallIntMethod(jThis, jCallRemoveEventMethodId, jPageId, jRef, jEventId);
    if (flag == -1) {
      LOGE("instance destroy JFM must stop callRemoveElement");
    }

    if (page != nullptr)
      page->CallBridgeTime(getCurrentTime() - startTime);
    env->DeleteLocalRef(jEventId);
    return flag;
  }

  int Bridge_Impl_Android::callUpdateStyle(const char* pageId, const char* ref,
                                           std::vector<std::pair<std::string, std::string>> *style,
                                           std::vector<std::pair<std::string, std::string>> *margin,
                                           std::vector<std::pair<std::string, std::string>> *padding,
                                           std::vector<std::pair<std::string, std::string>> *border) {
    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
    int64_t startTime = getCurrentTime();

    JNIEnv *env = getJNIEnv();

    jstring jPageId = getKeyFromCache(env, pageId);
    jstring jRef = getKeyFromCache(env, ref);

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

    if (jCallUpdateStyleMethodId == NULL)
      jCallUpdateStyleMethodId = env->GetMethodID(jBridgeClazz,
                                                            "callUpdateStyle",
                                                            "(Ljava/lang/String;Ljava/lang/String;Ljava/util/HashMap;Ljava/util/HashMap;Ljava/util/HashMap;Ljava/util/HashMap;)I");

    int flag = 0;
    flag = env->CallIntMethod(jThis, jCallUpdateStyleMethodId, jPageId, jRef, jStyles, jMargins, jPaddings, jBorders);

    if (flag == -1) {
      LOGE("instance destroy JFM must stop callUpdateStyle");
    }

    env->DeleteLocalRef(jStyles);
    env->DeleteLocalRef(jMargins);
    env->DeleteLocalRef(jPaddings);
    env->DeleteLocalRef(jBorders);

    if (page != nullptr)
      page->CallBridgeTime(getCurrentTime() - startTime);
    return flag;
  }

  int Bridge_Impl_Android::callUpdateAttr(const char* pageId, const char* ref, std::vector<std::pair<std::string, std::string>> *attrs) {

    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
    int64_t startTime = getCurrentTime();

    JNIEnv *env = getJNIEnv();

    jstring jPageId = getKeyFromCache(env, pageId);
    jstring jRef = getKeyFromCache(env, ref);

    jobject jAttrs = env->NewObject(jMapClazz, jMapConstructorMethodId);

    if (attrs != nullptr) {
      cpyCVector2JMap(attrs, jAttrs, env);
    }

    if (jCallUpdateAttrsMethodId == NULL) {
      jCallUpdateAttrsMethodId = env->GetMethodID(jBridgeClazz,
                                                            "callUpdateAttrs",
                                                            "(Ljava/lang/String;Ljava/lang/String;Ljava/util/HashMap;)I");
    }

    int flag = 0;
    flag = env->CallIntMethod(jThis, jCallUpdateAttrsMethodId, jPageId, jRef, jAttrs);

    if (flag == -1) {
      LOGE("instance destroy JFM must stop callUpdateStyle");
    }

    env->DeleteLocalRef(jAttrs);

    if (page != nullptr)
      page->CallBridgeTime(getCurrentTime() - startTime);
    return flag;
  }

  int Bridge_Impl_Android::callLayout(const char* pageId, const char* ref,
                                      int top, int bottom, int left, int right,
                                      int height, int width, int index) {

    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
    int64_t startTime = getCurrentTime();

    JNIEnv *env = getJNIEnv();

    jstring jPageId = getKeyFromCache(env, pageId);
    jstring jRef = getKeyFromCache(env, ref);

    if (jCallLayoutMethodId == NULL)
      jCallLayoutMethodId = env->GetMethodID(jBridgeClazz,
                                                       "callLayout",
                                                       "(Ljava/lang/String;Ljava/lang/String;IIIIIII)I");

    int flag = 0;
    flag = env->CallIntMethod(jThis, jCallLayoutMethodId, jPageId,
                              jRef, top, bottom, left, right, height, width, index);

    if (flag == -1) {
      LOGE("instance destroy JFM must stop callLayout");
    }

    if (page != nullptr)
      page->CallBridgeTime(getCurrentTime() - startTime);
    return flag;
  }

  int Bridge_Impl_Android::callCreateFinish(const char* pageId) {

    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
    int64_t startTime = getCurrentTime();

    JNIEnv *env = getJNIEnv();

    jstring jPageId = getKeyFromCache(env, pageId);

    if (jCallCreateFinishMethodId == NULL)
      jCallCreateFinishMethodId = env->GetMethodID(jBridgeClazz,
                                                             "callCreateFinish",
                                                             "(Ljava/lang/String;)I");

    int flag = env->CallIntMethod(jThis, jCallCreateFinishMethodId, jPageId);

    if (flag == -1) {
      LOGE("instance destroy JFM must stop callCreateFinish");
    }

    if (page != nullptr)
      page->CallBridgeTime(getCurrentTime() - startTime);
    return flag;
  }

  int Bridge_Impl_Android::callAppendTreeCreateFinish(const char *pageId, const char *ref) {

    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
    int64_t startTime = getCurrentTime();

    JNIEnv *env = getJNIEnv();

    jstring jPageId = getKeyFromCache(env, pageId);
    jstring jRef = getKeyFromCache(env, ref);

    if (jCallAppendTreeCreateFinishMethodId == NULL)
      jCallAppendTreeCreateFinishMethodId = env->GetMethodID(jBridgeClazz,
                                             "callAppendTreeCreateFinish",
                                             "(Ljava/lang/String;Ljava/lang/String;)I");

    int flag = 0;
    flag = env->CallIntMethod(jThis, jCallAppendTreeCreateFinishMethodId, jPageId, jRef);

    if (flag == -1) {
      LOGE("instance destroy JFM must stop callAppendTreeCreateFinish");
    }

    if (page != nullptr)
      page->CallBridgeTime(getCurrentTime() - startTime);
    return flag;
  }

  int Bridge_Impl_Android::callHasTransitionPros(const char* pageId, const char* ref, std::vector<std::pair<std::string, std::string>> *style) {

    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
    int64_t startTime = getCurrentTime();

    JNIEnv *env = getJNIEnv();

    jstring jPageId = getKeyFromCache(env, pageId);
    jstring jRef = getKeyFromCache(env, ref);

    jobject jStyles = env->NewObject(jMapClazz, jMapConstructorMethodId);

    if (style != nullptr) {
      cpyCVector2JMap(style, jStyles, env);
    }

    if (jCallHasTransitionProsMethodId == NULL)
        jCallHasTransitionProsMethodId = env->GetMethodID(jBridgeClazz,
                                                            "callHasTransitionPros",
                                                            "(Ljava/lang/String;Ljava/lang/String;Ljava/util/HashMap;)I");

    int flag = 0;
    flag = env->CallIntMethod(jThis, jCallHasTransitionProsMethodId, jPageId, jRef, jStyles);

    env->DeleteLocalRef(jStyles);

    if (page != nullptr)
      page->CallBridgeTime(getCurrentTime() - startTime);
    return flag;
  }


  void Bridge_Impl_Android::handlePostMessage(jstring jVmId, jbyteArray jData) {
    JNIEnv *env = getJNIEnv();
    if (jPostMessage == NULL) {
      jPostMessage = env->GetMethodID(jWMBridgeClazz,
                                    "postMessage",
                                    "(Ljava/lang/String;[B)V");
    }
    env->CallVoidMethod(jWMThis, jPostMessage, jVmId, jData);
  }

  void Bridge_Impl_Android::handleDispatchMessage(jstring jClientId, jstring jVmId, jbyteArray jData, jstring jCallback) {
    JNIEnv *env = getJNIEnv();
    if (jDispatchMeaasge == NULL) {
      jDispatchMeaasge = env->GetMethodID(jWMBridgeClazz,
                                        "dispatchMessage",
                                        "(Ljava/lang/String;Ljava/lang/String;[BLjava/lang/String;)V");
    }
    env->CallVoidMethod(jWMThis, jDispatchMeaasge, jClientId, jVmId, jData, jCallback);
  }

  jobject Bridge_Impl_Android::getMeasureFunc(const char* pageId, jlong renderObjectPtr) {
    JNIEnv *env = getJNIEnv();
    jstring jPageId = getKeyFromCache(env, pageId);
    if (jCallGetMeasurementMethodId == NULL) {
      jCallGetMeasurementMethodId = env->GetMethodID(jBridgeClazz,
                                                     "getMeasurementFunc",
                                                     "(Ljava/lang/String;J)Lcom/taobao/weex/layout/ContentBoxMeasurement;");
    }
    return env->CallObjectMethod(jThis, jCallGetMeasurementMethodId, jPageId, renderObjectPtr);
  }
} //end WeexCore
