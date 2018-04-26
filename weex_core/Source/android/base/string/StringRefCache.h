#ifndef StringCache_h
#define StringCache_h

#include <jni.h>
#include <map>

class StringRefCache;

class StringRefCache {

public:
    StringRefCache() {}

    ~StringRefCache() {}

public:
    jstring GetString(JNIEnv *env, const char *key);
    void clearRefCache(JNIEnv *env);
};

#endif
