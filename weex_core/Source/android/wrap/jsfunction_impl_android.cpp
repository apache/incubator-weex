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

#include <android/base/jni/jbytearray_ref.h>
#include <android/wrap/wx_bridge.h>
#include <core/manager/weex_core_manager.h>
#include <base/make_copyable.h>
#include <base/thread/waitable_event.h>
#include <wson_parser.h>
#include "jsfunction_impl_android.h"
#include "android/base/string/string_utils.h"
#include "android/jniprebuild/jniheader/WXJsFunctions_jni.h"
#include "core/render/manager/render_manager.h"
#include "IPC/IPCResult.h"
#include "core/bridge/platform_bridge.h"

using namespace WeexCore;

namespace WeexCore {
    bool RegisterWXJsFunction(JNIEnv *env) {
        return RegisterNativesImpl(env);
    }
}

inline static char *copyStr(const char *str, int length = 0) {
    char *ret = nullptr;
    if (str == nullptr) return ret;
    int strLen = length == 0 ? strlen(str) : length;
    ret = new char[strLen + 1];
    memcpy(ret, str, static_cast<size_t>(strLen));
    ret[strLen] = '\0';
    return ret;
}

static const char *getCharFromJByte(JNIEnv *env, jbyteArray jbyteArray1) {
    if (jbyteArray1 == nullptr)
        return "";

    return jByteArray2Str(env, jbyteArray1).c_str();
}


static const int getJByteArraySize(JNIEnv *env, jbyteArray array) {
    if (array == nullptr) {
        return 0;
    }
    jsize size = env->GetArrayLength(array);
    return size;
}


static const char *getCharFromJString(JNIEnv *env, jstring string) {
    if (string == nullptr)
        return "";
    return env->GetStringUTFChars(string, nullptr);
}

void initWxBridge(JNIEnv *env, jobject object, jobject bridge, jstring className) {
    ScopedJStringUTF8 classNameRef = ScopedJStringUTF8(env, className);
    WXBridge::Instance()->Reset(env, bridge);
    WXBridge::Instance()->reset_clazz(env, classNameRef.getChars());
}

void jsHandleSetJSVersion(JNIEnv *env, jobject object, jstring jsVersion) {
    const char *js_version = getCharFromJString(env, jsVersion);
    WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
            weex::base::MakeCopyable([version = std::string(js_version)] {
                WeexCoreManager::Instance()->script_bridge()->core_side()->SetJSVersion(
                        version.c_str());
            }));
}

void jsHandleReportException(JNIEnv *env, jobject object, jstring instanceId, jstring func,
                             jstring exceptionjstring) {
    const char *id = getCharFromJString(env, instanceId);
    const char *string = getCharFromJString(env, func);
    const char *exception_string = getCharFromJString(env,
                                                      exceptionjstring);

    WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
            weex::base::MakeCopyable([page_id = std::string(id),
                                             func = std::string(string),
                                             exception = std::string(exception_string)] {
                WeexCoreManager::Instance()
                        ->script_bridge()
                        ->core_side()
                        ->ReportException(page_id.c_str(), func.c_str(), exception.c_str());
            }));
}

void jsHandleCallNative(JNIEnv *env, jobject object, jstring instanceId, jbyteArray tasks,
                        jstring callback) {


    const char *page_id = getCharFromJString(env, instanceId);
    const char *callbacks = getCharFromJString(env, callback);

    const char *task = (char *)env->GetByteArrayElements(tasks,NULL);

#if JSAPI_LOG
    LOGD("[ExtendJSApi] handleCallNative >>>> pageId: %s, task: %s", pageId.c_str(), task.c_str());
#endif


    WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
            weex::base::MakeCopyable([page_id = std::string(page_id),
                                             task = std::string(task),
                                             callback = std::string(callbacks)] {
                WeexCoreManager::Instance()->script_bridge()->core_side()->CallNative(
                        page_id.c_str(), task.c_str(), callback.c_str());
            }));
}

void
jsHandleCallNativeModule(JNIEnv *env, jobject object, jstring instanceId, jstring module,
                         jstring method, jbyteArray
                         arguments, jbyteArray options, jboolean from) {

    auto arg = JByteArrayRef(env, arguments);
    auto opt = JByteArrayRef(env, options);

//    const char *argumentsChar = (char *)env->GetByteArrayElements(arguments,NULL);
//
//    int argLength = env->GetArrayLength(arguments);
//
//    const char *optionsChar = (char *)env->GetByteArrayElements(options,NULL);
//    int optionsLength = env->GetArrayLength(options);

    const char *id = getCharFromJString(env, instanceId);
    const char *string = getCharFromJString(env, module);
    const char *jString = getCharFromJString(env, method);
//    WeexCoreManager::Instance()->getPlatformBridge()->platform_side()->CallNativeModule(
//            id,
//            string,
//            jString,
//            argumentsRef.getBytes(),
//            argumentsRef.length(),
//            optionsRef.getBytes(),
//            optionsRef.length());

    WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
            weex::base::MakeCopyable(
                    [page_id = std::string(id), module = std::string(string),
                            method = std::string(jString),
                            arguments = std::unique_ptr<char[]>(
                                    copyStr(arg.getBytes(), arg.length())),
                            argLen = arg.length(),
                            options = std::unique_ptr<char[]>(
                                    copyStr(opt.getBytes(), opt.length())),optLen = opt.length()] {
                        WeexCoreManager::Instance()
                                ->script_bridge()
                                ->core_side()
                                ->CallNativeModule(page_id.c_str(), module.c_str(),
                                                   method.c_str(), arguments.get(),
                                                   argLen, options.get(),
                                                   optLen);
                    }));
    // add for android support
//    jobject result;
//    result = static_cast<jobject>(WeexCoreManager::getInstance()->getPlatformBridge()->callNativeModule(
//                getCharFromJString(env, instanceId),
//                getCharFromJString(env, module),
//                getCharFromJString(env, method),
//                argumentsRef.getBytes(),
//                argumentsRef.length(),
//                optionsRef.getBytes(),
//                optionsRef.length()));

//    jfieldID jTypeId = env->GetFieldID(jWXJSObject, "type", "I");
//    jint jTypeInt = env->GetIntField(result, jTypeId);
//    jfieldID jDataId = env->GetFieldID(jWXJSObject, "data", "Ljava/lang/Object;");
//    jobject jDataObj = env->GetObjectField(result, jDataId);
//    if (jTypeInt == 1) {
//        if (jDoubleValueMethodId == NULL) {
//            jclass jDoubleClazz = env->FindClass("java/lang/Double");
//            jDoubleValueMethodId = env->GetMethodID(jDoubleClazz, "doubleValue", "()D");
//            env->DeleteLocalRef(jDoubleClazz);
//        }
//        jdouble jDoubleObj = env->CallDoubleMethod(jDataObj, jDoubleValueMethodId);
//    } else if (jTypeInt == 2) {
//        jstring jDataStr = (jstring) jDataObj;
//        //ret = std::move(createStringResult(env, jDataStr));
//    } else if (jTypeInt == 3) {
//        jstring jDataStr = (jstring) jDataObj;
//        //ret = std::move(createJSONStringResult(env, jDataStr));
//    } else if (jTypeInt == 4) {
//        jbyteArray array = (jbyteArray)jDataObj;
//        if(array != nullptr){
//            int length = env->GetArrayLength(array);
//            void* data = env->GetByteArrayElements(array, 0);
//            //ret = std::move(createByteArrayResult((const char*)data, length));
//            env->ReleaseByteArrayElements(array, (jbyte*)data, 0);
//        }
//    }
//    env->DeleteLocalRef(jDataObj);
//    if(result != nullptr){
//        env->DeleteLocalRef(result);
//    }
    //return ret;
}

void
jsHandleCallNativeComponent(JNIEnv *env, jobject object, jstring instanceId, jstring componentRef,
                            jstring method,
                            jbyteArray arguments, jbyteArray options, jboolean from) {

    auto arg = JByteArrayRef(env, arguments);
    auto opt = JByteArrayRef(env, options);

    const char *id = getCharFromJString(env, instanceId);
    const char *ref = getCharFromJString(env, componentRef);
    const char *string = getCharFromJString(env, method);
//    WeexCoreManager::Instance()->getPlatformBridge()->platform_side()->CallNativeComponent(id,
//                                                                                           ref,
//                                                                                           string,
//                                                                                           argumentsRef.getBytes(),
//                                                                                           argumentsRef.length(),
//                                                                                           optionsRef.getBytes(),
//                                                                                           optionsRef.length());


    WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
            weex::base::MakeCopyable(
                    [page_id = std::string(id), ref = std::string(ref),
                            method = std::string(string),
                            arguments =
                            std::unique_ptr<char[]>(copyStr(arg.getBytes(), arg.length())),
                            arguments_length = arg.length(),
                            options = std::unique_ptr<char[]>(copyStr(opt.getBytes(), opt.length())),
                            options_length = opt.length()] {
                        WeexCoreManager::Instance()
                                ->script_bridge()
                                ->core_side()
                                ->CallNativeComponent(page_id.c_str(), ref.c_str(),
                                                      method.c_str(), arguments.get(),
                                                      arguments_length, options.get(),
                                                      options_length);
                    }));

}

void
jsHandleCallAddElement(JNIEnv *env, jobject object, jstring instanceId, jstring ref, jbyteArray dom,
                       jstring index) {

    const char *instanceChar = env->GetStringUTFChars(instanceId, 0);
    const char *refChar = env->GetStringUTFChars(ref, 0);

    const char *domChar = (char *)env->GetByteArrayElements(dom,NULL);
    int domLength = env->GetArrayLength(dom);


    const char *indexChar = env->GetStringUTFChars(index, 0);

//    int indexI = atoi(indexChar);
//    if (instanceChar == nullptr || refChar == nullptr || domRef.length() == 0 ||
//        indexChar == nullptr ||
//        indexI < -1)
//        return;
//
//    RenderManager::GetInstance()->AddRenderObject(instanceChar, refChar, indexI,  domRef.getBytes());

    WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
            weex::base::MakeCopyable([page_id = std::string(instanceChar),
                                             parent_ref = std::string(refChar),
                                             dom_str = std::unique_ptr<char[]>(
                                                     copyStr(domChar, domLength)),
                    dom_len = domLength,
                                             index_str = std::string(indexChar)] {
                WeexCoreManager::Instance()->script_bridge()->core_side()->AddElement(
                        page_id.c_str(), parent_ref.c_str(), dom_str.get(), dom_len,
                        index_str.c_str());
            }));
}

void jsHandleSetTimeout(JNIEnv *env, jobject object, jstring callbackId, jstring time) {
//
    const char *id = getCharFromJString(env, callbackId);
    const char *string = getCharFromJString(env, time);
//    WeexCoreManager::Instance()->getPlatformBridge()->platform_side()->SetTimeout(id,
//                                                                                  string);

    WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
            weex::base::MakeCopyable([callback_id = std::string(id),
                                             time = std::string(string)] {
                WeexCoreManager::Instance()->script_bridge()->core_side()->SetTimeout(
                        callback_id.c_str(), time.c_str());
            }));

}

void jsHandleCallNativeLog(JNIEnv *env, jobject object, jbyteArray str_array) {
    const char *array = getCharFromJByte(env, str_array);
//    WeexCoreManager::Instance()->getPlatformBridge()->platform_side()->NativeLog(array);

    WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
            weex::base::MakeCopyable([str_array = std::string(array)] {
                WeexCoreManager::Instance()->script_bridge()->core_side()->NativeLog(
                        str_array.c_str());
            }));
}

void jsFunctionCallCreateBody(JNIEnv *env, jobject object, jstring pageId, jbyteArray domStr,
                              jboolean from) {
    if (pageId == nullptr || domStr == nullptr)
        return;

    const char *page = env->GetStringUTFChars(pageId, NULL);
    const char *string = (char *)env->GetByteArrayElements(domStr,NULL);
    int length = env->GetArrayLength(domStr);

    if (page == nullptr || string == nullptr)
        return;

    WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
            weex::base::MakeCopyable([page_id = std::string(page),
                                             dom_str = std::unique_ptr<char[]>(
                                                     copyStr(string, length)),
                                             dom_len = length] {
                WeexCoreManager::Instance()->script_bridge()->core_side()->CreateBody(
                        page_id.c_str(), dom_str.get(), dom_len);
            }));
}

void
jsFunctionCallUpdateFinish(JNIEnv *env, jobject object, jstring instanceId, jbyteArray tasks,
                           jstring callback) {


    const char *taskChar = (char *)env->GetByteArrayElements(tasks,NULL);
    int taskLength = env->GetArrayLength(tasks);

    auto callbackS = getCharFromJString(env, callback);
    const char *id = getCharFromJString(env, instanceId);
//    WeexCoreManager::Instance()->getPlatformBridge()->platform_side()->UpdateFinish(id,
//                                                                                       task,strlen(task),
//                                                                                       callbackS,strlen(callbackS));


    weex::base::WaitableEvent event;
    int result = -1;
    WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
            weex::base::MakeCopyable(
                    [page_id = std::string(id),
                            task = std::unique_ptr<char[]>(copyStr(taskChar, taskLength)),
                            task_len = taskLength,
                            callback = std::unique_ptr<char[]>(
                                    copyStr(callbackS, strlen(callbackS))),
                            callback_len = strlen(callbackS), event = &event, result = &result] {
                        *result = WeexCoreManager::Instance()
                                ->script_bridge()
                                ->core_side()
                                ->UpdateFinish(page_id.c_str(), task.get(), task_len,
                                               callback.get(), callback_len);
                        event->Signal();
                    }));
    event.Wait();

}

void jsFunctionCallCreateFinish(JNIEnv *env, jobject object, jstring pageId) {
    const char *id = env->GetStringUTFChars(pageId, 0);
//    RenderManager::GetInstance()->CreateFinish(id);


    WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
            weex::base::MakeCopyable([page_id = std::string(id)] {
                WeexCoreManager::Instance()->script_bridge()->core_side()->CreateFinish(
                        page_id.c_str());
            }));

}

void
jsFunctionCallRefreshFinish(JNIEnv *env, jobject object, jstring instanceId, jbyteArray tasks,
                            jstring callback) {
    const char *id = getCharFromJString(env, instanceId);
    const char *task = getCharFromJByte(env, tasks);
    const char *string = getCharFromJString(env, callback);
//    WeexCoreManager::Instance()->getPlatformBridge()->platform_side()->RefreshFinish(id,
//                                                                                     task,
//                                                                                     string);

    WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
            weex::base::MakeCopyable(
                    [page_id = std::string(id), task = std::string(task),
                            callback = std::string(string)] {
                        WeexCoreManager::Instance()
                                ->script_bridge()
                                ->core_side()
                                ->RefreshFinish(page_id.c_str(), task.c_str(),
                                                callback.c_str());
                    }));
}

void
jsFunctionCallUpdateAttrs(JNIEnv *env, jobject object, jstring pageId, jstring ref, jbyteArray data,
                          jboolean from) {



    const char *byte = (char *)env->GetByteArrayElements(data,NULL);
    int length = env->GetArrayLength(data);

    const char *id = env->GetStringUTFChars(pageId, 0);
    const char *string = env->GetStringUTFChars(ref, 0);
//    RenderManager::GetInstance()->UpdateAttr(id,
//                                             string,
//                                             dataRef.getBytes());

    WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
            weex::base::MakeCopyable([page_id = std::string(id),
                                             ref = std::string(string),
                                             data = std::unique_ptr<char[]>(
                                                     copyStr(byte, length)),
                                             data_len = length] {
                WeexCoreManager::Instance()->script_bridge()->core_side()->UpdateAttrs(
                        page_id.c_str(), ref.c_str(), data.get(), data_len);
            }));
}

void
jsFunctionCallUpdateStyle(JNIEnv *env, jobject object, jstring pageId, jstring ref, jbyteArray data,
                          jboolean from) {

    const char *byte = (char *)env->GetByteArrayElements(data,NULL);
    int length = env->GetArrayLength(data);

    const char *id = env->GetStringUTFChars(pageId, 0);
    const char *string = env->GetStringUTFChars(ref, 0);
//    RenderManager::GetInstance()->UpdateStyle(id,
//                                              string,
//                                              dataRef.getBytes());

    WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
            weex::base::MakeCopyable([page_id = std::string(id),
                                             ref = std::string(string),
                                             data = std::unique_ptr<char[]>(
                                                     copyStr(byte, length)),
                                             data_len = length] {
                WeexCoreManager::Instance()->script_bridge()->core_side()->UpdateStyle(
                        page_id.c_str(), ref.c_str(), data.get(), data_len);
            }));
}

void jsFunctionCallRemoveElement(JNIEnv *env, jobject object, jstring pageId, jstring ref) {

    const char *id = env->GetStringUTFChars(pageId, 0);
    const char *string = env->GetStringUTFChars(ref, 0);
//    RenderManager::GetInstance()->RemoveRenderObject(id,
//                                                     string);


    WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
            weex::base::MakeCopyable(
                    [page_id = std::string(id), ref = std::string(string)] {
                        WeexCoreManager::Instance()
                                ->script_bridge()
                                ->core_side()
                                ->RemoveElement(page_id.c_str(), ref.c_str());
                    }));

}

void
jsFunctionCallMoveElement(JNIEnv *env, jobject object, jstring pageId, jstring ref,
                          jstring parentRef, jstring index_str) {

    int iii = atoi(env->GetStringUTFChars(index_str, 0));

    const char *id = env->GetStringUTFChars(pageId, 0);
    const char *string = env->GetStringUTFChars(ref, 0);
    const char *chars = env->GetStringUTFChars(parentRef, 0);
//    RenderManager::GetInstance()->MoveRenderObject(id,
//                                                   string,
//                                                   chars, iii);


    WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
            weex::base::MakeCopyable([page_id = std::string(id),
                                             ref = std::string(string),
                                             parent_ref = std::string(chars),
                                             index = iii] {
                WeexCoreManager::Instance()->script_bridge()->core_side()->MoveElement(
                        page_id.c_str(), ref.c_str(), parent_ref.c_str(), index);
            }));

}

void
jsFunctionCallAddEvent(JNIEnv *env, jobject object, jstring pageId, jstring ref, jstring event) {
    const char *id = env->GetStringUTFChars(pageId, 0);
    const char *string = env->GetStringUTFChars(ref, 0);
    const char *chars = env->GetStringUTFChars(event, 0);
//    RenderManager::GetInstance()->AddEvent(id,
//                                           string,
//                                           chars);

    WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
            weex::base::MakeCopyable([page_id = std::string(id),
                                             ref = std::string(string),
                                             event = std::string(chars)] {
                WeexCoreManager::Instance()->script_bridge()->core_side()->AddEvent(
                        page_id.c_str(), ref.c_str(), event.c_str());
            }));

}

void
jsFunctionCallRemoveEvent(JNIEnv *env, jobject object, jstring pageId, jstring ref,
                          jstring event) {

    const char *id = env->GetStringUTFChars(pageId, 0);
    const char *string = env->GetStringUTFChars(ref, 0);
    const char *chars = env->GetStringUTFChars(event, 0);
//    RenderManager::GetInstance()->RemoveEvent(id,
//                                              string,
//                                              chars);

    WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
            weex::base::MakeCopyable([page_id = std::string(id),
                                             ref = std::string(string),
                                             event = std::string(chars)] {
                WeexCoreManager::Instance()->script_bridge()->core_side()->RemoveEvent(
                        page_id.c_str(), ref.c_str(), event.c_str());
            }));

}

void jsHandleSetInterval(JNIEnv *env, jobject object, jstring instanceId, jstring callbackId,
                         jstring time) {}

void
jsHandleClearInterval(JNIEnv *env, jobject object, jstring instanceId, jstring callbackId) {}

void jsHandleCallGCanvasLinkNative(JNIEnv *env, jobject object, jstring contextId, int type,
                                   jstring val) {}