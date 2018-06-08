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

#if !defined(NDEBUG)
#include "IPCCheck.h"
#include <android/log.h>
#define TAG "linzj_IPC"

void reportCheckFailed(const char* msg, const char* file, int line)
{
    __android_log_print(ANDROID_LOG_ERROR, TAG, msg, file, line);
    __builtin_trap();
}

void reportUnreachable(const char* file, int line)
{
    __android_log_print(ANDROID_LOG_ERROR, TAG, "unreachable statement reached %s %d", file, line);
    __builtin_trap();
}
#endif
