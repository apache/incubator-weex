//
// Created by Darin on 27/03/2018.
//

#include "WXJsFunction_Impl_Android.h"
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
                         arguments, jbyteArray options) {


#if JSAPI_LOG
    LOGD("[ExtendJSApi] handleCallNativeModule >>>> pageId: %s, module: %s, method: %s, arg: %s",
      jString2StrFast(env, jInstanceId).c_str(), jString2StrFast(env, jmodule).c_str(),
      jString2StrFast(env, jmethod).c_str(), jByteArray2Str(env, jArgString).c_str());
#endif

    // add for android support
    jobject result;
    result = Bridge_Impl_Android::getInstance()->callNativeModule(
            getCharFromJString(env, instanceId),
            getCharFromJString(env, module),
            getCharFromJString(env, method),
            getCharFromJByte(env, arguments),
            getCharFromJByte(env, options));

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
    }
    env->DeleteLocalRef(jDataObj);
    //return ret;
}

void
jsHandleCallNativeComponent(JNIEnv *env, jobject object, jstring instanceId, jstring componentRef,
                            jstring method,
                            jbyteArray arguments, jbyteArray options) {

    Bridge_Impl_Android::getInstance()->callNativeComponent(getCharFromJString(env, instanceId),
                                                            getCharFromJString(env, componentRef),
                                                            getCharFromJString(env, method),
                                                            getCharFromJByte(env, arguments),
                                                            getCharFromJByte(env, options));

}

void
jsHandleCallAddElement(JNIEnv *env, jobject object, jstring instanceId, jstring ref, jstring dom,
                       jstring index) {

    const char *instanceChar = env->GetStringUTFChars(instanceId, 0);
    const char *refChar = env->GetStringUTFChars(ref, 0);
    const char *domChar = env->GetStringUTFChars(dom, 0);
    const char *indexChar = env->GetStringUTFChars(index, 0);

    int indexI = atoi(indexChar);
    if (instanceChar == nullptr || refChar == nullptr || domChar == nullptr ||
        indexChar == nullptr ||
        indexI < -1)
        return;

    RenderManager::GetInstance()->AddRenderObject(instanceChar, refChar, indexI, domChar);
}

void jsHandleSetTimeout(JNIEnv *env, jobject object, jstring callbackId, jstring time) {

    Bridge_Impl_Android::getInstance()->setTimeout(getCharFromJString(env, callbackId),
                                                   getCharFromJString(env, time));

}

void jsHandleCallNativeLog(JNIEnv *env, jobject object, jbyteArray str_array) {
    Bridge_Impl_Android::getInstance()->callNativeLog(getCharFromJByte(env, str_array));
}

void jsFunctionCallCreateBody(JNIEnv *env, jobject object, jstring pageId, jstring domStr) {

    if (pageId == nullptr || domStr == nullptr)
        return;

    const char *page = env->GetStringUTFChars(pageId, NULL);
    const char *dom = env->GetStringUTFChars(domStr, NULL);
    if (page == nullptr || dom == nullptr || strlen(dom) == 0)
        return;
    RenderManager::GetInstance()->CreatePage(page, dom);
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
jsFunctionCallUpdateAttrs(JNIEnv *env, jobject object, jstring pageId, jstring ref, jstring data) {
    RenderManager::GetInstance()->UpdateAttr(env->GetStringUTFChars(pageId, 0),
                                             env->GetStringUTFChars(ref, 0),
                                             env->GetStringUTFChars(data, 0));
}

void
jsFunctionCallUpdateStyle(JNIEnv *env, jobject object, jstring pageId, jstring ref, jstring data) {
    RenderManager::GetInstance()->UpdateStyle(env->GetStringUTFChars(pageId, 0),
                                              env->GetStringUTFChars(ref, 0),
                                              env->GetStringUTFChars(data, 0));
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