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

#include "script_bridge_in_multi_so.h"
#include <dlfcn.h>
#include <malloc.h>

#include "android/bridge/multi_so_initializer.h"
#include "android/bridge/script/script_side_in_multi_so.h"
#include "android/utils/ipc_string_result.h"
#include "android/utils/params_utils.h"
#include "base/android/log_utils.h"
#include "base/make_copyable.h"
#include "base/thread/waitable_event.h"
#include "core/bridge/script/core_side_in_script.h"
#include "core/manager/weex_core_manager.h"

namespace WeexCore {

inline static char *copyStr(const char *str, int length = 0) {
  char *ret = nullptr;
  if (str == nullptr) return ret;
  int strLen = length == 0 ? strlen(str) : length;
  ret = new char[strLen + 1];
  memcpy(ret, str, static_cast<size_t>(strLen));
  ret[strLen] = '\0';
  return ret;
}

ScriptBridgeInMultiSo::ScriptBridgeInMultiSo() {
  set_core_side(new CoreSideInScript);
  set_script_side(new bridge::script::ScriptSideInMultiSo);
  std::unique_ptr<MultiSoInitializer> initializer(new MultiSoInitializer);
  bool passable = initializer->Init(
      [this](void *handle) {
        LOGE("dlopen so and call function");

        typedef FunctionsExposedByJS *(*ExchangeJSBridgeFunctions)(
            FunctionsExposedByCore *);
        auto exchange_script_bridge_functions =
            (ExchangeJSBridgeFunctions)dlsym(handle,
                                             "ExchangeJSBridgeFunctions");
        if (!exchange_script_bridge_functions) {
          return false;
        }

        FunctionsExposedByCore *platform_expose_functions =
            GetExposedFunctions();
        auto script_side_functions =
            exchange_script_bridge_functions(platform_expose_functions);

        static_cast<bridge::script::ScriptSideInMultiSo *>(script_side())
            ->set_script_side_functions(script_side_functions);
        return true;
      },
      [this](const char *status_code, const char *error_msg) {
        WeexCoreManager::Instance()
            ->getPlatformBridge()
            ->platform_side()
            ->ReportNativeInitStatus(status_code, error_msg);
      });
  set_is_passable(passable);
}

ScriptBridgeInMultiSo::~ScriptBridgeInMultiSo() {}

static void CallNative(const char *page_id, const char *task,
                       const char *callback) {
  //  WeexCoreManager::Instance()->script_bridge()->core_side()->CallNative(
  //      page_id, task, callback);

  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable([page_id = std::string(page_id),
                                task = std::string(task),
                                callback = std::string(callback)] {
        WeexCoreManager::Instance()->script_bridge()->core_side()->CallNative(
            page_id.c_str(), task.c_str(), callback.c_str());
      }));
}

static std::unique_ptr<ValueWithType> CallNativeModule(
    const char *page_id, const char *module, const char *method,
    const char *arguments, int arguments_length, const char *options,
    int options_length) {
  //  auto ret = WeexCoreManager::Instance()
  //      ->script_bridge()
  //      ->core_side()
  //      ->CallNativeModule(page_id, module, method, arguments,
  //      arguments_length,
  //                         options, options_length);
  weex::base::WaitableEvent event;
  std::unique_ptr<ValueWithType> ret;
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable(
          [page_id = std::string(page_id), module = std::string(module),
           method = std::string(method),
           arguments =
               std::unique_ptr<char[]>(copyStr(arguments, arguments_length)),
           arguments_length = arguments_length,
           options = std::unique_ptr<char[]>(copyStr(options, options_length)),
           options_length = options_length, e = &event, ret = &ret] {
            *ret = WeexCoreManager::Instance()
                       ->script_bridge()
                       ->core_side()
                       ->CallNativeModule(page_id.c_str(), module.c_str(),
                                          method.c_str(), arguments.get(),
                                          arguments_length, options.get(),
                                          options_length);
            e->Signal();
          }));
  event.Wait();
    return ret;
}

static void CallNativeComponent(const char *page_id, const char *ref,
                                const char *method, const char *arguments,
                                int arguments_length, const char *options,
                                int options_length) {
  //  WeexCoreManager::Instance()
  //      ->script_bridge()
  //      ->core_side()
  //      ->CallNativeComponent(page_id, ref, method, arguments,
  //      arguments_length,
  //                            options, options_length);

  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable(
          [page_id = std::string(page_id), ref = std::string(ref),
           method = std::string(method),
           arguments =
               std::unique_ptr<char[]>(copyStr(arguments, arguments_length)),
           arguments_length = arguments_length,
           options = std::unique_ptr<char[]>(copyStr(options, options_length)),
           options_length = options_length] {
            WeexCoreManager::Instance()
                ->script_bridge()
                ->core_side()
                ->CallNativeComponent(page_id.c_str(), ref.c_str(),
                                      method.c_str(), arguments.get(),
                                      arguments_length, options.get(),
                                      options_length);
          }));
}

static void AddElement(const char *page_id, const char *parent_ref,
                       const char *dom_str, int domLen, const char *index_str) {
  //  WeexCoreManager::Instance()->script_bridge()->core_side()->AddElement(
  //      page_id, parent_ref, dom_str, domLen, index_str);

  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable([page_id = std::string(page_id),
                                parent_ref = std::string(parent_ref),
                                dom_str = std::unique_ptr<char[]>(
                                    copyStr(dom_str, domLen)),
                                dom_len = domLen,
                                index_str = std::string(index_str)] {
        WeexCoreManager::Instance()->script_bridge()->core_side()->AddElement(
            page_id.c_str(), parent_ref.c_str(), dom_str.get(), dom_len,
            index_str.c_str());
      }));
}

static void SetTimeout(const char *callback_id, const char *time) {
  //  WeexCoreManager::Instance()->script_bridge()->core_side()->SetTimeout(
  //      callback_id, time);

  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable([callback_id = std::string(callback_id),
                                time = std::string(time)] {
        WeexCoreManager::Instance()->script_bridge()->core_side()->SetTimeout(
            callback_id.c_str(), time.c_str());
      }));
}

static void NativeLog(const char *str_array) {
  //  WeexCoreManager::Instance()->script_bridge()->core_side()->NativeLog(
  //      str_array);

  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable([str_array = std::string(str_array)] {
        WeexCoreManager::Instance()->script_bridge()->core_side()->NativeLog(
            str_array.c_str());
      }));
}

static void CreateBody(const char *page_id, const char *dom_str, int domLen) {
  //  WeexCoreManager::Instance()->script_bridge()->core_side()->CreateBody(
  //      page_id, dom_str, domLen);

  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable([page_id = std::string(page_id),
                                dom_str = std::unique_ptr<char[]>(
                                    copyStr(dom_str, domLen)),
                                dom_len = domLen] {
        WeexCoreManager::Instance()->script_bridge()->core_side()->CreateBody(
            page_id.c_str(), dom_str.get(), dom_len);
      }));
}

static int UpdateFinish(const char *page_id, const char *task, int taskLen,
                        const char *callback, int callbackLen) {
  //  return WeexCoreManager::Instance()
  //      ->script_bridge()
  //      ->core_side()
  //      ->UpdateFinish(page_id, task, taskLen, callback, callbackLen);

  weex::base::WaitableEvent event;
  int result = -1;
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable(
          [page_id = std::string(page_id),
           task = std::unique_ptr<char[]>(copyStr(task, taskLen)),
           task_len = taskLen,
           callback = std::unique_ptr<char[]>(copyStr(callback, callbackLen)),
           callback_len = callbackLen, event = &event, result = &result] {
            *result = WeexCoreManager::Instance()
                          ->script_bridge()
                          ->core_side()
                          ->UpdateFinish(page_id.c_str(), task.get(), task_len,
                                         callback.get(), callback_len);
            event->Signal();
          }));
  event.Wait();
  return result;
}

static void CreateFinish(const char *page_id) {
  //  WeexCoreManager::Instance()->script_bridge()->core_side()->CreateFinish(
  //      page_id);
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable([page_id = std::string(page_id)] {
        WeexCoreManager::Instance()->script_bridge()->core_side()->CreateFinish(
            page_id.c_str());
      }));
}

static int RefreshFinish(const char *page_id, const char *task,
                         const char *callback) {
  //  return WeexCoreManager::Instance()
  //      ->script_bridge()
  //      ->core_side()
  //      ->RefreshFinish(page_id, task, callback);

  int result = -1;
  weex::base::WaitableEvent event;
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable(
          [page_id = std::string(page_id), task = std::string(task),
           callback = std::string(callback), event = &event] {
            WeexCoreManager::Instance()
                ->script_bridge()
                ->core_side()
                ->RefreshFinish(page_id.c_str(), task.c_str(),
                                callback.c_str());
            event->Signal();
          }));
  event.Wait();

  return result;
}

static void UpdateAttrs(const char *page_id, const char *ref, const char *data,
                        int dataLen) {
  //  WeexCoreManager::Instance()->script_bridge()->core_side()->UpdateAttrs(
  //      page_id, ref, data, dataLen);
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable([page_id = std::string(page_id),
                                ref = std::string(ref),
                                data = std::unique_ptr<char[]>(
                                    copyStr(data, dataLen)),
                                data_len = dataLen] {
        WeexCoreManager::Instance()->script_bridge()->core_side()->UpdateAttrs(
            page_id.c_str(), ref.c_str(), data.get(), data_len);
      }));
}

static void UpdateStyle(const char *page_id, const char *ref, const char *data,
                        int dataLen) {
  //  WeexCoreManager::Instance()->script_bridge()->core_side()->UpdateStyle(
  //      page_id, ref, data, dataLen);
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable([page_id = std::string(page_id),
                                ref = std::string(ref),
                                data = std::unique_ptr<char[]>(
                                    copyStr(data, dataLen)),
                                data_len = dataLen] {
        WeexCoreManager::Instance()->script_bridge()->core_side()->UpdateStyle(
            page_id.c_str(), ref.c_str(), data.get(), data_len);
      }));
}

static void RemoveElement(const char *page_id, const char *ref) {
  //  WeexCoreManager::Instance()->script_bridge()->core_side()->RemoveElement(
  //      page_id, ref);
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable(
          [page_id = std::string(page_id), ref = std::string(ref)] {
            WeexCoreManager::Instance()
                ->script_bridge()
                ->core_side()
                ->RemoveElement(page_id.c_str(), ref.c_str());
          }));
}

static void MoveElement(const char *page_id, const char *ref,
                        const char *parent_ref, int index) {
  //  WeexCoreManager::Instance()->script_bridge()->core_side()->MoveElement(
  //      page_id, ref, parent_ref, index);
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable([page_id = std::string(page_id),
                                ref = std::string(ref),
                                parent_ref = std::string(parent_ref),
                                index = index] {
        WeexCoreManager::Instance()->script_bridge()->core_side()->MoveElement(
            page_id.c_str(), ref.c_str(), parent_ref.c_str(), index);
      }));
}

static void AddEvent(const char *page_id, const char *ref, const char *event) {
  //  WeexCoreManager::Instance()->script_bridge()->core_side()->AddEvent(
  //      page_id, ref, event);
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable([page_id = std::string(page_id),
                                ref = std::string(ref),
                                event = std::string(event)] {
        WeexCoreManager::Instance()->script_bridge()->core_side()->AddEvent(
            page_id.c_str(), ref.c_str(), event.c_str());
      }));
}

static void RemoveEvent(const char *page_id, const char *ref,
                        const char *event) {
  //  WeexCoreManager::Instance()->script_bridge()->core_side()->RemoveEvent(
  //      page_id, ref, event);
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable([page_id = std::string(page_id),
                                ref = std::string(ref),
                                event = std::string(event)] {
        WeexCoreManager::Instance()->script_bridge()->core_side()->RemoveEvent(
            page_id.c_str(), ref.c_str(), event.c_str());
      }));
}
static const char *CallGCanvasLinkNative(const char *context_id, int type,
                                         const char *arg) {

    weex::base::WaitableEvent event;
    char *ret = nullptr;

    WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
            weex::base::MakeCopyable([page_id = std::string(context_id),
                                             t = type,
                                             args = std::string(arg), e = &event, ret = &ret] {
                *ret = const_cast<char *>(WeexCoreManager::Instance()
                                        ->script_bridge()
                                        ->core_side()
                                        ->CallGCanvasLinkNative(page_id.c_str(), t, args.c_str()));
                e->Signal();
            }));
    event.Wait();

  return ret;
}

static int SetInterval(const char *page_id, const char *callback_id,
                       const char *time) {
  return WeexCoreManager::Instance()->script_bridge()->core_side()->SetInterval(
      page_id, callback_id, time);
}

static void ClearInterval(const char *page_id, const char *callback_id) {
  //  WeexCoreManager::Instance()->script_bridge()->core_side()->ClearInterval(
  //      page_id, callback_id);

  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable([page_id = std::string(page_id),
                                callback_id = std::string(callback_id)] {
        WeexCoreManager::Instance()
            ->script_bridge()
            ->core_side()
            ->ClearInterval(page_id.c_str(), callback_id.c_str());
      }));
}

static const char *CallT3DLinkNative(int type, const char *arg) {

    weex::base::WaitableEvent event;
    char *ret = nullptr;

    WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
            weex::base::MakeCopyable([
                                             t = type,
                                             args = std::string(arg), e = &event, ret = &ret] {
                *ret = const_cast<char *>(WeexCoreManager::Instance()
                        ->script_bridge()
                        ->core_side()
                        ->CallT3DLinkNative(t, args.c_str()));
                e->Signal();
            }));
    event.Wait();

  return ret;
}

static void PostMessage(const char *vim_id, const char *data, int dataLength) {
  //  WeexCoreManager::Instance()->script_bridge()->core_side()->PostMessage(vim_id,
  //                                                                         data);
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable([vim_id = std::string(vim_id),
                                data = std::string(data), length = dataLength] {
        WeexCoreManager::Instance()->script_bridge()->core_side()->PostMessage(
            vim_id.c_str(), data.c_str(),length);
      }));
}

static void DispatchMessage(const char *client_id, const char *data, int dataLength,
                            const char *callback, const char *vm_id) {
  //  WeexCoreManager::Instance()->script_bridge()->core_side()->DispatchMessage(
  //      client_id, data, callback, vm_id);
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable(
          [client_id = std::string(client_id), data = std::string(data),
           callback = std::string(callback), vm_id = std::string(vm_id), length = dataLength] {
            WeexCoreManager::Instance()
                ->script_bridge()
                ->core_side()
                ->DispatchMessage(client_id.c_str(), data.c_str(),length,
                                  callback.c_str(), vm_id.c_str());
          }));
}

static std::unique_ptr<WeexJSResult> DispatchMessageSync(const char *client_id,
                                                         const char *data,
                                                         int dataLength,
                                                         const char *vm_id) {
  weex::base::WaitableEvent event;
  std::unique_ptr<WeexJSResult> result;
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable([client_id = std::string(client_id),
                                data = std::string(data),
                                vm_id = std::string(vm_id), length = dataLength,
                                e = &event, r = &result]() {
        *r = WeexCoreManager::Instance()
                 ->script_bridge()
                 ->core_side()
                 ->DispatchMessageSync(client_id.c_str(), data.c_str(), length,
                                       vm_id.c_str());
        e->Signal();
      }));
  event.Wait();
  return result;
}

static void OnReceivedResult(long callback_id,
                             std::unique_ptr<WeexJSResult> &result) {
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable([callback_id, result = std::move(result)]() {
        WeexCoreManager::Instance()
            ->script_bridge()
            ->core_side()
            ->OnReceivedResult(
                callback_id,
                const_cast<std::unique_ptr<WeexJSResult> &>(result));
      }));
}

static void UpdateComponentData(const char* page_id,
                                const char* cid,
                                const char* json_data) {
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable(
          [page_id = std::string(page_id), cid = std::string(cid), json_data = std::string(json_data)]() {
            WeexCoreManager::Instance()
                ->script_bridge()
                ->core_side()
                ->UpdateComponentData(page_id.c_str(), cid.c_str(), json_data.c_str());
          }));
}

static void ReportException(const char *page_id, const char *func,
                            const char *exception_string) {
  //  WeexCoreManager::Instance()->script_bridge()->core_side()->ReportException(
  //      page_id, func, exception_string);
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable([page_id = std::string(page_id),
                                func = std::string(func),
                                exception = std::string(exception_string)] {
        WeexCoreManager::Instance()
            ->script_bridge()
            ->core_side()
            ->ReportException(page_id.c_str(), func.c_str(), exception.c_str());
      }));
}

static void SetJSVersion(const char *js_version) {
  //  WeexCoreManager::Instance()->script_bridge()->core_side()->SetJSVersion(
  //      js_version);
  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable([version = std::string(js_version)] {
        WeexCoreManager::Instance()->script_bridge()->core_side()->SetJSVersion(
            version.c_str());
      }));
}

FunctionsExposedByCore *ScriptBridgeInMultiSo::GetExposedFunctions() {
  FunctionsExposedByCore temp = {SetJSVersion,
                                 ReportException,
                                 CallNative,
                                 CallNativeModule,
                                 CallNativeComponent,
                                 AddElement,
                                 SetTimeout,
                                 NativeLog,
                                 CreateBody,
                                 UpdateFinish,
                                 CreateFinish,
                                 RefreshFinish,
                                 UpdateAttrs,
                                 UpdateStyle,
                                 RemoveElement,
                                 MoveElement,
                                 AddEvent,
                                 RemoveEvent,
                                 SetInterval,
                                 ClearInterval,
                                 CallGCanvasLinkNative,
                                 CallT3DLinkNative,
                                 PostMessage,
                                 DispatchMessage,
                                 DispatchMessageSync,
                                 OnReceivedResult,
                                 UpdateComponentData};
  auto functions =
      (FunctionsExposedByCore *)malloc(sizeof(FunctionsExposedByCore));
  memset(functions, 0, sizeof(FunctionsExposedByCore));
  memcpy(functions, &temp, sizeof(FunctionsExposedByCore));
  return functions;
}
}  // namespace WeexCore
