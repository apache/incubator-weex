/*
 * Copyright (C) 2013-2016 Apple Inc. All Rights Reserved.
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
#include "RemoteInspector.h"

#if ENABLE(REMOTE_INSPECTOR)

#include "RemoteAutomationTarget.h"
#include "RemoteConnectionToTarget.h"
#include "RemoteInspectionTarget.h"
#include "RemoteInspectorConstants.h"
#include <wtf/MainThread.h>
#include <wtf/text/WTFString.h>

namespace Inspector {

bool RemoteInspector::startEnabled = true;

void RemoteInspector::startDisabled()
{
    RemoteInspector::startEnabled = false;
}

unsigned RemoteInspector::nextAvailableTargetIdentifier()
{
    unsigned nextValidTargetIdentifier;
    do {
        nextValidTargetIdentifier = m_nextAvailableTargetIdentifier++;
    } while (!nextValidTargetIdentifier || nextValidTargetIdentifier == std::numeric_limits<unsigned>::max() || m_targetMap.contains(nextValidTargetIdentifier));
    return nextValidTargetIdentifier;
}

void RemoteInspector::registerTarget(RemoteControllableTarget* target)
{
    ASSERT_ARG(target, target);

    std::lock_guard<Lock> lock(m_mutex);

    unsigned targetIdentifier = nextAvailableTargetIdentifier();
    target->setTargetIdentifier(targetIdentifier);

    {
        auto result = m_targetMap.set(targetIdentifier, target);
        ASSERT_UNUSED(result, result.isNewEntry);
    }

    // If remote control is not allowed, a null listing is returned.
    if (auto targetListing = listingForTarget(*target)) {
        auto result = m_targetListingMap.set(targetIdentifier, targetListing);
        ASSERT_UNUSED(result, result.isNewEntry);
    }

    pushListingsSoon();
}

void RemoteInspector::unregisterTarget(RemoteControllableTarget* target)
{
    ASSERT_ARG(target, target);

    std::lock_guard<Lock> lock(m_mutex);

    unsigned targetIdentifier = target->targetIdentifier();
    if (!targetIdentifier)
        return;

    bool wasRemoved = m_targetMap.remove(targetIdentifier);
    ASSERT_UNUSED(wasRemoved, wasRemoved);

    // The listing may never have been added if remote control isn't allowed.
    m_targetListingMap.remove(targetIdentifier);

    if (auto connectionToTarget = m_targetConnectionMap.take(targetIdentifier))
        connectionToTarget->targetClosed();

    pushListingsSoon();
}

void RemoteInspector::updateTarget(RemoteControllableTarget* target)
{
    ASSERT_ARG(target, target);

    std::lock_guard<Lock> lock(m_mutex);

    unsigned targetIdentifier = target->targetIdentifier();
    if (!targetIdentifier)
        return;

    {
        auto result = m_targetMap.set(targetIdentifier, target);
        ASSERT_UNUSED(result, !result.isNewEntry);
    }

    // If the target has just allowed remote control, then the listing won't exist yet.
    // If the target has no identifier remove the old listing.
    if (auto targetListing = listingForTarget(*target))
        m_targetListingMap.set(targetIdentifier, targetListing);
    else
        m_targetListingMap.remove(targetIdentifier);

    pushListingsSoon();
}

void RemoteInspector::updateClientCapabilities()
{
    ASSERT(isMainThread());

    std::lock_guard<Lock> lock(m_mutex);

    if (!m_client)
        m_clientCapabilities = std::nullopt;
    else {
        RemoteInspector::Client::Capabilities updatedCapabilities = {
            m_client->remoteAutomationAllowed() // remoteAutomationAllowed
        };

        m_clientCapabilities = updatedCapabilities;
    }
}

void RemoteInspector::setRemoteInspectorClient(RemoteInspector::Client* client)
{
    ASSERT_ARG(client, client);
    ASSERT(!m_client);

    {
        std::lock_guard<Lock> lock(m_mutex);
        m_client = client;
    }

    // Send an updated listing that includes whether the client allows remote automation.
    updateClientCapabilities();
    pushListingsSoon();
}

void RemoteInspector::setupFailed(unsigned targetIdentifier)
{
    std::lock_guard<Lock> lock(m_mutex);

    m_targetConnectionMap.remove(targetIdentifier);

    if (targetIdentifier == m_automaticInspectionCandidateTargetIdentifier)
        m_automaticInspectionPaused = false;

    updateHasActiveDebugSession();
    updateTargetListing(targetIdentifier);
    pushListingsSoon();
}

void RemoteInspector::setupCompleted(unsigned targetIdentifier)
{
    std::lock_guard<Lock> lock(m_mutex);

    if (targetIdentifier == m_automaticInspectionCandidateTargetIdentifier)
        m_automaticInspectionPaused = false;
}

bool RemoteInspector::waitingForAutomaticInspection(unsigned)
{
    // We don't take the lock to check this because we assume it will be checked repeatedly.
    return m_automaticInspectionPaused;
}

void RemoteInspector::clientCapabilitiesDidChange()
{
    updateClientCapabilities();
    pushListingsSoon();
}

void RemoteInspector::stop()
{
    std::lock_guard<Lock> lock(m_mutex);

    stopInternal(StopSource::API);
}

TargetListing RemoteInspector::listingForTarget(const RemoteControllableTarget& target) const
{
    if (is<RemoteInspectionTarget>(target))
        return listingForInspectionTarget(downcast<RemoteInspectionTarget>(target));
    if (is<RemoteAutomationTarget>(target))
        return listingForAutomationTarget(downcast<RemoteAutomationTarget>(target));

    ASSERT_NOT_REACHED();
    return nullptr;
}

void RemoteInspector::updateTargetListing(unsigned targetIdentifier)
{
    auto target = m_targetMap.get(targetIdentifier);
    if (!target)
        return;

    updateTargetListing(*target);
}

void RemoteInspector::updateTargetListing(const RemoteControllableTarget& target)
{
    auto targetListing = listingForTarget(target);
    if (!targetListing)
        return;

    m_targetListingMap.set(target.targetIdentifier(), targetListing);

    pushListingsSoon();
}

void RemoteInspector::updateHasActiveDebugSession()
{
    bool hasActiveDebuggerSession = !m_targetConnectionMap.isEmpty();
    if (hasActiveDebuggerSession == m_hasActiveDebugSession)
        return;

    m_hasActiveDebugSession = hasActiveDebuggerSession;

    // FIXME: Expose some way to access this state in an embedder.
    // Legacy iOS WebKit 1 had a notification. This will need to be smarter with WebKit2.
}

} // namespace Inspector

#endif // ENABLE(REMOTE_INSPECTOR)
