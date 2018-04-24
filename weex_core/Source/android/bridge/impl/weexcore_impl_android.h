#ifndef _WEEX_CORE_H_
#define _WEEX_CORE_H_

#include <android/base/log_utils.h>
#include <android/base/string/scoped_jstring.h>
#include <android/base/string/scoped_jstring_utf8.h>
#include <android/jsengine/multiprocess/WeexJSConnection.h>
#include <jni.h>
#include <string>
#include <unistd.h>
#include <map>
#include <string>

extern jclass jBridgeClazz;
extern jclass jWXJSObject;
extern jclass jWXLogUtils;
extern jmethodID jDoubleValueMethodId;
extern jobject jThis;
extern jclass jMapClazz;
extern jclass jSetClazz;
extern JNIEnv *getJNIEnv();
extern jstring getComponentTypeFromCache(const std::string type);
extern jstring putComponentTypeToCache(const std::string type);
extern jstring getStyleKeyFromCache(const std::string key);
extern jstring putStyleKeyToCache(const std::string key);

namespace WeexCore {

bool RegisterJNIUtils(JNIEnv *env);

jint OnLoad(JavaVM *vm, void *reserved);

void Unload(JavaVM *vm, void *reserved);

}

#endif //_WEEX_CORE_H_
