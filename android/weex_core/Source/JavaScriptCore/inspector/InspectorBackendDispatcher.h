/*
 * Copyright (C) 2013, 2015 Apple Inc. All Rights Reserved.
 * Copyright (C) 2011 The Chromium Authors. All rights reserved.
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

#include "InspectorFrontendRouter.h"
#include "InspectorProtocolTypes.h"
#include <wtf/DeprecatedOptional.h>
#include <wtf/Optional.h>
#include <wtf/RefCounted.h>
#include <wtf/text/WTFString.h>

namespace Inspector {

class BackendDispatcher;

typedef String ErrorString;

class JS_EXPORT_PRIVATE SupplementalBackendDispatcher : public RefCounted<SupplementalBackendDispatcher> {
public:
    SupplementalBackendDispatcher(BackendDispatcher&);
    virtual ~SupplementalBackendDispatcher();
    virtual void dispatch(long requestId, const String& method, Ref<InspectorObject>&& message) = 0;
protected:
    Ref<BackendDispatcher> m_backendDispatcher;
};

class JS_EXPORT_PRIVATE BackendDispatcher : public RefCounted<BackendDispatcher> {
public:
    static Ref<BackendDispatcher> create(Ref<FrontendRouter>&&);

    class JS_EXPORT_PRIVATE CallbackBase : public RefCounted<CallbackBase> {
    public:
        CallbackBase(Ref<BackendDispatcher>&&, long requestId);

        bool isActive() const;
        void disable() { m_alreadySent = true; }

        void sendSuccess(RefPtr<InspectorObject>&&);
        void sendFailure(const ErrorString&);

    private:
        Ref<BackendDispatcher> m_backendDispatcher;
        long m_requestId;
        bool m_alreadySent { false };
    };

    bool isActive() const;

    bool hasProtocolErrors() const { return m_protocolErrors.size() > 0; }

    enum CommonErrorCode {
        ParseError = 0,
        InvalidRequest,
        MethodNotFound,
        InvalidParams,
        InternalError,
        ServerError
    };

    void registerDispatcherForDomain(const String& domain, SupplementalBackendDispatcher*);
    void dispatch(const String& message);

    void sendResponse(long requestId, RefPtr<InspectorObject>&& result);
    void sendPendingErrors();

    void reportProtocolError(CommonErrorCode, const String& errorMessage);
    void reportProtocolError(std::optional<long> relatedRequestId, CommonErrorCode, const String& errorMessage);

    template<typename T>
    WTF_HIDDEN_DECLARATION
    T getPropertyValue(InspectorObject*, const String& name, bool* out_optionalValueFound, T defaultValue, std::function<bool(InspectorValue&, T&)>, const char* typeName);

    int getInteger(InspectorObject*, const String& name, bool* valueFound);
    double getDouble(InspectorObject*, const String& name, bool* valueFound);
    String getString(InspectorObject*, const String& name, bool* valueFound);
    bool getBoolean(InspectorObject*, const String& name, bool* valueFound);
    RefPtr<InspectorValue> getValue(InspectorObject*, const String& name, bool* valueFound);
    RefPtr<InspectorObject> getObject(InspectorObject*, const String& name, bool* valueFound);
    RefPtr<InspectorArray> getArray(InspectorObject*, const String& name, bool* valueFound);

private:
    BackendDispatcher(Ref<FrontendRouter>&&);

#if PLATFORM(MAC)
    // This is necessary for some versions of Safari. Remove it when those versions of Safari are no longer supported.
    void reportProtocolError(WTF::DeprecatedOptional<long> relatedRequestId, CommonErrorCode, const String& errorMessage);
#endif

    Ref<FrontendRouter> m_frontendRouter;
    HashMap<String, SupplementalBackendDispatcher*> m_dispatchers;

    // Protocol errors reported for the top-level request being processed.
    // If processing a request triggers async responses, then any related errors will
    // be attributed to the top-level request, but generate separate error messages.
    Vector<std::tuple<CommonErrorCode, String>> m_protocolErrors;

    // For synchronously handled requests, avoid plumbing requestId through every
    // call that could potentially fail with a protocol error.
    std::optional<long> m_currentRequestId { std::nullopt };
};

} // namespace Inspector
