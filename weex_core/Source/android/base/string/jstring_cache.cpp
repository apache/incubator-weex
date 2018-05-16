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
#include "jstring_cache.h"

std::map<std::string, jobject> mCache;

void JStringCache::clearRefCache(JNIEnv *env) {
    for (auto iter = mCache.begin(); iter != mCache.end(); iter++) {
        if (iter->second != nullptr) {
            env->DeleteWeakGlobalRef(iter->second);
            iter->second = nullptr;
        }
    }
    mCache.clear();
}

jstring JStringCache::GetString(JNIEnv *env, std::string key) {
    std::map<std::string, jobject>::iterator iter = mCache.find(key);
    if (iter != mCache.end()) {
        // Has found
        if(env->IsSameObject(iter->second, NULL) == JNI_FALSE) {
            // JObject is still active
//            LOGE("FOUND cache jstring_cache GetString key: %s,for cache key: %s, map size: %d", key.c_str(), env->GetStringUTFChars((jstring) iter->second, false), mCache.size());
            return (jstring) iter->second;
        }
        if(env->IsSameObject(iter->second, NULL) == JNI_TRUE) {
            // Should delete WeakGlobalRef
            env->DeleteWeakGlobalRef(iter->second);
            iter->second = nullptr;
            mCache.erase(iter);
        }
    }
    const jstring jRef = env->NewStringUTF(key.c_str());
    const jobject jGlobalRef = env->NewWeakGlobalRef(jRef);
    mCache.insert(std::pair<std::string, jobject>(key, jGlobalRef));
    env->DeleteLocalRef(jRef);
    return (jstring) jGlobalRef;
}

