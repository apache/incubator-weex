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

#ifndef IPCARGUMENTS_H
#define IPCARGUMENTS_H
#include "IPCType.h"

struct IPCString;
struct IPCByteArray;
class IPCArguments {
public:
    virtual ~IPCArguments() = default;
    template <typename T>
    T get(int index)
    {
        return *static_cast<const T*>(getData(index));
    }

    const IPCString* getString(int index)
    {
        return static_cast<const IPCString*>(getData(index));
    }

    const IPCByteArray* getByteArray(int index)
    {
        return static_cast<const IPCByteArray*>(getData(index));
    }

    virtual const void* getData(int index) = 0;
    virtual IPCType getType(int index) = 0;
    virtual size_t getCount() = 0;
};
#endif /* IPCARGUMENTS_H */
