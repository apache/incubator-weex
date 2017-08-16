/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 * Copyright (C) 2012 Google Inc. All rights reserved.
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

#include "InjectedScriptBase.h"
#include <wtf/Forward.h>
#include <wtf/Noncopyable.h>
#include <wtf/RefPtr.h>

namespace Deprecated {
class ScriptObject;
}

namespace Inspector {

class InjectedScriptModule;
class InspectorEnvironment;

class JS_EXPORT_PRIVATE InjectedScript : public InjectedScriptBase {
public:
    InjectedScript();
    InjectedScript(Deprecated::ScriptObject, InspectorEnvironment*);
    virtual ~InjectedScript();

    void evaluate(ErrorString&, const String& expression, const String& objectGroup, bool includeCommandLineAPI, bool returnByValue, bool generatePreview, bool saveResult, RefPtr<Protocol::Runtime::RemoteObject>* result, Protocol::OptOutput<bool>* wasThrown, Inspector::Protocol::OptOutput<int>* savedResultIndex);
    void callFunctionOn(ErrorString&, const String& objectId, const String& expression, const String& arguments, bool returnByValue, bool generatePreview, RefPtr<Protocol::Runtime::RemoteObject>* result, Protocol::OptOutput<bool>* wasThrown);
    void evaluateOnCallFrame(ErrorString&, JSC::JSValue callFrames, const String& callFrameId, const String& expression, const String& objectGroup, bool includeCommandLineAPI, bool returnByValue, bool generatePreview, bool saveResult, RefPtr<Protocol::Runtime::RemoteObject>* result, Protocol::OptOutput<bool>* wasThrown, Inspector::Protocol::OptOutput<int>* savedResultIndex);
    void getFunctionDetails(ErrorString&, const String& functionId, RefPtr<Protocol::Debugger::FunctionDetails>* result);
    void functionDetails(ErrorString&, JSC::JSValue, RefPtr<Protocol::Debugger::FunctionDetails>* result);
    void getProperties(ErrorString&, const String& objectId, bool ownProperties, bool generatePreview, RefPtr<Protocol::Array<Protocol::Runtime::PropertyDescriptor>>* result);
    void getDisplayableProperties(ErrorString&, const String& objectId, bool generatePreview, RefPtr<Protocol::Array<Protocol::Runtime::PropertyDescriptor>>* result);
    void getInternalProperties(ErrorString&, const String& objectId, bool generatePreview, RefPtr<Protocol::Array<Protocol::Runtime::InternalPropertyDescriptor>>* result);
    void getCollectionEntries(ErrorString&, const String& objectId, const String& objectGroup, int startIndex, int numberToFetch, RefPtr<Protocol::Array<Protocol::Runtime::CollectionEntry>>* entries);
    void saveResult(ErrorString&, const String& callArgumentJSON, Inspector::Protocol::OptOutput<int>* savedResultIndex);

    Ref<Protocol::Array<Protocol::Debugger::CallFrame>> wrapCallFrames(JSC::JSValue) const;
    RefPtr<Protocol::Runtime::RemoteObject> wrapObject(JSC::JSValue, const String& groupName, bool generatePreview = false) const;
    RefPtr<Protocol::Runtime::RemoteObject> wrapTable(JSC::JSValue table, JSC::JSValue columns) const;
    RefPtr<Protocol::Runtime::ObjectPreview> previewValue(JSC::JSValue) const;

    void setExceptionValue(JSC::JSValue);
    void clearExceptionValue();

    JSC::JSValue findObjectById(const String& objectId) const;
    void inspectObject(JSC::JSValue);
    void releaseObject(const String& objectId);
    void releaseObjectGroup(const String& objectGroup);

private:
    friend class InjectedScriptModule;
};

} // namespace Inspector
