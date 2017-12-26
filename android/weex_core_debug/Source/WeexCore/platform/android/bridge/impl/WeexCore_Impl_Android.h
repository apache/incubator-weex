#ifndef _WEEX_CORE_H_
#define _WEEX_CORE_H_

#include <WeexCore/platform/android/base/LogUtils.h>
#include <WeexCore/platform/android/base/string/ScopedJString.h>
#include <WeexCore/platform/android/base/string/ScopedJStringUTF8.h>
#include <WeexCore/platform/android/jsengine/WeexJSConnection.h>

#include <IPC/Buffering/IPCBuffer.h>
#include <IPC/IPCArguments.h>
#include <IPC/IPCByteArray.h>
#include <IPC/IPCException.h>
#include <IPC/IPCHandler.h>
#include <IPC/IPCMessageJS.h>
#include <IPC/IPCResult.h>
#include <IPC/IPCSender.h>
#include <IPC/IPCString.h>
#include <IPC/Serializing/IPCSerializer.h>

#include <jni.h>
#include <string>
#include <unistd.h>

extern jclass jBridgeClazz;
extern jclass jWXJSObject;
extern jclass jWXLogUtils;
extern jmethodID jDoubleValueMethodId;
extern jobject jThis;
extern jclass jMapClazz;
extern jclass jSetClazz;
extern JNIEnv *getJNIEnv();

namespace WeexCore {

bool RegisterJNIUtils(JNIEnv *env);

jint OnLoad(JavaVM *vm, void *reserved);

void Unload(JavaVM *vm, void *reserved);

}

#endif //_WEEX_CORE_H_
