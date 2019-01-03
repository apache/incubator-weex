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

#ifndef WEEXV8_WEEXIPCCLIENT_H
#define WEEXV8_WEEXIPCCLIENT_H


#include "third_party/IPC/IPCFutexPageQueue.h"
#include "third_party/IPC/IPCSender.h"
#include "third_party/IPC/IPCListener.h"
#include "third_party/IPC/Serializing/IPCSerializer.h"
#include "third_party/IPC/IPCException.h"
#include "third_party/IPC/IPCHandler.h"

class IPCSender;
class IPCSerializer;
class IPCHandler;
class IPCFutexPageQueue;

class WeexIPCClient {

public:
    explicit WeexIPCClient(int fd);

    inline IPCSerializer *getSerializer() {
        return serializer.get();;
    }

    inline IPCSender *getSender() {
        return sender.get();
    };

public:
    std::unique_ptr<IPCFutexPageQueue> futexPageQueue;
    std::unique_ptr<IPCSender> sender;
    std::unique_ptr<IPCHandler> handler;
    std::unique_ptr<IPCSerializer> serializer;

};


#endif //WEEXV8_WEEXIPCCLIENT_H
