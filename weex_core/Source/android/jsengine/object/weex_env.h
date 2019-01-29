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

#ifndef WEEXV8_WEEXENV_H
#define WEEXV8_WEEXENV_H

#include <mutex>
#include "android/jsengine/task/back_to_weex_core_queue.h"

#include "android/jsengine/task/timer_queue.h"
#include "android/jsengine/weex_ipc_server.h"
#include "android/jsengine/weex_ipc_client.h"

class WeexEnv {

public:
    static WeexEnv *getEnv() {
        static std::once_flag once_flag;
        std::call_once(once_flag, [](){ env_ = new WeexEnv(); });
        return env_;
    }

    WeexEnv();

    bool useWson();

    void setUseWson(bool useWson);

    WeexCore::ScriptBridge *scriptBridge();

    void setScriptBridge(WeexCore::ScriptBridge *scriptBridge);

    int getIpcClientFd() const {
        return ipcClientFd_;
    }

    void setIpcClientFd(int ipcClientFd_) {
        WeexEnv::ipcClientFd_ = ipcClientFd_;
    }

    int getIpcServerFd() const {
        return ipcServerFd_;
    }

    void setIpcServerFd(int ipcServerFd_) {
        WeexEnv::ipcServerFd_ = ipcServerFd_;
    }

    bool enableTrace() const {
        return enableTrace_;
    }

    void setEnableTrace(bool enableTrace_) {
        WeexEnv::enableTrace_ = enableTrace_;
    }


    void initIPC();

    void setEnableBackupThread(bool enable) {
        this->enableBackupThread__ = enable;
    }

    bool enableBackupThread() {
        return enableBackupThread__;
    }

public:
    std::unique_ptr<BackToWeexCoreQueue> m_back_to_weex_core_thread;
    volatile bool isMultiProcess = true;
    std::unique_ptr<WeexIPCClient> m_ipc_client_;
private:
    static WeexEnv *env_;

    volatile bool isUsingWson = true;

    std::unique_ptr<TimerQueue> weexTimerQueue_;

    WeexCore::ScriptBridge *scriptBridge_;

    bool enableBackupThread__ = false;

    volatile int ipcClientFd_;
    volatile int ipcServerFd_;
    volatile bool enableTrace_;
};


#endif //WEEXV8_WEEXENV_H
