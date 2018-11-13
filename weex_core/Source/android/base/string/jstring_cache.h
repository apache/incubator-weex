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
#ifndef jstring_cache_h
#define jstring_cache_h

#include <jni.h>
#include <list>
#include <unordered_map>

class JStringCache;

class JStringCache {

public:
    JStringCache(int capacity) : capacity(capacity) {}

    ~JStringCache() {}

public:
    int capacity;
    std::list<std::pair<std::string, jobject>> cacheList;
    std::unordered_map<std::string, std::list<std::pair<std::string, jobject>>::iterator> posMap;
    jstring GetString(JNIEnv *env, std::string key);
    void clearRefCache(JNIEnv *env);

private:
    void put(JNIEnv *env, std::string key, jobject value);
};

#endif
