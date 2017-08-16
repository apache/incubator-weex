#ifndef _WEEX_CORE_H_
#define _WEEX_CORE_H_

#include "../base/LogUtils.h"
#include "../base/ScopedJString.h"
#include "../base/ScopedJStringUTF8.h"
#include "../base/StringUtils.h"
#include "Buffering/IPCBuffer.h"
#include "IPCArguments.h"
#include "IPCByteArray.h"
#include "IPCException.h"
#include "IPCHandler.h"
#include "IPCMessageJS.h"
#include "IPCResult.h"
#include "IPCSender.h"
#include "IPCString.h"
#include "Serializing/IPCSerializer.h"
#include "Trace.h"
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
