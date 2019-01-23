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

#ifndef WEEX_PROJECT_IPC_BRIDGE_IMPL_H
#define WEEX_PROJECT_IPC_BRIDGE_IMPL_H

#include "android/multiprocess/weex_js_connection.h"
#include "core/bridge/platform_bridge.h"
#include "third_party/IPC/IPCHandler.h"

class IPCArguments;
class IPCResult;
class IPCSender;

namespace WeexCore {

// For multi-process and multi so
class AndroidBridgeInMultiProcess : public PlatformBridge {
 public:
  AndroidBridgeInMultiProcess();
  virtual ~AndroidBridgeInMultiProcess();

  void RegisterIPCCallback(IPCHandler *handler);

#define IPC_METHOD(method_name) \
  static std::unique_ptr<IPCResult> method_name(IPCArguments *arguments);

  IPC_METHOD(InvokeMeasureFunction)
  IPC_METHOD(InvokeLayoutBefore)
  IPC_METHOD(InvokeLayoutAfter)
  IPC_METHOD(SetJSVersion)
  IPC_METHOD(ReportException)
  IPC_METHOD(CallNative)
  IPC_METHOD(CallNativeModule)
  IPC_METHOD(CallNativeComponent)
  IPC_METHOD(SetTimeout)
  IPC_METHOD(NativeLog)
  IPC_METHOD(UpdateFinish)
  IPC_METHOD(RefreshFinish)
  IPC_METHOD(AddEvent)
  IPC_METHOD(RemoveEvent)
  IPC_METHOD(CreateBody)
  IPC_METHOD(AddElement)
  IPC_METHOD(Layout)
  IPC_METHOD(UpdateStyle)
  IPC_METHOD(UpdateAttr)
  IPC_METHOD(CreateFinish)
  IPC_METHOD(RenderSuccess)
  IPC_METHOD(RemoveElement)
  IPC_METHOD(MoveElement)

  IPC_METHOD(AppendTreeCreateFinish)
  IPC_METHOD(HasTransitionPros)
  IPC_METHOD(PostMessage)
  IPC_METHOD(DispatchMessage)

 private:
  std::unique_ptr<WeexJSConnection> connection_;
  std::unique_ptr<IPCHandler> handler_;
  std::unique_ptr<IPCHandler> server_handler_;
  DISALLOW_COPY_AND_ASSIGN(AndroidBridgeInMultiProcess);
};
}  // namespace WeexCore

#endif  // WEEX_PROJECT_IPC_BRIDGE_IMPL_H
