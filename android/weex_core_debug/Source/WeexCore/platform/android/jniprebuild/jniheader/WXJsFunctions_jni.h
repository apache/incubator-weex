//
// Created by Darin on 27/03/2018.
//

#ifndef WEEX_PROJECT_WXJSFUNCTIONS_JNI_H
#define WEEX_PROJECT_WXJSFUNCTIONS_JNI_H

#include <jni.h>
#include "../../base/jni/android_jni.h"

namespace {
    const char kWXJsFunctionClassPath[] = "com/taobao/weex/bridge/WXJsFunctions";
// Leaking this jclass as we cannot use LazyInstance from some threads.
    jclass g_WXJsFunction_clazz = NULL;
#define WXJsFunction_clazz(env) g_WXJsFunction_clazz

}  // namespace

static void initWxBridge(JNIEnv *env, jobject object, jobject bridge, jstring className);

static void jsHandleSetJSVersion(JNIEnv *env, jobject object, jstring jsVersion);

static void jsHandleReportException(JNIEnv *env, jobject object, jstring instanceId, jstring func,
                                    jstring exceptionjstring);

static void jsHandleCallNative(JNIEnv *env, jobject object, jstring instanceId, jbyteArray tasks,
                               jstring callback);

static void
jsHandleCallNativeModule(JNIEnv *env, jobject object, jstring instanceId, jstring module,
                         jstring method, jbyteArray
                         arguments, jbyteArray options);

static void
jsHandleCallNativeComponent(JNIEnv *env, jobject object, jstring instanceId, jstring componentRef,
                            jstring method,
                            jbyteArray arguments, jbyteArray options);

static void
jsHandleCallAddElement(JNIEnv *env, jobject object, jstring instanceId, jstring ref, jstring dom,
                       jstring index,
                       jstring callback);

static void jsHandleSetTimeout(JNIEnv *env, jobject object, jstring callbackId, jstring time);

static void jsHandleCallNativeLog(JNIEnv *env, jobject object, jbyteArray str_array);

static void jsFunctionCallCreateBody(JNIEnv *env, jobject object, jstring pageId, jstring domStr);

static void
jsFunctionCallUpdateFinish(JNIEnv *env, jobject object, jstring instanceId, jbyteArray tasks,
                           jstring callback);

static void jsFunctionCallCreateFinish(JNIEnv *env, jobject object, jstring pageId);

static void
jsFunctionCallRefreshFinish(JNIEnv *env, jobject object, jstring instanceId, jbyteArray tasks,
                            jstring callback);

static void
jsFunctionCallUpdateAttrs(JNIEnv *env, jobject object, jstring pageId, jstring ref, jstring data);

static void
jsFunctionCallUpdateStyle(JNIEnv *env, jobject object, jstring pageId, jstring ref, jstring data);

static void jsFunctionCallRemoveElement(JNIEnv *env, jobject object, jstring pageId, jstring ref);

static void
jsFunctionCallMoveElement(JNIEnv *env, jobject object, jstring pageId, jstring ref,
                          jstring parentRef, jstring index_str);

static void
jsFunctionCallAddEvent(JNIEnv *env, jobject object, jstring pageId, jstring ref, jstring event);

static void
jsFunctionCallRemoveEvent(JNIEnv *env, jobject object, jstring pageId, jstring ref, jstring event);

static void jsHandleSetInterval(JNIEnv *env, jobject object, jstring instanceId, jstring callbackId,
                                jstring time);

static void
jsHandleClearInterval(JNIEnv *env, jobject object, jstring instanceId, jstring callbackId);

static void jsHandleCallGCanvasLinkNative(JNIEnv *env, jobject object, jstring contextId, int type,
                                          jstring val);


static const JNINativeMethod kMethodsWXJsFunctions[] = {
        {"initWxBridge",
                "(Ljava/lang/Object;Ljava/lang/String;)V",
                reinterpret_cast<void *>(initWxBridge)},
        {"jsHandleSetJSVersion",
                "(Ljava/lang/String;)V",
                reinterpret_cast<void *>(jsHandleSetJSVersion)},
        {"jsHandleReportException",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
                reinterpret_cast<void *>(jsHandleReportException)},
        {"jsHandleCallNative",
                "(Ljava/lang/String;[BLjava/lang/String;)V",
                reinterpret_cast<void *>(jsHandleCallNative)},
        {"jsHandleCallNativeModule",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[B[B)V",
                reinterpret_cast<void *>(jsHandleCallNativeModule)},
        {"jsHandleCallNativeComponent",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[B[B)V",
                reinterpret_cast<void *>(jsHandleCallNativeComponent)},
        {"jsHandleCallAddElement",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
                reinterpret_cast<void *>(jsHandleCallAddElement)},
        {"jsHandleSetTimeout",
                "(Ljava/lang/String;Ljava/lang/String;)V",
                reinterpret_cast<void *>(jsHandleSetTimeout)},
        {"jsHandleCallNativeLog",
                "([B)V",
                reinterpret_cast<void *>(jsHandleCallNativeLog)},
        {"jsFunctionCallCreateBody",
                "(Ljava/lang/String;Ljava/lang/String;)V",
                reinterpret_cast<void *>(jsFunctionCallCreateBody)},
        {"jsFunctionCallUpdateFinish",
                "(Ljava/lang/String;[BLjava/lang/String;)V",
                reinterpret_cast<void *>(jsFunctionCallUpdateFinish)},
        {"jsFunctionCallCreateFinish",
                "(Ljava/lang/String;)V",
                reinterpret_cast<void *>(jsFunctionCallCreateFinish)},
        {"jsFunctionCallRefreshFinish",
                "(Ljava/lang/String;[BLjava/lang/String;)V",
                reinterpret_cast<void *>(jsFunctionCallRefreshFinish)},
        {"jsFunctionCallUpdateAttrs",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
                reinterpret_cast<void *>(jsFunctionCallUpdateAttrs)},
        {"jsFunctionCallUpdateStyle",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
                reinterpret_cast<void *>(jsFunctionCallUpdateStyle)},
        {"jsFunctionCallRemoveElement",
                "(Ljava/lang/String;Ljava/lang/String;)V",
                reinterpret_cast<void *>(jsFunctionCallRemoveElement)},
        {"jsFunctionCallMoveElement",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
                reinterpret_cast<void *>(jsFunctionCallMoveElement)},
        {"jsFunctionCallAddEvent",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
                reinterpret_cast<void *>(jsFunctionCallAddEvent)},
        {"jsFunctionCallRemoveEvent",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
                reinterpret_cast<void *>(jsFunctionCallRemoveEvent)},
        {"jsHandleSetInterval",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
                reinterpret_cast<void *>(jsHandleSetInterval)},
        {"jsHandleClearInterval",
                "(Ljava/lang/String;Ljava/lang/String;)V",
                reinterpret_cast<void *>(jsHandleClearInterval)},
        {"jsHandleCallGCanvasLinkNative",
                "(Ljava/lang/String;ILjava/lang/String;)V",
                reinterpret_cast<void *>(jsHandleCallGCanvasLinkNative)}
};


static bool RegisterNativesImpl(JNIEnv *env) {

    g_WXJsFunction_clazz = reinterpret_cast<jclass>(env->NewGlobalRef(
            base::android::GetClass(env, kWXJsFunctionClassPath).Get()));

    const int kMethodsWXJsFunctionsSize =
            sizeof(kMethodsWXJsFunctions) / sizeof(kMethodsWXJsFunctions[0]);

    return 0 <= env->RegisterNatives(WXJsFunction_clazz(env),
                                     kMethodsWXJsFunctions,
                                     kMethodsWXJsFunctionsSize);

}

#endif //WEEX_PROJECT_WXJSFUNCTIONS_JNI_H
