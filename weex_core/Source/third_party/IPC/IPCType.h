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

#ifndef IPCTYPE_H
#define IPCTYPE_H
#include <stdint.h>
struct IPCPackage {
    uint32_t packageSize;
    uint32_t msg;
    uint32_t type[1];
};

enum class IPCType {
    INT32,
    INT64,
    FLOAT,
    DOUBLE,
    JSONSTRING,
    STRING,
    BYTEARRAY, /* terminated with zero. */
    CHARARRAY,
    VOID,
    JSUNDEFINED,
    END,
};

static const uint32_t MSG_MASK = (1U << 31) - 1;
static const uint32_t MSG_END = static_cast<uint32_t>(-1) & MSG_MASK;
static const uint32_t MSG_TERMINATE = static_cast<uint32_t>(-2) & MSG_MASK;
static const uint32_t MSG_NOT_SET = static_cast<uint32_t>(-3) & MSG_MASK;
static const uint32_t MSG_FLAG_ASYNC = 1U << 31;
#endif /* IPCTYPE_H */
