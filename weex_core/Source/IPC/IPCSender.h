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

#ifndef IPCSENDER_H
#define IPCSENDER_H
#include <memory>
class IPCBuffer;
class IPCResult;
class IPCHandler;
class IPCFutexPageQueue;

class IPCSender {
public:
    virtual ~IPCSender() = default;
    virtual std::unique_ptr<IPCResult> send(IPCBuffer* buffer) = 0;
};
std::unique_ptr<IPCSender> createIPCSender(IPCFutexPageQueue*, IPCHandler* handler);
#endif /* IPCSENDER_H */
