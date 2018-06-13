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
#ifndef _WEEX_PROXY_H_
#define _WEEX_PROXY_H_


#include <jni.h>
#include <unistd.h>
#include <IPC/Serializing/IPCSerializer.h>
#include <IPC/IPCHandler.h>
#include <android/bridge/impl/bridge_impl_android.h>
#include <IPC/Buffering/IPCBuffer.h>
#include <IPC/IPCArguments.h>
#include <IPC/IPCByteArray.h>
#include <IPC/IPCException.h>
#include <IPC/IPCHandler.h>
#include <IPC/IPCMessageJS.h>
#include <IPC/IPCResult.h>
#include <IPC/IPCSender.h>
#include <IPC/IPCString.h>
#include <IPC/Serializing/IPCSerializer.h>
#include "core/../include/WeexApiHeader.h"

namespace WeexCore {

class WeexProxy {
private:
public:
    static void reset();

    static jint doInitFramework(JNIEnv *env, jobject object, jstring script, jobject params,
                                jstring cacheDir, jboolean pieSupport);

    static jint doInitFramework(JNIEnv *env, jobject object, jstring script, jobject params);

    static std::vector<INIT_FRAMEWORK_PARAMS *>
    initFromParam(JNIEnv *env, jstring script, jobject params, IPCSerializer *serializer);

    static jint
    initFrameworkInSingleProcess(JNIEnv *env, jstring script,
                                    const std::vector<INIT_FRAMEWORK_PARAMS *> initFrameworkParams);
    static jint initFrameworkInMultiProcess(JNIEnv *env, jstring script, jobject params, IPCSerializer *serializer);

    static const char *getCacheDir(JNIEnv *env);

    static void setCacheDir(JNIEnv *env);

    static bool execJSService(JNIEnv *env, jobject object, jstring script);

    static std::string findLibJssSoPath();
    static bool
    execJS(JNIEnv *env, jobject jthis, jstring jinstanceid, jstring jnamespace, jstring jfunction,
           jobjectArray jargs);

    static void
    reportException(const char *instanceID, const char *func, const char *exception_string);

    static void reportServerCrash(jstring jinstanceid);
    static void reportNativeInitStatus(const char *statusCode, const char *errorMsg);
    static jbyteArray execJSWithResult(JNIEnv* env, jobject jthis,
                                       jstring jinstanceid,
                                       jstring jnamespace,
                                       jstring jfunction,
                                       jobjectArray jargs);

    static void updateGlobalConfig(JNIEnv* env, jobject jcaller,
                                 jstring config);
    static jint createInstanceContext(JNIEnv* env, jobject jcaller, jstring instanceId,
                                             jstring name,
                                             jstring function,
                                             jobjectArray args);

    static jint destoryInstance(JNIEnv* env, jobject jcaller,
                                jstring instanceId,
                                jstring name,
                                jstring function,
                                jobjectArray args);

    static jstring execJSOnInstance(JNIEnv* env, jobject jcaller,
                                    jstring instanceId,
                                    jstring script,
                                    jint type);
    static jint initAppFramework(JNIEnv* env,
                                    jobject jcaller,
                                    jstring jinstanceid,
                                    jstring jframwork,
                                    jobjectArray jargs);
    static jint destoryAppContext(JNIEnv* env,
                                  jobject jcaller,
                                  jstring jinstanceid);
    static jint createAppContext(JNIEnv* env,
                                 jobject jcaller,
                                 jstring jinstanceid,
                                 jstring jbundle,
                                 jobject jargs);
    static jbyteArray execJsOnAppWithResult(JNIEnv* env,
                                      jobject jcaller,
                                      jstring jinstanceid,
                                      jstring jbundle,
                                      jobject jargs);
    static jint execJsOnApp(JNIEnv* env,
                            jobject jcaller,
                            jstring jinstanceid,
                            jstring jfunction,
                            jobjectArray jargs);
};
}
#endif //_WEEX_PROXY_H_
