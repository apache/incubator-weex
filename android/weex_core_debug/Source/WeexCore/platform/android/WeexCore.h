#ifndef _WEEX_CORE_H_
#define _WEEX_CORE_H_

#include <base/LogUtils.h>
#include <base/ScopedJString.h>
#include <base/ScopedJStringUTF8.h>
#include <base/StringUtils.h>
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

#include "WeexJSConnection.h"

#include <jni.h>
#include <string>
#include <unistd.h>

extern  jclass jBridgeClazz;
extern  jclass jWXJSObject;
extern  jclass jWXLogUtils;
extern  jmethodID jDoubleValueMethodId;
extern  jobject jThis;

extern JNIEnv* getJNIEnv();


#endif //_WEEX_CORE_H_
