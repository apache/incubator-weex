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

#include "IPCHandler.h"
#include "IPCLog.h"
#include "IPCResult.h"
#include <unordered_map>

namespace {
class IPCHandlerImpl : public IPCHandler {
public:
    std::unique_ptr<IPCResult> handle(uint32_t msg, IPCArguments*) override;
    void registerHandler(int msg, const std::function<std::unique_ptr<IPCResult>(IPCArguments*)>& handler) override;

private:
    typedef std::unordered_map<int, std::function<std::unique_ptr<IPCResult>(IPCArguments*)>> MapType;
    MapType m_map;
};

std::unique_ptr<IPCResult> IPCHandlerImpl::handle(uint32_t msg, IPCArguments* arguments)
{
    auto it = m_map.find(msg);
    if (it == m_map.end()) {
        IPC_LOGE("unable to find msg: %d", msg);
        return createVoidResult();
    }
    return it->second(arguments);
}

void IPCHandlerImpl::registerHandler(int msg, const std::function<std::unique_ptr<IPCResult>(IPCArguments*)>& handler)
{
    m_map.insert(MapType::value_type(msg, handler));
}
}

std::unique_ptr<IPCHandler> createIPCHandler()
{
    return std::unique_ptr<IPCHandler>(new IPCHandlerImpl);
}
