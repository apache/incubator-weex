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
//
// Created by Darin on 2018/7/22.
//

#include "weex_env.h"

WeexEnv *WeexEnv::env_ = nullptr;

WeexCore::ScriptBridge *WeexEnv::scriptBridge() { return scriptBridge_; }

bool WeexEnv::useWson() { return isUsingWson; }

void WeexEnv::setUseWson(bool useWson) { isUsingWson = useWson; }

void WeexEnv::setScriptBridge(WeexCore::ScriptBridge *scriptBridge) { scriptBridge_ = scriptBridge; }

void WeexEnv::initIPC() {
    // init IpcClient in io Thread
    isMultiProcess = true;
    m_ipc_client_.reset(new WeexIPCClient(ipcClientFd_));
}
WeexEnv::WeexEnv() {
    this->enableBackupThread__ = false;
    this->isUsingWson = true;
    this->isJscInitOk_ = false;
    this->m_cache_task_ = true;
}

void WeexEnv::initJSC(bool isMultiProgress) {
    static std::once_flag initJSCFlag;
    std::call_once(initJSCFlag, [isMultiProgress]{
      if (!WEEXICU::initICUEnv(isMultiProgress)) {
          LOGE("failed to init ICUEnv single process");
          // return false;
      }

      Options::enableRestrictedOptions(true);
// Initialize JSC before getting VM.
      WTF::initializeMainThread();
      initHeapTimer();
      JSC::initializeThreading();
#if ENABLE(WEBASSEMBLY)
      JSC::Wasm::enableFastMemory();
#endif
    });
}
void WeexEnv::init_crash_handler(std::string crashFileName) {
  // initialize signal handler
  isMultiProcess = true;
  crashHandler.reset(new crash_handler::CrashHandlerInfo(crashFileName));
  crashHandler->initializeCrashHandler();
}
bool WeexEnv::is_app_crashed() {
  if(!isMultiProcess)
    return false;
  return crashHandler->is_crashed();
}
volatile bool WeexEnv::can_m_cache_task_() const {
  return m_cache_task_;
}
void WeexEnv::set_m_cache_task_(volatile bool m_cache_task_) {
  WeexEnv::m_cache_task_ = m_cache_task_;
}
WeexEnv::~WeexEnv() {
  wson::destory();
}


bool WeexEnv::sendLog(int level,
                      const char *tag,
                      const char *file,
                      unsigned long line,
                      const char *log) {

  if (!isMultiProcess) {
    return false;
  }

  if (m_back_to_weex_core_thread.get() == nullptr
  || m_back_to_weex_core_thread.get()->isInitOk == false) {
    return false;
  }

  BackToWeexCoreQueue::IPCTask *ipc_task = new BackToWeexCoreQueue::IPCTask(
      IPCProxyMsg::POSTLOGDETAIL);
  auto level_str = std::to_string(level);
  ipc_task->addParams(level_str.c_str(), level_str.length());
  ipc_task->addParams(tag);
  ipc_task->addParams(file);
  auto line_str = std::to_string(line);
  ipc_task->addParams(line_str.c_str(), line_str.length());
  ipc_task->addParams(log);
  m_back_to_weex_core_thread.get()->addTask(ipc_task);
  return true;
}
