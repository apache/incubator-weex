#include <WeexCore/platform/android/base/string/StringUtils.h>
#include "ExtendJSApi.h"

using namespace WeexCore;

/**
* This class aim to extend JS Api
**/
ExtendJSApi::ExtendJSApi() {
}

void ExtendJSApi::initFunction(IPCHandler *handler) {
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::SETJSFVERSION), handleSetJSVersion);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::REPORTEXCEPTION),
                           handleReportException);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLNATIVE), handleCallNative);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLNATIVEMODULE),
                           handleCallNativeModule);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLNATIVECOMPONENT),
                           handleCallNativeComponent);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLADDELEMENT),
                           handleCallAddElement);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::SETTIMEOUT), handleSetTimeout);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::NATIVELOG), handleCallNativeLog);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLCREATEBODY),
                           functionCallCreateBody);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLUPDATEFINISH),
                           functionCallUpdateFinish);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLCREATEFINISH),
                           functionCallCreateFinish);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLREFRESHFINISH),
                           functionCallRefreshFinish);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLUPDATEATTRS),
                           functionCallUpdateAttrs);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLUPDATESTYLE),
                           functionCallUpdateStyle);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLREMOVEELEMENT),
                           functionCallRemoveElement);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLMOVEELEMENT),
                           functionCallMoveElement);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLADDEVENT), functionCallAddEvent);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLREMOVEEVENT),
                           functionCallRemoveEvent);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::SETINTERVAL), handleSetInterval);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CLEARINTERVAL), handleClearInterval);
}

std::unique_ptr<IPCResult> handleSetJSVersion(IPCArguments *arguments) {
  JNIEnv *env = getJNIEnv();
  const IPCByteArray *ba = arguments->getByteArray(0);
  LOGA("init JSFrm version %s", ba->content);
  jstring jversion = env->NewStringUTF(ba->content);
  Bridge_Impl_Android::getInstance()->setJSVersion(jversion);
  return createVoidResult();
}

void reportException(const char *instanceID, const char *func, const char *exception_string) {
  JNIEnv *env = getJNIEnv();
  jstring jExceptionString = env->NewStringUTF(exception_string);
  jstring jInstanceId = env->NewStringUTF(instanceID);
  jstring jFunc = env->NewStringUTF(func);
  Bridge_Impl_Android::getInstance()->reportException(jInstanceId, jFunc, jExceptionString);
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

std::unique_ptr<IPCResult> handleSetTimeout(IPCArguments *arguments) {
  JNIEnv *env = getJNIEnv();
  jstring jCallbackID = getArgumentAsJString(env, arguments, 0);
  jstring jTime = getArgumentAsJString(env, arguments, 1);

  Bridge_Impl_Android::getInstance()->setTimeout(jCallbackID, jTime);

  return createInt32Result(static_cast<int32_t>(true));
}

std::unique_ptr<IPCResult> handleCallNativeLog(IPCArguments *arguments) {
  JNIEnv *env = getJNIEnv();
  jbyteArray str_array = getArgumentAsJByteArray(env, arguments, 0);
  Bridge_Impl_Android::getInstance()->callNativeLog(str_array);
  return createInt32Result(static_cast<int32_t>(true));
}


std::unique_ptr<IPCResult> handleSetInterval(IPCArguments *arguments) {
  JNIEnv *env = getJNIEnv();
  jstring jInstanceId = getArgumentAsJString(env, arguments, 0);
  const char *instanceID = env->GetStringUTFChars(jInstanceId, NULL);
  jstring jCallbackId = getArgumentAsJString(env, arguments, 1);
  const char *callbackID = env->GetStringUTFChars(jCallbackId, NULL);
  jstring jTime = getArgumentAsJString(env, arguments, 2);
  const char *_time = env->GetStringUTFChars(jTime, NULL);
  long time_ = atoi(_time);
//  LOGE("functionSetIntervalWeex instanceId:%s time_:%ld callbackID:%s", instanceID, time_,
//       callbackID);
  int _timerId = (atoi(instanceID) << 16) | (atoi(callbackID));
  env->DeleteLocalRef(jInstanceId);
  env->DeleteLocalRef(jCallbackId);
  env->DeleteLocalRef(jTime);
  return createInt32Result(_timerId);
}

std::unique_ptr<IPCResult> handleClearInterval(IPCArguments *arguments) {
  JNIEnv *env = getJNIEnv();
  jstring jInstanceId = getArgumentAsJString(env, arguments, 0);
  const char *instanceID = env->GetStringUTFChars(jInstanceId, NULL);
  jstring jCallbackId = getArgumentAsJString(env, arguments, 1);
  const char *callbackID = env->GetStringUTFChars(jCallbackId, NULL);
//  LOGE("functionClearIntervalWeex instanceID:%s, callbackID:%s", instanceID, callbackID);
  long id = atoi(callbackID);

  env->DeleteLocalRef(jInstanceId);
  env->DeleteLocalRef(jCallbackId);
  return createVoidResult();
}


/******************* WeexCore *******************/

std::unique_ptr<IPCResult> functionCallCreateBody(IPCArguments *arguments) {

  char *pageId = getArumentAsCStr(arguments, 0);
  char *domStr = getArumentAsCStr(arguments, 1);

  if (pageId == nullptr || domStr == nullptr)
    return createInt32Result(0);

#if JSAPI_LOG
  LOGD("[ExtendJSApi] functionCallCreateBody >>>> pageId: %s, domStr: %s", pageId, domStr);
#endif

  RenderManager::GetInstance()->CreatePage(pageId, domStr) ? 0 : -1;

  delete[]pageId;
  delete[]domStr;
  return createInt32Result(0);
}

std::unique_ptr<IPCResult> handleCallAddElement(IPCArguments *arguments) {

  char *pageId = getArumentAsCStr(arguments, 0);
  char *parentRef = getArumentAsCStr(arguments, 1);
  char *domStr = getArumentAsCStr(arguments, 2);
  char *index_cstr = getArumentAsCStr(arguments, 3);
  int index = atoi(index_cstr);

  if (pageId == nullptr || parentRef == nullptr || domStr == nullptr || index_cstr == nullptr ||
      index < -1)
    return createInt32Result(0);

#if JSAPI_LOG
  LOGD("[ExtendJSApi] functionCallAddElement >>>> pageId: %s, parentRef: %s, domStr: %s, index: %d",
       pageId, parentRef, domStr, index);
#endif

  RenderManager::GetInstance()->AddRenderObject(pageId, parentRef, index, domStr);

  delete[]pageId;
  delete[]parentRef;
  delete[]domStr;
  delete[]index_cstr;
  return createInt32Result(0);
}

std::unique_ptr<IPCResult> functionCallRemoveElement(IPCArguments *arguments) {
  JNIEnv *env = getJNIEnv();
  jstring jPageId = getArgumentAsJString(env, arguments, 0);
  jstring jRef = getArgumentAsJString(env, arguments, 1);
  std::string pageId = jString2Str(env, jPageId);
  std::string ref = jString2Str(env, jRef);

#if JSAPI_LOG
  LOGD("[ExtendJSApi] functionCallRemoveElement >>>> pageId: %s, ref: %s", pageId.c_str(),
       ref.c_str());
#endif

  RenderManager::GetInstance()->RemoveRenderObject(pageId, ref);

  env->DeleteLocalRef(jPageId);
  env->DeleteLocalRef(jRef);
  return createInt32Result(0);
}

std::unique_ptr<IPCResult> functionCallMoveElement(IPCArguments *arguments) {
  JNIEnv *env = getJNIEnv();
  jstring jPageId = getArgumentAsJString(env, arguments, 0);
  jstring jRef = getArgumentAsJString(env, arguments, 1);
  jstring jParentRef = getArgumentAsJString(env, arguments, 2);
  jstring jIndex = getArgumentAsJString(env, arguments, 3);
  std::string pageId = jString2Str(env, jPageId);
  std::string ref = jString2Str(env, jRef);
  std::string parentRef = jString2Str(env, jParentRef);
  int index = atoi(jString2Str(env, jIndex).c_str());

#if JSAPI_LOG
  LOGD("[ExtendJSApi] functionCallRemoveElement >>>> pageId: %s, ref: %s, parentRef: %s, index: %d",
       pageId.c_str(), ref.c_str(), parentRef.c_str(), index);
#endif

  RenderManager::GetInstance()->MoveRenderObject(pageId, ref, parentRef, index);

  env->DeleteLocalRef(jPageId);
  env->DeleteLocalRef(jRef);
  env->DeleteLocalRef(jParentRef);
  env->DeleteLocalRef(jIndex);
  return createInt32Result(0);
}

std::unique_ptr<IPCResult> functionCallUpdateStyle(IPCArguments *arguments) {
  JNIEnv *env = getJNIEnv();
  jstring jPageId = getArgumentAsJString(env, arguments, 0);
  jstring jRef = getArgumentAsJString(env, arguments, 1);
  jbyteArray jData = getArgumentAsJByteArray(env, arguments, 2);
  std::string pageId = jString2Str(env, jPageId);
  std::string ref = jString2Str(env, jRef);
  std::string data = jByteArray2Str(env, jData);

#if JSAPI_LOG
  LOGD("[ExtendJSApi] functionCallUpdateStyle >>>> pageId: %s, ref: %s, data: %s", pageId.c_str(),
       ref.c_str(), data.c_str());
#endif

  RenderManager::GetInstance()->UpdateStyle(pageId, ref, data);

  env->DeleteLocalRef(jPageId);
  env->DeleteLocalRef(jRef);
  env->DeleteLocalRef(jData);
  return createInt32Result(0);
}

std::unique_ptr<IPCResult> functionCallUpdateAttrs(IPCArguments *arguments) {
  JNIEnv *env = getJNIEnv();
  jstring jPageId = getArgumentAsJString(env, arguments, 0);
  jstring jRef = getArgumentAsJString(env, arguments, 1);
  jbyteArray jData = getArgumentAsJByteArray(env, arguments, 2);
  std::string pageId = jString2Str(env, jPageId);
  std::string ref = jString2Str(env, jRef);
  std::string data = jByteArray2Str(env, jData);

#if JSAPI_LOG
  LOGD("[ExtendJSApi] functionCallUpdateAttrs >>>> pageId: %s, ref: %s, data: %s", pageId.c_str(),
       ref.c_str(), data.c_str());
#endif

  RenderManager::GetInstance()->UpdateAttr(pageId, ref, data);

  env->DeleteLocalRef(jPageId);
  env->DeleteLocalRef(jRef);
  env->DeleteLocalRef(jData);
  return createInt32Result(0);
}

std::unique_ptr<IPCResult> functionCallCreateFinish(IPCArguments *arguments) {
  JNIEnv *env = getJNIEnv();
  jstring jPageId = getArgumentAsJString(env, arguments, 0);
  std::string pageId = jString2Str(env, jPageId);

#if JSAPI_LOG
  LOGD("[ExtendJSApi] functionCallCreateFinish >>>> pageId: %s", pageId.c_str());
#endif

  RenderManager::GetInstance()->CreateFinish(pageId);

  env->DeleteLocalRef(jPageId);
  return createInt32Result(0);
}

std::unique_ptr<IPCResult> handleCallNative(IPCArguments *arguments) {
  JNIEnv *env = getJNIEnv();
  jstring jInstanceId = getArgumentAsJString(env, arguments, 0);
  jbyteArray jTaskString = getArgumentAsJByteArray(env, arguments, 1);
  jstring jCallback = getArgumentAsJString(env, arguments, 2);

  std::string pageId = jString2Str(env, jInstanceId);
  std::string task = jByteArray2Str(env, jTaskString);

#if JSAPI_LOG
  LOGD("[ExtendJSApi] handleCallNative >>>> pageId: %s, task: %s", pageId.c_str(), task.c_str());
#endif

  if (task == "[{\"module\":\"dom\",\"method\":\"createFinish\",\"args\":[]}]") {
    RenderManager::GetInstance()->CreateFinish(pageId) ? 0 : -1;
    env->DeleteLocalRef(jInstanceId);
    env->DeleteLocalRef(jTaskString);
    env->DeleteLocalRef(jCallback);
  } else {
    Bridge_Impl_Android::getInstance()->callNative(jInstanceId, jTaskString, jCallback);
  }

  return createInt32Result(0);
}

std::unique_ptr<IPCResult> functionCallAddEvent(IPCArguments *arguments) {
  JNIEnv *env = getJNIEnv();
  jstring jPageId = getArgumentAsJString(env, arguments, 0);
  jstring jRef = getArgumentAsJString(env, arguments, 1);
  jstring jEvent = getArgumentAsJString(env, arguments, 2);
  std::string pageId = jString2Str(env, jPageId);
  std::string ref = jString2Str(env, jRef);
  std::string event = jString2Str(env, jEvent);

#if JSAPI_LOG
  LOGD("[ExtendJSApi] functionCallAddEvent >>>> pageId: %s, ref: %s, event: %s", pageId.c_str(),
       ref.c_str(), event.c_str());
#endif

  RenderManager::GetInstance()->AddEvent(pageId, ref, event);

  env->DeleteLocalRef(jPageId);
  env->DeleteLocalRef(jRef);
  env->DeleteLocalRef(jEvent);
  return createInt32Result(0);
}

std::unique_ptr<IPCResult> functionCallRemoveEvent(IPCArguments *arguments) {
  JNIEnv *env = getJNIEnv();
  jstring jPageId = getArgumentAsJString(env, arguments, 0);
  jstring jRef = getArgumentAsJString(env, arguments, 1);
  jstring jEvent = getArgumentAsJString(env, arguments, 2);
  std::string pageId = jString2Str(env, jPageId);
  std::string ref = jString2Str(env, jRef);
  std::string event = jString2Str(env, jEvent);

#if JSAPI_LOG
  LOGD("[ExtendJSApi] functionCallRemoveEvent >>>> pageId: %s, ref: %s, event: %s", pageId.c_str(),
       ref.c_str(), event.c_str());
#endif

  RenderManager::GetInstance()->RemoveEvent(pageId, ref, event);

  env->DeleteLocalRef(jPageId);
  env->DeleteLocalRef(jRef);
  env->DeleteLocalRef(jEvent);
  return createInt32Result(0);
}

std::unique_ptr<IPCResult> handleCallNativeModule(IPCArguments *arguments) {
  JNIEnv *env = getJNIEnv();
  jstring jInstanceId = getArgumentAsJString(env, arguments, 0);
  jstring jmodule = getArgumentAsJString(env, arguments, 1);
  jstring jmethod = getArgumentAsJString(env, arguments, 2);
  jbyteArray jArgString = getArgumentAsJByteArray(env, arguments, 3);
  jbyteArray jOptString = getArgumentAsJByteArray(env, arguments, 4);

  // add for android support
  jobject result;
  result = Bridge_Impl_Android::getInstance()->callNativeModule(jInstanceId, jmodule, jmethod,
                                                                jArgString,
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
  jstring jInstanceId = getArgumentAsJString(env, arguments, 0);
  jstring jcomponentRef = getArgumentAsJString(env, arguments, 1);
  jstring jmethod = getArgumentAsJString(env, arguments, 2);
  jbyteArray jArgString = getArgumentAsJByteArray(env, arguments, 3);
  jbyteArray jOptString = getArgumentAsJByteArray(env, arguments, 4);

  Bridge_Impl_Android::getInstance()->callNativeComponent(jInstanceId, jcomponentRef, jmethod,
                                                          jArgString,
                                                          jOptString);

  return createInt32Result(static_cast<int32_t>(true));
}

std::unique_ptr<IPCResult> functionCallUpdateFinish(IPCArguments *arguments) {
  JNIEnv *env = getJNIEnv();
  jstring jInstanceId = getArgumentAsJString(env, arguments, 0);
  jbyteArray jTaskString = getArgumentAsJByteArray(env, arguments, 1);
  jstring jCallback = getArgumentAsJString(env, arguments, 2);

  int flag = 0;
  flag = Bridge_Impl_Android::getInstance()->callUpdateFinish(jInstanceId, jTaskString, jCallback);

  return createInt32Result(flag);
}

std::unique_ptr<IPCResult> functionCallRefreshFinish(IPCArguments *arguments) {
  JNIEnv *env = getJNIEnv();
  jstring jInstanceId = getArgumentAsJString(env, arguments, 0);
  jbyteArray jTaskString = getArgumentAsJByteArray(env, arguments, 1);
  jstring jCallback = getArgumentAsJString(env, arguments, 2);

  int flag = 0;
  flag = Bridge_Impl_Android::getInstance()->callRefreshFinish(jInstanceId, jTaskString, jCallback);

  return createInt32Result(flag);
}