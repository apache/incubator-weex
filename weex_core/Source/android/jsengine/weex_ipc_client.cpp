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
// Created by Darin on 06/07/2018.
//
#include "android/jsengine/weex_ipc_client.h"

#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>
#include <memory>
#include "base/utils/log_utils.h"

WeexIPCClient::WeexIPCClient(int fd) {
    void *base = mmap(nullptr, IPCFutexPageQueue::ipc_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (base == MAP_FAILED) {
        int _errno = errno;
        close(fd);
        //throw IPCException("failed to map ashmem region: %s", strerror(_errno));
    }


    futexPageQueue.reset(new IPCFutexPageQueue(base, IPCFutexPageQueue::ipc_size, 1));
    handler = std::move(createIPCHandler());
    sender = std::move(createIPCSender(futexPageQueue.get(), handler.get()));
    serializer = std::move(createIPCSerializer());
    close(fd);
}

