/*
 * Copyright (C) 2008, 2013 Apple Inc. All rights reserved.
 * Copyright (C) 2010 Google Inc. All rights reserved.
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

#pragma once

#include "debugger/Debugger.h"
#include "parser/SourceProvider.h"
#include <wtf/text/WTFString.h>

namespace Inspector {

struct ScriptBreakpointAction;

class ScriptDebugListener {
public:
    struct Script {
        String url;
        String source;
        String sourceURL;
        String sourceMappingURL;
        RefPtr<JSC::SourceProvider> sourceProvider;
        int startLine {0};
        int startColumn {0};
        int endLine {0};
        int endColumn {0};
        bool isContentScript {false};
    };

    virtual ~ScriptDebugListener() { }

    virtual void didParseSource(JSC::SourceID, const Script&) = 0;
    virtual void failedToParseSource(const String& url, const String& data, int firstLine, int errorLine, const String& errorMessage) = 0;
    virtual void didPause(JSC::ExecState&, JSC::JSValue callFrames, JSC::JSValue exception) = 0;
    virtual void didContinue() = 0;

    virtual void breakpointActionLog(JSC::ExecState&, const String&) = 0;
    virtual void breakpointActionSound(int breakpointActionIdentifier) = 0;
    virtual void breakpointActionProbe(JSC::ExecState&, const ScriptBreakpointAction&, unsigned batchId, unsigned sampleId, JSC::JSValue result) = 0;
};

} // namespace Inspector
