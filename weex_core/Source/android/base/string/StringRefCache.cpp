#include <iostream>
#include <map>
#include "StringRefCache.h"

constexpr auto cmp = [](const char* a, const char* b){return strcmp(a,b);};

std::map<const char *, jobject, decltype(cmp)> mCache(cmp);

void StringRefCache::clearRefCache(JNIEnv *env) {
    for (auto iter = mCache.begin(); iter != mCache.end(); iter++) {
        if (iter->second != nullptr) {
            env->DeleteWeakGlobalRef(iter->second);
            iter->second = nullptr;
        }
    }
    mCache.clear();
}

jstring StringRefCache::GetString(JNIEnv *env, const char *key) {
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

