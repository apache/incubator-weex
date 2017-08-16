/*
 * Copyright (C) 2013, 2015 Apple Inc. All Rights Reserved.
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

#pragma once

#if ENABLE(REMOTE_INSPECTOR)

#include "RemoteControllableTarget.h"
#include <wtf/RetainPtr.h>
#include <wtf/TypeCasts.h>
#include <wtf/text/WTFString.h>

namespace Inspector {

class FrontendChannel;

class JS_EXPORT_PRIVATE RemoteInspectionTarget : public RemoteControllableTarget {
public:
    bool remoteDebuggingAllowed() const { return m_allowed; }
    void setRemoteDebuggingAllowed(bool);

#if USE(CF)
    CFRunLoopRef targetRunLoop() override { return m_runLoop.get(); }
    void setTargetRunLoop(CFRunLoopRef runLoop) { m_runLoop = runLoop; }
#endif

    virtual String name() const { return String(); } // JavaScript and Web
    virtual String url() const { return String(); } // Web
    virtual bool hasLocalDebugger() const = 0;

    virtual void setIndicating(bool) { } // Default is to do nothing.
    virtual void pause() { };

    virtual bool automaticInspectionAllowed() const { return false; }
    virtual void pauseWaitingForAutomaticInspection();
    virtual void unpauseForInitializedInspector();

    // RemoteControllableTarget overrides.
    bool remoteControlAllowed() const override;
private:
    bool m_allowed {false};
#if USE(CF)
    RetainPtr<CFRunLoopRef> m_runLoop;
#endif
};

} // namespace Inspector

SPECIALIZE_TYPE_TRAITS_BEGIN(Inspector::RemoteInspectionTarget) \
    static bool isType(const Inspector::RemoteControllableTarget& target) \
    { \
        return target.type() == Inspector::RemoteControllableTarget::Type::JavaScript \
            || target.type() == Inspector::RemoteControllableTarget::Type::Web; \
    }
SPECIALIZE_TYPE_TRAITS_END()

#endif // ENABLE(REMOTE_INSPECTOR)
