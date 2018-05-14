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
#include <iostream>
#include <map>
#include <android/base/log_utils.h>
#include "jstring_cache.h"

constexpr auto cmp = [](const char* a, const char* b) -> bool {

    if(std::strcmp(a, b) == 0) {
        LOGW("TRUE !!!! strcmp match a: %s, b: %s, result: %d", a, b, strcmp(a, b));
        return true;
    } else{
        LOGD("FALSE !!! strcmp match a: %s, b: %s, result: %d", a, b, strcmp(a, b));
        return false;
    }
};

std::map<const char *, jobject, decltype(cmp)> mCache(cmp);

void JStringCache::clearRefCache(JNIEnv *env) {
//    for (auto iter = mCache.begin(); iter != mCache.end(); iter++) {
//        if (iter->second != nullptr) {
//            env->DeleteWeakGlobalRef(iter->second);
//            iter->second = nullptr;
//        }
//    }
//    mCache.clear();
}

char* jstringToChar(JNIEnv* env, jstring jstr) {
    char* rtn = NULL;
    jclass clsstring = env->FindClass("java/lang/String");
    jstring strencode = env->NewStringUTF("UTF-8");
    jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
    jbyteArray barr = (jbyteArray) env->CallObjectMethod(jstr, mid, strencode);
    jsize alen = env->GetArrayLength(barr);
    jbyte* ba = env->GetByteArrayElements(barr, JNI_FALSE);
    if (alen > 0) {
        rtn = (char*) malloc(alen + 1);
        memcpy(rtn, ba, alen);
        rtn[alen] = 0;
    }
    env->ReleaseByteArrayElements(barr, ba, 0);
    return rtn;
}

jstring JStringCache::GetString(JNIEnv *env, const char *key) {
    std::map<const char *, jobject>::iterator iter = mCache.find(key);
    std::string result = "";
    for(iter=mCache.begin(); iter!=mCache.end(); iter++) {
        result.append("[");
        result.append(iter->first);
        result.append(",");
        result.append(jstringToChar(env, (jstring) iter->second));
        result.append("] ");
    }

    LOGD("map find key: %s, cache end equals: %s, map size: %d, map is: %s", key, (iter == mCache.end() ? "TRUE" : "FALSE"), mCache.size(), result.c_str());
    if (iter != mCache.end()) {
        // Has found
        if(env->IsSameObject(iter->second, NULL) == JNI_FALSE) {
            // JObject is still active
            LOGE("FOUND cache jstring_cache GetString key: %s,for cache key: %s",key, jstringToChar(env, (jstring) iter->second));
            return (jstring) iter->second;
        }
        if(env->IsSameObject(iter->second, NULL) == JNI_TRUE) {
            // Should delete WeakGlobalRef
            LOGE("DELETE!!!! WeakGlobalRef key: %s", key);
            env->DeleteWeakGlobalRef(iter->second);
            iter->second = nullptr;
            mCache.erase(iter);
        }
    }
    const jstring jRef = env->NewStringUTF(key);
    const jobject jGlobalRef = env->NewWeakGlobalRef(jRef);
    mCache.insert(std::pair<const char *, jobject>(key, jGlobalRef));
    LOGE("NEW!!!! jstring_cache GetString new key: %s, insert cache key: %s", key, jstringToChar(env, (jstring) jGlobalRef));
    env->DeleteLocalRef(jRef);
    return (jstring) jGlobalRef;
}

