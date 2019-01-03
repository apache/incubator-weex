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
#ifndef IPCSTRINGRESULT_H
#define IPCSTRINGRESULT_H
#include <jni.h>
#include "IPCResult.h"
#include "include/WeexApiHeader.h"

class IPCStringResult : public IPCResult {
public:
    IPCStringResult(WeexString *weexString);
    virtual ~IPCStringResult();

    const void* getData() override;
    virtual IPCType getType() override;
    const uint16_t* getStringContent() override;
    size_t getStringLength() override;
    const char* getByteArrayContent() override;
    size_t getByteArrayLength() override;

private:
    const jchar* m_cvalue{ nullptr };
    size_t m_length{ 0U };
    WeexString *string_;
};

class IPCJSONStringResult : public IPCStringResult {
public:
    IPCJSONStringResult(WeexString *weexString);
    ~IPCJSONStringResult();

    IPCType getType() override;
};

#endif //IPCSTRINGRESULT_H
