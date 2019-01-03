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

#include "IPCCommunicator.h"
#include "Buffering/IPCBuffer.h"
#include "IPCArguments.h"
#include "IPCByteArray.h"
#include "IPCCheck.h"
#include "IPCException.h"
#include "IPCFutexPageQueue.h"
#include "IPCLog.h"
#include "IPCResult.h"
#include "IPCString.h"
#include "Serializing/IPCSerializer.h"
#include "futex.h"
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <vector>

namespace {
class BufferAssembler
    : public IPCResult,
      public IPCArguments {
public:
    void readFromBuffer(const char*);
    void readTypes(const char*&);
    void readData(const char*&);
    // IPCResult
    const void* getData() override;
    IPCType getType() override;
    const uint16_t* getStringContent() override;
    size_t getStringLength() override;
    const char* getByteArrayContent() override;
    size_t getByteArrayLength() override;

    // IPCArguments
    const void* getData(int index) override;
    IPCType getType(int index) override;
    size_t getCount() override;

private:
    std::vector<uint32_t> m_types;
    std::vector<std::unique_ptr<char[]>> m_datas;
};

void BufferAssembler::readFromBuffer(const char* blob)
{
    readTypes(blob);
    readData(blob);
}

void BufferAssembler::readTypes(const char*& blob)
{
    while (true) {
        uint32_t type;
        type = *reinterpret_cast<const uint32_t*>(blob);
        blob += sizeof(uint32_t);
        if (type == static_cast<uint32_t>(IPCType::END))
            break;
        m_types.emplace_back(type);
    }
}

void BufferAssembler::readData(const char*& blob)
{
    for (uint32_t type : m_types) {
        switch (static_cast<IPCType>(type)) {
        case IPCType::INT32: {
            int32_t n;
            memcpy(&n, blob, sizeof(n));
            blob += sizeof(n);
            std::unique_ptr<char[]> data(new char[sizeof(n)]);
            *reinterpret_cast<int32_t*>(data.get()) = n;
            m_datas.emplace_back(std::move(data));
        } break;
        case IPCType::INT64: {
            int64_t n;
            memcpy(&n, blob, sizeof(n));
            blob += sizeof(n);
            std::unique_ptr<char[]> data(new char[sizeof(n)]);
            *reinterpret_cast<int64_t*>(data.get()) = n;
            m_datas.emplace_back(std::move(data));
        } break;
        case IPCType::FLOAT: {
            float n;
            memcpy(&n, blob, sizeof(n));
            blob += sizeof(n);
            std::unique_ptr<char[]> data(new char[sizeof(n)]);
            *reinterpret_cast<float*>(data.get()) = n;
            m_datas.emplace_back(std::move(data));
        } break;
        case IPCType::DOUBLE: {
            double n;
            memcpy(&n, blob, sizeof(n));
            blob += sizeof(n);
            std::unique_ptr<char[]> data(new char[sizeof(n)]);
            *reinterpret_cast<double*>(data.get()) = n;
            m_datas.emplace_back(std::move(data));
        } break;
        case IPCType::JSONSTRING:
        case IPCType::STRING: {
            uint32_t n;
            memcpy(&n, blob, sizeof(n));
            blob += sizeof(n);
            size_t byteLength = n * sizeof(uint16_t);
            std::unique_ptr<char[]> data(new char[byteLength + sizeof(IPCString)]);
            IPCString* s = reinterpret_cast<IPCString*>(data.get());
            s->length = n;
            memcpy(s->content, blob, byteLength);
            blob += byteLength;
            m_datas.emplace_back(std::move(data));
        } break;
        case IPCType::BYTEARRAY: {
            uint32_t n;
            memcpy(&n, blob, sizeof(n));
            blob += sizeof(n);
            size_t byteLength = n;
            std::unique_ptr<char[]> data(new char[byteLength + sizeof(IPCByteArray)]);
            IPCByteArray* s = reinterpret_cast<IPCByteArray*>(data.get());
            s->length = n;
            memcpy(s->content, blob, byteLength);
            blob += byteLength;
            s->content[byteLength] = '\0';
            m_datas.emplace_back(std::move(data));
        } break;
        case IPCType::JSUNDEFINED:
        case IPCType::VOID:
            m_datas.emplace_back();
            break;
        case IPCType::END:
        default:
            IPC_UNREACHABLE();
        }
    }
}

// IPCResult
const void* BufferAssembler::getData()
{
    return getData(0);
}

IPCType BufferAssembler::getType()
{
    return getType(0);
}

const uint16_t* BufferAssembler::getStringContent()
{
    IPC_DCHECK((getType(0) == IPCType::STRING) || (getType(0) == IPCType::JSONSTRING));
    const IPCString* s = reinterpret_cast<const IPCString*>(getData(0));
    return s->content;
}

size_t BufferAssembler::getStringLength()
{
    IPC_DCHECK((getType(0) == IPCType::STRING) || (getType(0) == IPCType::JSONSTRING));
    const IPCString* s = reinterpret_cast<const IPCString*>(getData(0));
    return s->length;
}

const char* BufferAssembler::getByteArrayContent()
{
    IPC_DCHECK(getType(0) == IPCType::BYTEARRAY);
    const IPCByteArray* s = reinterpret_cast<const IPCByteArray*>(getData(0));
    return s->content;
}

size_t BufferAssembler::getByteArrayLength()
{
    IPC_DCHECK(getType(0) == IPCType::BYTEARRAY);
    const IPCByteArray* s = reinterpret_cast<const IPCByteArray*>(getData(0));
    return s->length;
}

// IPCArguments
const void* BufferAssembler::getData(int index)
{
    return m_datas.at(index).get();
}

IPCType BufferAssembler::getType(int index)
{
    return static_cast<IPCType>(m_types.at(index));
}

size_t BufferAssembler::getCount()
{
    return m_types.size();
}
}

IPCCommunicator::IPCCommunicator(IPCFutexPageQueue* futexPageQueue)
    : m_futexPageQueue(futexPageQueue)
{
}

IPCCommunicator::~IPCCommunicator()
{
}

std::unique_ptr<IPCResult> IPCCommunicator::assembleResult()
{
    std::unique_ptr<BufferAssembler> bufferAssembler(new BufferAssembler());
    bufferAssembler->readFromBuffer(getBlob());
    return std::unique_ptr<IPCResult>(bufferAssembler.release());
}

std::unique_ptr<IPCArguments> IPCCommunicator::assembleArguments()
{
    std::unique_ptr<BufferAssembler> bufferAssembler(new BufferAssembler());
    bufferAssembler->readFromBuffer(getBlob());
    return std::unique_ptr<IPCArguments>(bufferAssembler.release());
}

std::unique_ptr<IPCBuffer> IPCCommunicator::generateResultBuffer(IPCResult* result)
{
    std::unique_ptr<IPCSerializer> serializer = createIPCSerializer();
    serializer->setMsg(MSG_END);
    switch (result->getType()) {
    case IPCType::INT32:
        serializer->add(result->get<int32_t>());
        break;
    case IPCType::INT64:
        serializer->add(result->get<int64_t>());
        break;
    case IPCType::FLOAT:
        serializer->add(result->get<float>());
        break;
    case IPCType::DOUBLE:
        serializer->add(result->get<double>());
        break;
    case IPCType::BYTEARRAY:
        serializer->add(result->getByteArrayContent(), result->getByteArrayLength());
        break;
	case IPCType::CHARARRAY:
        serializer->add(static_cast<const char*>(result->getData()), result->getStringLength());
        break;
    case IPCType::JSONSTRING:
        serializer->addJSON(result->getStringContent(), result->getStringLength());
        break;
    case IPCType::STRING:
        serializer->add(result->getStringContent(), result->getStringLength());
        break;
    case IPCType::JSUNDEFINED:
        serializer->addJSUndefined();
        break;
    case IPCType::VOID:
        serializer->addVoid();
        break;
    default:
        IPC_UNREACHABLE();
    }
    return serializer->finish();
}

void IPCCommunicator::doSendBufferOnly(IPCBuffer* buffer)
{
    const char* data = static_cast<const char*>(buffer->get());
    uint32_t length = buffer->length();
    doSendBufferOnly(data, length);
}

uint32_t IPCCommunicator::doReadPackage()
{
    uint32_t msg;
    uint32_t length;
    m_futexPageQueue->lockReadPage();
    void* sharedMemory = m_futexPageQueue->getCurrentReadPage();
    length = static_cast<uint32_t*>(sharedMemory)[0];
    uint32_t availableSize = m_futexPageQueue->getPageSize() - sizeof(uint32_t);
    if (length < 2 * sizeof(uint32_t)) {
        releaseBlob();
        throw IPCException("Not a vaild msg");
    }
    if (length > availableSize) {
        m_package.reset(new char[length]);
        doRecvBufferOnly(m_package.get(), length);
        return *reinterpret_cast<uint32_t*>(m_package.get());
    }
    return static_cast<uint32_t*>(sharedMemory)[1];
}

void IPCCommunicator::doSendBufferOnly(const void* _data, size_t length)
{
    const char* data = static_cast<const char*>(_data);
    size_t pageSize = m_futexPageQueue->getPageSize();
    ssize_t byteTransfered;
    uint32_t* dstLength = static_cast<uint32_t*>(m_futexPageQueue->getCurrentWritePage());
    // special handle the first part, which need a size
    // as header.
    dstLength[0] = length;

    IPC_LOGD("send bytes: length: %zu", length);
    byteTransfered = std::min(length, pageSize - sizeof(uint32_t));
    memcpy(dstLength + 1, data, byteTransfered);
    m_futexPageQueue->stepWrite();
    // multiple page package
    if (length > byteTransfered) {
        data += byteTransfered;
        length -= byteTransfered;
        IPC_LOGD("sent bytes: remaining length: %zu, transfered: %zu", length, byteTransfered);

        while (length > 0) {
            byteTransfered = doSendBufferPage(data, length, pageSize);
            data += byteTransfered;
            length -= byteTransfered;
            IPC_LOGD("sent bytes: remaining length: %zu, transfered: %zu", length, byteTransfered);
        }
    }
}

void IPCCommunicator::doRecvBufferOnly(void* _data, size_t length)
{
    char* data = static_cast<char*>(_data);
    size_t pageSize = m_futexPageQueue->getPageSize();
    bool firstRun = true;
    IPC_LOGD("recv bytes: length: %zu", length);
    while (true) {
        ssize_t byteTransfered;
        byteTransfered = std::min(length, pageSize);
        const void* src = m_futexPageQueue->getCurrentReadPage();
        // skip the package size.
        // package starts with msg.
        if (firstRun) {
            src = static_cast<const char*>(src) + sizeof(uint32_t);
            byteTransfered -= sizeof(uint32_t);
        }
        memcpy(data, src, byteTransfered);
        data += byteTransfered;
        length -= byteTransfered;
        IPC_LOGD("recv bytes: remaining length: %zu, transfered: %zu", length, byteTransfered);
        if (length > 0) {
            firstRun = false;
            m_futexPageQueue->unlockReadPageAndStep();
            m_futexPageQueue->lockReadPage();
        } else {
            break;
        }
    }
}

const char* IPCCommunicator::getBlob()
{
    if (m_package.get())
        return m_package.get() + sizeof(uint32_t);
    return static_cast<const char*>(m_futexPageQueue->getCurrentReadPage()) + sizeof(uint32_t) * 2;
}

void IPCCommunicator::releaseBlob()
{
    m_package.reset();
    m_futexPageQueue->unlockReadPageAndStep();
}

size_t IPCCommunicator::doSendBufferPage(const void* data, size_t length, size_t pageSize)
{
    ssize_t byteTransfered;
    byteTransfered = std::min(length, pageSize);
    void* dst = m_futexPageQueue->getCurrentWritePage();
    memcpy(dst, data, byteTransfered);
    m_futexPageQueue->stepWrite();
    return byteTransfered;
}
