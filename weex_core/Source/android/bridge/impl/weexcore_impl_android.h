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
#ifndef _WEEX_CORE_H_
#define _WEEX_CORE_H_

#include <android/base/log_utils.h>
#include <android/base/string/scoped_jstring.h>
#include <android/base/string/scoped_jstring_utf8.h>
#include <android/jsengine/multiprocess/WeexJSConnection.h>
#include <android/base/string/jstring_cache.h>
#include <jni.h>
#include <string>
#include <unistd.h>
#include <map>
#include <string>

extern jclass jBridgeClazz;
extern jclass jWXJSObject;
extern jclass jWXLogUtils;
extern jmethodID jDoubleValueMethodId;
extern jobject jThis;
extern jobject jWMThis;
extern jclass jMapClazz;
extern jclass jSetClazz;
extern jmethodID jMapConstructorMethodId;
extern jmethodID jMapPutMethodId;
extern jmethodID jSetConstructorMethodId;
extern jmethodID jSetAddMethodId;
extern jclass jWMBridgeClazz;
extern JNIEnv *getJNIEnv();
extern jstring getComponentTypeFromCache(const std::string type);
extern jstring putComponentTypeToCache(const std::string type);
extern jstring getKeyFromCache(JNIEnv *env, const char *key);
extern jfloatArray c2jFloatArray(JNIEnv *env, const float c_array[]);

namespace WeexCore {

bool RegisterJNIUtils(JNIEnv *env);

jint OnLoad(JavaVM *vm, void *reserved);

void Unload(JavaVM *vm, void *reserved);

}

#endif //_WEEX_CORE_H_
