/*
 * Copyright (C) 2013, 2015 Apple Inc. All rights reserved.
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

#include "JSGlobalObjectInspectorController.h"
#include "RemoteInspectionTarget.h"
#include <wtf/Noncopyable.h>

namespace Inspector {
class FrontendChannel;
enum class DisconnectReason;
}

namespace JSC {

class JSGlobalObject;

class JSGlobalObjectDebuggable final : public Inspector::RemoteInspectionTarget {
    WTF_MAKE_FAST_ALLOCATED;
    WTF_MAKE_NONCOPYABLE(JSGlobalObjectDebuggable);
public:
    JSGlobalObjectDebuggable(JSGlobalObject&);
    ~JSGlobalObjectDebuggable() { }

    Inspector::RemoteControllableTarget::Type type() const override { return Inspector::RemoteControllableTarget::Type::JavaScript; }

    String name() const override;
    bool hasLocalDebugger() const override { return false; }

    void connect(Inspector::FrontendChannel*, bool automaticInspection) override;
    void disconnect(Inspector::FrontendChannel*) override;
    void dispatchMessageFromRemote(const String& message) override;
    void pause() override;

    bool automaticInspectionAllowed() const override { return true; }
    void pauseWaitingForAutomaticInspection() override;

private:
    JSGlobalObject& m_globalObject;
};

} // namespace JSC

SPECIALIZE_TYPE_TRAITS_CONTROLLABLE_TARGET(JSC::JSGlobalObjectDebuggable, JavaScript);

#endif // ENABLE(REMOTE_INSPECTOR)
