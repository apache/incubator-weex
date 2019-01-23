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

#include "multi_process_and_so_initializer.h"

#include "android/weex_extend_js_api.h"
#include "android/multiprocess/weex_js_connection.h"
#include "base/android/log_utils.h"
#include "third_party/IPC/Buffering/IPCBuffer.h"
#include "third_party/IPC/IPCException.h"
#include "third_party/IPC/IPCHandler.h"
#include "third_party/IPC/IPCResult.h"
#include "third_party/IPC/Serializing/IPCSerializer.h"


namespace WeexCore {

bool MultiProcessAndSoInitializer::Init(const std::function<void(IPCHandler*)>& OnHandlerCreated,
                                        const std::function<bool(std::unique_ptr<WeexJSConnection>, std::unique_ptr<IPCHandler>, std::unique_ptr<IPCHandler>)>& OnInitFinished,
                                        const std::function<void(const char*, const char*, const char*)>& ReportException){
  bool reinit = false;
  LOGE("MultiProcessAndSoInitializer IS IN init");
startInitFrameWork:
  try {
    auto handler = std::move(createIPCHandler());
    auto server_handler = std::move(createIPCHandler());
    OnHandlerCreated(server_handler.get());
    std::unique_ptr<WeexJSConnection> connection(new WeexJSConnection());
    auto sender = connection->start(handler.get(), server_handler.get(), reinit);
    if (sender == nullptr) {
      LOGE("JSFramwork init start sender is null");
      if (!reinit) {
        reinit = true;
        goto startInitFrameWork;
      } else {
        return false;
      }
    } else {
      OnInitFinished(std::move(connection), std::move(handler), std::move(server_handler));
    }
  } catch (IPCException& e) {
    LOGE("WeexProxy catch：%s", e.msg());
    if (!reinit) {
      reinit = true;
      goto startInitFrameWork;
    } else {
      LOGE("%s", e.msg());
      ReportException("", "initFramework", e.msg());
      return false;
    }
  }
  return true;
}
}  // namespace WeexCore
