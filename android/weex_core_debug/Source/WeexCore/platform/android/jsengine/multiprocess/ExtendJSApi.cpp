#include <WeexCore/platform/android/base/string/StringUtils.h>
#include <base/ViewUtils.h>
#include "ExtendJSApi.h"

using namespace WeexCore;

static WeexCore::FunType gCanvasFunc = nullptr;

static WeexCore::FunTypeT3d t3dFunc = nullptr;

/**
* This class aim to extend JS Api
**/
ExtendJSApi::ExtendJSApi() {
}

void ExtendJSApi::initFunction(IPCHandler *handler) {
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::SETJSFVERSION),
                           handleSetJSVersion);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::REPORTEXCEPTION),
                           handleReportException);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLNATIVE),
                           handleCallNative);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLNATIVEMODULE),
                           handleCallNativeModule);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLNATIVECOMPONENT),
                           handleCallNativeComponent);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLADDELEMENT),
                           handleCallAddElement);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::SETTIMEOUT),
                           handleSetTimeout);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::NATIVELOG),
                           handleCallNativeLog);
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
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLADDEVENT),
                           functionCallAddEvent);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLREMOVEEVENT),
                           functionCallRemoveEvent);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::SETINTERVAL),
                           handleSetInterval);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CLEARINTERVAL),
                           handleClearInterval);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLGCANVASLINK),
                           handleCallGCanvasLinkNative);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLT3DLINK),
                           handleT3DLinkNative);
}

std::unique_ptr<IPCResult> handleSetJSVersion(IPCArguments *arguments) {
  const char* version = getArumentAsCStr(arguments, 0);
  LOGA("init JSFrm version %s", version);
  Bridge_Impl_Android::getInstance()->setJSVersion(version);
  return createVoidResult();
}

void reportException(const int pageId, const char *func, const char *exception_string) {
  Bridge_Impl_Android::getInstance()->reportException(pageId, func, exception_string);
}

std::unique_ptr<IPCResult> handleReportException(IPCArguments *arguments) {
  const char *pageId = nullptr;
  const char *func = nullptr;
  const char *exceptionInfo = nullptr;

  if (arguments->getType(0) == IPCType::BYTEARRAY) {
    const IPCByteArray *instanceIDBA = arguments->getByteArray(0);
    pageId = instanceIDBA->content;
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
  reportException(RefactorPageID(pageId), func, exceptionInfo);
  return createVoidResult();
}

std::unique_ptr<IPCResult> handle(IPCArguments *arguments) {
  const char* str_array = getArumentAsCStr(arguments, 0);
  Bridge_Impl_Android::getInstance()->callNativeLog(str_array);
  return createInt32Result(static_cast<int32_t>(true));
}

std::unique_ptr<IPCResult> handleCallNativeLog(IPCArguments *arguments) {
  const char* str_array = getArumentAsCStr(arguments, 0);
  Bridge_Impl_Android::getInstance()->callNativeLog(str_array);
  return createInt32Result(static_cast<int32_t>(true));
}

std::unique_ptr<IPCResult> handleSetTimeout(IPCArguments *arguments) {
  char* callbackID = getArumentAsCStr(arguments, 0);
  char* time = getArumentAsCStr(arguments, 1);

  if (callbackID == nullptr || time == nullptr)
    return createInt32Result(static_cast<int32_t>(false));

  Bridge_Impl_Android::getInstance()->setTimeout(callbackID, time);

  if (callbackID != nullptr) {
    delete[]callbackID;
    callbackID = nullptr;
  }
  if (time != nullptr) {
    delete[]time;
    time = nullptr;
  }
  return createInt32Result(static_cast<int32_t>(true));
}

std::unique_ptr<IPCResult> handleSetInterval(IPCArguments *arguments) {
  const char *pageId = getArumentAsCStr(arguments, 0);
  const char *callbackID = getArumentAsCStr(arguments, 1);
  const char *_time = getArumentAsCStr(arguments, 2);
  if (pageId == nullptr || callbackID == nullptr || _time == nullptr)
    return createInt32Result(-1);

  long time_ = atoi(_time);
  int _timerId = (atoi(pageId) << 16) | (atoi(callbackID));

  if (pageId != nullptr) {
    delete[]pageId;
    pageId = nullptr;
  }
  if (callbackID != nullptr) {
    delete[]callbackID;
    callbackID = nullptr;
  }
  if (_time != nullptr) {
    delete[]_time;
    _time = nullptr;
  }
  return createInt32Result(_timerId);
}

std::unique_ptr<IPCResult> handleClearInterval(IPCArguments *arguments) {
  const char *pageId = getArumentAsCStr(arguments, 0);
  const char *callbackID = getArumentAsCStr(arguments, 1);
  long id = atoi(callbackID);

  if (pageId != nullptr) {
    delete[]pageId;
    pageId = nullptr;
  }
  if (callbackID != nullptr) {
    delete[]callbackID;
    callbackID = nullptr;
  }
  return createVoidResult();
}

std::unique_ptr<IPCResult> handleCallNative(IPCArguments *arguments) {
  char* pageId = getArumentAsCStr(arguments, 0);
  char* task = getArumentAsCStr(arguments, 1);
  char* callback = getArumentAsCStr(arguments, 2);

  if (pageId != nullptr && task != nullptr) {
#if JSAPI_LOG
    LOGD("[ExtendJSApi] handleCallNative >>>> pageId: %s, task: %s", pageId, task);
#endif

    if (strcmp(task, "[{\"module\":\"dom\",\"method\":\"createFinish\",\"args\":[]}]") == 0) {
      RenderManager::GetInstance()->CreateFinish(RefactorPageID(pageId)) ? 0 : -1;
    } else {
      Bridge_Impl_Android::getInstance()->callNative(RefactorPageID(pageId), task, callback);
    }
  }

  if (pageId != nullptr) {
    delete[]pageId;
    pageId = nullptr;
  }
  if (task != nullptr) {
    delete[]task;
    task = nullptr;
  }
  if (callback != nullptr) {
    delete[]callback;
    callback = nullptr;
  }
  return createInt32Result(0);
}

static std::unique_ptr<IPCResult> handleCallGCanvasLinkNative(IPCArguments *arguments) {
  JNIEnv *env = getJNIEnv();
  jstring jPageId = getArgumentAsJString(env, arguments, 0);
  const char *pageId = env->GetStringUTFChars(jPageId, NULL);
  int type = getArgumentAsInt32(env, arguments, 1);
  jstring val = getArgumentAsJString(env, arguments, 2);
  const char *args = env->GetStringUTFChars(val, NULL);

#if JSAPI_LOG
  LOGD("[ExtendJSApi] handleCallGCanvasLinkNative >>>> pageId: %s, type: %d, args: %s", pageId, type, args);
#endif

  const char *retVal = NULL;
  if (gCanvasFunc) {
    retVal = callGCanvasFun(gCanvasFunc, pageId, type, args);
  }

  std::unique_ptr<IPCResult> ret = createVoidResult();
  if (retVal) {
    jstring jDataStr = env->NewStringUTF(retVal);
    ret = std::move(createStringResult(env, jDataStr));
    env->DeleteLocalRef(jDataStr);
    retVal = NULL;
  }
  env->DeleteLocalRef(jPageId);
  env->DeleteLocalRef(val);
  return ret;
}

static std::unique_ptr<IPCResult> handleT3DLinkNative(IPCArguments* arguments)
{
  JNIEnv* env = getJNIEnv();
  int type = getArgumentAsInt32(env, arguments, 0);
  jstring val = getArgumentAsJString(env, arguments, 1);
  const char* args = env->GetStringUTFChars(val, NULL);

#if JSAPI_LOG
  LOGD("[ExtendJSApi] handleT3DLinkNative >>>> type: %d, args: %s", type, args);
#endif

  const char* retVal = NULL;
  if (t3dFunc) {
    retVal = WeexCore::weexCallT3dFunc(t3dFunc, type, args);
  }



  std::unique_ptr<IPCResult> ret = createVoidResult();
  if (retVal) {
    jstring jDataStr = env->NewStringUTF(retVal);
    ret = std::move(createStringResult(env, jDataStr));
    env->DeleteLocalRef(jDataStr);
    retVal = NULL;
  }
  env->DeleteLocalRef(val);
  return ret;
}

std::unique_ptr<IPCResult> handleCallNativeModule(IPCArguments *arguments) {
  char* pageId = getArumentAsCStr(arguments, 0);
  char* module = getArumentAsCStr(arguments, 1);
  char* method = getArumentAsCStr(arguments, 2);
  char* argString = getArumentAsCStr(arguments, 3);
  char* optString = getArumentAsCStr(arguments, 4);

  std::unique_ptr<IPCResult> ret = createInt32Result(-1);

  if (pageId != nullptr && module != nullptr && method != nullptr) {
#if JSAPI_LOG
    LOGD("[ExtendJSApi] handleCallNativeModule >>>> pageId: %s, module: %s, method: %s, arg: %s, opt: %s",
         pageId, module, method, argString, optString);
#endif

    // add for android support
    jobject result;
    result = Bridge_Impl_Android::getInstance()->callNativeModule(RefactorPageID(pageId), module, method,
                                                                  argString, optString);

    if (result == nullptr)
      return ret;

    JNIEnv *env = getJNIEnv();
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
  }

  if (pageId != nullptr) {
    delete[]pageId;
    pageId = nullptr;
  }
  if (module != nullptr) {
    delete[]module;
    module = nullptr;
  }
  if (method != nullptr) {
    delete[]method;
    method = nullptr;
  }
  if (argString != nullptr) {
    delete[]argString;
    argString = nullptr;
  }
  if (optString != nullptr) {
    delete[]optString;
    optString = nullptr;
  }
  return ret;
}

std::unique_ptr<IPCResult> handleCallNativeComponent(IPCArguments *arguments) {
  char* pageId = getArumentAsCStr(arguments, 0);
  char* ref = getArumentAsCStr(arguments, 1);
  char* method = getArumentAsCStr(arguments, 2);
  char* argString = getArumentAsCStr(arguments, 3);
  char* optString = getArumentAsCStr(arguments, 4);

  if (pageId != nullptr && ref != nullptr && method != nullptr) {

#if JSAPI_LOG
    LOGD("[ExtendJSApi] handleCallNativeComponent >>>> pageId: %s, ref: %s, method: %s, arg: %s, opt: %s",
         pageId, ref, method, argString, optString);
#endif

    Bridge_Impl_Android::getInstance()->callNativeComponent(RefactorPageID(pageId), ref, method, argString, optString);
  }

  if (pageId != nullptr) {
    delete[]pageId;
    pageId = nullptr;
  }
  if (ref != nullptr) {
    delete[]ref;
    ref = nullptr;
  }
  if (method != nullptr) {
    delete[]method;
    method = nullptr;
  }
  if (argString != nullptr) {
    delete[]argString;
    argString = nullptr;
  }
  if (optString != nullptr) {
    delete[]optString;
    optString = nullptr;
  }
  return createInt32Result(static_cast<int32_t>(true));
}

std::unique_ptr<IPCResult> functionCallCreateBody(IPCArguments *arguments) {

  char *pageId = getArumentAsCStr(arguments, 0);
  char *domStr = getArumentAsCStr(arguments, 1);

  if (pageId == nullptr || domStr == nullptr)
    return createInt32Result(0);

#if JSAPI_LOG
  LOGD("[ExtendJSApi] functionCallCreateBody >>>> pageId: %s, domStr: %s", pageId, domStr);
#endif

  RenderManager::GetInstance()->CreatePage(RefactorPageID(pageId), domStr) ? 0 : -1;

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

  std::string log = "";
  log.append("pageId: ").append(pageId).append(", parentRef: ").append(parentRef).append(", domStr: ").append(domStr);
  int log_index = 0;
  int maxLength = 800;
  std::string sub;
  while (log_index < log.length()) {
    if (log.length() <= log_index + maxLength) {
      sub = log.substr(log_index);
    } else {
      sub = log.substr(log_index, maxLength);
    }


    if (log_index == 0)
      LOGD("[ExtendJSApi] functionCallAddElement >>>> %s", sub.c_str());
    else
      LOGD("      [ExtendJSApi] functionCallAddElement >>>> %s", sub.c_str());

    log_index += maxLength;
  }
#endif

  RenderManager::GetInstance()->AddRenderObject(RefactorPageID(pageId), RefactorRef(parentRef), index, domStr);

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

  RenderManager::GetInstance()->RemoveRenderObject(RefactorPageID(pageId), RefactorRef(ref));

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

  RenderManager::GetInstance()->MoveRenderObject(RefactorPageID(pageId), RefactorRef(ref), RefactorRef(parentRef), index);

  delete[]pageId;
  delete[]ref;
  delete[]parentRef;
  delete[]index_str;
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

  RenderManager::GetInstance()->AddEvent(RefactorPageID(pageId), RefactorRef(ref), event);

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

  RenderManager::GetInstance()->RemoveEvent(RefactorPageID(pageId), RefactorRef(ref), event);

  delete[]pageId;
  delete[]ref;
  delete[]event;
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

  RenderManager::GetInstance()->UpdateStyle(RefactorPageID(pageId), RefactorRef(ref), data);

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

  RenderManager::GetInstance()->UpdateAttr(RefactorPageID(pageId), RefactorRef(ref), data);

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

  RenderManager::GetInstance()->CreateFinish(RefactorPageID(pageId));

  delete[]pageId;
  return createInt32Result(0);
}

std::unique_ptr<IPCResult> functionCallUpdateFinish(IPCArguments *arguments) {
  char *pageId = getArumentAsCStr(arguments, 0);
  char *task = getArumentAsCStr(arguments, 1);
  char *callback = getArumentAsCStr(arguments, 2);

  int flag = 0;

  if (pageId == nullptr || task == nullptr)
    return createInt32Result(flag);

  flag = Bridge_Impl_Android::getInstance()->callUpdateFinish(RefactorPageID(pageId), task, callback);

  if (pageId != nullptr) {
    delete[]pageId;
    pageId = nullptr;
  }
  if (task != nullptr) {
    delete[]task;
    task = nullptr;
  }
  if (callback != nullptr) {
    delete[]callback;
    callback = nullptr;
  }
  return createInt32Result(flag);
}

std::unique_ptr<IPCResult> functionCallRefreshFinish(IPCArguments *arguments) {
  char *pageId = getArumentAsCStr(arguments, 0);
  char *task = getArumentAsCStr(arguments, 1);
  char *callback = getArumentAsCStr(arguments, 2);

  int flag = 0;

  if (pageId == nullptr || task == nullptr)
    return createInt32Result(flag);

  flag = Bridge_Impl_Android::getInstance()->callRefreshFinish(RefactorPageID(pageId), task, callback);

  if (pageId != nullptr) {
    delete[]pageId;
    pageId = nullptr;
  }
  if (task != nullptr) {
    delete[]task;
    task = nullptr;
  }
  if (callback != nullptr) {
    delete[]callback;
    callback = nullptr;
  }
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

  extern "C" void Inject_T3dFunc(FunTypeT3d fp) {
    t3dFunc = fp;
    LOGE("weexjsc Inject_T3dFunc t3d Func");
  }

const char* weexCallT3dFunc(FunTypeT3d fp, int x, const char* args) {
  return fp(x, args);
}

}
