/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Implementation of the user-space ashmem API for devices, which have our
 * ashmem-enabled kernel. See ashmem-sim.c for the "fake" tmp-based version,
 * used by the simulator.
 */

#include "IPCSender.h"
#include "Buffering/IPCBuffer.h"
#include "IPCArguments.h"
#include "IPCByteArray.h"
#include "IPCCheck.h"
#include "IPCCommunicator.h"
#include "IPCException.h"
#include "IPCHandler.h"
#include "IPCResult.h"
#include "IPCString.h"
#include "Serializing/IPCSerializer.h"
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <vector>

namespace {
class IPCSenderImpl : public IPCCommunicator, public IPCSender {
public:
    IPCSenderImpl(IPCFutexPageQueue*, IPCHandler* handler);
    ~IPCSenderImpl();
    std::unique_ptr<IPCResult> send(IPCBuffer*) override;

private:
    bool checkBufferAsync(IPCBuffer* buffer);
    IPCHandler* m_handler;
};

IPCSenderImpl::IPCSenderImpl(IPCFutexPageQueue* futexPageQueue, IPCHandler* handler)
    : IPCCommunicator(futexPageQueue)
    , m_handler(handler)
{
}

IPCSenderImpl::~IPCSenderImpl()
{
}

std::unique_ptr<IPCResult> IPCSenderImpl::send(IPCBuffer* buffer)
{
    doSendBufferOnly(buffer);
    if (checkBufferAsync(buffer))
        return createVoidResult();
    while (true) {
        uint32_t msg = doReadPackage();
        bool isAsync = !!(msg & MSG_FLAG_ASYNC);
        msg &= MSG_MASK;
        if (msg == MSG_END) {
            std::unique_ptr<IPCResult> result = assembleResult();
            releaseBlob();
            return result;
        } else if (msg == MSG_TERMINATE) {
            releaseBlob();
            throw IPCException("peer terminates");
        }
        std::unique_ptr<IPCArguments> arguments = assembleArguments();
        releaseBlob();
        std::unique_ptr<IPCResult> sendBack = m_handler->handle(msg, arguments.get());
        if (!isAsync) {
            std::unique_ptr<IPCBuffer> resultBuffer = generateResultBuffer(sendBack.get());
            doSendBufferOnly(resultBuffer.get());
        }
    }
}

bool IPCSenderImpl::checkBufferAsync(IPCBuffer* buffer)
{
    uint32_t msg = *static_cast<const uint32_t*>(buffer->get());
    return !!(msg & MSG_FLAG_ASYNC);
}
}

std::unique_ptr<IPCSender> createIPCSender(IPCFutexPageQueue* futexPageQueue, IPCHandler* handler)
{
    return std::unique_ptr<IPCSender>(new IPCSenderImpl(futexPageQueue, handler));
}
