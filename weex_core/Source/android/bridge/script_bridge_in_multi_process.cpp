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
#include "base/log_defines.h"
#include "script_bridge_in_multi_process.h"

#include "android/utils/params_utils.h"
#include "android/base/string/string_utils.h"
#include "android/bridge/multi_process_and_so_initializer.h"
#include "android/bridge/script/script_side_in_multi_process.h"
#include "android/multiprocess/weex_js_connection.h"
#include "android/utils/ipc_string_result.h"
#include "base/android/log_utils.h"
#include "base/make_copyable.h"
#include "base/message_loop/message_loop.h"
#include "base/thread/waitable_event.h"
#include "base/android/jni/android_jni.h"
#include "core/bridge/script/core_side_in_script.h"
#include "core/manager/weex_core_manager.h"
#include "third_party/IPC/IPCArguments.h"                                         
#include "third_party/IPC/IPCHandler.h"                                           
#include "third_party/IPC/IPCMessageJS.h" 
#include "third_party/IPC/IPCResult.h"

namespace WeexCore {

static std::unique_ptr<IPCResult> HandleSetJSVersion(IPCArguments *arguments) {
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable([version = std::unique_ptr<char[]>(
                                    getArumentAsCStr(arguments, 0))] {
        WeexCoreManager::Instance()->script_bridge()->core_side()->SetJSVersion(
            version.get());
      }));

  return createVoidResult();
}

static std::unique_ptr<IPCResult> HandleReportException(
    IPCArguments *arguments) {
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

  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable([pageId = std::string(pageId == nullptr ? "" : pageId),
                                funcS = std::string(func == nullptr ? "" : func),
                                exceptionStr = std::string(exceptionInfo == nullptr ? "" : exceptionInfo)] {
        WeexCoreManager::Instance()
            ->script_bridge()
            ->core_side()
            ->ReportException(pageId.c_str(), funcS.c_str(),
                              exceptionStr.c_str());
      }));
  return createVoidResult();
}

static std::unique_ptr<IPCResult> HandleCallNativeLog(IPCArguments *arguments) {
  auto arg1 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0));
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable([str_array = std::move(arg1)] {
        WeexCoreManager::Instance()->script_bridge()->core_side()->NativeLog(
            str_array.get());
      }));

  //  const char *str_array = getArumentAsCStr(arguments, 0);
  //  WeexCoreManager::Instance()->script_bridge()->core_side()->NativeLog(
  //      str_array);
  return createInt32Result(static_cast<int32_t>(true));
}

static std::unique_ptr<IPCResult> HandleSetTimeout(IPCArguments *arguments) {
  auto arg1 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0));
  auto arg2 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 1));
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable([callbackID = std::move(arg1),
                                time = std::move(arg2)] {
        WeexCoreManager::Instance()->script_bridge()->core_side()->SetTimeout(
            callbackID.get(), time.get());
      }));

  //  char *callbackID = getArumentAsCStr(arguments, 0);
  //  char *time = getArumentAsCStr(arguments, 1);
  //
  //  if (callbackID == nullptr || time == nullptr)
  //    return createInt32Result(static_cast<int32_t>(false));
  //
  //  WeexCoreManager::Instance()->script_bridge()->core_side()->SetTimeout(
  //      callbackID, time);
  //
  //  if (callbackID != nullptr) {
  //    delete[] callbackID;
  //    callbackID = nullptr;
  //  }
  //  if (time != nullptr) {
  //    delete[] time;
  //    time = nullptr;
  //  }
  return createInt32Result(static_cast<int32_t>(true));
}

static std::unique_ptr<IPCResult> HandleSetInterval(IPCArguments *arguments) {
  auto arg1 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0));
  auto arg2 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 1));
  auto arg3 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 2));
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable([pageId = std::move(arg1),
                                callbackID = std::move(arg2),
                                time = std::move(arg3)] {
        WeexCoreManager::Instance()->script_bridge()->core_side()->SetInterval(
            pageId.get(), callbackID.get(), time.get());
      }));

  //  const char *pageId = getArumentAsCStr(arguments, 0);
  //  const char *callbackID = getArumentAsCStr(arguments, 1);
  //  const char *_time = getArumentAsCStr(arguments, 2);
  //  if (pageId == nullptr || callbackID == nullptr || _time == nullptr)
  //    return createInt32Result(-1);
  //
  //  long time_ = atoi(_time);
  //  int _timerId =
  //      WeexCoreManager::Instance()->script_bridge()->core_side()->SetInterval(
  //          pageId, callbackID, _time);
  //
  //  if (pageId != nullptr) {
  //    delete[] pageId;
  //    pageId = nullptr;
  //  }
  //  if (callbackID != nullptr) {
  //    delete[] callbackID;
  //    callbackID = nullptr;
  //  }
  //  if (_time != nullptr) {
  //    delete[] _time;
  //    _time = nullptr;
  //  }

  // TODO timerId
  return createInt32Result(1);
}

static std::unique_ptr<IPCResult> HandleClearInterval(IPCArguments *arguments) {
  const char *pageId = getArumentAsCStr(arguments, 0);
  const char *callbackID = getArumentAsCStr(arguments, 1);
  long id = atoi(callbackID);

  if (pageId != nullptr) {
    delete[] pageId;
    pageId = nullptr;
  }
  if (callbackID != nullptr) {
    delete[] callbackID;
    callbackID = nullptr;
  }
  return createVoidResult();
}

static std::unique_ptr<IPCResult> HandleCallNative(IPCArguments *arguments) {
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable(
          [pageId = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0)),
           task = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 1)),
           callback = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 2))] {
            if (pageId != nullptr && task != nullptr) {
              WeexCoreManager::Instance()
                  ->script_bridge()
                  ->core_side()
                  ->CallNative(pageId.get(), task.get(), callback.get());
            }
          }));
  return createInt32Result(0);
}

static std::unique_ptr<IPCResult> HandleCallGCanvasLinkNative(
    IPCArguments *arguments) {

  auto arg1 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0));
  auto typeStr = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 1));
  int type = atoi(typeStr.get());
  auto arg3 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 2));
  weex::base::WaitableEvent event;
  char *retVal = nullptr;
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
          weex::base::MakeCopyable(
                  [pageId = std::move(arg1), t = type,
                          args = std::move(arg3), returnResult = &retVal, e = &event] {
                      *returnResult =
                              const_cast<char *>(WeexCoreManager::Instance()
                                      ->script_bridge()
                                      ->core_side()
                                      ->CallGCanvasLinkNative(pageId.get(),t,args.get()));
                      e->Signal();
                  }));

  event.Wait();


  JNIEnv *env = base::android::AttachCurrentThread();
//  jstring jPageId = getArgumentAsJString(env, arguments, 0);
//  const char *pageId = env->GetStringUTFChars(jPageId, NULL);
//  int type = getArgumentAsInt32(env, arguments, 1);
//  jstring val = getArgumentAsJString(env, arguments, 2);
//  const char *args = env->GetStringUTFChars(val, NULL);
//
//#if JSAPI_LOG
//      LOGD("[ExtendJSApi] HandleCallGCanvasLinkNative >>>> pageId: %s, type:
//      %d, args: %s", pageId, type, args);
//#endif
//      const char *retVal = NULL;
//    retVal = WeexCoreManager::Instance()->script_bridge()->core_side()->CallGCanvasLinkNative(pageId, type,
//                                                                                     args);

      std::unique_ptr<IPCResult> ret = createVoidResult();
      if (retVal) {
    jstring jDataStr = env->NewStringUTF(retVal);
    ret = std::unique_ptr<IPCResult>(
        new IPCStringResult(jstring2WeexString(env, jDataStr)));
    env->DeleteLocalRef(jDataStr);
    retVal = NULL;
      }
//      env->DeleteLocalRef(jPageId);
//      env->DeleteLocalRef(val);
      return ret;
}

static std::unique_ptr<IPCResult> HandleT3DLinkNative(IPCArguments *arguments) {


  auto typeStr = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0));
  int type = atoi(typeStr.get());
  auto arg1 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 1));
  weex::base::WaitableEvent event;
  char *retVal = nullptr;
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
          weex::base::MakeCopyable(
                  [args = std::move(arg1), t = type,
                          returnResult = &retVal, e = &event] {
                      *returnResult =
                              const_cast<char *>(WeexCoreManager::Instance()
                                      ->script_bridge()
                                      ->core_side()
                                      ->CallT3DLinkNative(t, args.get()));
                      e->Signal();
                  }));

  event.Wait();

  JNIEnv *env = base::android::AttachCurrentThread();
//  int type = getArgumentAsInt32(env, arguments, 0);
//  jstring val = getArgumentAsJString(env, arguments, 1);
//  const char *args = env->GetStringUTFChars(val, NULL);

#if JSAPI_LOG
  LOGD("[ExtendJSApi] handleT3DLinkNative >>>> type: %d, args: %s", type, args);
#endif
//  const char *retVal = NULL;
//  retVal = WeexCoreManager::Instance()
//               ->script_bridge()
//               ->core_side()
//               ->CallT3DLinkNative(type, args);

  std::unique_ptr<IPCResult> ret = createVoidResult();
  if (retVal) {
    jstring jDataStr = env->NewStringUTF(retVal);
    ret = std::unique_ptr<IPCResult>(
        new IPCStringResult(jstring2WeexString(env, jDataStr)));
    env->DeleteLocalRef(jDataStr);
    retVal = NULL;
  }
//  env->DeleteLocalRef(val);
  return ret;
}

static std::unique_ptr<IPCResult> HandleCallNativeModule(
    IPCArguments *arguments) {
  int argumentsDataLength = getArumentAsCStrLen(arguments, 3);
  int optionsDataLength = getArumentAsCStrLen(arguments, 4);

  auto arg1 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0));
  auto arg2 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 1));
  auto arg3 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 2));
  auto arg4 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 3));
  auto arg5 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 4));

  weex::base::WaitableEvent event;
  std::unique_ptr<ValueWithType> ret;
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable(
          [pageId = std::move(arg1), module = std::move(arg2),
           method = std::move(arg3), argumentsData = std::move(arg4),
           optionsData = std::move(arg5), length1 = argumentsDataLength,
           length2 = optionsDataLength, result = &ret, e = &event] {
            *result =
                WeexCoreManager::Instance()
                    ->script_bridge()
                    ->core_side()
                    ->CallNativeModule(pageId.get(), module.get(), method.get(),
                                       argumentsData.get(), length1,
                                       optionsData.get(), length2);
            e->Signal();
          }));

  event.Wait();
  /**
   * when Wait timeout, ret is null.  switch case will crash.
   * make default value, to avoid the crash
   * */
  if(ret.get() == nullptr){
     ret = std::make_unique<ValueWithType>((int32_t)-1);
  }

  std::unique_ptr<IPCResult> result;
  switch (ret->type) {
    case ParamsType::INT32:
      result = createInt32Result(ret->value.int32Value);
      break;
    case ParamsType::INT64:
      result = createInt64Result(ret->value.int64Value);
      break;
    case ParamsType::FLOAT:
    case ParamsType::DOUBLE:
      result = createDoubleResult(ret->value.doubleValue);
      break;
    case ParamsType::VOID:
      result = createVoidResult();
      break;
    case ParamsType::BYTEARRAY:
      result = createByteArrayResult(ret->value.byteArray->content,
                                     ret->value.byteArray->length);
      break;
    case ParamsType::JSONSTRING:
      result = std::unique_ptr<IPCResult>(
          new IPCJSONStringResult(ret->value.string));
      break;
    case ParamsType::STRING:
      result =
          std::unique_ptr<IPCResult>(new IPCStringResult(ret->value.string));
      break;
    default:
      result = createVoidResult();
      break;
  }
  // Need to free
  //  freeValueWithType(ret.get());

  return result;
}

static std::unique_ptr<IPCResult> HandleCallNativeComponent(
    IPCArguments *arguments) {
  //  char *pageId = getArumentAsCStr(arguments, 0);
  //  char *ref = getArumentAsCStr(arguments, 1);
  //  char *method = getArumentAsCStr(arguments, 2);
  //  char *argumentsData = getArumentAsCStr(arguments, 3);
  int argumentsDataLength = getArumentAsCStrLen(arguments, 3);
  //  char *optionsData = getArumentAsCStr(arguments, 4);
  int optionsDataLength = getArumentAsCStrLen(arguments, 4);

  auto arg1 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0));
  auto arg2 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 1));
  auto arg3 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 2));
  auto arg4 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 3));
  auto arg5 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 4));
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable(
          [pageId = std::move(arg1), ref = std::move(arg2),
           method = std::move(arg3), argumentsData = std::move(arg4),
           optionsData = std::move(arg5), l1 = argumentsDataLength,
           l2 = optionsDataLength] {
            if (pageId != nullptr && ref != nullptr && method != nullptr) {
              WeexCoreManager::Instance()
                  ->script_bridge()
                  ->core_side()
                  ->CallNativeComponent(pageId.get(), ref.get(), method.get(),
                                        argumentsData.get(), l1,
                                        optionsData.get(), l2);
            }
          }));

  //  if (pageId != nullptr && ref != nullptr && method != nullptr) {
  //#if JSAPI_LOG
  //    LOGD("[ExtendJSApi] handleCallNativeComponent >>>> pageId: %s, ref:
  //             % s,
  //         method
  //         : % s, arg
  //         : % s, opt
  //         : % s ",
  //               pageId,
  //           ref, method, argString, optString);
  //#endif
  //    WeexCoreManager::Instance()
  //        ->script_bridge()
  //        ->core_side()
  //        ->CallNativeComponent(pageId, ref, method, argumentsData,
  //                              argumentsDataLength, optionsData,
  //                              optionsDataLength);
  //  }
  //
  //  if (pageId != nullptr) {
  //    delete[] pageId;
  //    pageId = nullptr;
  //  }
  //  if (ref != nullptr) {
  //    delete[] ref;
  //    ref = nullptr;
  //  }
  //  if (method != nullptr) {
  //    delete[] method;
  //    method = nullptr;
  //  }
  //  if (argumentsData != nullptr) {
  //    delete[] argumentsData;
  //    argumentsData = nullptr;
  //  }
  //  if (optionsData != nullptr) {
  //    delete[] optionsData;
  //    optionsData = nullptr;
  //  }
  return createInt32Result(static_cast<int32_t>(true));
}

static std::unique_ptr<IPCResult> FunctionCallCreateBody(
    IPCArguments *arguments) {
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable([page_id = std::unique_ptr<char[]>(
                                    getArumentAsCStr(arguments, 0)),
                                dom_str = std::unique_ptr<char[]>(
                                    getArumentAsCStr(arguments, 1))] {
        WeexCoreManager::Instance()->script_bridge()->core_side()->CreateBody(
            page_id.get(), dom_str.get(), strlen(dom_str.get()));
      }));

  //  auto page_id = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0));
  //  auto dom_str = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 1));
  //  int dom_str_len = getArumentAsCStrLen(arguments, 1);
  //
  //  if (page_id.get() == nullptr || dom_str == nullptr)
  //    return createInt32Result(0);
  //
  //  WeexCoreManager::Instance()->script_bridge()->core_side()->CreateBody(
  //      page_id.get(), dom_str.get(), dom_str_len);

  return createInt32Result(0);
}

static std::unique_ptr<IPCResult> HandleCallAddElement(
    IPCArguments *arguments) {
  auto arg1 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0));
  auto arg2 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 1));
  auto arg3 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 2));
  auto arg4 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 3));
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable([page_id = std::move(arg1),
                                parent_ref = std::move(arg2),
                                dom_str = std::move(arg3),
                                index_cstr = std::move(arg4)] {
        const char *index_char =
            index_cstr.get() == nullptr ? "\0" : index_cstr.get();
        int index = atoi(index_char);
        if (page_id.get() != nullptr && parent_ref.get() != nullptr &&
            dom_str.get() != nullptr && index >= -1) {
          WeexCoreManager::Instance()->script_bridge()->core_side()->AddElement(
              page_id.get(), parent_ref.get(), dom_str.get(),
              strlen(dom_str.get()), index_char);
        }
      }));

  //  char *pageId = getArumentAsCStr(arguments, 0);
  //  char *parentRef = getArumentAsCStr(arguments, 1);
  //  char *dom_str = getArumentAsCStr(arguments, 2);
  //  char *index_cstr = getArumentAsCStr(arguments, 3);
  //  LOGE("AddRenderObject in Handle %s", dom_str);
  //  const char *indexChar = index_cstr == nullptr ? "\0" : index_cstr;
  //
  //  int index = atoi(indexChar);
  //  if (pageId != nullptr && parentRef != nullptr && dom_str != nullptr &&
  //      index >= -1) {
  //#if JSAPI_LOG
  //
  //    std::string log = "";
  //    log.append("pageId: ").append(pageId).append(", parentRef:
  //    ").append(parentRef).append(", dom_str: ").append(dom_str); int
  //    log_index = 0; int maxLength = 800; std::string sub; while (log_index <
  //    log.length()) {
  //      if (log.length() <= log_index + maxLength) {
  //        sub = log.substr(log_index);
  //      } else {
  //        sub = log.substr(log_index, maxLength);
  //      }
  //
  //      if (log_index == 0)
  //        LOGD("[ExtendJSApi] functionCallAddElement >>>> %s", sub.c_str());
  //      else
  //        LOGD("      [ExtendJSApi] functionCallAddElement >>>> %s",
  //        sub.c_str());
  //
  //      log_index += maxLength;
  //    }
  //#endif
  //        WeexCoreManager::Instance()->script_bridge()->core_side()->AddElement(
  //          pageId, parentRef, dom_str, getArumentAsCStrLen(arguments, 2),
  //          indexChar);
  //  }
  //
  //  delete[] pageId;
  //  delete[] parentRef;
  //  delete[] dom_str;
  //  delete[] index_cstr;
  return createInt32Result(0);
}

static std::unique_ptr<IPCResult> FunctionCallRemoveElement(
    IPCArguments *arguments) {
  auto arg1 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0));
  auto arg2 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 1));
  if (arg1.get() == nullptr || arg2.get() == nullptr)
    return createInt32Result(0);
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable(
          [pageId = std::move(arg1), ref = std::move(arg2)] {
            WeexCoreManager::Instance()
                ->script_bridge()
                ->core_side()
                ->RemoveElement(pageId.get(), ref.get());
          }));

  //  char *pageId = getArumentAsCStr(arguments, 0);
  //  char *ref = getArumentAsCStr(arguments, 1);
  //
  //  if (pageId == nullptr || ref == nullptr) return createInt32Result(0);
  //
  //#if JSAPI_LOG
  //  LOGD("[ExtendJSApi] functionCallRemoveElement >>>> pageId: %s, ref: %s",
  //       pageId, ref);
  //#endif
  //  WeexCoreManager::Instance()->script_bridge()->core_side()->RemoveElement(
  //      pageId, ref);
  //
  //  delete[] pageId;
  //  delete[] ref;
  return createInt32Result(0);
}

static std::unique_ptr<IPCResult> FunctionCallMoveElement(
    IPCArguments *arguments) {
  auto arg1 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0));
  auto arg2 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 1));
  auto arg3 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 2));
  auto arg4 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 3));
  int index = atoi(arg4.get());
  if (arg1 == nullptr || arg2 == nullptr || arg3 == nullptr ||
      arg4 == nullptr || index < -1)
    return createInt32Result(0);
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable([pageId = std::move(arg1), ref = std::move(arg2),
                                parentRef = std::move(arg3), index = index] {
        WeexCoreManager::Instance()->script_bridge()->core_side()->MoveElement(
            pageId.get(), ref.get(), parentRef.get(), index);
      }));

  //  char *pageId = getArumentAsCStr(arguments, 0);
  //  char *ref = getArumentAsCStr(arguments, 1);
  //  char *parentRef = getArumentAsCStr(arguments, 2);
  //  char *index_str = getArumentAsCStr(arguments, 3);
  //  int index = atoi(index_str);
  //
  //  if (pageId == nullptr || ref == nullptr || parentRef == nullptr ||
  //      index_str == nullptr || index < -1)
  //    return createInt32Result(0);
  //
  //  WeexCoreManager::Instance()->script_bridge()->core_side()->MoveElement(
  //      pageId, ref, parentRef, index);
  //
  //  delete[] pageId;
  //  delete[] ref;
  //  delete[] parentRef;
  //  delete[] index_str;
  return createInt32Result(0);
}

static std::unique_ptr<IPCResult> FunctionCallAddEvent(
    IPCArguments *arguments) {
  auto arg1 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0));
  auto arg2 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 1));
  auto arg3 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 2));
  if (arg1 == nullptr || arg2 == nullptr || arg3 == nullptr)
    return createInt32Result(0);
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable([pageId = std::move(arg1), ref = std::move(arg2),
                                event = std::move(arg3)] {
        WeexCoreManager::Instance()->script_bridge()->core_side()->AddEvent(
            pageId.get(), ref.get(), event.get());
      }));

  //  char *pageId = getArumentAsCStr(arguments, 0);
  //  char *ref = getArumentAsCStr(arguments, 1);
  //  char *event = getArumentAsCStr(arguments, 2);
  //
  //  if (pageId == nullptr || ref == nullptr || event == nullptr)
  //    return createInt32Result(0);
  //
  //  WeexCoreManager::Instance()->script_bridge()->core_side()->AddEvent(
  //      pageId, ref, event);
  //
  //  delete[] pageId;
  //  delete[] ref;
  //  delete[] event;
  return createInt32Result(0);
}

static std::unique_ptr<IPCResult> FunctionCallRemoveEvent(
    IPCArguments *arguments) {
  auto arg1 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0));
  auto arg2 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 1));
  auto arg3 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 2));
  if (arg1 == nullptr || arg2 == nullptr || arg3 == nullptr)
    return createInt32Result(0);
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable([pageId = std::move(arg1), ref = std::move(arg2),
                                event = std::move(arg3)] {
        WeexCoreManager::Instance()->script_bridge()->core_side()->RemoveEvent(
            pageId.get(), ref.get(), event.get());
      }));

  //  char *pageId = getArumentAsCStr(arguments, 0);
  //  char *ref = getArumentAsCStr(arguments, 1);
  //  char *event = getArumentAsCStr(arguments, 2);
  //
  //  if (pageId == nullptr || ref == nullptr || event == nullptr)
  //    return createInt32Result(0);
  //
  //  WeexCoreManager::Instance()->script_bridge()->core_side()->RemoveEvent(
  //      pageId, ref, event);
  //
  //  delete[] pageId;
  //  delete[] ref;
  //  delete[] event;
  return createInt32Result(0);
}

static std::unique_ptr<IPCResult> FunctionCallUpdateStyle(
    IPCArguments *arguments) {
  auto arg1 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0));
  auto arg2 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 1));
  auto arg3 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 2));
  if (arg1 == nullptr || arg2 == nullptr || arg3 == nullptr)
    return createInt32Result(0);
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable([page_id = std::move(arg1),
                                ref = std::move(arg2), data = std::move(arg3)] {
        if (page_id.get() == nullptr || ref.get() == nullptr ||
            data.get() == nullptr)
          return;

        WeexCoreManager::Instance()->script_bridge()->core_side()->UpdateStyle(
            page_id.get(), ref.get(), data.get(), strlen(data.get()));
      }));
  //  char *pageId = getArumentAsCStr(arguments, 0);
  //  char *ref = getArumentAsCStr(arguments, 1);
  //  char *data = getArumentAsCStr(arguments, 2);
  //  int data_length = getArumentAsCStrLen(arguments, 2);
  //  if (pageId == nullptr || ref == nullptr || data == nullptr)
  //    return createInt32Result(0);
  //
  //  WeexCoreManager::Instance()->script_bridge()->core_side()->UpdateStyle(
  //      pageId, ref, data, data_length);
  //
  //  delete[] pageId;
  //  delete[] ref;
  //  delete[] data;
  return createInt32Result(0);
}

static std::unique_ptr<IPCResult> FunctionCallUpdateAttrs(
    IPCArguments *arguments) {
  auto arg1 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0));
  auto arg2 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 1));
  auto arg3 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 2));
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable([page_id = std::move(arg1),
                                ref = std::move(arg2), data = std::move(arg3)] {
        if (page_id.get() == nullptr || ref.get() == nullptr ||
            data.get() == nullptr)
          return;

        WeexCoreManager::Instance()->script_bridge()->core_side()->UpdateAttrs(
            page_id.get(), ref.get(), data.get(), strlen(data.get()));
      }));
  //  char *pageId = getArumentAsCStr(arguments, 0);
  //  char *ref = getArumentAsCStr(arguments, 1);
  //  char *data = getArumentAsCStr(arguments, 2);
  //  int data_length = getArumentAsCStrLen(arguments, 2);
  //  if (pageId == nullptr || ref == nullptr || data == nullptr)
  //    return createInt32Result(0);
  //
  //  WeexCoreManager::Instance()->script_bridge()->core_side()->UpdateAttrs(
  //      pageId, ref, data, data_length);
  //
  //  delete[] pageId;
  //  delete[] ref;
  //  delete[] data;
  return createInt32Result(0);
}

static std::unique_ptr<IPCResult> FunctionCallCreateFinish(
    IPCArguments *arguments) {
  auto arg1 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0));
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable([page_id = std::move(arg1)] {
        if (page_id.get() == nullptr) return;

        WeexCoreManager::Instance()->script_bridge()->core_side()->CreateFinish(
            page_id.get());
      }));
  //  char *pageId = getArumentAsCStr(arguments, 0);
  //
  //  if (pageId == nullptr) return createInt32Result(0);
  //
  //  WeexCoreManager::Instance()->script_bridge()->core_side()->CreateFinish(
  //      pageId);
  //
  //  delete[] pageId;
  return createInt32Result(0);
}

static std::unique_ptr<IPCResult> FunctionCallUpdateFinish(
    IPCArguments *arguments) {
  auto arg1 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0));
  auto arg2 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 1));
  auto arg3 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 2));
  weex::base::WaitableEvent event;
  int result = -1;
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable([pageId = std::move(arg1),
                                task = std::move(arg2),
                                callback = std::move(arg3), ret = &result,
                                event = &event] {
        WeexCoreManager::Instance()->script_bridge()->core_side()->UpdateFinish(
            pageId.get(), task.get(), strlen(task.get()), callback.get(),
            strlen(callback.get()));
        event->Signal();
      }));
  event.Wait();
  return createInt32Result(result);

  //  char *pageId = getArumentAsCStr(arguments, 0);
  //  char *task = getArumentAsCStr(arguments, 1);
  //  int task_length = getArumentAsCStrLen(arguments, 1);
  //  char *callback = getArumentAsCStr(arguments, 2);
  //  int callback_length = getArumentAsCStrLen(arguments, 2);
  //
  //  int flag = 0;
  //
  //  if (pageId == nullptr || task == nullptr) return createInt32Result(flag);
  //
  //  flag =
  //      WeexCoreManager::Instance()->script_bridge()->core_side()->UpdateFinish(
  //          pageId, task, task_length, callback, callback_length);
  //
  //  if (pageId != nullptr) {
  //    delete[] pageId;
  //    pageId = nullptr;
  //  }
  //  if (task != nullptr) {
  //    delete[] task;
  //    task = nullptr;
  //  }
  //  if (callback != nullptr) {
  //    delete[] callback;
  //    callback = nullptr;
  //  }
  // todo result
  //  return createInt32Result(1);
}

static std::unique_ptr<IPCResult> FunctionCallRefreshFinish(
    IPCArguments *arguments) {
  auto arg1 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0));
  auto arg2 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 1));
  auto arg3 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 2));
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable([pageId = std::move(arg1),
                                task = std::move(arg2),
                                callback = std::move(arg3)] {
        WeexCoreManager::Instance()
            ->script_bridge()
            ->core_side()
            ->RefreshFinish(pageId.get(), task.get(), callback.get());
      }));

  //  char *pageId = getArumentAsCStr(arguments, 0);
  //  char *task = getArumentAsCStr(arguments, 1);
  //  char *callback = getArumentAsCStr(arguments, 2);
  //
  //  int flag = 0;
  //
  //  if (pageId == nullptr) return createInt32Result(flag);
  //
  //  flag =
  //      WeexCoreManager::Instance()->script_bridge()->core_side()->RefreshFinish(
  //          pageId, task, callback);
  //
  //  if (pageId != nullptr) {
  //    delete[] pageId;
  //    pageId = nullptr;
  //  }
  //  if (task != nullptr) {
  //    delete[] task;
  //    task = nullptr;
  //  }
  //  if (callback != nullptr) {
  //    delete[] callback;
  //    callback = nullptr;
  //  }
  // todo result
  return createInt32Result(1);
}

static std::unique_ptr<IPCResult> HandlePostMessage(IPCArguments *arguments) {
  //  char *jData = getArumentAsCStr(arguments, 0);
  //  char *jVmId = getArumentAsCStr(arguments, 1);
  //  WeexCoreManager::Instance()->script_bridge()->core_side()->PostMessage(jVmId,
  //                                                                         jData);
  int i = getArumentAsCStrLen(arguments, 0);
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable([jData = std::unique_ptr<char[]>(
                                    getArumentAsCStr(arguments, 0)),
              length = i,
                                jVmId = std::unique_ptr<char[]>(
                                    getArumentAsCStr(arguments, 1))] {
        WeexCoreManager::Instance()->script_bridge()->core_side()->PostMessage(
            jVmId.get(), jData.get(),length);
      }));
  return createInt32Result(static_cast<int32_t>(true));
}

std::unique_ptr<IPCResult> HandleDispatchMessage(IPCArguments *arguments) {
  //  char *jClientId = getArumentAsCStr(arguments, 0);
  //  char *jData = getArumentAsCStr(arguments, 1);
  //  char *jCallback = getArumentAsCStr(arguments, 2);
  //  char *jVmId = getArumentAsCStr(arguments, 3);
  //  WeexCoreManager::Instance()->script_bridge()->core_side()->DispatchMessage(
  //      jClientId, jVmId, jData, jCallback);
  int i = getArumentAsCStrLen(arguments, 1);
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable(
          [clientId = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0)),
           dataS = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 1)),
                  length = i,
           callbackS = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 2)),
           vmId = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 3))] {
            WeexCoreManager::Instance()
                ->script_bridge()
                ->core_side()
                ->DispatchMessage(clientId.get(), dataS.get(),length,
                                  callbackS.get(), vmId.get());
          }));
  return createInt32Result(static_cast<int32_t>(true));
}

std::unique_ptr<IPCResult> HandleDispatchMessageSync(IPCArguments *arguments) {
  int i = getArumentAsCStrLen(arguments, 1);
  weex::base::WaitableEvent event;
  std::unique_ptr<WeexJSResult> result;
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable(
          [clientId = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0)),
           dataS = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 1)),
           vmId = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 2)),
           length = i, e = &event, r = &result]() {
            *r = WeexCoreManager::Instance()
                     ->script_bridge()
                     ->core_side()
                     ->DispatchMessageSync(clientId.get(), dataS.get(), length,
                                           vmId.get());
            e->Signal();
          }));
  event.Wait();
  if (result->length > 0) {
    return createByteArrayResult(result->data.get(), result->length);
  } else {
    return createVoidResult();
  }
}

std::unique_ptr<IPCResult> OnReceivedResult(IPCArguments *arguments) {
  long callback_id = arguments->get<long>(0);
  std::unique_ptr<WeexJSResult> result;
  result.reset(new WeexJSResult);
  if (arguments->getCount() > 1 && arguments->getType(1) == IPCType::BYTEARRAY &&
      arguments->getByteArray(1)->length > 0) {
    result->length = arguments->getByteArray(1)->length;
    char *string = new char[result->length + 1];
    result->data.reset(string);
    memset(string, 0, result->length);
    memcpy(string, arguments->getByteArray(1)->content, result->length);
    string[result->length] = '\0';
  }
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable([callback_id, result = std::move(result)]() {
        WeexCoreManager::Instance()
            ->script_bridge()
            ->core_side()
            ->OnReceivedResult(
                callback_id,
                const_cast<std::unique_ptr<WeexJSResult> &>(result));
      }));
  return createInt32Result(static_cast<int32_t>(true));
}

std::unique_ptr<IPCResult> UpdateComponentData(IPCArguments *arguments) {
    auto arg1 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0));
    auto arg2 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 1));
    auto arg3 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 2));
    WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
        weex::base::MakeCopyable(
            [page_id = std::move(arg1), cid = std::move(arg2), json_data = std::move(arg3)]() {
              WeexCoreManager::Instance()
                  ->script_bridge()
                  ->core_side()
                  ->UpdateComponentData(page_id.get(), cid.get(), json_data.get());
            }));
    return createInt32Result(static_cast<int32_t>(true));
}




std::unique_ptr<IPCResult> HeartBeat(IPCArguments *arguments) {
  auto arg1 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0));
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
          weex::base::MakeCopyable(
                  [pageId = std::move(arg1)]() {
                      if (pageId != nullptr) {
                        LOGE("HeartBeat %s", pageId.get());
                        WeexCoreManager::Instance()
                                ->script_bridge()
                                ->core_side()
                                ->CallNative(pageId.get(), "HeartBeat", "HeartBeat");
                      }
                  }));
  return createInt32Result(static_cast<int32_t>(true));
}

std::unique_ptr<IPCResult> HandleLogDetail(IPCArguments *arguments) {
  auto arg1 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0));
  auto arg2 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 1));
  auto arg3 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 2));
  auto arg4 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 3));
  auto arg5 = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 4));

  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable(
          [level_str = std::move(arg1), tag_ptr = std::move(arg2),
              file_ptr = std::move(arg3), line_ptr = std::move(arg4), log_ptr = std::move(arg5)]() {
            int level = (level_str == nullptr || level_str.get() == nullptr)
                        ? ((int) (WeexCore::LogLevel::Debug)) : atoi(level_str.get());
            long line =
                (line_ptr == nullptr || line_ptr.get() == nullptr) ? 0 : atol(line_ptr.get());
            weex::base::LogImplement::getLog()->log((WeexCore::LogLevel) level,
                                                    tag_ptr.get(),
                                                    file_ptr.get(),
                                                    line,
                                                    log_ptr.get());
          }));
  return createInt32Result(static_cast<int32_t>(true));
}

ScriptBridgeInMultiProcess::ScriptBridgeInMultiProcess() {
  set_script_side(new bridge::script::ScriptSideInMultiProcess);
  set_core_side(new CoreSideInScript);
  std::unique_ptr<MultiProcessAndSoInitializer> initializer(
      new MultiProcessAndSoInitializer);

  LOGD("ScriptBridgeInMultiProcess");
  bool passable = initializer->Init(
      [this](IPCHandler *handler) { RegisterIPCCallback(handler); },
      [this](std::unique_ptr<WeexJSConnection> connection) {
        static_cast<bridge::script::ScriptSideInMultiProcess *>(script_side())
            ->set_sender(connection->sender());
        connection_ = std::move(connection);
        LOGD("ScriptBridgeInMultiProcess finish");
        return true;
      },
      [this](const char *page_id, const char *func,
             const char *exception_string) {
        WeexCoreManager::Instance()
            ->getPlatformBridge()
            ->platform_side()
            ->ReportException(page_id, func, exception_string);
      });
  set_is_passable(passable);
}

ScriptBridgeInMultiProcess::~ScriptBridgeInMultiProcess() {
  LOGE("ScriptBridgeInMultiProcess DELETE");
}

void ScriptBridgeInMultiProcess::RegisterIPCCallback(IPCHandler *handler) {
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::SETJSFVERSION),
                           HandleSetJSVersion);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::REPORTEXCEPTION),
                           HandleReportException);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLNATIVE),
                           HandleCallNative);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLNATIVEMODULE),
                           HandleCallNativeModule);
  handler->registerHandler(
      static_cast<uint32_t>(IPCProxyMsg::CALLNATIVECOMPONENT),
      HandleCallNativeComponent);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLADDELEMENT),
                           HandleCallAddElement);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::SETTIMEOUT),
                           HandleSetTimeout);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::NATIVELOG),
                           HandleCallNativeLog);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLCREATEBODY),
                           FunctionCallCreateBody);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLUPDATEFINISH),
                           FunctionCallUpdateFinish);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLCREATEFINISH),
                           FunctionCallCreateFinish);
  handler->registerHandler(
      static_cast<uint32_t>(IPCProxyMsg::CALLREFRESHFINISH),
      FunctionCallRefreshFinish);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLUPDATEATTRS),
                           FunctionCallUpdateAttrs);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLUPDATESTYLE),
                           FunctionCallUpdateStyle);
  handler->registerHandler(
      static_cast<uint32_t>(IPCProxyMsg::CALLREMOVEELEMENT),
      FunctionCallRemoveElement);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLMOVEELEMENT),
                           FunctionCallMoveElement);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLADDEVENT),
                           FunctionCallAddEvent);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLREMOVEEVENT),
                           FunctionCallRemoveEvent);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLGCANVASLINK),
                           HandleCallGCanvasLinkNative);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CALLT3DLINK),
                           HandleT3DLinkNative);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::SETINTERVAL),
                           HandleSetInterval);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::CLEARINTERVAL),
                           HandleClearInterval);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::POSTMESSAGE),
                           HandlePostMessage);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::DISPATCHMESSAGE),
                           HandleDispatchMessage);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::DISPATCHMESSAGESYNC),
                           HandleDispatchMessageSync);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::ONRECEIVEDRESULT),
                           OnReceivedResult);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::UPDATECOMPONENTDATA),
                           UpdateComponentData);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::HEARTBEAT),
                           HeartBeat);
  handler->registerHandler(static_cast<uint32_t>(IPCProxyMsg::POSTLOGDETAIL),
                           HandleLogDetail);
}

}  // namespace WeexCore
