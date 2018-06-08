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
/*
 * Implementation of the user-space ashmem API for devices, which have our
 * ashmem-enabled kernel. See ashmem-sim.c for the "fake" tmp-based version,
 * used by the simulator.
 */

#ifndef IPCCOMMUNICATOR_H
#define IPCCOMMUNICATOR_H
#include <memory>

class IPCResult;
class IPCArguments;
class IPCBuffer;
class IPCFutexPageQueue;
class IPCCommunicator {
protected:
    explicit IPCCommunicator(IPCFutexPageQueue* futexPageQueue);
    virtual ~IPCCommunicator();

    std::unique_ptr<IPCResult> assembleResult();
    std::unique_ptr<IPCArguments> assembleArguments();
    std::unique_ptr<IPCBuffer> generateResultBuffer(IPCResult*);
    void doSendBufferOnly(IPCBuffer* buffer);
    uint32_t doReadPackage();
    const char* getBlob();
    void releaseBlob();

private:
    void doSendBufferOnly(const void* data, size_t s);
    size_t doSendBufferPage(const void* data, size_t s, size_t pageSize);
    void doRecvBufferOnly(void* data, size_t s);
    std::unique_ptr<char[]> m_package;
    // weakref to a IPCFutexPageQueue object.
    IPCFutexPageQueue* m_futexPageQueue;
};
#endif /* IPCCOMMUNICATOR_H */
