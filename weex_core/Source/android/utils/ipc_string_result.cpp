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

#include <malloc.h>
#include "ipc_string_result.h"


IPCStringResult::~IPCStringResult()
{
    if(string_)
        free(string_);
}

const void* IPCStringResult::getData()
{
    return nullptr;
}

IPCType IPCStringResult::getType()
{
    return IPCType::STRING;
}

const uint16_t* IPCStringResult::getStringContent()
{
    return m_cvalue;
}

size_t IPCStringResult::getStringLength()
{
    return m_length;
}

const char* IPCStringResult::getByteArrayContent()
{
    return nullptr;
}

size_t IPCStringResult::getByteArrayLength()
{
    return 0U;
}

IPCStringResult::IPCStringResult(WeexString *weexString) {
    string_ = weexString;
    m_cvalue = weexString->content;
    m_length = weexString->length;
}

IPCJSONStringResult::IPCJSONStringResult(WeexString *weexString)
        : IPCStringResult(weexString) {}

IPCJSONStringResult::~IPCJSONStringResult() {}

IPCType IPCJSONStringResult::getType()
{
    return IPCType::JSONSTRING;
}
