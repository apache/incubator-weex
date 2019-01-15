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
#include "android/utils/cache_utils.h"
#include "android/base/string/jstring_cache.h"

namespace WeexCore {
std::map<std::string, jobject> componentTypeCache;
JStringCache refCache(2048);

jstring getComponentTypeFromCache(const std::string type) {
  std::map<std::string, jobject>::const_iterator iter =
      componentTypeCache.find(type);
  if (iter != componentTypeCache.end()) {
    return (jstring)(iter->second);
  } else {
    return nullptr;
  }
}

jstring putComponentTypeToCache(const std::string type) {
  JNIEnv *env = base::android::AttachCurrentThread();
  if (env == nullptr)
    return nullptr;

  jstring jType = env->NewStringUTF(type.c_str());
  jobject jGlobalType = env->NewGlobalRef(jType);
  componentTypeCache.insert(std::pair<std::string, jobject>(type, jGlobalType));
  env->DeleteLocalRef(jType);
  return (jstring)jGlobalType;
}

void clearComponentTypeCache() {
  JNIEnv *env = base::android::AttachCurrentThread();
  if (env == nullptr)
    return;

  for (auto iter = componentTypeCache.begin(); iter != componentTypeCache.end();
       iter++) {
    if (iter->second != nullptr) {
      env->DeleteGlobalRef(iter->second);
      iter->second = nullptr;
    }
  }
  componentTypeCache.clear();
}

jstring getKeyFromCache(JNIEnv *env, const char *key) {
  return refCache.GetString(env, key);
}
}  // namespace WeexCore
