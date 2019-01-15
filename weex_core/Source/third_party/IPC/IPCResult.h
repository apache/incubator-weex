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

#ifndef IPCRESULT_H
#define IPCRESULT_H
#include "IPCType.h"
#include <memory>
#include "IPCByteArray.h"

struct IPCString;
class IPCResult {
public:
    virtual ~IPCResult() = default;
    template <typename T>
    T get()
    {
        return *static_cast<const T*>(getData());
    }

    virtual const void* getData() = 0;
    virtual IPCType getType() = 0;
    virtual const uint16_t* getStringContent() = 0;
    virtual size_t getStringLength() = 0;
    virtual const char* getByteArrayContent() = 0;
    virtual size_t getByteArrayLength() = 0;
};

std::unique_ptr<IPCResult> createVoidResult();
std::unique_ptr<IPCResult> createInt32Result(int32_t val);
std::unique_ptr<IPCResult> createInt64Result(int64_t val);
std::unique_ptr<IPCResult> createDoubleResult(double val);
std::unique_ptr<IPCResult> createCharArrayResult(char* bytes);
std::unique_ptr<IPCResult> createByteArrayResult(const char* data, size_t length);

#endif /* IPCRESULT_H */
