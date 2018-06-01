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
#include <android/jsengine/multiprocess/ExtendJSApi.h>
#include <android/base/string/string_utils.h>
#include <wson/wson_parser.h>
#include <core/manager/weex_core_manager.h>

using namespace WeexCore;

extern WeexCore::FunType gCanvasFunc;
extern WeexCore::FunTypeT3d t3dFunc;

void _setJSVersion(const char *jsVersion) {
    LOGA("init JSFrm version %s", jsVersion);
    WeexCoreManager::getInstance()->getJSBridge()->onSetJSVersion(jsVersion);
}

void _reportException(const char *pageId, const char *func, const char *exception_string) {
    WeexCoreManager::getInstance()->getJSBridge()->onReportException(
            pageId,
            func,
            exception_string
    );
}

void _callNative(const char *pageId, const char *task, const char *callback) {
    WeexCoreManager::getInstance()->getJSBridge()->onCallNative(
            pageId,
            task,
            callback
    );


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
        result = static_cast<jobject>(
                WeexCoreManager::getInstance()->getJSBridge()->onCallNativeModule(
                        pageId, module, method, arguments,
                        argumentsLength, options,
                        optionsLength)
        );

        if (result == nullptr) {
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
            jbyteArray array = (jbyteArray) jDataObj;
            if (array != nullptr) {
                int length = env->GetArrayLength(array);
                void *data = env->GetByteArrayElements(array, 0);
                ret = std::move(createByteArrayResult((const char *) data, length));
                env->ReleaseByteArrayElements(array, (jbyte *) data, 0);
            }
        }
        env->DeleteLocalRef(jDataObj);
        if (result != nullptr) {
            env->DeleteLocalRef(result);
        }
    }

    return ret;
}

void _callNativeComponent(const char *pageId, const char *ref, const char *method,
                          const char *arguments, int argumentsLength, const char *options, int optionsLength) {
  WeexCoreManager::getInstance()->getJSBridge()->onCallNativeComponent(
          pageId, ref, method, arguments, argumentsLength, options, optionsLength
  );
}

void _callAddElement(const char *pageId, const char *parentRef, const char *domStr,
                     const char *index_cstr) {

  WeexCoreManager::getInstance()->getJSBridge()->onCallAddElement(pageId,parentRef,domStr,index_cstr);
}

void _setTimeout(const char *callbackId, const char *time) {
    WeexCoreManager::getInstance()->getJSBridge()->onSetTimeout(callbackId,time);
}

void _callNativeLog(const char *str_array) {
    WeexCoreManager::getInstance()->getJSBridge()->onCallNativeLog(str_array);
}

void _callCreateBody(const char *pageId, const char *domStr) {
    WeexCoreManager::getInstance()->getJSBridge()->onCallCreateBody(pageId,domStr);
}

int _callUpdateFinish(const char *pageId, const char *task, const char *callback) {
    return  WeexCoreManager::getInstance()->getJSBridge()->onCallUpdateFinish(pageId,task,callback);
}

void _callCreateFinish(const char *pageId) {
    WeexCoreManager::getInstance()->getJSBridge()->onCallCreateFinish(pageId);
}

int _callRefreshFinish(const char *pageId, const char *task, const char *callback) {
    return WeexCoreManager::getInstance()->getJSBridge()->onCallRefreshFinish(
            pageId,task,callback);
}

void _callUpdateAttrs(const char *pageId, const char *ref, const char *data) {
    WeexCoreManager::getInstance()->getJSBridge()->onCallUpdateAttrs(pageId,ref,data);
}

void _callUpdateStyle(const char *pageId, const char *ref, const char *data) {
    WeexCoreManager::getInstance()->getJSBridge()->onCallUpdateStyle(
            pageId, ref, data
    );
}

void _callRemoveElement(const char *pageId, const char *ref) {
    WeexCoreManager::getInstance()->getJSBridge()->onCallRemoveElement(pageId,ref);
}

void _callMoveElement(const char *pageId, const char *ref, const char *parentRef, int index) {
    WeexCoreManager::getInstance()->getJSBridge()->onCallMoveElement(
            pageId,ref,parentRef,index
    );
}

void _callAddEvent(const char *pageId, const char *ref, const char *event) {
    WeexCoreManager::getInstance()->getJSBridge()->onCallAddEvent(
            pageId,ref,event
    );
}

void _callRemoveEvent(const char *pageId, const char *ref, const char *event) {
    WeexCoreManager::getInstance()->getJSBridge()->onCallRemoveEvent(
            pageId,ref,event
    );
}

int _setInterval(const char *pageId, const char *callbackId, const char *_time) {
    return WeexCoreManager::getInstance()->getJSBridge()->onSetInterval(pageId,callbackId,_time);
}

void _clearInterval(const char *pageId, const char *callbackId) {
    WeexCoreManager::getInstance()->getJSBridge()->onClearInterval(pageId,callbackId);
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


