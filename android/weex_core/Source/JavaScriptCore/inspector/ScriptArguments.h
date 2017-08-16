/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
 * Copyright (c) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <heap/Strong.h>
#include <wtf/Forward.h>
#include <wtf/RefCounted.h>
#include <wtf/Vector.h>
#include <wtf/text/WTFString.h>

namespace Deprecated {
class ScriptValue;
}

namespace JSC {
class ExecState;
class JSGlobalObject;
}

namespace Inspector {

class JS_EXPORT_PRIVATE ScriptArguments : public RefCounted<ScriptArguments> {
public:
    static Ref<ScriptArguments> create(JSC::ExecState*, Vector<Deprecated::ScriptValue>& arguments);
    static Ref<ScriptArguments> createEmpty(JSC::ExecState*);
    ~ScriptArguments();

    const Deprecated::ScriptValue& argumentAt(size_t) const;
    size_t argumentCount() const { return m_arguments.size(); }

    JSC::ExecState* globalState() const;

    bool getFirstArgumentAsString(String& result);
    bool isEqual(ScriptArguments*) const;

private:
    ScriptArguments(JSC::ExecState*);
    ScriptArguments(JSC::ExecState*, Vector<Deprecated::ScriptValue>& arguments);

    JSC::Strong<JSC::JSGlobalObject> m_globalObject;
    Vector<Deprecated::ScriptValue> m_arguments;
};

} // namespace Inspector
