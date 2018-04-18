#ifndef _WEEX_CORE_H_
#define _WEEX_CORE_H_

#include <WeexCore/platform/android/base/LogUtils.h>
#include <WeexCore/platform/android/base/string/ScopedJString.h>
#include <WeexCore/platform/android/base/string/ScopedJStringUTF8.h>
#include <WeexCore/platform/android/jsengine/multiprocess/WeexJSConnection.h>
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

namespace WeexCore {

bool RegisterJNIUtils(JNIEnv *env);

jint OnLoad(JavaVM *vm, void *reserved);

void Unload(JavaVM *vm, void *reserved);

}

#endif //_WEEX_CORE_H_
