#include <iostream>
#include <map>
#include <android/base/log_utils.h>
#include "StringRefCache.h"


void StringRefCache::clearRefCache(JNIEnv *env) {
    for (auto iter = mCache.begin(); iter != mCache.end(); iter++) {
        if (iter->second != nullptr) {
            env->DeleteGlobalRef(iter->second);
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
        const jobject jGlobalRef = env->NewGlobalRef(jRef);
        mCache.insert(std::pair<const char *, jobject>(key, jGlobalRef));
        env->DeleteLocalRef(jRef);
        return (jstring) jGlobalRef;
    }
}

