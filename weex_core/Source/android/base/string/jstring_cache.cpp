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

constexpr auto cmp = [](const char* a, const char* b){return strcmp(a,b);};

std::map<const char *, jobject, decltype(cmp)> mCache(cmp);

void JStringCache::clearRefCache(JNIEnv *env) {
    for (auto iter = mCache.begin(); iter != mCache.end(); iter++) {
        if (iter->second != nullptr) {
            env->DeleteWeakGlobalRef(iter->second);
            iter->second = nullptr;
        }
    }
    mCache.clear();
}

jstring JStringCache::GetString(JNIEnv *env, const char *key) {
    std::map<const char *, jobject>::iterator iter = mCache.find(key);
    if (iter != mCache.end() && env->IsSameObject(iter->second, NULL) == JNI_FALSE) {
        return (jstring) iter->second;
    } else {
        const jstring jRef = env->NewStringUTF(key);
        const jobject jGlobalRef = env->NewWeakGlobalRef(jRef);
        mCache.insert(std::pair<const char *, jobject>(key, jGlobalRef));
        env->DeleteLocalRef(jRef);
        return (jstring) jGlobalRef;
    }
}

