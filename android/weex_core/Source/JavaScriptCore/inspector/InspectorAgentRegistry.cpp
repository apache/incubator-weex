/*
 * Copyright (C) 2013, 2015 Apple Inc. All rights reserved.
 * Copyright (C) 2011 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "InspectorAgentRegistry.h"

#include "InspectorAgentBase.h"

namespace Inspector {

AgentRegistry::AgentRegistry()
{
}

AgentRegistry::~AgentRegistry()
{
    // Allow agents to remove cross-references to other agents that would otherwise
    // make it difficult to establish a correct destruction order for all agents.
    for (auto& agent : m_agents)
        agent->discardAgent();
}

void AgentRegistry::append(std::unique_ptr<InspectorAgentBase> agent)
{
    m_agents.append(WTFMove(agent));
}

#if ENABLE(INSPECTOR_ALTERNATE_DISPATCHERS)
void AgentRegistry::appendExtraAgent(std::unique_ptr<InspectorAgentBase> agent)
{
    m_extraDomains.append(agent->domainName());

    append(WTFMove(agent));
}
#endif

void AgentRegistry::didCreateFrontendAndBackend(FrontendRouter* frontendRouter, BackendDispatcher* backendDispatcher)
{
    for (auto& agent : m_agents)
        agent->didCreateFrontendAndBackend(frontendRouter, backendDispatcher);
}

void AgentRegistry::willDestroyFrontendAndBackend(DisconnectReason reason)
{
    for (auto& agent : m_agents)
        agent->willDestroyFrontendAndBackend(reason);
}

void AgentRegistry::discardValues()
{
    for (auto& agent : m_agents)
        agent->discardValues();
}

} // namespace Inspector
