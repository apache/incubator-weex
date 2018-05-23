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

#ifndef IPCHANDLER_H
#define IPCHANDLER_H
#include <functional>
#include <memory>
#include <stdint.h>

class IPCResult;
class IPCArguments;

class IPCHandler {
public:
    virtual ~IPCHandler() = default;
    virtual std::unique_ptr<IPCResult> handle(uint32_t msg, IPCArguments*) = 0;
    virtual void registerHandler(int msg, const std::function<std::unique_ptr<IPCResult>(IPCArguments*)>& handler) = 0;
};

std::unique_ptr<IPCHandler> createIPCHandler();
#endif /* IPCHANDLER_H */
