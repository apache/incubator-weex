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
//
// Created by Darin on 13/02/2018.
//

#include "WeexJSCoreApi.h"
#include <android/bridge/impl/bridge_impl_android.h>
#include <core/render/manager/render_manager.h>
#include <android/jsengine/multiprocess/ExtendJSApi.h>
#include <android/base/string/string_utils.h>
#include <wson/wson_parser.h>

using namespace WeexCore;

extern WeexCore::FunType gCanvasFunc;
extern WeexCore::FunTypeT3d t3dFunc;

void _setJSVersion(const char *jsVersion) {
    LOGA("init JSFrm version %s", jsVersion);
    Bridge_Impl_Android::getInstance()->setJSVersion(jsVersion);
}

void _reportException(const char *pageId, const char *func, const char *exception_string) {
    Bridge_Impl_Android::getInstance()->reportException(pageId, func, exception_string);
}

void _callNative(const char *pageId, const char *task, const char *callback) {
    if (pageId == nullptr || task == nullptr)
        return;
#if JSAPI_LOG
    LOGD("[ExtendJSApi] handleCallNative >>>> pageId: %s, task: %s", pageId, task);
#endif
    if (strcmp(task, "[{\"module\":\"dom\",\"method\":\"createFinish\",\"args\":[]}]") == 0) {
        RenderManager::GetInstance()->CreateFinish(pageId) ? 0 : -1;
    } else {
        Bridge_Impl_Android::getInstance()->callNative(pageId, task, callback);
    }

};

std::unique_ptr<IPCResult>
_callNativeModule(const char *pageId, const char *module, const char *method,
                  const char *arguments, int argumentsLength, const char *options, int optionsLength) {
    std::unique_ptr<IPCResult> ret = createInt32Result(-1);
    if (pageId != nullptr && module != nullptr && method != nullptr) {
#if JSAPI_LOG
        LOGD("[ExtendJSApi] handleCallNativeModule >>>> pageId: %s, module: %s, method: %s, arg: %s, opt: %s",
         pageId, module, method, argString, optString);
#endif

        // add for android support
        jobject result;
        result = static_cast<jobject>(Bridge_Impl_Android::getInstance()->callNativeModule(pageId, module, method,
                                                                                           arguments, argumentsLength, options, optionsLength));
        if (result == nullptr){
            return ret;
        }

        JNIEnv *env = getJNIEnv();
        jfieldID jTypeId = env->GetFieldID(jWXJSObject, "type", "I");
        jint jTypeInt = env->GetIntField(result, jTypeId);
        jfieldID jDataId = env->GetFieldID(jWXJSObject, "data", "Ljava/lang/Object;");
        jobject jDataObj = env->GetObjectField(result, jDataId);
        if (jTypeInt == 1) {
            if (jDoubleValueMethodId == NULL) {
                jclass jDoubleClazz = env->FindClass("java/lang/Double");
                jDoubleValueMethodId = env->GetMethodID(jDoubleClazz, "doubleValue", "()D");
                env->DeleteLocalRef(jDoubleClazz);
            }
            jdouble jDoubleObj = env->CallDoubleMethod(jDataObj, jDoubleValueMethodId);
            ret = std::move(createDoubleResult(jDoubleObj));

        } else if (jTypeInt == 2) {
            jstring jDataStr = (jstring) jDataObj;
            ret = std::move(createStringResult(env, jDataStr));
        } else if (jTypeInt == 3) {
            jstring jDataStr = (jstring) jDataObj;
            ret = std::move(createJSONStringResult(env, jDataStr));
        } else if (jTypeInt == 4) {
            jbyteArray array = (jbyteArray)jDataObj;
            if(array != nullptr){
                int length = env->GetArrayLength(array);
                void* data = env->GetByteArrayElements(array, 0);
                ret = std::move(createByteArrayResult((const char*)data, length));
                env->ReleaseByteArrayElements(array, (jbyte*)data, 0);
            }
        }
        env->DeleteLocalRef(jDataObj);
        if(result != nullptr){
            env->DeleteLocalRef(result);
        }
    }

    return ret;
}

void _callNativeComponent(const char *pageId, const char *ref, const char *method,
                          const char *arguments, int argumentsLength, const char *options, int optionsLength) {
    if (pageId != nullptr && ref != nullptr && method != nullptr) {

#if JSAPI_LOG
        LOGD("[ExtendJSApi] handleCallNativeComponent >>>> pageId: %s, ref: %s, method: %s, arg: %s, opt: %s",
         pageId, ref, method, argString, optString);
#endif

        Bridge_Impl_Android::getInstance()->callNativeComponent(pageId, ref, method,
                                                                arguments, argumentsLength, options, optionsLength);
    }
}

void _callAddElement(const char *pageId, const char *parentRef, const char *domStr,
                     const char *index_cstr) {


    const char *indexChar = index_cstr == nullptr ? "\0" : index_cstr;

    int index = atoi(indexChar);
    if (pageId == nullptr || parentRef == nullptr || domStr == nullptr || index < -1)
        return;

#if JSAPI_LOG

    std::string log = "";
  log.append("pageId: ").append(pageId).append(", parentRef: ").append(parentRef).append(", domStr: ").append(domStr);
  int log_index = 0;
  int maxLength = 800;
  std::string sub;
  while (log_index < log.length()) {
    if (log.length() <= log_index + maxLength) {
      sub = log.substr(log_index);
    } else {
      sub = log.substr(log_index, maxLength);
    }


    if (log_index == 0)
      LOGD("[ExtendJSApi] functionCallAddElement >>>> %s", sub.c_str());
    else
      LOGD("      [ExtendJSApi] functionCallAddElement >>>> %s", sub.c_str());

    log_index += maxLength;
  }
#endif

    RenderManager::GetInstance()->AddRenderObject(pageId, parentRef, index, domStr);
}

void _setTimeout(const char *callbackId, const char *time) {
    Bridge_Impl_Android::getInstance()->setTimeout(callbackId, time);
}

void _callNativeLog(const char *str_array) {
    Bridge_Impl_Android::getInstance()->callNativeLog(str_array);
}

void _callCreateBody(const char *pageId, const char *domStr) {
#if JSAPI_LOG
    LOGD("[ExtendJSApi] functionCallCreateBody >>>> pageId: %s, domStr: %s", pageId, domStr);
#endif

    RenderManager::GetInstance()->CreatePage(pageId, domStr) ? 0 : -1;
}

int _callUpdateFinish(const char *pageId, const char *task, const char *callback) {
    return Bridge_Impl_Android::getInstance()->callUpdateFinish(pageId, task, callback);
}

void _callCreateFinish(const char *pageId) {
#if JSAPI_LOG
    LOGD("[ExtendJSApi] functionCallCreateFinish >>>> pageId: %s", pageId);
#endif

    RenderManager::GetInstance()->CreateFinish(pageId);
}

int _callRefreshFinish(const char *pageId, const char *task, const char *callback) {
    if (pageId == nullptr)
        return -1;
    return Bridge_Impl_Android::getInstance()->callRefreshFinish(pageId, task, callback);
}

void _callUpdateAttrs(const char *pageId, const char *ref, const char *data) {
#if JSAPI_LOG
    LOGD("[ExtendJSApi] functionCallUpdateAttrs >>>> pageId: %s, ref: %s, data: %s", pageId,
       ref, data);
#endif

    RenderManager::GetInstance()->UpdateAttr(pageId, ref, data);
}

void _callUpdateStyle(const char *pageId, const char *ref, const char *data) {
#if JSAPI_LOG
    LOGD("[ExtendJSApi] functionCallUpdateStyle >>>> pageId: %s, ref: %s, data: %s", pageId,
       ref, data);
#endif

    RenderManager::GetInstance()->UpdateStyle(pageId, ref, data);
}

void _callRemoveElement(const char *pageId, const char *ref) {
#if JSAPI_LOG
    LOGD("[ExtendJSApi] functionCallRemoveElement >>>> pageId: %s, ref: %s", pageId,
       ref);
#endif

    RenderManager::GetInstance()->RemoveRenderObject(pageId, ref);
}

void _callMoveElement(const char *pageId, const char *ref, const char *parentRef, int index) {
#if JSAPI_LOG
    LOGD("[ExtendJSApi] functionCallRemoveElement >>>> pageId: %s, ref: %s, parentRef: %s, index: %d",
       pageId, ref, parentRef, index);
#endif

    RenderManager::GetInstance()->MoveRenderObject(pageId, ref, parentRef, index);
}

void _callAddEvent(const char *pageId, const char *ref, const char *event) {
#if JSAPI_LOG
    LOGD("[ExtendJSApi] functionCallAddEvent >>>> pageId: %s, ref: %s, event: %s", pageId,
       ref, event);
#endif

    RenderManager::GetInstance()->AddEvent(pageId, ref, event);
}

void _callRemoveEvent(const char *pageId, const char *ref, const char *event) {
#if JSAPI_LOG
    LOGD("[ExtendJSApi] functionCallRemoveEvent >>>> pageId: %s, ref: %s, event: %s", pageId,
       ref, event);
#endif

    RenderManager::GetInstance()->RemoveEvent(pageId, ref, event);
}

int _setInterval(const char *pageId, const char *callbackId, const char *_time) {
    return (atoi(pageId) << 16) | (atoi(callbackId));
}

void _clearInterval(const char *pageId, const char *callbackId) {
    return;
}

const char *_callGCanvasLinkNative(const char *pageId, int type, const char *args) {
    const char *retVal = NULL;
    if (gCanvasFunc) {
        retVal = callGCanvasFun(gCanvasFunc, pageId, type, args);
    }

    return retVal;
}

const char *_t3dLinkNative(int type, const char *args) {
    const char *retVal = NULL;
    if (t3dFunc) {
        retVal = WeexCore::weexCallT3dFunc(t3dFunc, type, args);
    }

    return retVal;
}

void callHandlePostMessage(const char *vimId, const char *data) {
    JNIEnv *pEnv = getJNIEnv();
    jstring pJstring = pEnv->NewStringUTF(vimId);
    jbyteArray pArray = newJByteArray(pEnv, data);
    Bridge_Impl_Android::getInstance()->handlePostMessage(pJstring, pArray);
    pEnv->DeleteLocalRef(pJstring);
    pEnv->DeleteLocalRef(pArray);
}

void
callDIspatchMessage(const char *clientId, const char *data, const char *callback,
                    const char *vmId) {
    JNIEnv *pEnv = getJNIEnv();
    jstring pClientId = newJString(pEnv, clientId);
    jbyteArray pArray = newJByteArray(pEnv, data);
    jstring pCallback = newJString(pEnv, callback);
    jstring pVmId = newJString(pEnv, vmId);
    Bridge_Impl_Android::getInstance()->handleDispatchMessage(pClientId, pVmId, pArray, pCallback);
    pEnv->DeleteLocalRef(pClientId);
    pEnv->DeleteLocalRef(pArray);
    pEnv->DeleteLocalRef(pCallback);
    pEnv->DeleteLocalRef(pVmId);
}


