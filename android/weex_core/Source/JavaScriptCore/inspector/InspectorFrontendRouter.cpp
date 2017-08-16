/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "InspectorFrontendRouter.h"

#include "InspectorFrontendChannel.h"
#include <wtf/Assertions.h>

namespace Inspector {

Ref<FrontendRouter> FrontendRouter::create()
{
    return adoptRef(*new FrontendRouter);
}

void FrontendRouter::connectFrontend(FrontendChannel* connection)
{
    ASSERT_ARG(connection, connection);

    if (m_connections.contains(connection)) {
        ASSERT_NOT_REACHED();
        return;
    }

    m_connections.append(connection);
}

void FrontendRouter::disconnectFrontend(FrontendChannel* connection)
{
    ASSERT_ARG(connection, connection);

    if (!m_connections.contains(connection)) {
        ASSERT_NOT_REACHED();
        return;
    }

    m_connections.removeFirst(connection);
}

void FrontendRouter::disconnectAllFrontends()
{
    m_connections.clear();
}

bool FrontendRouter::hasLocalFrontend() const
{
    for (auto* connection : m_connections) {
        if (connection->connectionType() == FrontendChannel::ConnectionType::Local)
            return true;
    }

    return false;
}

bool FrontendRouter::hasRemoteFrontend() const
{
    for (auto* connection : m_connections) {
        if (connection->connectionType() == FrontendChannel::ConnectionType::Remote)
            return true;
    }

    return false;
}

void FrontendRouter::sendEvent(const String& message) const
{
    for (auto* connection : m_connections)
        connection->sendMessageToFrontend(message);
}

void FrontendRouter::sendResponse(const String& message) const
{
    // FIXME: send responses to the appropriate frontend.
    for (auto* connection : m_connections)
        connection->sendMessageToFrontend(message);
}

} // namespace Inspector
