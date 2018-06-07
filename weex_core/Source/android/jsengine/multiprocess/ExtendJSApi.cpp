/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#include <android/base/string/string_utils.h>
#include <base/ViewUtils.h>
#include <android/jsengine/api/WeexJSCoreApi.h>
#include "ExtendJSApi.h"

using namespace WeexCore;

WeexCore::FunType gCanvasFunc = nullptr;

WeexCore::FunTypeT3d t3dFunc = nullptr;

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
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::POSTMESSAGE),
                           handlePostMessage);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::DISPATCHMESSAGE),
                           handleDispatchMessage);
}

std::unique_ptr<IPCResult> handleSetJSVersion(IPCArguments *arguments) {
  const char* version = getArumentAsCStr(arguments, 0);
  _setJSVersion(version);
  return createVoidResult();
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
  _reportException(pageId, func, exceptionInfo);
  return createVoidResult();
}

std::unique_ptr<IPCResult> handle(IPCArguments *arguments) {
  const char* str_array = getArumentAsCStr(arguments, 0);
  Bridge_Impl_Android::getInstance()->callNativeLog(str_array);
  return createInt32Result(static_cast<int32_t>(true));
}

std::unique_ptr<IPCResult> handleCallNativeLog(IPCArguments *arguments) {
  const char* str_array = getArumentAsCStr(arguments, 0);
  _callNativeLog(str_array);
  return createInt32Result(static_cast<int32_t>(true));
}

std::unique_ptr<IPCResult> handleSetTimeout(IPCArguments *arguments) {
  char* callbackID = getArumentAsCStr(arguments, 0);
  char* time = getArumentAsCStr(arguments, 1);

  if (callbackID == nullptr || time == nullptr)
    return createInt32Result(static_cast<int32_t>(false));

  _setTimeout(callbackID,time);

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
  int _timerId = _setInterval(pageId,callbackID,_time);

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

    _callNative(pageId, task, callback);

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

  const char *retVal = _callGCanvasLinkNative(pageId,type,args);

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

  const char*retVal = _t3dLinkNative(type, args);

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
  char* argumentsData = getArumentAsCStr(arguments, 3);
  int   argumentsDataLength = getArumentAsCStrLen(arguments, 3);
  char* optionsData = getArumentAsCStr(arguments, 4);
  int   optionsDataLength = getArumentAsCStrLen(arguments, 4);

  std::unique_ptr<IPCResult> ret = _callNativeModule(pageId,module,method,argumentsData,
                                                     argumentsDataLength, optionsData, optionsDataLength);

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
  if (argumentsData != nullptr) {
    delete[]argumentsData;
    argumentsData = nullptr;
  }
  if (optionsData != nullptr) {
    delete[]optionsData;
    optionsData = nullptr;
  }
  return ret;
}

std::unique_ptr<IPCResult> handleCallNativeComponent(IPCArguments *arguments) {
  char* pageId = getArumentAsCStr(arguments, 0);
  char* ref = getArumentAsCStr(arguments, 1);
  char* method = getArumentAsCStr(arguments, 2);
  char* argumentsData = getArumentAsCStr(arguments, 3);
  int   argumentsDataLength = getArumentAsCStrLen(arguments, 3);
  char* optionsData = getArumentAsCStr(arguments, 4);
  int   optionsDataLength = getArumentAsCStrLen(arguments, 4);

  _callNativeComponent(pageId, ref, method, argumentsData, argumentsDataLength, optionsData, optionsDataLength);

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
  if (argumentsData != nullptr) {
    delete[]argumentsData;
    argumentsData = nullptr;
  }
  if (optionsData != nullptr) {
    delete[]optionsData;
    optionsData = nullptr;
  }
  return createInt32Result(static_cast<int32_t>(true));
}

std::unique_ptr<IPCResult> functionCallCreateBody(IPCArguments *arguments) {

  char *pageId = getArumentAsCStr(arguments, 0);
  char *domStr = getArumentAsCStr(arguments, 1);

  if (pageId == nullptr || domStr == nullptr)
    return createInt32Result(0);

  _callCreateBody(pageId,domStr);

  delete[]pageId;
  delete[]domStr;
  return createInt32Result(0);
}

std::unique_ptr<IPCResult> handleCallAddElement(IPCArguments *arguments) {

  char *pageId = getArumentAsCStr(arguments, 0);
  char *parentRef = getArumentAsCStr(arguments, 1);
  char *domStr = getArumentAsCStr(arguments, 2);
  char *index_cstr = getArumentAsCStr(arguments, 3);

    _callAddElement(pageId,parentRef,domStr,index_cstr);

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

 _callRemoveElement(pageId,ref);

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

  _callMoveElement(pageId, ref, parentRef, index);

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

  _callAddEvent(pageId,ref,event);

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

 _callRemoveEvent(pageId,ref,event);

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

  _callUpdateStyle(pageId,ref,data);

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

 _callUpdateAttrs(pageId, ref,data);

  delete[] pageId;
  delete[] ref;
  delete[] data;
  return createInt32Result(0);
}

std::unique_ptr<IPCResult> functionCallCreateFinish(IPCArguments *arguments) {

  char *pageId = getArumentAsCStr(arguments, 0);

  if (pageId == nullptr)
    return createInt32Result(0);

 _callCreateFinish(pageId);

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

  flag = _callUpdateFinish(pageId,task,callback);

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

  if (pageId == nullptr)
    return createInt32Result(flag);

  flag = _callRefreshFinish(pageId, task, callback);

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

std::unique_ptr<IPCResult> handlePostMessage(IPCArguments *arguments) {
  // LOGE("handlePostMessage");
  JNIEnv* env = getJNIEnv();
  jbyteArray jData = getArgumentAsJByteArray(env, arguments, 0);
  jstring jVmId = getArgumentAsJString(env, arguments, 1);
//  if (jPostMessage == NULL) {
//    jPostMessage = env->GetMethodID(jWMBridgeClazz,
//                                    "postMessage",
//                                    "(Ljava/lang/String;[B)V");
//  }
//  env->CallVoidMethod(jWMThis, jPostMessage, jVmId, jData);
  Bridge_Impl_Android::getInstance()->handlePostMessage(jVmId, jData);
  env->DeleteLocalRef(jData);
  env->DeleteLocalRef(jVmId);

  return createInt32Result(static_cast<int32_t>(true));

}

std::unique_ptr<IPCResult> handleDispatchMessage(IPCArguments *arguments) {
  // LOGE("handleDispatchMessage");
  JNIEnv* env = getJNIEnv();
  jstring jClientId = getArgumentAsJString(env, arguments, 0);
  jbyteArray jData = getArgumentAsJByteArray(env, arguments, 1);
  jstring jCallback = getArgumentAsJString(env, arguments, 2);
  jstring jVmId = getArgumentAsJString(env, arguments, 3);
//  if (jDispatchMeaasge == NULL) {
//    jDispatchMeaasge = env->GetMethodID(jWMBridgeClazz,
//                                        "dispatchMessage",
//                                        "(Ljava/lang/String;Ljava/lang/String;[BLjava/lang/String;)V");
//  }
//  env->CallVoidMethod(jWMThis, jDispatchMeaasge, jClientId, jVmId, jData, jCallback);
  Bridge_Impl_Android::getInstance()->handleDispatchMessage(jClientId, jVmId, jData, jCallback);
  env->DeleteLocalRef(jClientId);
  env->DeleteLocalRef(jData);
  env->DeleteLocalRef(jCallback);
  return createInt32Result(static_cast<int32_t>(true));
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
