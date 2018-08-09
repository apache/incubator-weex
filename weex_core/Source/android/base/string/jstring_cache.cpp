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
#include "jstring_cache.h"

void JStringCache::clearRefCache(JNIEnv *env) {
    for (auto iter = cacheList.begin(); iter != cacheList.end(); iter++) {
        std::pair<std::string, jobject> pair = *iter;
        if (nullptr != pair.second) {
            env->DeleteWeakGlobalRef(pair.second);
            pair.second = nullptr;
        }
    }
    if (!posMap.empty()) {
        posMap.clear();
    }
    cacheList.clear();
}

void JStringCache::put(JNIEnv *env, std::string key, jobject value) {
//    LOGD("Remove cache jstring_cache key: %s, find: %s, max: %s", key.c_str(), posMap.find(key) != posMap.end() ? "TRUE" : "FALSE", cacheList.size() >= capacity ? "TRUE" : "FALSE");
    if (posMap.find(key) != posMap.end()) {
        cacheList.erase(posMap[key]);
    } else if (cacheList.size() >= capacity) {
        env->DeleteWeakGlobalRef(cacheList.back().second);
        posMap.erase(cacheList.back().first);
        cacheList.pop_back();
    }
    cacheList.push_front({key, value});
    posMap[key] = cacheList.begin();
}

// TODO Should set max size to control performance
jstring JStringCache::GetString(JNIEnv *env, std::string key) {
//    LOGW("JStringCache map size: %d, list size: %d", posMap.size(), cacheList.size());
    if (posMap.find(key) != posMap.end()) {
        jobject obj = posMap[key]->second;
        if (env->IsSameObject(obj, NULL) == JNI_FALSE) {
            // JObject is still active
            put(env, key, obj);
//            LOGE("FOUND cache jstring_cache GetString key: %s,for cache key: %s", key.c_str(), env->GetStringUTFChars((jstring) obj, JNI_FALSE));
            return (jstring) posMap[key]->second;
        }
        else if (env->IsSameObject(obj, NULL) == JNI_TRUE) {
            // Should delete WeakGlobalRef.
//            LOGD("delete WeakGlobalRef: key: %s", key.c_str());
          cacheList.erase(posMap[key]);
          posMap.erase(key);
          env->DeleteWeakGlobalRef(obj);
        }
    }
    const jstring jRef = env->NewStringUTF(key.c_str());
    const jobject jGlobalRef = env->NewWeakGlobalRef(jRef);
    put(env, key, jGlobalRef);
    env->DeleteLocalRef(jRef);
    return (jstring) jGlobalRef;
}

