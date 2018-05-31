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
#include "jsfunction_impl_android.h"
#include "../../base/string/string_utils.h"
#include "../../jniprebuild/jniheader/WXJsFunctions_jni.h"
#include "bridge_impl_android.h"
#include "../../../core/render/manager/render_manager.h"

using namespace WeexCore;

extern jobject jThis;
extern jclass jBridgeClazz;

namespace WeexCore {
    bool RegisterWXJsFunction(JNIEnv *env) {
        RegisterNativesImpl(env);
    }
}

static const char *getCharFromJByte(JNIEnv *env, jbyteArray jbyteArray1) {
    if (jbyteArray1 == nullptr)
        return "";

    return jByteArray2Str(env, jbyteArray1).c_str();
}


static const int getJByteArraySize(JNIEnv *env, jbyteArray array){
    if(array == nullptr){
        return  0;
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
    jThis = env->NewGlobalRef(bridge);
    const char *classNameChar = env->GetStringUTFChars(className, 0);
    jclass tempClass = env->FindClass(classNameChar);
    jBridgeClazz = (jclass) env->NewGlobalRef(tempClass);
    Bridge_Impl_Android::getInstance()->setGlobalRef(jThis);
}

void jsHandleSetJSVersion(JNIEnv *env, jobject object, jstring jsVersion) {
    Bridge_Impl_Android::getInstance()->setJSVersion(getCharFromJString(env, jsVersion));
}

void jsHandleReportException(JNIEnv *env, jobject object, jstring instanceId, jstring func,
                             jstring exceptionjstring) {
    Bridge_Impl_Android::getInstance()->reportException(getCharFromJString(env, instanceId),
                                                        getCharFromJString(env, func),
                                                        getCharFromJString(env,
                                                                           exceptionjstring));
}

void jsHandleCallNative(JNIEnv *env, jobject object, jstring instanceId, jbyteArray tasks,
                        jstring callback) {
    std::string pageId = jString2StrFast(env, instanceId);
    std::string task = jByteArray2Str(env, tasks);

#if JSAPI_LOG
    LOGD("[ExtendJSApi] handleCallNative >>>> pageId: %s, task: %s", pageId.c_str(), task.c_str());
#endif

    if (task == "[{\"module\":\"dom\",\"method\":\"createFinish\",\"args\":[]}]") {
        RenderManager::GetInstance()->CreateFinish(pageId) ? 0 : -1;
        env->DeleteLocalRef(instanceId);
        env->DeleteLocalRef(tasks);
        env->DeleteLocalRef(callback);
    } else {
        Bridge_Impl_Android::getInstance()->callNative(getCharFromJString(env, instanceId),
                                                       task.c_str(),
                                                       getCharFromJString(env, callback));
    }
}

void
jsHandleCallNativeModule(JNIEnv *env, jobject object, jstring instanceId, jstring module,
                         jstring method, jbyteArray
                         arguments, jbyteArray options, jboolean from) {


#if JSAPI_LOG
    LOGD("[ExtendJSApi] handleCallNativeModule >>>> pageId: %s, module: %s, method: %s, arg: %s",
         jString2StrFast(env, instanceId).c_str(), jString2StrFast(env, module).c_str(),
         jString2StrFast(env, method).c_str(), jByteArray2Str(env, arguments).c_str());
#endif
    JByteArrayRef argumentsRef(env, arguments);
    JByteArrayRef optionsRef(env, options);


    // add for android support
    jobject result;
    result = static_cast<jobject>(Bridge_Impl_Android::getInstance()->callNativeModule(
                getCharFromJString(env, instanceId),
                getCharFromJString(env, module),
                getCharFromJString(env, method),
                argumentsRef.getBytes(),
                argumentsRef.length(),
                optionsRef.getBytes(),
                optionsRef.length()));

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
    } else if (jTypeInt == 2) {
        jstring jDataStr = (jstring) jDataObj;
        //ret = std::move(createStringResult(env, jDataStr));
    } else if (jTypeInt == 3) {
        jstring jDataStr = (jstring) jDataObj;
        //ret = std::move(createJSONStringResult(env, jDataStr));
    } else if (jTypeInt == 4) {
        jbyteArray array = (jbyteArray)jDataObj;
        if(array != nullptr){
            int length = env->GetArrayLength(array);
            void* data = env->GetByteArrayElements(array, 0);
            //ret = std::move(createByteArrayResult((const char*)data, length));
            env->ReleaseByteArrayElements(array, (jbyte*)data, 0);
        }
    }
    env->DeleteLocalRef(jDataObj);
    if(result != nullptr){
        env->DeleteLocalRef(result);
    }
    //return ret;
}

void
jsHandleCallNativeComponent(JNIEnv *env, jobject object, jstring instanceId, jstring componentRef,
                            jstring method,
                            jbyteArray arguments, jbyteArray options, jboolean from) {

    JByteArrayRef argumentsRef(env, arguments);
    JByteArrayRef optionsRef(env, options);
    Bridge_Impl_Android::getInstance()->callNativeComponent(getCharFromJString(env, instanceId),
                                                            getCharFromJString(env, componentRef),
                                                            getCharFromJString(env, method),
                                                            argumentsRef.getBytes(),
                                                            argumentsRef.length(),
                                                            optionsRef.getBytes(),
                                                            optionsRef.length());

}

void
jsHandleCallAddElement(JNIEnv *env, jobject object, jstring instanceId, jstring ref, jbyteArray dom,
                       jstring index) {

    const char *instanceChar = env->GetStringUTFChars(instanceId, 0);
    const char *refChar = env->GetStringUTFChars(ref, 0);
    JByteArrayRef domRef(env, dom);
    const char *indexChar = env->GetStringUTFChars(index, 0);

    int indexI = atoi(indexChar);
    if (instanceChar == nullptr || refChar == nullptr || domRef.length() == 0 ||
        indexChar == nullptr ||
        indexI < -1)
        return;

    RenderManager::GetInstance()->AddRenderObject(instanceChar, refChar, indexI,  domRef.getBytes());
}

void jsHandleSetTimeout(JNIEnv *env, jobject object, jstring callbackId, jstring time) {

    Bridge_Impl_Android::getInstance()->setTimeout(getCharFromJString(env, callbackId),
                                                   getCharFromJString(env, time));

}

void jsHandleCallNativeLog(JNIEnv *env, jobject object, jbyteArray str_array) {
    Bridge_Impl_Android::getInstance()->callNativeLog(getCharFromJByte(env, str_array));
}

void jsFunctionCallCreateBody(JNIEnv *env, jobject object, jstring pageId, jbyteArray domStr, jboolean from) {
    if (pageId == nullptr || domStr == nullptr)
        return;

    const char *page = env->GetStringUTFChars(pageId, NULL);
    JByteArrayRef dom(env, domStr);
    if (page == nullptr || dom.length() == 0)
        return;
    RenderManager::GetInstance()->CreatePage(page, dom.getBytes());
}

void
jsFunctionCallUpdateFinish(JNIEnv *env, jobject object, jstring instanceId, jbyteArray tasks,
                           jstring callback) {

    Bridge_Impl_Android::getInstance()->callUpdateFinish(getCharFromJString(env, instanceId),
                                                         getCharFromJByte(env, tasks),
                                                         getCharFromJString(env, callback));

}

void jsFunctionCallCreateFinish(JNIEnv *env, jobject object, jstring pageId) {
    RenderManager::GetInstance()->CreateFinish(env->GetStringUTFChars(pageId, 0));
}

void
jsFunctionCallRefreshFinish(JNIEnv *env, jobject object, jstring instanceId, jbyteArray tasks,
                            jstring callback) {
    Bridge_Impl_Android::getInstance()->callRefreshFinish(getCharFromJString(env, instanceId),
                                                          getCharFromJByte(env, tasks),
                                                          getCharFromJString(env, callback));
}

void
jsFunctionCallUpdateAttrs(JNIEnv *env, jobject object, jstring pageId, jstring ref, jbyteArray data, jboolean from) {

    JByteArrayRef dataRef(env, data);
    RenderManager::GetInstance()->UpdateAttr(env->GetStringUTFChars(pageId, 0),
                                             env->GetStringUTFChars(ref, 0),
                                             dataRef.getBytes());
}

void
jsFunctionCallUpdateStyle(JNIEnv *env, jobject object, jstring pageId, jstring ref, jbyteArray data, jboolean from) {

    JByteArrayRef dataRef(env, data);
    RenderManager::GetInstance()->UpdateStyle(env->GetStringUTFChars(pageId, 0),
                                              env->GetStringUTFChars(ref, 0),
                                              dataRef.getBytes());
}

void jsFunctionCallRemoveElement(JNIEnv *env, jobject object, jstring pageId, jstring ref) {

    RenderManager::GetInstance()->RemoveRenderObject(env->GetStringUTFChars(pageId, 0),
                                                     env->GetStringUTFChars(ref, 0));

}

void
jsFunctionCallMoveElement(JNIEnv *env, jobject object, jstring pageId, jstring ref,
                          jstring parentRef, jstring index_str) {

    int index = atoi(env->GetStringUTFChars(index_str, 0));

    RenderManager::GetInstance()->MoveRenderObject(env->GetStringUTFChars(pageId, 0),
                                                   env->GetStringUTFChars(ref, 0),
                                                   env->GetStringUTFChars(parentRef, 0), index);

}

void
jsFunctionCallAddEvent(JNIEnv *env, jobject object, jstring pageId, jstring ref, jstring event) {
    RenderManager::GetInstance()->AddEvent(env->GetStringUTFChars(pageId, 0),
                                           env->GetStringUTFChars(ref, 0),
                                           env->GetStringUTFChars(event, 0));

}

void
jsFunctionCallRemoveEvent(JNIEnv *env, jobject object, jstring pageId, jstring ref,
                          jstring event) {

    RenderManager::GetInstance()->RemoveEvent(env->GetStringUTFChars(pageId, 0),
                                              env->GetStringUTFChars(ref, 0),
                                              env->GetStringUTFChars(event, 0));

}

void jsHandleSetInterval(JNIEnv *env, jobject object, jstring instanceId, jstring callbackId,
                         jstring time) {}

void
jsHandleClearInterval(JNIEnv *env, jobject object, jstring instanceId, jstring callbackId) {}

void jsHandleCallGCanvasLinkNative(JNIEnv *env, jobject object, jstring contextId, int type,
                                   jstring val) {}