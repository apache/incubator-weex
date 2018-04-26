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

private:

    struct ptrCmp {
        bool operator()(const char *s1, const char *s2) const {
            return strcmp(s1, s2) < 0;
        }
    };

    std::map<const char *, jobject, ptrCmp> mCache;
};

#endif
