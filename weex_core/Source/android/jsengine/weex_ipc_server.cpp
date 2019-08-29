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
#include "android/jsengine/weex_ipc_server.h"
#include "android/jsengine/bridge/platform/platform_side_multi_process.h"
#include "android/jsengine/bridge/platform/platform_bridge_in_multi_process.h"
#include "android/jsengine/object/weex_env.h"
#include "android/jsengine/weex_runtime.h"
#include "core/manager/weex_core_manager.h"
#include "android/jsengine/weex_jsc_utils.h"
#include "base/log_defines.h"
#include "android/jsengine/object/log_utils_jss.h"
#ifdef USE_JS_RUNTIME
#include "base/crash/crash_handler.h"
#include <unistd.h>
#include <sys/mman.h>
//using namespace crash_handler;
#else
using namespace JSC;
using namespace WTF;
using namespace WEEXICU;
#endif

struct WeexJSServer::WeexJSServerImpl {
    WeexJSServerImpl(int serverFd, int clientFd, bool enableTrace, std::string crashFileName);

    std::unique_ptr<IPCFutexPageQueue> futexPageQueue;
    std::unique_ptr<IPCSender> sender;
    std::unique_ptr<IPCHandler> handler;
    std::unique_ptr<IPCListener> listener;
    std::unique_ptr<IPCSerializer> serializer;
};

WeexJSServer::WeexJSServerImpl::WeexJSServerImpl(int serverFd, int clientFd, bool enableTrace, std::string crashFileName) {
    WeexEnv::getEnv()->setIpcServerFd(serverFd);
    WeexEnv::getEnv()->setIpcClientFd(clientFd);
    WeexEnv::getEnv()->setEnableTrace(enableTrace);
    int _fd = serverFd;
    void *base = mmap(nullptr, IPCFutexPageQueue::ipc_size, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, 0);
    if (base == MAP_FAILED) {
        int _errno = errno;
        close(_fd);
        //throw IPCException("failed to map ashmem region: %s", strerror(_errno));
    }
    close(_fd);
    futexPageQueue.reset(new IPCFutexPageQueue(base, IPCFutexPageQueue::ipc_size, 1));
    handler = std::move(createIPCHandler());
    sender = std::move(createIPCSender(futexPageQueue.get(), handler.get()));
    listener = std::move(createIPCListener(futexPageQueue.get(), handler.get()));
    serializer = std::move(createIPCSerializer());
    weex::base::LogImplement::getLog()->setLogImplement(new LogUtilsJSS());
    WeexEnv::getEnv()->init_crash_handler(crashFileName);

    WeexEnv::getEnv()->m_back_to_weex_core_thread.reset(new BackToWeexCoreQueue());
    WeexEnv::getEnv()->m_back_to_weex_core_thread.get()->init();

    auto pQueue = WeexEnv::getEnv()->m_back_to_weex_core_thread.get();
    while (!pQueue->isInitOk) {
        continue;
    }

    //WeexEnv::getEnv()->initJSC(true);
}

WeexJSServer::WeexJSServer(int serverFd, int clientFd, bool enableTrace, std::string crashFileName)
        : m_impl(new WeexJSServerImpl(serverFd, clientFd, enableTrace, crashFileName)) {


    IPCHandler *handler = m_impl->handler.get();
}

WeexJSServer::~WeexJSServer() {
}

void WeexJSServer::loop() {
    m_impl->listener->listen();
}

IPCSender *WeexJSServer::getSender() {
    return m_impl->sender.get();
}

IPCSerializer *WeexJSServer::getSerializer() {
    return m_impl->serializer.get();
}

IPCHandler *WeexJSServer::getHandler() {
    return m_impl->handler.get();
}
