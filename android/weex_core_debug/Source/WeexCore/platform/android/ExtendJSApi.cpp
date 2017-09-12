#include "ExtendJSApi.h"
#include "WeexCore.h"
#include "BridgeAndroid.h"

using namespace WeexCore;

static bool isWeexCore = true;

/**
* This class aim to extend JS Api
**/
ExtendJSApi::ExtendJSApi() {
}

void ExtendJSApi::initFunction(IPCHandler *handler) {
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::SETJSFVERSION), handleSetJSVersion);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::REPORTEXCEPTION), handleReportException);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLNATIVE), handleCallNative);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLNATIVEMODULE), handleCallNativeModule);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLNATIVECOMPONENT), handleCallNativeComponent);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLADDELEMENT), handleCallAddElement);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::SETTIMEOUT), handleSetTimeout);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::NATIVELOG), handleCallNativeLog);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLCREATEBODY), functionCallCreateBody);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLUPDATEFINISH), functionCallUpdateFinish);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLCREATEFINISH), functionCallCreateFinish);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLREFRESHFINISH), functionCallRefreshFinish);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLUPDATEATTRS), functionCallUpdateAttrs);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLUPDATESTYLE), functionCallUpdateStyle);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLREMOVEELEMENT), functionCallRemoveElement);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLMOVEELEMENT), functionCallMoveElement);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLADDEVENT), functionCallAddEvent);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLREMOVEEVENT), functionCallRemoveEvent);
}

std::unique_ptr<IPCResult> handleSetJSVersion(IPCArguments *arguments) {
  JNIEnv *env = getJNIEnv();
  const IPCByteArray *ba = arguments->getByteArray(0);
  LOGA("init JSFrm version %s", ba->content);
  jstring jversion = env->NewStringUTF(ba->content);
  BridgeAndroid::getInstance()->setJSVersion(jversion);
  return createVoidResult();
}

void reportException(const char *instanceID, const char *func, const char *exception_string) {
  JNIEnv *env = getJNIEnv();
  jstring jExceptionString = env->NewStringUTF(exception_string);
  jstring jInstanceId = env->NewStringUTF(instanceID);
  jstring jFunc = env->NewStringUTF(func);
  BridgeAndroid::getInstance()->reportException(jInstanceId, jFunc, jExceptionString);
}

std::unique_ptr<IPCResult> handleReportException(IPCArguments *arguments) {
  const char *instanceID = nullptr;
  const char *func = nullptr;
  const char *exceptionInfo = nullptr;
  if (arguments->getType(0) == IPCType::BYTEARRAY) {
    const IPCByteArray *instanceIDBA = arguments->getByteArray(0);
    instanceID = instanceIDBA->content;
  }
  if (arguments->getType(1) == IPCType::BYTEARRAY) {
    const IPCByteArray *funcBA = arguments->getByteArray(1);
    func = funcBA->content;
  }
  if (arguments->getType(2) == IPCType::BYTEARRAY) {
    const IPCByteArray *exceptionInfoBA = arguments->getByteArray(2);
    exceptionInfo = exceptionInfoBA->content;
  }
  LOGE(" ReportException : %s", exceptionInfo);
  reportException(instanceID, func, exceptionInfo);
  return createVoidResult();
}

std::unique_ptr<IPCResult> handleCallNative(IPCArguments *arguments) {
  JNIEnv *env = getJNIEnv();
  //instacneID args[0]
  jstring jInstanceId = getArgumentAsJString(env, arguments, 0);
  //task args[1]
  jbyteArray jTaskString = getArgumentAsJByteArray(env, arguments, 1);
  //callback args[2]
  jstring jCallback = getArgumentAsJString(env, arguments, 2);

  int flag = 0;
  flag = BridgeAndroid::getInstance()->callNative(jInstanceId, jTaskString, jCallback);

  return createInt32Result(flag);
}

std::unique_ptr<IPCResult> functionCallCreateBody(IPCArguments *arguments) {
  JNIEnv *env = getJNIEnv();
  //instacneID args[0]
  jstring jInstanceId = getArgumentAsJString(env, arguments, 0);
  //task args[1]
  jbyteArray jTaskString = getArgumentAsJByteArray(env, arguments, 1);
  //callback args[2]
  jstring jCallback = getArgumentAsJString(env, arguments, 2);

  if (isWeexCore) {
    RenderManager::getInstance()->createPage(jString2Str(env, jInstanceId),
                                             jByteArray2Str(env, jTaskString));
  }

  int flag = 0;
  flag = BridgeAndroid::getInstance()->callCreateBody(jInstanceId, jTaskString, jCallback);

  return createInt32Result(flag);
}

std::unique_ptr<IPCResult> functionCallUpdateFinish(IPCArguments *arguments) {
  JNIEnv *env = getJNIEnv();
  //instacneID args[0]
  jstring jInstanceId = getArgumentAsJString(env, arguments, 0);
  //task args[1]
  jbyteArray jTaskString = getArgumentAsJByteArray(env, arguments, 1);
  //callback args[2]
  jstring jCallback = getArgumentAsJString(env, arguments, 2);

  int flag = 0;
  flag = BridgeAndroid::getInstance()->callUpdateFinish(jInstanceId, jTaskString, jCallback);

  return createInt32Result(flag);
}

std::unique_ptr<IPCResult> functionCallCreateFinish(IPCArguments *arguments) {
  JNIEnv *env = getJNIEnv();
  //instacneID args[0]
  jstring jInstanceId = getArgumentAsJString(env, arguments, 0);
  //task args[1]
  jbyteArray jTaskString = getArgumentAsJByteArray(env, arguments, 1);
  //callback args[2]
  jstring jCallback = getArgumentAsJString(env, arguments, 2);

  int flag = 0;
  flag = BridgeAndroid::getInstance()->callCreateFinish(jInstanceId, jTaskString, jCallback);

  return createInt32Result(flag);
}

std::unique_ptr<IPCResult> functionCallRefreshFinish(IPCArguments *arguments) {
  JNIEnv *env = getJNIEnv();
  //instacneID args[0]
  jstring jInstanceId = getArgumentAsJString(env, arguments, 0);
  //task args[1]
  jbyteArray jTaskString = getArgumentAsJByteArray(env, arguments, 1);
  //callback args[2]
  jstring jCallback = getArgumentAsJString(env, arguments, 2);

  int flag = 0;
  flag = BridgeAndroid::getInstance()->callRefreshFinish(jInstanceId, jTaskString, jCallback);

  return createInt32Result(flag);
}

std::unique_ptr<IPCResult> functionCallUpdateAttrs(IPCArguments *arguments) {
  JNIEnv *env = getJNIEnv();
  //instacneID args[0]
  jstring jInstanceId = getArgumentAsJString(env, arguments, 0);

  //instacneID args[1]
  jstring jRef = getArgumentAsJString(env, arguments, 1);

  //task args[2]
  jbyteArray jTaskString = getArgumentAsJByteArray(env, arguments, 2);

  //callback args[2]
  jstring jCallback = getArgumentAsJString(env, arguments, 3);

  int flag = 0;
  flag = BridgeAndroid::getInstance()->callUpdateAttrs(jInstanceId, jRef, jTaskString, jCallback);

  return createInt32Result(flag);
}

std::unique_ptr<IPCResult> functionCallUpdateStyle(IPCArguments *arguments) {
  JNIEnv *env = getJNIEnv();
  //instacneID args[0]
  jstring jInstanceId = getArgumentAsJString(env, arguments, 0);

  //instacneID args[1]
  jstring jRef = getArgumentAsJString(env, arguments, 1);

  //task args[2]
  jbyteArray jTaskString = getArgumentAsJByteArray(env, arguments, 2);

  //callback args[2]
  jstring jCallback = getArgumentAsJString(env, arguments, 3);

  int flag = 0;
  flag = BridgeAndroid::getInstance()->callUpdateStyle(jInstanceId, jRef, jTaskString, jCallback);

  return createInt32Result(flag);
}

std::unique_ptr<IPCResult> functionCallRemoveElement(IPCArguments *arguments) {
  JNIEnv *env = getJNIEnv();
  //instacneID args[0]
  jstring jInstanceId = getArgumentAsJString(env, arguments, 0);

  //instacneID args[1]
  jstring jRef = getArgumentAsJString(env, arguments, 1);

  //callback args[2]
  jstring jCallback = getArgumentAsJString(env, arguments, 2);

  int flag = 0;
  flag = BridgeAndroid::getInstance()->callRemoveElement(jInstanceId, jRef, jCallback);

  return createInt32Result(flag);
}

std::unique_ptr<IPCResult> functionCallMoveElement(IPCArguments *arguments) {
  JNIEnv *env = getJNIEnv();
  //instacneID args[0]
  jstring jInstanceId = getArgumentAsJString(env, arguments, 0);

  //instacneID args[1]
  jstring jRef = getArgumentAsJString(env, arguments, 1);

  //callback args[2]
  jstring jParentRef = getArgumentAsJString(env, arguments, 2);

  //callback args[3]
  jstring jIndex = getArgumentAsJString(env, arguments, 3);

  //callback args[4]
  jstring jCallback = getArgumentAsJString(env, arguments, 4);

  int flag = 0;
  flag = BridgeAndroid::getInstance()->callMoveElement(jInstanceId, jRef, jParentRef, jIndex,
                                                       jCallback);

  return createInt32Result(flag);
}

std::unique_ptr<IPCResult> functionCallAddEvent(IPCArguments *arguments) {
  LOGE("functionCallAddEvent");
  JNIEnv *env = getJNIEnv();
  //instacneID args[0]
  jstring jInstanceId = getArgumentAsJString(env, arguments, 0);

  //instacneID args[1]
  jstring jRef = getArgumentAsJString(env, arguments, 1);

  //callback args[2]
  jstring jEvent = getArgumentAsJString(env, arguments, 2);

  //callback args[3]
  jstring jCallback = getArgumentAsJString(env, arguments, 3);

  int flag = 0;
  flag = BridgeAndroid::getInstance()->callAddEvent(jInstanceId, jRef, jEvent, jCallback);

  return createInt32Result(flag);
}

std::unique_ptr<IPCResult> functionCallRemoveEvent(IPCArguments *arguments) {
  LOGE("functionCallRemoveEvent");
  JNIEnv *env = getJNIEnv();
  //instacneID args[0]
  jstring jInstanceId = getArgumentAsJString(env, arguments, 0);

  //instacneID args[1]
  jstring jRef = getArgumentAsJString(env, arguments, 1);

  //callback args[2]
  jstring jEvent = getArgumentAsJString(env, arguments, 2);

  //callback args[3]
  jstring jCallback = getArgumentAsJString(env, arguments, 3);

  int flag = 0;
  flag = BridgeAndroid::getInstance()->callRemoveEvent(jInstanceId, jRef, jEvent, jCallback);

  return createInt32Result(flag);
}

std::unique_ptr<IPCResult> handleCallNativeModule(IPCArguments *arguments) {

  JNIEnv *env = getJNIEnv();
  //instacneID args[0]
  jstring jInstanceId = getArgumentAsJString(env, arguments, 0);

  //module args[1]
  jstring jmodule = getArgumentAsJString(env, arguments, 1);

  //method args[2]
  jstring jmethod = getArgumentAsJString(env, arguments, 2);

  // arguments args[3]
  jbyteArray jArgString = getArgumentAsJByteArray(env, arguments, 3);
  //arguments args[4]
  jbyteArray jOptString = getArgumentAsJByteArray(env, arguments, 4);

  // add for android support
  jobject result;
  result = BridgeAndroid::getInstance()->callNativeModule(jInstanceId, jmodule, jmethod, jArgString,
                                                          jOptString);

  std::unique_ptr<IPCResult> ret;
  jfieldID jTypeId = env->GetFieldID(jWXJSObject, "type", "I");
  jint jTypeInt = env->GetIntField(result, jTypeId);
  jfieldID jDataId = env->GetFieldID(jWXJSObject, "data", "Ljava/lang/Object;");
  jobject jDataObj = env->GetObjectField(result, jDataId);
  if (jTypeInt == 1) {
    if (jDoubleValueMethodId == NULL) {
      jclass jDoubleClazz = env->FindClass("java/lang/Double");
      jDoubleValueMethodId = env->GetMethodID(jDoubleClazz, "doubleValue", "()D");
      env->DeleteLocalRef(jDoubleClazz);
    }
    jdouble jDoubleObj = env->CallDoubleMethod(jDataObj, jDoubleValueMethodId);
    ret = std::move(createDoubleResult(jDoubleObj));

  } else if (jTypeInt == 2) {
    jstring jDataStr = (jstring) jDataObj;
    ret = std::move(createStringResult(env, jDataStr));
  } else if (jTypeInt == 3) {
    jstring jDataStr = (jstring) jDataObj;
    ret = std::move(createJSONStringResult(env, jDataStr));
  }
  env->DeleteLocalRef(jDataObj);

  return ret;
}

std::unique_ptr<IPCResult> handleCallNativeComponent(IPCArguments *arguments) {
  JNIEnv *env = getJNIEnv();

  //instacneID args[0]
  jstring jInstanceId = getArgumentAsJString(env, arguments, 0);

  //module args[1]
  jstring jcomponentRef = getArgumentAsJString(env, arguments, 1);

  //method args[2]
  jstring jmethod = getArgumentAsJString(env, arguments, 2);

  // arguments args[3]
  jbyteArray jArgString = getArgumentAsJByteArray(env, arguments, 3);

  //arguments args[4]
  jbyteArray jOptString = getArgumentAsJByteArray(env, arguments, 4);

  BridgeAndroid::getInstance()->callNativeComponent(jInstanceId, jcomponentRef, jmethod, jArgString,
                                                    jOptString);

  return createInt32Result(static_cast<int32_t>(true));
}

std::unique_ptr<IPCResult> handleCallAddElement(IPCArguments *arguments) {
  JNIEnv *env = getJNIEnv();
  //instacneID args[0]
  jstring jInstanceId = getArgumentAsJString(env, arguments, 0);
  //instacneID args[1]
  jstring jref = getArgumentAsJString(env, arguments, 1);
  //dom node args[2]
  jbyteArray jdomString = getArgumentAsJByteArray(env, arguments, 2);
  //index  args[3]
  jstring jindex = getArgumentAsJString(env, arguments, 3);
  //callback  args[4]
  jstring jCallback = getArgumentAsJString(env, arguments, 4);

  if (isWeexCore) {
    std::string pageId = jString2Str(env, jInstanceId);
    std::string parentRef = jString2Str(env, jref);
    std::string str_index = jString2Str(env, jindex);
    int index = stringToNum<int>(str_index);
    std::string data = jByteArray2Str(env, jdomString);

    RenderManager::getInstance()->addRenderObject(pageId, parentRef, index, data);
    RenderManager::getInstance()->printRenderAndLayoutTree(jString2Str(env, jInstanceId));
  }

  int flag = 0;
  flag = BridgeAndroid::getInstance()->callAddElement(jInstanceId, jref, jdomString, jindex,
                                                      jCallback);

  return createInt32Result(flag);
}

std::unique_ptr<IPCResult> handleSetTimeout(IPCArguments *arguments) {
  JNIEnv *env = getJNIEnv();
  //callbackId
  jstring jCallbackID = getArgumentAsJString(env, arguments, 0);

  //time
  jstring jTime = getArgumentAsJString(env, arguments, 1);

  BridgeAndroid::getInstance()->setTimeout(jCallbackID, jTime);

  return createInt32Result(static_cast<int32_t>(true));
}

std::unique_ptr<IPCResult> handleCallNativeLog(IPCArguments *arguments) {
  JNIEnv *env = getJNIEnv();
  jstring str_msg = getArgumentAsJString(env, arguments, 0);
  BridgeAndroid::getInstance()->callNativeLog(str_msg);

  return createInt32Result(static_cast<int32_t>(true));
}
