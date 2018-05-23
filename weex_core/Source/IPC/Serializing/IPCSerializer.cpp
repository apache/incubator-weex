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

#include "IPCSerializer.h"
#include "../Buffering/IPCBuffer.h"
#include "../IPCByteArray.h"
#include "../IPCCheck.h"
#include "../IPCString.h"
#include "../IPCType.h"
#include <sstream>
#include <stdlib.h>
#include <string>
#include <vector>

namespace {
class IPCSerializerImpl : public IPCSerializer {
public:
    void setMsg(uint32_t msg) override;
    void add(int32_t) override;
    void add(int64_t) override;
    void add(float) override;
    void add(double) override;
    void add(const uint16_t* data, size_t len) override;
    void addJSON(const uint16_t* data, size_t len) override;
    void add(const char* data, size_t len) override;
    void add(const IPCByteArray* bytes) override;
    void addJSUndefined() override;
    void addVoid() override;
    std::unique_ptr<IPCBuffer> finish() override;

private:
    uint32_t m_msg{ MSG_END };
    std::vector<uint32_t> m_types;
    std::vector<std::unique_ptr<char[]>> m_datas;
};

class StringBasedIPCBufferImpl : public IPCBuffer {
public:
    explicit StringBasedIPCBufferImpl(std::string&& s);
    const void* get() override;
    size_t length() override;

private:
    std::string m_str;
};

void IPCSerializerImpl::setMsg(uint32_t msg)
{
    m_msg = msg;
}

void IPCSerializerImpl::add(int32_t n)
{
    std::unique_ptr<char[]> buffer(new char[sizeof(n)]);
    *reinterpret_cast<int32_t*>(buffer.get()) = n;
    m_types.emplace_back(static_cast<uint32_t>(IPCType::INT32));
    m_datas.emplace_back(std::move(buffer));
}

void IPCSerializerImpl::add(int64_t n)
{
    std::unique_ptr<char[]> buffer(new char[sizeof(n)]);
    *reinterpret_cast<int64_t*>(buffer.get()) = n;
    m_types.emplace_back(static_cast<uint32_t>(IPCType::INT64));
    m_datas.emplace_back(std::move(buffer));
}

void IPCSerializerImpl::add(float n)
{
    std::unique_ptr<char[]> buffer(new char[sizeof(n)]);
    *reinterpret_cast<float*>(buffer.get()) = n;
    m_types.emplace_back(static_cast<uint32_t>(IPCType::FLOAT));
    m_datas.emplace_back(std::move(buffer));
}

void IPCSerializerImpl::add(double n)
{
    std::unique_ptr<char[]> buffer(new char[sizeof(n)]);
    *reinterpret_cast<double*>(buffer.get()) = n;
    m_types.emplace_back(static_cast<uint32_t>(IPCType::DOUBLE));
    m_datas.emplace_back(std::move(buffer));
}

void IPCSerializerImpl::add(const uint16_t* data, size_t len)
{
    size_t byteLength = len * sizeof(uint16_t);
    std::unique_ptr<char[]> buffer(new char[byteLength + sizeof(IPCString)]);
    IPCString* s = reinterpret_cast<IPCString*>(buffer.get());
    s->length = len;
    memcpy(s->content, data, byteLength);
    m_types.emplace_back(static_cast<uint32_t>(IPCType::STRING));
    m_datas.emplace_back(std::move(buffer));
}

void IPCSerializerImpl::addJSON(const uint16_t* data, size_t len)
{
    size_t byteLength = len * sizeof(uint16_t);
    std::unique_ptr<char[]> buffer(new char[byteLength + sizeof(IPCString)]);
    IPCString* s = reinterpret_cast<IPCString*>(buffer.get());
    s->length = len;
    memcpy(s->content, data, byteLength);
    m_types.emplace_back(static_cast<uint32_t>(IPCType::JSONSTRING));
    m_datas.emplace_back(std::move(buffer));
}

void IPCSerializerImpl::add(const char* data, size_t len)
{
    size_t byteLength = len;
    std::unique_ptr<char[]> buffer(new char[byteLength + sizeof(IPCByteArray)]);
    IPCByteArray* s = reinterpret_cast<IPCByteArray*>(buffer.get());
    s->length = len;
    memcpy(s->content, data, byteLength);
    s->content[byteLength] = '\0';
    m_types.emplace_back(static_cast<uint32_t>(IPCType::BYTEARRAY));
    m_datas.emplace_back(std::move(buffer));
}

void IPCSerializerImpl::add(const IPCByteArray* bytes) {
    m_types.emplace_back(static_cast<uint32_t>(IPCType::BYTEARRAY));
    // m_datas.emplace_back(std::move((std::unique_ptr<char[]>)bytes));
}

void IPCSerializerImpl::addJSUndefined()
{
    m_types.emplace_back(static_cast<uint32_t>(IPCType::JSUNDEFINED));
    m_datas.emplace_back();
}

void IPCSerializerImpl::addVoid()
{
    m_types.emplace_back(static_cast<uint32_t>(IPCType::VOID));
    m_datas.emplace_back();
}

std::unique_ptr<IPCBuffer> IPCSerializerImpl::finish()
{
    IPC_DCHECK(m_types.size() > 0);
    IPC_DCHECK(m_msg != MSG_NOT_SET);
    std::ostringstream oss;
    oss.write(reinterpret_cast<const char*>(&m_msg), sizeof(m_msg));
    for (auto& type : m_types) {
        oss.write(reinterpret_cast<const char*>(&type), sizeof(type));
    }
    uint32_t endOfType = static_cast<uint32_t>(IPCType::END);
    oss.write(reinterpret_cast<const char*>(&endOfType), sizeof(endOfType));

    int i = 0;
    for (auto& data : m_datas) {
        switch (static_cast<IPCType>(m_types[i])) {
        case IPCType::INT32: {
            oss.write(data.get(), sizeof(int32_t));
        } break;
        case IPCType::INT64: {
            oss.write(data.get(), sizeof(int64_t));
        } break;
        case IPCType::FLOAT: {
            oss.write(data.get(), sizeof(float));
        } break;
        case IPCType::DOUBLE: {
            oss.write(data.get(), sizeof(double));
        } break;
        case IPCType::BYTEARRAY: {
            IPCByteArray* a = reinterpret_cast<IPCByteArray*>(data.get());
            size_t byteLength = a->length;
            oss.write(reinterpret_cast<const char*>(&a->length), sizeof(a->length));
            oss.write(reinterpret_cast<const char*>(a->content), byteLength);
        } break;
        case IPCType::JSONSTRING:
        case IPCType::STRING: {
            IPCString* s = reinterpret_cast<IPCString*>(data.get());
            size_t byteLength = s->length * sizeof(uint16_t);
            oss.write(reinterpret_cast<const char*>(&s->length), sizeof(s->length));
            oss.write(reinterpret_cast<const char*>(s->content), byteLength);
        } break;
        case IPCType::JSUNDEFINED:
        case IPCType::VOID:
            break;
        default:
            IPC_UNREACHABLE();
        }
        ++i;
    }
    m_types.clear();
    m_datas.clear();
    m_msg = MSG_NOT_SET;
    std::unique_ptr<StringBasedIPCBufferImpl> bufferReturn(new StringBasedIPCBufferImpl(std::move(oss.str())));
    return std::unique_ptr<IPCBuffer>(bufferReturn.release());
}

StringBasedIPCBufferImpl::StringBasedIPCBufferImpl(std::string&& s)
    : m_str(std::move(s))
{
}

const void* StringBasedIPCBufferImpl::get()
{
    return m_str.data();
}

size_t StringBasedIPCBufferImpl::length()
{
    return m_str.length();
}
}

std::unique_ptr<IPCSerializer> createIPCSerializer()
{
    return std::unique_ptr<IPCSerializer>(new IPCSerializerImpl);
}
