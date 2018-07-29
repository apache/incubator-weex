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
#include <android/base/jni/scoped_java_ref.h>
#include <core/render/node/factory/render_creator.h>
#include <core/render/node/factory/render_type.h>
#include <core/render/node/render_list.h>
#include <core/config/core_environment.h>
#include <android/base/jni/android_jni.h>
#include "android/utils/IPCStringResult.h"
#include "IPC/IPCResult.h"
#include "core/layout/measure_func_adapter.h"

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

    PlatformBridge *Bridge_Impl_Android::m_instance = nullptr;

  Bridge_Impl_Android::Bridge_Impl_Android() {}

  Bridge_Impl_Android::~Bridge_Impl_Android() {}

//  void Bridge_Impl_Android::setGlobalRef(jobject &jRef) {
//    jThis = jRef;
//    jSetJSFrmVersionMethodId = NULL;
//    jReportExceptionMethodId = NULL;
//    jCallNativeMethodId = NULL;
//    jCallNativeModuleMethodId = NULL;
//    jCallNativeComponentMethodId = NULL;
//
//    jSetTimeoutNativeMethodId = NULL;
//    jLogMethodId = NULL;
//    jCallUpdateFinishMethodId = NULL;
//
//    jCallRefreshFinishMethodId = NULL;
//    jCallRemoveElementMethodId = NULL;
//    jCallMoveElementMethodId = NULL;
//    jCallAddEventMethodId = NULL;
//    jCallRemoveEventMethodId = NULL;
//
//    jCallCreateBodyMethodId = NULL;
//    jCallAddElementMethodId = NULL;
//    jCallUpdateStyleMethodId = NULL;
//    jCallHasTransitionProsMethodId = NULL;
//    jCallUpdateAttrsMethodId = NULL;
//    jCallLayoutMethodId = NULL;
//    jCallAppendTreeCreateFinishMethodId = NULL;
//    jCallCreateFinishMethodId = NULL;
//
//    jCallGetMeasurementMethodId = NULL;
//  }
//
//  void static cpyCMap2JMap(std::map<std::string, std::string> *cMap, jobject &jMap, JNIEnv *env) {
//    std::map<std::string, std::string>::const_iterator it = cMap->begin();
//    std::map<std::string, std::string>::const_iterator end = cMap->end();
//    jstring jKey;
//    jbyteArray jValue;
//
//    for (; it != end; ++it) {
//      jKey = getKeyFromCache(env, it->first.c_str());
//      jValue = newJByteArray(env, it->second.c_str());
//      env->CallObjectMethod(jMap, jMapPutMethodId, jKey, jValue);
//      env->DeleteLocalRef(jValue);
//    }
//  }
//
//  void static cpyCVector2JMap(std::vector<std::pair<std::string, std::string>> *cVector, jobject &jMap, JNIEnv *env) {
//
//    jstring jKey;
//    jbyteArray jValue;
//
//    for (int i = 0; i < cVector->size(); ++i) {
//      jKey = getKeyFromCache(env, (*cVector)[i].first.c_str());
//      jValue = newJByteArray(env, (*cVector)[i].second.c_str());
//      env->CallObjectMethod(jMap, jMapPutMethodId, jKey, jValue);
//      env->DeleteLocalRef(jValue);
//    }
//  }
//
//  void static cpyCSet2JSet(std::set<std::string> *cSet, jobject &jSet, JNIEnv *env) {
//
//    jstring jValue;
//    std::set<std::string>::const_iterator it = cSet->begin();
//    std::set<std::string>::const_iterator end = cSet->end();
//
//    for (; it != end; ++it) {
//      jValue = env->NewStringUTF((*it).c_str());
//      env->CallBooleanMethod(jSet, jSetAddMethodId, jValue);
//      env->DeleteLocalRef(jValue);
//    }
//  }
//
//    void Bridge_Impl_Android::SetDefaultHeightAndWidthIntoRootDom(const std::string &instance_id,
//                                                                  float default_width,
//                                                                  float default_height,
//                                                                  bool is_width_wrap_content,
//                                                                  bool is_height_wrap_content) {
//        RenderPage *page = RenderManager::GetInstance()->GetPage(instance_id);
//        if (page == nullptr)
//            return;
//
//#if RENDER_LOG
//        LOGD("[JNI] SetDefaultHeightAndWidthIntoRootDom >>>> pageId: %s, defaultWidth: %f, defaultHeight: %f",
//       page->PageId().c_str(), defaultWidth,defaultHeight);
//#endif
//
//        page->SetDefaultHeightAndWidthIntoRootRender(default_width, default_height,
//                                                     is_width_wrap_content, is_height_wrap_content);
//    }
//
//    void Bridge_Impl_Android::OnInstanceClose(const std::string &instance_id) {
//        RenderManager::GetInstance()->ClosePage(instance_id);
//    }
//
//    void Bridge_Impl_Android::SetStyleWidth(const std::string &instance_id, const std::string& render_ref, float width) {
//        RenderPage *page = RenderManager::GetInstance()->GetPage(instance_id);
//        if (page == nullptr)
//            return;
//
//        RenderObject *render = page->GetRenderObject(render_ref);
//        if (render == nullptr)
//            return;
//
//        render->setStyleWidthLevel(CSS_STYLE);
//        render->setStyleWidth(width, true);
//        page->set_is_dirty(true);
//    }
//
//    void Bridge_Impl_Android::SetStyleHeight(const std::string &instance_id, const std::string& render_ref, float height) {
//        RenderPage *page = RenderManager::GetInstance()->GetPage(instance_id);
//          if (page == nullptr)
//            return;
//
//          RenderObject *render = page->GetRenderObject(render_ref);
//          if (render == nullptr)
//            return;
//
//          render->setStyleHeightLevel(CSS_STYLE);
//          render->setStyleHeight(height);
//          page->set_is_dirty(true);
//    }
//
//    void Bridge_Impl_Android::SetMargin(const std::string &instance_id, const std::string& render_ref, int edge, float value) {
//        RenderPage *page = RenderManager::GetInstance()->GetPage(instance_id);
//        if (page == nullptr)
//            return;
//
//        RenderObject *render = page->GetRenderObject(render_ref);
//        if (render == nullptr)
//            return;
//
//        if (edge == 0) {
//            render->setMargin(kMarginTop, value);
//        } else if (edge == 1) {
//            render->setMargin(kMarginBottom, value);
//        } else if (edge == 2) {
//            render->setMargin(kMarginLeft, value);
//        } else if (edge == 3) {
//            render->setMargin(kMarginRight, value);
//        } else if (edge == 4) {
//            render->setMargin(kMarginALL, value);
//        }
//        page->set_is_dirty(true);
//    }
//
//    void Bridge_Impl_Android::SetPadding(const std::string &instance_id, const std::string& render_ref, int edge, float value) {
//        RenderPage *page = RenderManager::GetInstance()->GetPage(instance_id);
//        if (page == nullptr)
//            return;
//
//        RenderObject *render = page->GetRenderObject(render_ref);
//        if (render == nullptr)
//            return;
//
//        if (edge == 0) {
//            render->setPadding(kPaddingTop, value);
//        } else if (edge == 1) {
//            render->setPadding(kPaddingBottom, value);
//        } else if (edge == 2) {
//            render->setPadding(kPaddingLeft, value);
//        } else if (edge == 3) {
//            render->setPadding(kPaddingRight, value);
//        } else if (edge == 4) {
//            render->setPadding(kPaddingALL, value);
//        }
//        page->set_is_dirty(true);
//    }
//
//    void Bridge_Impl_Android::SetPosition(const std::string &instance_id, const std::string& render_ref, int edge, float value) {
//        RenderPage *page = RenderManager::GetInstance()->GetPage(instance_id);
//        if (page == nullptr)
//            return;
//
//        RenderObject *render = page->GetRenderObject(render_ref);
//        if (render == nullptr)
//            return;
//
//        if (edge == 0) {
//            render->setStylePosition(kPositionEdgeTop, value);
//        } else if (edge == 1) {
//            render->setStylePosition(kPositionEdgeBottom, value);
//        } else if (edge == 2) {
//            render->setStylePosition(kPositionEdgeLeft, value);
//        } else if (edge == 3) {
//            render->setStylePosition(kPositionEdgeRight, value);
//        }
//        page->set_is_dirty(true);
//    }
//
//    void Bridge_Impl_Android::MarkDirty(const std::string &instance_id, const std::string& render_ref) {
//
//        RenderPage *page = RenderManager::GetInstance()->GetPage(instance_id);
//        if (page == nullptr)
//            return;
//
//        RenderObject *render = page->GetRenderObject(render_ref);
//        if (render == nullptr)
//            return;
//        render->markDirty();
//    }
//
//    void Bridge_Impl_Android::SetViewPortWidth(const std::string &instance_id, float width) {
//
//        RenderPage *page = RenderManager::GetInstance()->GetPage(instance_id);
//        if (page == nullptr)
//            return;
//
//        page->set_viewport_width(width);
//    }
//
//    void Bridge_Impl_Android::ForceLayout(const std::string &instance_id) {
//        RenderPage *page = RenderManager::GetInstance()->GetPage(instance_id);
//        if (page != nullptr) {
//
//#if RENDER_LOG
//            LOGD("[JNI] ForceLayout >>>> pageId: %s, needForceLayout: %s", jString2StrFast(env, instanceId).c_str(), page->hasForeLayoutAction.load()?"true":"false");
//#endif
//
//            page->LayoutImmediately();
//            page->has_fore_layout_action_.store(false);
//        }
//    }
//
//    bool Bridge_Impl_Android::NotifyLayout(const std::string &instance_id) {
//        RenderPage *page = RenderManager::GetInstance()->GetPage(instance_id);
//        if (page != nullptr) {
//
//            if (!page->need_layout_.load()) {
//                page->need_layout_.store(true);
//            }
//
//            bool ret = !page->has_fore_layout_action_.load() && page->is_dirty();
//            if (ret) {
//                page->has_fore_layout_action_.store(true);
//            }
//            return ret ? true : false;
//        }
//        return false;
//    }
//
//    std::vector<int64_t > Bridge_Impl_Android::GetFirstScreenRenderTime(const std::string &instance_id) {
//        RenderPage *page = RenderManager::GetInstance()->GetPage(instance_id);
//        if (page == nullptr) {
//            return std::vector<int64_t>();
//        } else {
//            return page->PrintFirstScreenLog();
//        }
//    }
//
//    std::vector<int64_t> Bridge_Impl_Android::GetRenderFinishTime(const std::string &instance_id) {
//        RenderPage *page = RenderManager::GetInstance()->GetPage(instance_id);
//        if (page == nullptr) {
//            return std::vector<int64_t>();
//        } else {
//            return page->PrintRenderSuccessLog();
//        }
//    }
//
//    void Bridge_Impl_Android::SetRenderContainerWrapContent(const std::string &instance_id, bool wrap) {
//        RenderPage *page = RenderManager::GetInstance()->GetPage(instance_id);
//        if (page == nullptr)
//            return;
//
//        page->set_is_render_container_width_wrap_content(wrap);
//    }
//
//    void Bridge_Impl_Android::BindMeasurementToRenderObject(long ptr) {
//        RenderObject *render =  convert_long_to_render_object(ptr);
//        if(render && measure_function_adapter() != NULL){
//            render->BindMeasureFunc();
//        }
//    }
//
//    void Bridge_Impl_Android::RegisterCoreEnv(const std::string& key, const std::string& value) {
//        WXCoreEnvironment::getInstance()->AddOption(key, value);
//    }
//
//    long Bridge_Impl_Android::GetRenderObject(const std::string &instance_id, const std::string& render_ref) {
//        RenderPage *page = RenderManager::GetInstance()->GetPage(instance_id);
//        if (page == nullptr){
//            return 0;
//        }
//
//        RenderObject *render = page->GetRenderObject(render_ref);
//        if (render == nullptr){
//            return 0;
//        }
//        return convert_render_object_to_long(render);
//    }
//
//    void Bridge_Impl_Android::UpdateRenderObjectStyle(long render_ptr, const std::string& key, const std::string& value) {
//        RenderObject *render  = convert_long_to_render_object(render_ptr);
//        render->UpdateStyle(key, value);
//    }
//
//    void Bridge_Impl_Android::UpdateRenderObjectAttr(long render_ptr, const std::string& key, const std::string& value) {
//        RenderObject *render  = convert_long_to_render_object(render_ptr);
//        render->UpdateAttr(key, value);
//        render->markDirty(true);
//    }
//
//    long Bridge_Impl_Android::CopyRenderObject(long render_ptr) {
//        RenderObject *render = convert_long_to_render_object(render_ptr);
//        RenderObject *copy = (RenderObject*)RenderCreator::GetInstance()->CreateRender(render->type(), render->ref());
//        copy->copyFrom(render);
//        if(render->type() == WeexCore::kRenderCellSlot || render->type() == WeexCore::kRenderCell){
//            RenderList* renderList = static_cast<RenderList*>(render->getParent());
//            if(renderList != nullptr){
//                renderList->AddCellSlotCopyTrack(copy);
//            }else{
//                __android_log_print(ANDROID_LOG_ERROR, " LayoutRenderObject","copy error parent null");
//            }
//        }
//        return  convert_render_object_to_long(copy);
//    }
//
//    WXCoreSize Bridge_Impl_Android::InvokeMeasureFunction(const char* page_id, long render_ptr, float width, int width_measure_mode, float height, int height_measure_mode) {
//        return measure_function_adapter()->Measure(page_id, render_ptr, width,
//                                            static_cast<MeasureMode>(width_measure_mode), height,
//                                            static_cast<MeasureMode>(height_measure_mode));
//    }
//
//    void Bridge_Impl_Android::InvokeLayoutBefore(const char* page_id, long render_ptr) {
//        measure_function_adapter()->LayoutBefore(page_id, render_ptr);
//    }
//
//    void Bridge_Impl_Android::InvokeLayoutAfter(const char* page_id, long render_ptr, float width, float height) {
//        measure_function_adapter()->LayoutAfter(page_id, render_ptr, width, height);
//    }
//
//  void Bridge_Impl_Android::setJSVersion(const char* version) {
//
//    JNIEnv *env = base::android::AttachCurrentThread();
//    jstring jVersion = env->NewStringUTF(version);
//    if (jSetJSFrmVersionMethodId == NULL) {
//      jSetJSFrmVersionMethodId = env->GetMethodID(jBridgeClazz,
//                                                  "setJSFrmVersion",
//                                                  "(Ljava/lang/String;)V");
//    }
//    env->CallVoidMethod(jThis, jSetJSFrmVersionMethodId, jVersion);
//
//    if (jVersion != nullptr)
//      env->DeleteLocalRef(jVersion);
//  }
//
//  void Bridge_Impl_Android::reportException(const char* pageId, const char *func, const char *exception_string) {
//
////    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
////    int64_t startTime = getCurrentTime();
//
//    JNIEnv *env = getJNIEnv();
//    jstring jFunc = env->NewStringUTF(func);
//    jstring jExceptionString = env->NewStringUTF(exception_string);
//    jstring jPageId = getKeyFromCache(env, pageId);
//
//    if (jReportExceptionMethodId == NULL) {
//      jReportExceptionMethodId = env->GetMethodID(jBridgeClazz,
//                                                  "reportJSException",
//                                                  "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
//    }
//    env->CallVoidMethod(jThis, jReportExceptionMethodId, jPageId, jFunc, jExceptionString);
//
//    if (jFunc != nullptr)
//      env->DeleteLocalRef(jFunc);
//    if (jExceptionString != nullptr)
//      env->DeleteLocalRef(jExceptionString);
//
////    if (page != nullptr)
////      page->CallBridgeTime(getCurrentTime() - startTime);
//  }
//
//  int Bridge_Impl_Android::callNative(const char* pageId, const char *task, const char *callback) {
//
////    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
////    int64_t startTime = getCurrentTime();
//
//    JNIEnv *env = getJNIEnv();
//    jbyteArray jTask = newJByteArray(env, task);
//    jstring jCallback = env->NewStringUTF(callback);
//    jstring jPageId = getKeyFromCache(env, pageId);
//
//    int flag = -1;
//
//    if (jTask != nullptr) {
//      if (jCallNativeMethodId == NULL) {
//        jCallNativeMethodId = env->GetMethodID(jBridgeClazz,
//                                               "callNative",
//                                               "(Ljava/lang/String;[BLjava/lang/String;)I");
//      }
//
//      flag = env->CallIntMethod(jThis, jCallNativeMethodId, jPageId, jTask, jCallback);
//    }
//
//    if (flag == -1) {
//      LOGE("instance destroy JFM must stop callNative");
//    }
//
//    if (jTask != nullptr)
//      env->DeleteLocalRef(jTask);
//    if (jCallback != nullptr)
//      env->DeleteLocalRef(jCallback);
//
////    if (page != nullptr)
////      page->CallBridgeTime(getCurrentTime() - startTime);
//    return flag;
//  }
//
//  std::unique_ptr<IPCResult> Bridge_Impl_Android::callNativeModule(const char* pageId, const char *module, const char *method,
//                                                const char *arguments, int argumentsLen,  const char *options, int optionsLen) {
////    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
////    int64_t startTime = getCurrentTime();
//
//    JNIEnv *env = getJNIEnv();
//    jstring jModule = env->NewStringUTF(module);
//    jstring jMethod = env->NewStringUTF(method);
//    jbyteArray jArgString = newJByteArray(env, arguments, argumentsLen);
//    jbyteArray jOptString = newJByteArray(env, options, optionsLen);
//    jstring jPageId = getKeyFromCache(env, pageId);
//    jobject result = nullptr;
//
//    if (jModule != nullptr && jMethod != nullptr) {
//      if (jCallNativeModuleMethodId == NULL) {
//        jCallNativeModuleMethodId = env->GetMethodID(jBridgeClazz,
//                                                     "callNativeModule",
//                                                     "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[B[B)Ljava/lang/Object;");
//      }
//
//      result = env->CallObjectMethod(jThis, jCallNativeModuleMethodId, jPageId, jModule, jMethod, jArgString, jOptString);
//    }
//
//    if (jModule != nullptr)
//      env->DeleteLocalRef(jModule);
//    if (jMethod != nullptr)
//      env->DeleteLocalRef(jMethod);
//    if (jArgString != nullptr)
//      env->DeleteLocalRef(jArgString);
//    if (jOptString != nullptr)
//      env->DeleteLocalRef(jOptString);
//
////    if (page != nullptr)
////      page->CallBridgeTime(getCurrentTime() - startTime);
//
//    std::unique_ptr<IPCResult> ipc_result = createInt32Result(-1);
//    if (result == nullptr) {
//      return ipc_result;
//    }
//
//    jfieldID jTypeId = env->GetFieldID(jWXJSObject, "type", "I");
//    jint jTypeInt = env->GetIntField(result, jTypeId);
//    jfieldID jDataId = env->GetFieldID(jWXJSObject, "data", "Ljava/lang/Object;");
//    jobject jDataObj = env->GetObjectField(result, jDataId);
//    if (jTypeInt == 1) {
//      if (jDoubleValueMethodId == NULL) {
//        jclass jDoubleClazz = env->FindClass("java/lang/Double");
//        jDoubleValueMethodId = env->GetMethodID(jDoubleClazz, "doubleValue", "()D");
//        env->DeleteLocalRef(jDoubleClazz);
//      }
//      jdouble jDoubleObj = env->CallDoubleMethod(jDataObj, jDoubleValueMethodId);
//      ipc_result = std::move(createDoubleResult(jDoubleObj));
//
//    } else if (jTypeInt == 2) {
//      jstring jDataStr = (jstring) jDataObj;
//      ipc_result = std::unique_ptr<IPCResult>(new IPCStringResult(env, jDataStr));
//    } else if (jTypeInt == 3) {
//      jstring jDataStr = (jstring) jDataObj;
//      ipc_result = std::unique_ptr<IPCResult>(new IPCJSONStringResult(env, jDataStr));
//    } else if (jTypeInt == 4) {
//      jbyteArray array = (jbyteArray) jDataObj;
//      if (array != nullptr) {
//        int length = env->GetArrayLength(array);
//        void *data = env->GetByteArrayElements(array, 0);
//        ipc_result = std::move(createByteArrayResult((const char *) data, length));
//        env->ReleaseByteArrayElements(array, (jbyte *) data, 0);
//      }
//    }
//    env->DeleteLocalRef(jDataObj);
//    env->DeleteLocalRef(result);
//
//    return ipc_result;
//  }
//
//  void Bridge_Impl_Android::callNativeComponent(const char* pageId, const char* ref, const char *method,
//                                                const char *arguments, int argumentsLength, const char *options, int optionsLength) {
////    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
////    int64_t startTime = getCurrentTime();
//
//    JNIEnv *env = getJNIEnv();
//    jstring jMethod = env->NewStringUTF(method);
//    jbyteArray jArgString = newJByteArray(env, arguments, argumentsLength);
//    jbyteArray jOptString = newJByteArray(env, options, optionsLength);
//    jstring jPageId = getKeyFromCache(env, pageId);
//    jstring jRef = getKeyFromCache(env, ref);
//
//    if (jMethod != nullptr) {
//      if (jCallNativeComponentMethodId == NULL) {
//        jCallNativeComponentMethodId = env->GetMethodID(jBridgeClazz,
//                                                        "callNativeComponent",
//                                                        "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[B[B)V");
//      }
//      env->CallVoidMethod(jThis, jCallNativeComponentMethodId, jPageId, jRef, jMethod, jArgString, jOptString);
//    }
//
//    if (jMethod != nullptr)
//      env->DeleteLocalRef(jMethod);
//    if (jArgString != nullptr)
//      env->DeleteLocalRef(jArgString);
//    if (jOptString != nullptr)
//      env->DeleteLocalRef(jOptString);
//
////    if (page != nullptr)
////      page->CallBridgeTime(getCurrentTime() - startTime);
//  }
//
//  void Bridge_Impl_Android::setTimeout(const char* callbackID, const char* time) {
//    JNIEnv *env = getJNIEnv();
//    jstring jCallbackID = env->NewStringUTF(callbackID);
//    jstring jTime = env->NewStringUTF(time);
//
//    if (jCallbackID == nullptr || jTime == nullptr)
//      return;
//
//    if (jSetTimeoutNativeMethodId == NULL) {
//      jSetTimeoutNativeMethodId = env->GetMethodID(jBridgeClazz,
//                                                   "setTimeoutNative",
//                                                   "(Ljava/lang/String;Ljava/lang/String;)V");
//    }
//
//    env->CallVoidMethod(jThis, jSetTimeoutNativeMethodId, jCallbackID, jTime);
//
//    if (jCallbackID != nullptr)
//      env->DeleteLocalRef(jCallbackID);
//    if (jTime != nullptr)
//      env->DeleteLocalRef(jTime);
//  }
//
//  void Bridge_Impl_Android::callNativeLog(const char* str_array) {
//    JNIEnv *env = getJNIEnv();
//    if (jWXLogUtils != NULL) {
//      if (jLogMethodId == NULL) {
//        jLogMethodId = env->GetStaticMethodID(jWXLogUtils, "d",
//                                              "(Ljava/lang/String;[B)V");
//      }
//      if (jLogMethodId != NULL) {
//        jstring str_tag = env->NewStringUTF("jsLog");
//        jbyteArray str_msg = newJByteArray(env, str_array);
//        env->CallStaticVoidMethod(jWXLogUtils, jLogMethodId, str_tag, str_msg);
//        env->DeleteLocalRef(str_tag);
//        env->DeleteLocalRef(str_msg);
//      }
//    }
//  }
//
//  int Bridge_Impl_Android::callUpdateFinish(const char* pageId, const char *task, const char *callback) {
//
////    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
////    int64_t startTime = getCurrentTime();
//
//    JNIEnv *env = getJNIEnv();
//    jbyteArray jTask = newJByteArray(env, task);
//    jstring jCallback = env->NewStringUTF(callback);
//    jstring jPageId = getKeyFromCache(env, pageId);
//
//    if (jCallUpdateFinishMethodId == NULL) {
//      jCallUpdateFinishMethodId = env->GetMethodID(jBridgeClazz,
//                                                   "callUpdateFinish",
//                                                   "(Ljava/lang/String;[BLjava/lang/String;)I");
//    }
//
//    int flag = env->CallIntMethod(jThis, jCallUpdateFinishMethodId, jPageId, jTask, jCallback);
//
//    if (flag == -1) {
//      LOGE("instance destroy JFM must stop callUpdateFinish");
//    }
//
//    if (jTask != nullptr)
//      env->DeleteLocalRef(jTask);
//    if (jCallback != nullptr)
//      env->DeleteLocalRef(jCallback);
//
////    if (page != nullptr)
////      page->CallBridgeTime(getCurrentTime() - startTime);
//    return flag;
//  }
//
//  int Bridge_Impl_Android::callRefreshFinish(const char* pageId, const char *task, const char *callback) {
//
////    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
////    int64_t startTime = getCurrentTime();
//
//    JNIEnv *env = getJNIEnv();
//    jbyteArray jTask = newJByteArray(env, task);
//    jstring jCallback = env->NewStringUTF(callback);
//    jstring jPageId = getKeyFromCache(env, pageId);
//
//    if (jCallRefreshFinishMethodId == NULL) {
//      jCallRefreshFinishMethodId = env->GetMethodID(jBridgeClazz,
//                                                    "callRefreshFinish",
//                                                    "(Ljava/lang/String;[BLjava/lang/String;)I");
//    }
//    int flag = env->CallIntMethod(jThis, jCallRefreshFinishMethodId, jPageId, jTask, jCallback);
//
//    if (flag == -1) {
//      LOGE("instance destroy JFM must stop callNative");
//    }
//
//    if (jTask != nullptr)
//      env->DeleteLocalRef(jTask);
//    if (jCallback != nullptr)
//      env->DeleteLocalRef(jCallback);
//
////    if (page != nullptr)
////      page->CallBridgeTime(getCurrentTime() - startTime);
//    return flag;
//  }
//
//  int Bridge_Impl_Android::callCreateBody(const char* pageId, const char *componentType, const char* ref,
//                                          std::map<std::string, std::string> *styles,
//                                          std::map<std::string, std::string> *attributes,
//                                          std::set<std::string> *events,
//                                          const WXCoreMargin &margins,
//                                          const WXCorePadding &paddings,
//                                          const WXCoreBorderWidth &borders) {
//      // TODO call bridge time
////    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
////    long long startTime = getCurrentTime();
//
//    JNIEnv *env = getJNIEnv();
//
//    jstring jPageId = getKeyFromCache(env, pageId);
//    jstring jRef = getKeyFromCache(env, ref);
//
//    jobject jStyles = env->NewObject(jMapClazz, jMapConstructorMethodId);
//    jobject jAttributes = env->NewObject(jMapClazz, jMapConstructorMethodId);
//    jobject jEvents = env->NewObject(jSetClazz, jSetConstructorMethodId);
//
//      intptr_t l = reinterpret_cast<intptr_t >(styles);
//    cpyCMap2JMap(styles, jStyles, env);
//    cpyCMap2JMap(attributes, jAttributes, env);
//    cpyCSet2JSet(events, jEvents, env);
//
//    float c_margins[4];
//    float c_paddings[4];
//    float c_borders[4];
//
//    c_margins[0] = margins.getMargin(kMarginTop);
//    c_margins[1] = margins.getMargin(kMarginBottom);
//    c_margins[2] = margins.getMargin(kMarginLeft);
//    c_margins[3] = margins.getMargin(kMarginRight);
//
//    c_paddings[0] = paddings.getPadding(kPaddingTop);
//    c_paddings[1] = paddings.getPadding(kPaddingBottom);
//    c_paddings[2] = paddings.getPadding(kPaddingLeft);
//    c_paddings[3] = paddings.getPadding(kPaddingRight);
//
//    c_borders[0] = borders.getBorderWidth(kBorderWidthTop);
//    c_borders[1] = borders.getBorderWidth(kBorderWidthBottom);
//    c_borders[2] = borders.getBorderWidth(kBorderWidthLeft);
//    c_borders[3] = borders.getBorderWidth(kBorderWidthRight);
//
//    jfloatArray jMargins = c2jFloatArray(env, c_margins);
//    jfloatArray jPaddings = c2jFloatArray(env, c_paddings);
//    jfloatArray jBorders = c2jFloatArray(env, c_borders);
//
//    if (jCallCreateBodyMethodId == NULL)
//      jCallCreateBodyMethodId = env->GetMethodID(jBridgeClazz,
//                                                 "callCreateBody",
//                                                 "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/util/HashMap;Ljava/util/HashMap;Ljava/util/HashSet;[F[F[F)I");
//
//
//    jstring jComponentType = getComponentTypeFromCache(componentType);
//    if (jComponentType == nullptr) {
//      jComponentType = putComponentTypeToCache(componentType);
//    }
//
//    int flag = 0;
//    flag = env->CallIntMethod(jThis, jCallCreateBodyMethodId, jPageId,
//                              jComponentType, jRef, jStyles, jAttributes,
//                              jEvents, jMargins, jPaddings, jBorders);
//
//    if (flag == -1) {
//      LOGE("instance destroy JFM must stop callCreateBody");
//    }
//
//    env->DeleteLocalRef(jStyles);
//    env->DeleteLocalRef(jAttributes);
//    env->DeleteLocalRef(jEvents);
//    env->DeleteLocalRef(jMargins);
//    env->DeleteLocalRef(jPaddings);
//    env->DeleteLocalRef(jBorders);
//
////    if (page != nullptr)
////      page->CallBridgeTime(getCurrentTime() - startTime);
//    return flag;
//  }
//
//  int Bridge_Impl_Android::callAddElement(const char* pageId, const char *componentType,
//                                          const char* ref, int &index, const char* parentRef,
//                                          std::map<std::string, std::string> *styles,
//                                          std::map<std::string, std::string> *attributes,
//                                          std::set<std::string> *events,
//                                          const WXCoreMargin &margins,
//                                          const WXCorePadding &paddings,
//                                          const WXCoreBorderWidth &borders,
//                                          bool willLayout) {
//    JNIEnv *env = getJNIEnv();
//    jstring jPageId = getKeyFromCache(env, pageId);
//    jstring jRef = getKeyFromCache(env, ref);
//    jstring jParentRef = getKeyFromCache(env, parentRef);
//
//    jobject jStyles = env->NewObject(jMapClazz, jMapConstructorMethodId);
//    jobject jAttributes = env->NewObject(jMapClazz, jMapConstructorMethodId);
//    jobject jEvents = env->NewObject(jSetClazz, jSetConstructorMethodId);
//
//    cpyCMap2JMap(styles, jStyles, env);
//    cpyCMap2JMap(attributes, jAttributes, env);
//    cpyCSet2JSet(events, jEvents, env);
//
//    float c_margins[4];
//    float c_paddings[4];
//    float c_borders[4];
//
//    c_margins[0] = margins.getMargin(kMarginTop);
//    c_margins[1] = margins.getMargin(kMarginBottom);
//    c_margins[2] = margins.getMargin(kMarginLeft);
//    c_margins[3] = margins.getMargin(kMarginRight);
//
//    c_paddings[0] = paddings.getPadding(kPaddingTop);
//    c_paddings[1] = paddings.getPadding(kPaddingBottom);
//    c_paddings[2] = paddings.getPadding(kPaddingLeft);
//    c_paddings[3] = paddings.getPadding(kPaddingRight);
//
//    c_borders[0] = borders.getBorderWidth(kBorderWidthTop);
//    c_borders[1] = borders.getBorderWidth(kBorderWidthBottom);
//    c_borders[2] = borders.getBorderWidth(kBorderWidthLeft);
//    c_borders[3] = borders.getBorderWidth(kBorderWidthRight);
//
//    jfloatArray jMargins = c2jFloatArray(env, c_margins);
//    jfloatArray jPaddings = c2jFloatArray(env, c_paddings);
//    jfloatArray jBorders = c2jFloatArray(env, c_borders);
//
//    if (jCallAddElementMethodId == NULL)
//      jCallAddElementMethodId = env->GetMethodID(jBridgeClazz,
//                                                           "callAddElement",
//                                                           "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ILjava/lang/String;Ljava/util/HashMap;Ljava/util/HashMap;Ljava/util/HashSet;[F[F[FZ)I");
//
//    jstring jComponentType = getComponentTypeFromCache(componentType);
//    if (jComponentType == nullptr) {
//      jComponentType = putComponentTypeToCache(componentType);
//    }
//
//    int flag = 0;
//    flag = env->CallIntMethod(jThis, jCallAddElementMethodId, jPageId, jComponentType, jRef, index,
//                              jParentRef, jStyles, jAttributes, jEvents, jMargins, jPaddings, jBorders, willLayout);
//
//    if (flag == -1) {
//      LOGE("instance destroy JFM must stop callAddElement");
//    }
//
//    env->DeleteLocalRef(jStyles);
//    env->DeleteLocalRef(jAttributes);
//    env->DeleteLocalRef(jEvents);
//    env->DeleteLocalRef(jMargins);
//    env->DeleteLocalRef(jPaddings);
//    env->DeleteLocalRef(jBorders);
//
//    return flag;
//  }
//
//  int Bridge_Impl_Android::callRemoveElement(const char* pageId, const char* ref) {
//
////    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
////    long long startTime = getCurrentTime();
//
//    JNIEnv *env = getJNIEnv();
//
//    jstring jPageId = getKeyFromCache(env, pageId);
//    jstring jRef = getKeyFromCache(env, ref);
//
//    if (jCallRemoveElementMethodId == NULL) {
//      jCallRemoveElementMethodId = env->GetMethodID(jBridgeClazz,
//                                                    "callRemoveElement",
//                                                    "(Ljava/lang/String;Ljava/lang/String;)I");
//    }
//
//    int flag = env->CallIntMethod(jThis, jCallRemoveElementMethodId, jPageId, jRef);
//    if (flag == -1) {
//      LOGE("instance destroy JFM must stop callRemoveElement");
//    }
//
////    if (page != nullptr)
////      page->CallBridgeTime(getCurrentTime() - startTime);
//    return 0;
//  }
//
//  int Bridge_Impl_Android::callMoveElement(const char* pageId, const char* ref, const char* parentRef, int index) {
//
////    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
////    long long startTime = getCurrentTime();
//
//    JNIEnv *env = getJNIEnv();
//
//    jstring jPageId = getKeyFromCache(env, pageId);
//    jstring jRef = getKeyFromCache(env, ref);
//    jstring jParentRef = getKeyFromCache(env, parentRef);
//
//    if (jCallMoveElementMethodId == NULL) {
//      jCallMoveElementMethodId = env->GetMethodID(jBridgeClazz,
//                                                  "callMoveElement",
//                                                  "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)I");
//    }
//
//    int flag = env->CallIntMethod(jThis, jCallMoveElementMethodId, jPageId, jRef, jParentRef, index);
//    if (flag == -1) {
//      LOGE("instance destroy JFM must stop callRemoveElement");
//    }
//
////    if (page != nullptr)
////      page->CallBridgeTime(getCurrentTime() - startTime);
//    return 0;
//  }
//
//  int Bridge_Impl_Android::callAddEvent(const char* pageId, const char* ref, const char *event) {
//
////    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
////    long long startTime = getCurrentTime();
//
//    JNIEnv *env = getJNIEnv();
//    jstring jPageId = getKeyFromCache(env, pageId);
//    jstring jRef = getKeyFromCache(env, ref);
//
//    if (jCallAddEventMethodId == NULL) {
//      jCallAddEventMethodId = env->GetMethodID(jBridgeClazz,
//                                               "callAddEvent",
//                                               "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I");
//    }
//    jstring jEventId = env->NewStringUTF(event);
//
//    int flag = env->CallIntMethod(jThis, jCallAddEventMethodId, jPageId, jRef, jEventId);
//    if (flag == -1) {
//      LOGE("instance destroy JFM must stop callAddEvent");
//    }
//
////    if (page != nullptr)
////      page->CallBridgeTime(getCurrentTime() - startTime);
//    env->DeleteLocalRef(jEventId);
//    return flag;
//  }
//
//  int Bridge_Impl_Android::callRemoveEvent(const char* pageId, const char* ref, const char *event) {
//
////    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
////    long long startTime = getCurrentTime();
//
//    JNIEnv *env = getJNIEnv();
//
//    jstring jPageId = getKeyFromCache(env, pageId);
//    jstring jRef = getKeyFromCache(env, ref);
//
//    if (jCallRemoveEventMethodId == NULL) {
//      jCallRemoveEventMethodId = env->GetMethodID(jBridgeClazz,
//                                                  "callRemoveEvent",
//                                                  "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I");
//    }
//    jstring jEventId = env->NewStringUTF(event);
//
//    int flag = env->CallIntMethod(jThis, jCallRemoveEventMethodId, jPageId, jRef, jEventId);
//    if (flag == -1) {
//      LOGE("instance destroy JFM must stop callRemoveElement");
//    }
//
////    if (page != nullptr)
////      page->CallBridgeTime(getCurrentTime() - startTime);
//    env->DeleteLocalRef(jEventId);
//    return flag;
//  }
//
//  int Bridge_Impl_Android::callUpdateStyle(const char* pageId, const char* ref,
//                                           std::vector<std::pair<std::string, std::string>> *style,
//                                           std::vector<std::pair<std::string, std::string>> *margin,
//                                           std::vector<std::pair<std::string, std::string>> *padding,
//                                           std::vector<std::pair<std::string, std::string>> *border) {
////    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
////    long long startTime = getCurrentTime();
//
//    JNIEnv *env = getJNIEnv();
//
//    jstring jPageId = getKeyFromCache(env, pageId);
//    jstring jRef = getKeyFromCache(env, ref);
//
//    jobject jStyles = env->NewObject(jMapClazz, jMapConstructorMethodId);
//    jobject jMargins = env->NewObject(jMapClazz, jMapConstructorMethodId);
//    jobject jPaddings = env->NewObject(jMapClazz, jMapConstructorMethodId);
//    jobject jBorders = env->NewObject(jMapClazz, jMapConstructorMethodId);
//
//    if (style != nullptr && style->size() != 0) {
//      cpyCVector2JMap(style, jStyles, env);
//    }
//
//    if (margin != nullptr && margin->size() != 0) {
//      cpyCVector2JMap(margin, jMargins, env);
//    }
//
//    if (padding != nullptr && padding->size() != 0) {
//      cpyCVector2JMap(padding, jPaddings, env);
//    }
//
//    if (border != nullptr && border->size() != 0) {
//      cpyCVector2JMap(border, jBorders, env);
//    }
//
//    if (jCallUpdateStyleMethodId == NULL)
//      jCallUpdateStyleMethodId = env->GetMethodID(jBridgeClazz,
//                                                            "callUpdateStyle",
//                                                            "(Ljava/lang/String;Ljava/lang/String;Ljava/util/HashMap;Ljava/util/HashMap;Ljava/util/HashMap;Ljava/util/HashMap;)I");
//
//    int flag = 0;
//    flag = env->CallIntMethod(jThis, jCallUpdateStyleMethodId, jPageId, jRef, jStyles, jMargins, jPaddings, jBorders);
//
//    if (flag == -1) {
//      LOGE("instance destroy JFM must stop callUpdateStyle");
//    }
//
//    env->DeleteLocalRef(jStyles);
//    env->DeleteLocalRef(jMargins);
//    env->DeleteLocalRef(jPaddings);
//    env->DeleteLocalRef(jBorders);
//
////    if (page != nullptr)
////      page->CallBridgeTime(getCurrentTime() - startTime);
//    return flag;
//  }
//
//  int Bridge_Impl_Android::callUpdateAttr(const char* pageId, const char* ref, std::vector<std::pair<std::string, std::string>> *attrs) {
//
////    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
////    long long startTime = getCurrentTime();
//
//    JNIEnv *env = getJNIEnv();
//
//    jstring jPageId = getKeyFromCache(env, pageId);
//    jstring jRef = getKeyFromCache(env, ref);
//
//    jobject jAttrs = env->NewObject(jMapClazz, jMapConstructorMethodId);
//
//    if (attrs != nullptr) {
//      cpyCVector2JMap(attrs, jAttrs, env);
//    }
//
//    if (jCallUpdateAttrsMethodId == NULL) {
//      jCallUpdateAttrsMethodId = env->GetMethodID(jBridgeClazz,
//                                                            "callUpdateAttrs",
//                                                            "(Ljava/lang/String;Ljava/lang/String;Ljava/util/HashMap;)I");
//    }
//
//    int flag = 0;
//    flag = env->CallIntMethod(jThis, jCallUpdateAttrsMethodId, jPageId, jRef, jAttrs);
//
//    if (flag == -1) {
//      LOGE("instance destroy JFM must stop callUpdateStyle");
//    }
//
//    env->DeleteLocalRef(jAttrs);
//
////    if (page != nullptr)
////      page->CallBridgeTime(getCurrentTime() - startTime);
//    return flag;
//  }
//
//  int Bridge_Impl_Android::callLayout(const char* pageId, const char* ref,
//                                      int top, int bottom, int left, int right,
//                                      int height, int width, int index) {
//
////    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
////    long long startTime = getCurrentTime();
//
//    JNIEnv *env = getJNIEnv();
//
//    jstring jPageId = getKeyFromCache(env, pageId);
//    jstring jRef = getKeyFromCache(env, ref);
//
//    if (jCallLayoutMethodId == NULL)
//      jCallLayoutMethodId = env->GetMethodID(jBridgeClazz,
//                                                       "callLayout",
//                                                       "(Ljava/lang/String;Ljava/lang/String;IIIIIII)I");
//
//    int flag = 0;
//    flag = env->CallIntMethod(jThis, jCallLayoutMethodId, jPageId,
//                              jRef, top, bottom, left, right, height, width, index);
//
//    if (flag == -1) {
//      LOGE("instance destroy JFM must stop callLayout");
//    }
//
////    if (page != nullptr)
////      page->CallBridgeTime(getCurrentTime() - startTime);
//    return flag;
//  }
//
//  int Bridge_Impl_Android::callCreateFinish(const char* pageId) {
//
////    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
////    long long startTime = getCurrentTime();
//
//    JNIEnv *env = getJNIEnv();
//
//    jstring jPageId = getKeyFromCache(env, pageId);
//
//    if (jCallCreateFinishMethodId == NULL)
//      jCallCreateFinishMethodId = env->GetMethodID(jBridgeClazz,
//                                                             "callCreateFinish",
//                                                             "(Ljava/lang/String;)I");
//
//    int flag = env->CallIntMethod(jThis, jCallCreateFinishMethodId, jPageId);
//
//    if (flag == -1) {
//      LOGE("instance destroy JFM must stop callCreateFinish");
//    }
//
////    if (page != nullptr)
////      page->CallBridgeTime(getCurrentTime() - startTime);
//    return flag;
//  }
//
//  int Bridge_Impl_Android::callAppendTreeCreateFinish(const char *pageId, const char *ref) {
//
////    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
////    long long startTime = getCurrentTime();
//
//    JNIEnv *env = getJNIEnv();
//
//    jstring jPageId = getKeyFromCache(env, pageId);
//    jstring jRef = getKeyFromCache(env, ref);
//
//    if (jCallAppendTreeCreateFinishMethodId == NULL)
//      jCallAppendTreeCreateFinishMethodId = env->GetMethodID(jBridgeClazz,
//                                             "callAppendTreeCreateFinish",
//                                             "(Ljava/lang/String;Ljava/lang/String;)I");
//
//    int flag = 0;
//    flag = env->CallIntMethod(jThis, jCallAppendTreeCreateFinishMethodId, jPageId, jRef);
//
//    if (flag == -1) {
//      LOGE("instance destroy JFM must stop callAppendTreeCreateFinish");
//    }
//
////    if (page != nullptr)
////      page->CallBridgeTime(getCurrentTime() - startTime);
//    return flag;
//  }
//
//  int Bridge_Impl_Android::callHasTransitionPros(const char* pageId, const char* ref, std::vector<std::pair<std::string, std::string>> *style) {
//
////    RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
////    long long startTime = getCurrentTime();
//
//    JNIEnv *env = getJNIEnv();
//
//    jstring jPageId = getKeyFromCache(env, pageId);
//    jstring jRef = getKeyFromCache(env, ref);
//
//    jobject jStyles = env->NewObject(jMapClazz, jMapConstructorMethodId);
//
//    if (style != nullptr) {
//      cpyCVector2JMap(style, jStyles, env);
//    }
//
//    if (jCallHasTransitionProsMethodId == NULL)
//        jCallHasTransitionProsMethodId = env->GetMethodID(jBridgeClazz,
//                                                            "callHasTransitionPros",
//                                                            "(Ljava/lang/String;Ljava/lang/String;Ljava/util/HashMap;)I");
//
//    int flag = 0;
//    flag = env->CallIntMethod(jThis, jCallHasTransitionProsMethodId, jPageId, jRef, jStyles);
//
//    env->DeleteLocalRef(jStyles);
//
////    if (page != nullptr)
////      page->CallBridgeTime(getCurrentTime() - startTime);
//    return flag;
//  }
//
//  void Bridge_Impl_Android::handlePostMessage(const char* vm_id, const char *data ) {
//    JNIEnv *env = getJNIEnv();
//    auto jni_str = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(vm_id));
//    auto jni_array = base::android::ScopedLocalJavaRef<jbyteArray>(env, newJByteArray(env, data));
//
//    if (jPostMessage == NULL) {
//      jPostMessage = env->GetMethodID(jWMBridgeClazz,
//                                      "postMessage",
//                                      "(Ljava/lang/String;[B)V");
//    }
//    env->CallVoidMethod(jWMThis, jPostMessage, jni_str.Get(), jni_array.Get());
//  }
//
//  void Bridge_Impl_Android::handleDispatchMessage(const char* client_id, const char* vm_id, const char *data, const char* callback) {
//    JNIEnv *env = getJNIEnv();
//    auto jni_client_id = base::android::ScopedLocalJavaRef<jstring>(env, newJString(env, client_id));
//    auto jni_array = base::android::ScopedLocalJavaRef<jbyteArray>(env, newJByteArray(env, data));
//    auto jni_callback = base::android::ScopedLocalJavaRef<jstring>(env, newJString(env, callback));
//    auto jni_vm_id = base::android::ScopedLocalJavaRef<jstring>(env, newJString(env, vm_id));
//    if (jDispatchMeaasge == NULL) {
//      jDispatchMeaasge = env->GetMethodID(jWMBridgeClazz,
//                                          "dispatchMessage",
//                                          "(Ljava/lang/String;Ljava/lang/String;[BLjava/lang/String;)V");
//    }
//    env->CallVoidMethod(jWMThis, jDispatchMeaasge, jni_client_id.Get(), jni_vm_id.Get(), jni_array.Get(), jni_callback.Get());
//  }
//
////  void Bridge_Impl_Android::handlePostMessage(jstring jVmId, jbyteArray jData) {
////    JNIEnv *env = getJNIEnv();
////    if (jPostMessage == NULL) {
////      jPostMessage = env->GetMethodID(jWMBridgeClazz,
////                                    "postMessage",
////                                    "(Ljava/lang/String;[B)V");
////    }
////    env->CallVoidMethod(jWMThis, jPostMessage, jVmId, jData);
////  }
//
////  void Bridge_Impl_Android::handleDispatchMessage(jstring jClientId, jstring jVmId, jbyteArray jData, jstring jCallback) {
////    JNIEnv *env = getJNIEnv();
////    if (jDispatchMeaasge == NULL) {
////      jDispatchMeaasge = env->GetMethodID(jWMBridgeClazz,
////                                        "dispatchMessage",
////                                        "(Ljava/lang/String;Ljava/lang/String;[BLjava/lang/String;)V");
////    }
////    env->CallVoidMethod(jWMThis, jDispatchMeaasge, jClientId, jVmId, jData, jCallback);
////  }
//
//  jobject Bridge_Impl_Android::getMeasureFunc(const char* pageId, jlong renderObjectPtr) {
//    JNIEnv *env = getJNIEnv();
//    jstring jPageId = getKeyFromCache(env, pageId);
//    if (jCallGetMeasurementMethodId == NULL) {
//      jCallGetMeasurementMethodId = env->GetMethodID(jBridgeClazz,
//                                                     "getMeasurementFunc",
//                                                     "(Ljava/lang/String;J)Lcom/taobao/weex/layout/ContentBoxMeasurement;");
//    }
//    return env->CallObjectMethod(jThis, jCallGetMeasurementMethodId, jPageId, renderObjectPtr);
//  }
} //end WeexCore
