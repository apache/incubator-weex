/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
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

#include "ScriptDebugServer.h"

namespace Inspector {

class JSGlobalObjectScriptDebugServer final : public ScriptDebugServer {
    WTF_MAKE_NONCOPYABLE(JSGlobalObjectScriptDebugServer);
public:
    JSGlobalObjectScriptDebugServer(JSC::JSGlobalObject&);
    virtual ~JSGlobalObjectScriptDebugServer() { }

    JSC::JSGlobalObject& globalObject() const { return m_globalObject; }

private:
    void attachDebugger() override;
    void detachDebugger(bool isBeingDestroyed) override;

    void didPause(JSC::JSGlobalObject*) override { }
    void didContinue(JSC::JSGlobalObject*) override { }
    void runEventLoopWhilePaused() override;
    bool isContentScript(JSC::ExecState*) const override { return false; }

    // NOTE: Currently all exceptions are reported at the API boundary through reportAPIException.
    // Until a time comes where an exception can be caused outside of the API (e.g. setTimeout
    // or some other async operation in a pure JSContext) we can ignore exceptions reported here.
    void reportException(JSC::ExecState*, JSC::Exception*) const override { }

    JSC::JSGlobalObject& m_globalObject;
};

} // namespace Inspector
