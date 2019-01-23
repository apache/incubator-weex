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

#ifndef IPCSERIALIZER_H
#define IPCSERIALIZER_H
#include <memory>
#include <stdint.h>
#include "IPCByteArray.h"

class IPCBuffer;
class IPCSerializer {
public:
    virtual ~IPCSerializer() = default;
    virtual void setMsg(uint32_t msg) = 0;
    virtual void add(int32_t) = 0;
    virtual void add(int64_t) = 0;
    virtual void add(float) = 0;
    virtual void add(double) = 0;
    virtual void add(const uint16_t* data, size_t len) = 0;
    virtual void addJSON(const uint16_t* data, size_t len) = 0;
    virtual void add(const char* data, size_t len) = 0;
    virtual void add(const IPCByteArray* bytes) = 0;
    virtual void addJSUndefined() = 0;
    virtual void addVoid() = 0;
    virtual std::unique_ptr<IPCBuffer> finish() = 0;
};

std::unique_ptr<IPCSerializer> createIPCSerializer();
#endif /* IPCSERIALIZER_H */
