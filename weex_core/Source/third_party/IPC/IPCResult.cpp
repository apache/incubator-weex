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

#include "IPCResult.h"
#include <cstdlib>

namespace {
class VoidResult : public IPCResult {
public:
    const void* getData() override;
    IPCType getType() override;
    const uint16_t* getStringContent() override;
    size_t getStringLength() override;
    const char* getByteArrayContent() override;
    size_t getByteArrayLength() override;
};

template <typename T>
struct TypeTrait {
};

template <>
struct TypeTrait<int32_t> {
    static const IPCType s_type = IPCType::INT32;
};

template <>
struct TypeTrait<int64_t> {
    static const IPCType s_type = IPCType::INT64;
};

template <>
struct TypeTrait<double> {
    static const IPCType s_type = IPCType::DOUBLE;
};

template <typename T>
class ValueResult : public IPCResult {
public:
    explicit ValueResult(T val);
    const void* getData() override;
    IPCType getType() override;
    const uint16_t* getStringContent() override;
    size_t getStringLength() override;
    const char* getByteArrayContent() override;
    size_t getByteArrayLength() override;

private:
    typedef T value_type;
    typedef TypeTrait<value_type> trait_type;

    value_type m_value;
    static const IPCType s_type = trait_type::s_type;
};

class ByteArrayResult: public IPCResult{
public:
    ByteArrayResult(const char* data, size_t length);
    ~ByteArrayResult();

    const void* getData() override;
    IPCType getType() override;
    const uint16_t* getStringContent() override;
    size_t getStringLength() override;
    const char* getByteArrayContent() override;
    size_t getByteArrayLength() override;
private:
    char* m_data;
    size_t m_length;
};

const void*
VoidResult::getData()
{
    return nullptr;
}

IPCType VoidResult::getType()
{
    return IPCType::VOID;
}

const uint16_t* VoidResult::getStringContent()
{
    return nullptr;
}

size_t VoidResult::getStringLength()
{
    return 0U;
}

const char* VoidResult::getByteArrayContent()
{
    return nullptr;
}

size_t VoidResult::getByteArrayLength()
{
    return 0U;
}

template <typename T>
ValueResult<T>::ValueResult(T val)
    : m_value(val)
{
}

template <typename T>
const void* ValueResult<T>::getData()
{
    return &m_value;
}

template <typename T>
IPCType ValueResult<T>::getType()
{
    return s_type;
}

template <typename T>
const uint16_t* ValueResult<T>::getStringContent()
{
    return nullptr;
}

template <typename T>
size_t ValueResult<T>::getStringLength()
{
    return 0U;
}

template <typename T>
const char* ValueResult<T>::getByteArrayContent()
{
    return nullptr;
}

template <typename T>
size_t ValueResult<T>::getByteArrayLength()
{
    return 0U;
}

ByteArrayResult::ByteArrayResult(const char* data, size_t length):m_length(length)
{
    if(length > 0){
        m_data = (char*)malloc(length*sizeof(char));
        memcpy(m_data,  data, length);
    }else{
        m_data = nullptr;
    }
}
IPCType  ByteArrayResult::getType()
{
    return IPCType::BYTEARRAY;
}

const void* ByteArrayResult::getData()
{
    return nullptr;
}

const uint16_t* ByteArrayResult::getStringContent(){
    return nullptr;
}

size_t ByteArrayResult::getStringLength(){
    return 0l;
}

const char* ByteArrayResult::getByteArrayContent(){
    return  m_data;
}

size_t ByteArrayResult::getByteArrayLength(){
    return m_length;
}
ByteArrayResult::~ByteArrayResult()
{
    if (m_data){
        free(m_data);
        m_data= NULL;
    }
    m_length = 0;
}

}




std::unique_ptr<IPCResult> createVoidResult()
{
    return std::unique_ptr<IPCResult>(new VoidResult);
}

std::unique_ptr<IPCResult> createInt32Result(int32_t val)
{
    return std::unique_ptr<IPCResult>(new ValueResult<int32_t>(val));
}

std::unique_ptr<IPCResult> createInt64Result(int64_t val)
{
    return std::unique_ptr<IPCResult>(new ValueResult<int64_t>(val));
}

std::unique_ptr<IPCResult> createDoubleResult(double val)
{
    return std::unique_ptr<IPCResult>(new ValueResult<double>(val));
}

std::unique_ptr<IPCResult> createByteArrayResult(const char* data, size_t length){
    return std::unique_ptr<IPCResult>(new ByteArrayResult(data, length));
}

class CharArrayResult : public IPCResult {
public:
    CharArrayResult(char* chars);
    // ByteArrayResult(const char* data);
    ~CharArrayResult();

    const void* getData() override;
    IPCType getType() override;
    const uint16_t* getStringContent() override;
    size_t getStringLength() override;
    const char* getByteArrayContent() override;
    size_t getByteArrayLength() override;

private:
    char*  m_char;
    size_t m_length{ 0U };
};

CharArrayResult::CharArrayResult(char* chars)
    : m_char(chars) {
    m_length = strlen(m_char);
}

CharArrayResult::~CharArrayResult() {
    delete[] m_char;
}

const void* CharArrayResult::getData()
{
    return m_char;
}

IPCType CharArrayResult::getType()
{
    return IPCType::CHARARRAY;
}

const uint16_t* CharArrayResult::getStringContent()
{
    return nullptr;
}

size_t CharArrayResult::getStringLength()
{
    return m_length;
}

const char* CharArrayResult::getByteArrayContent() {
    return nullptr;
}

size_t CharArrayResult::getByteArrayLength() {

    return 0U;
}

std::unique_ptr<IPCResult> createCharArrayResult(char* bytes) {
    return std::unique_ptr<IPCResult>(new CharArrayResult(bytes));
}
