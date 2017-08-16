/*
 * Copyright (C) 2007-2010, 2015 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Matt Lilek <webkit@mattlilek.com>
 * Copyright (C) 2011 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "InspectorAgent.h"

#include "InspectorEnvironment.h"
#include "InspectorFrontendRouter.h"
#include "InspectorValues.h"
#include "ScriptValue.h"

namespace Inspector {

InspectorAgent::InspectorAgent(AgentContext& context)
    : InspectorAgentBase(ASCIILiteral("Inspector"))
    , m_environment(context.environment)
    , m_frontendDispatcher(std::make_unique<InspectorFrontendDispatcher>(context.frontendRouter))
    , m_backendDispatcher(InspectorBackendDispatcher::create(context.backendDispatcher, this))
{
}

InspectorAgent::~InspectorAgent()
{
}

void InspectorAgent::didCreateFrontendAndBackend(FrontendRouter*, BackendDispatcher*)
{
}

void InspectorAgent::willDestroyFrontendAndBackend(DisconnectReason)
{
    m_pendingEvaluateTestCommands.clear();

    ErrorString unused;
    disable(unused);
}

void InspectorAgent::enable(ErrorString&)
{
    m_enabled = true;

    if (m_pendingInspectData.first)
        inspect(m_pendingInspectData.first.copyRef(), m_pendingInspectData.second.copyRef());

#if ENABLE(INSPECTOR_ALTERNATE_DISPATCHERS)
    if (m_pendingExtraDomainsData)
        m_frontendDispatcher->activateExtraDomains(m_pendingExtraDomainsData);
#endif

    for (auto& testCommand : m_pendingEvaluateTestCommands)
        m_frontendDispatcher->evaluateForTestInFrontend(testCommand);

    m_pendingEvaluateTestCommands.clear();
}

void InspectorAgent::disable(ErrorString&)
{
    m_enabled = false;
}

void InspectorAgent::initialized(ErrorString&)
{
    m_environment.frontendInitialized();
}

void InspectorAgent::inspect(RefPtr<Protocol::Runtime::RemoteObject>&& objectToInspect, RefPtr<InspectorObject>&& hints)
{
    if (m_enabled) {
        m_frontendDispatcher->inspect(objectToInspect, hints);
        m_pendingInspectData.first = nullptr;
        m_pendingInspectData.second = nullptr;
        return;
    }

    m_pendingInspectData.first = objectToInspect;
    m_pendingInspectData.second = hints;
}

void InspectorAgent::evaluateForTestInFrontend(const String& script)
{
    if (m_enabled)
        m_frontendDispatcher->evaluateForTestInFrontend(script);
    else
        m_pendingEvaluateTestCommands.append(script);
}

#if ENABLE(INSPECTOR_ALTERNATE_DISPATCHERS)
void InspectorAgent::activateExtraDomain(const String& domainName)
{
    if (!m_enabled) {
        if (!m_pendingExtraDomainsData)
            m_pendingExtraDomainsData = Inspector::Protocol::Array<String>::create();
        m_pendingExtraDomainsData->addItem(domainName);
        return;
    }

    auto domainNames = Inspector::Protocol::Array<String>::create();
    domainNames->addItem(domainName);
    m_frontendDispatcher->activateExtraDomains(WTFMove(domainNames));
}

void InspectorAgent::activateExtraDomains(const Vector<String>& extraDomains)
{
    if (extraDomains.isEmpty())
        return;

    auto domainNames = Inspector::Protocol::Array<String>::create();
    for (auto domainName : extraDomains)
        domainNames->addItem(domainName);

    if (!m_enabled)
        m_pendingExtraDomainsData = WTFMove(domainNames);
    else
        m_frontendDispatcher->activateExtraDomains(WTFMove(domainNames));
}
#endif

} // namespace Inspector
