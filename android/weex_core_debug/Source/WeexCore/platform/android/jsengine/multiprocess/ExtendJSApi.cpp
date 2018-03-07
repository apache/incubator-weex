#include <WeexCore/platform/android/base/string/StringUtils.h>
#include "ExtendJSApi.h"

using namespace WeexCore;

static WeexCore::FunType gCanvasFunc = nullptr;

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
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLGCANVASLINK),
                           handleCallGCanvasLinkNative);
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

static std::unique_ptr<IPCResult> handleCallGCanvasLinkNative(IPCArguments *arguments) {
//  base::debug::TraceScope traceScope("weex", "callGCanvasLinkNative");
  JNIEnv *env = getJNIEnv();
  //instacneID args[0]
  jstring jContextId = getArgumentAsJString(env, arguments, 0);
  const char *conextId = env->GetStringUTFChars(jContextId, NULL);

  // jstring jType = getArgumentAsJString(env, arguments, 1);
  int type = getArgumentAsInt32(env, arguments, 1);
  // need tansfer jtype to type

  jstring val = getArgumentAsJString(env, arguments, 2);
  const char *args = env->GetStringUTFChars(val, NULL);

  // LOGE("handleCallGCanvasLinkNative conextId:%s, type:%d, args:%s", conextId, type, args);

  const char *retVal = NULL;
  if (gCanvasFunc) {
    retVal = callGCanvasFun(gCanvasFunc, conextId, type, args);
  }
  // LOGE("handleCallGCanvasLinkNative retVal:%s", retVal);
  std::unique_ptr<IPCResult> ret = createVoidResult();
  if (retVal) {
    jstring jDataStr = env->NewStringUTF(retVal);
    ret = std::move(createStringResult(env, jDataStr));
    env->DeleteLocalRef(jDataStr);
    retVal = NULL;
  }
  env->DeleteLocalRef(jContextId);
  // env->DeleteLocalRef(jType);
  env->DeleteLocalRef(val);
  return ret;
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

  char *pageId = getArumentAsCStr(arguments, 0);
  char *ref = getArumentAsCStr(arguments, 1);

  if (pageId == nullptr || ref == nullptr)
    return createInt32Result(0);

#if JSAPI_LOG
  LOGD("[ExtendJSApi] functionCallRemoveElement >>>> pageId: %s, ref: %s", pageId,
       ref);
#endif

  RenderManager::GetInstance()->RemoveRenderObject(pageId, ref);

  delete[]pageId;
  delete[]ref;
  return createInt32Result(0);
}

std::unique_ptr<IPCResult> functionCallMoveElement(IPCArguments *arguments) {

  char *pageId = getArumentAsCStr(arguments, 0);
  char *ref = getArumentAsCStr(arguments, 1);
  char *parentRef = getArumentAsCStr(arguments, 2);
  char *index_str = getArumentAsCStr(arguments, 3);
  int index = atoi(index_str);

  if (pageId == nullptr || ref == nullptr || parentRef == nullptr || index_str == nullptr ||
      index < -1)
    return createInt32Result(0);

#if JSAPI_LOG
  LOGD("[ExtendJSApi] functionCallRemoveElement >>>> pageId: %s, ref: %s, parentRef: %s, index: %d",
       pageId, ref, parentRef, index);
#endif

  RenderManager::GetInstance()->MoveRenderObject(pageId, ref, parentRef, index);

  delete[]pageId;
  delete[]ref;
  delete[]parentRef;
  delete[]index_str;
  return createInt32Result(0);
}

std::unique_ptr<IPCResult> functionCallUpdateStyle(IPCArguments *arguments) {

  char *pageId = getArumentAsCStr(arguments, 0);
  char *ref = getArumentAsCStr(arguments, 1);
  char *data = getArumentAsCStr(arguments, 2);

  if (pageId == nullptr || ref == nullptr || data == nullptr)
    return createInt32Result(0);

#if JSAPI_LOG
  LOGD("[ExtendJSApi] functionCallUpdateStyle >>>> pageId: %s, ref: %s, data: %s", pageId,
       ref, data);
#endif

  RenderManager::GetInstance()->UpdateStyle(pageId, ref, data);

  delete[] pageId;
  delete[] ref;
  delete[] data;
  return createInt32Result(0);
}

std::unique_ptr<IPCResult> functionCallUpdateAttrs(IPCArguments *arguments) {

  char *pageId = getArumentAsCStr(arguments, 0);
  char *ref = getArumentAsCStr(arguments, 1);
  char *data = getArumentAsCStr(arguments, 2);

  if (pageId == nullptr || ref == nullptr || data == nullptr)
    return createInt32Result(0);

#if JSAPI_LOG
  LOGD("[ExtendJSApi] functionCallUpdateAttrs >>>> pageId: %s, ref: %s, data: %s", pageId,
       ref, data);
#endif

  RenderManager::GetInstance()->UpdateAttr(pageId, ref, data);

  delete[] pageId;
  delete[] ref;
  delete[] data;
  return createInt32Result(0);
}

std::unique_ptr<IPCResult> functionCallCreateFinish(IPCArguments *arguments) {

  char *pageId = getArumentAsCStr(arguments, 0);

  if (pageId == nullptr)
    return createInt32Result(0);

#if JSAPI_LOG
  LOGD("[ExtendJSApi] functionCallCreateFinish >>>> pageId: %s", pageId);
#endif

  RenderManager::GetInstance()->CreateFinish(pageId);

  delete[]pageId;
  return createInt32Result(0);
}

std::unique_ptr<IPCResult> handleCallNative(IPCArguments *arguments) {
  JNIEnv *env = getJNIEnv();
  jstring jInstanceId = getArgumentAsJString(env, arguments, 0);
  jbyteArray jTaskString = getArgumentAsJByteArray(env, arguments, 1);
  jstring jCallback = getArgumentAsJString(env, arguments, 2);

  std::string pageId = jString2StrFast(env, jInstanceId);
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

  char *pageId = getArumentAsCStr(arguments, 0);
  char *ref = getArumentAsCStr(arguments, 1);
  char *event = getArumentAsCStr(arguments, 2);

  if (pageId == nullptr || ref == nullptr || event == nullptr)
    return createInt32Result(0);

#if JSAPI_LOG
  LOGD("[ExtendJSApi] functionCallAddEvent >>>> pageId: %s, ref: %s, event: %s", pageId,
       ref, event);
#endif

  RenderManager::GetInstance()->AddEvent(pageId, ref, event);

  delete[]pageId;
  delete[]ref;
  delete[]event;
  return createInt32Result(0);
}

std::unique_ptr<IPCResult> functionCallRemoveEvent(IPCArguments *arguments) {

  char *pageId = getArumentAsCStr(arguments, 0);
  char *ref = getArumentAsCStr(arguments, 1);
  char *event = getArumentAsCStr(arguments, 2);

  if (pageId == nullptr || ref == nullptr || event == nullptr)
    return createInt32Result(0);

#if JSAPI_LOG
  LOGD("[ExtendJSApi] functionCallRemoveEvent >>>> pageId: %s, ref: %s, event: %s", pageId,
       ref, event);
#endif

  RenderManager::GetInstance()->RemoveEvent(pageId, ref, event);

  delete[]pageId;
  delete[]ref;
  delete[]event;
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

namespace WeexCore {

  extern "C" void Inject_GCanvasFunc(FunType fp) {
    gCanvasFunc = fp;
    LOGE("weexjsc injectGCanvasFunc gCanvasFunc");
  }

  const char *callGCanvasFun(FunType fp, const char *conextId, int x, const char *args) {
    return fp(conextId, x, args);
  }

}
