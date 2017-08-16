#!/usr/bin/env python
#
# Copyright (c) 2014, 2015 Apple Inc. All rights reserved.
# Copyright (c) 2014 University of Washington. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGE.

# Generator templates, which can be filled with string.Template.
# Following are classes that fill the templates from the typechecked model.

class CppGeneratorTemplates:

    HeaderPrelude = (
    """#pragma once

${includes}

namespace Inspector {

${typedefs}""")

    HeaderPostlude = (
    """} // namespace Inspector""")

    ImplementationPrelude = (
    """#include "config.h"
#include ${primaryInclude}

${secondaryIncludes}

namespace Inspector {""")

    ImplementationPostlude = (
    """} // namespace Inspector
""")

    AlternateDispatchersHeaderPrelude = (
    """#pragma once

#if ENABLE(INSPECTOR_ALTERNATE_DISPATCHERS)

${includes}

namespace Inspector {

class AlternateBackendDispatcher {
public:
    void setBackendDispatcher(RefPtr<BackendDispatcher>&& dispatcher) { m_backendDispatcher = WTFMove(dispatcher); }
    BackendDispatcher* backendDispatcher() const { return m_backendDispatcher.get(); }
private:
    RefPtr<BackendDispatcher> m_backendDispatcher;
};
""")

    AlternateDispatchersHeaderPostlude = (
    """} // namespace Inspector

#endif // ENABLE(INSPECTOR_ALTERNATE_DISPATCHERS)""")

    AlternateBackendDispatcherHeaderDomainHandlerInterfaceDeclaration = (
    """class Alternate${domainName}BackendDispatcher : public AlternateBackendDispatcher {
public:
    virtual ~Alternate${domainName}BackendDispatcher() { }
${commandDeclarations}
};""")

    BackendDispatcherHeaderDomainHandlerDeclaration = (
    """${classAndExportMacro} ${domainName}BackendDispatcherHandler {
public:
${commandDeclarations}
protected:
    virtual ~${domainName}BackendDispatcherHandler();
};""")

    BackendDispatcherHeaderDomainDispatcherDeclaration = (
    """${classAndExportMacro} ${domainName}BackendDispatcher final : public SupplementalBackendDispatcher {
public:
    static Ref<${domainName}BackendDispatcher> create(BackendDispatcher&, ${domainName}BackendDispatcherHandler*);
    void dispatch(long requestId, const String& method, Ref<InspectorObject>&& message) override;
${commandDeclarations}
private:
    ${domainName}BackendDispatcher(BackendDispatcher&, ${domainName}BackendDispatcherHandler*);
    ${domainName}BackendDispatcherHandler* m_agent { nullptr };
};""")

    BackendDispatcherHeaderDomainDispatcherAlternatesDeclaration = (
    """#if ENABLE(INSPECTOR_ALTERNATE_DISPATCHERS)
public:
    void setAlternateDispatcher(Alternate${domainName}BackendDispatcher* alternateDispatcher) { m_alternateDispatcher = alternateDispatcher; }
private:
    Alternate${domainName}BackendDispatcher* m_alternateDispatcher { nullptr };
#endif""")

    BackendDispatcherHeaderAsyncCommandDeclaration = (
    """    ${classAndExportMacro} ${callbackName} : public BackendDispatcher::CallbackBase {
    public:
        ${callbackName}(Ref<BackendDispatcher>&&, int id);
        void sendSuccess(${outParameters});
    };
    virtual void ${commandName}(${inParameters}) = 0;""")

    BackendDispatcherImplementationSmallSwitch = (
    """void ${domainName}BackendDispatcher::dispatch(long requestId, const String& method, Ref<InspectorObject>&& message)
{
    Ref<${domainName}BackendDispatcher> protect(*this);

    RefPtr<InspectorObject> parameters;
    message->getObject(ASCIILiteral("params"), parameters);

${dispatchCases}
    else
        m_backendDispatcher->reportProtocolError(BackendDispatcher::MethodNotFound, makeString('\\'', "${domainName}", '.', method, "' was not found"));
}""")

    BackendDispatcherImplementationLargeSwitch = (
"""void ${domainName}BackendDispatcher::dispatch(long requestId, const String& method, Ref<InspectorObject>&& message)
{
    Ref<${domainName}BackendDispatcher> protect(*this);

    RefPtr<InspectorObject> parameters;
    message->getObject(ASCIILiteral("params"), parameters);

    typedef void (${domainName}BackendDispatcher::*CallHandler)(long requestId, RefPtr<InspectorObject>&& message);
    typedef HashMap<String, CallHandler> DispatchMap;
    static NeverDestroyed<DispatchMap> dispatchMap;
    if (dispatchMap.get().isEmpty()) {
        static const struct MethodTable {
            const char* name;
            CallHandler handler;
        } commands[] = {
${dispatchCases}
        };
        size_t length = WTF_ARRAY_LENGTH(commands);
        for (size_t i = 0; i < length; ++i)
            dispatchMap.get().add(commands[i].name, commands[i].handler);
    }

    auto findResult = dispatchMap.get().find(method);
    if (findResult == dispatchMap.get().end()) {
        m_backendDispatcher->reportProtocolError(BackendDispatcher::MethodNotFound, makeString('\\'', "${domainName}", '.', method, "' was not found"));
        return;
    }

    ((*this).*findResult->value)(requestId, WTFMove(parameters));
}""")

    BackendDispatcherImplementationDomainConstructor = (
    """Ref<${domainName}BackendDispatcher> ${domainName}BackendDispatcher::create(BackendDispatcher& backendDispatcher, ${domainName}BackendDispatcherHandler* agent)
{
    return adoptRef(*new ${domainName}BackendDispatcher(backendDispatcher, agent));
}

${domainName}BackendDispatcher::${domainName}BackendDispatcher(BackendDispatcher& backendDispatcher, ${domainName}BackendDispatcherHandler* agent)
    : SupplementalBackendDispatcher(backendDispatcher)
    , m_agent(agent)
{
    m_backendDispatcher->registerDispatcherForDomain(ASCIILiteral("${domainName}"), this);
}""")

    BackendDispatcherImplementationPrepareCommandArguments = (
"""${inParameterDeclarations}
    if (m_backendDispatcher->hasProtocolErrors()) {
        m_backendDispatcher->reportProtocolError(BackendDispatcher::InvalidParams, String::format("Some arguments of method \'%s\' can't be processed", "${domainName}.${commandName}"));
        return;
    }
""")

    BackendDispatcherImplementationAsyncCommand = (
"""${domainName}BackendDispatcherHandler::${callbackName}::${callbackName}(Ref<BackendDispatcher>&& backendDispatcher, int id) : BackendDispatcher::CallbackBase(WTFMove(backendDispatcher), id) { }

void ${domainName}BackendDispatcherHandler::${callbackName}::sendSuccess(${formalParameters})
{
    Ref<InspectorObject> jsonMessage = InspectorObject::create();
${outParameterAssignments}
    CallbackBase::sendSuccess(WTFMove(jsonMessage));
}""")

    FrontendDispatcherDomainDispatcherDeclaration = (
"""${classAndExportMacro} ${domainName}FrontendDispatcher {
public:
    ${domainName}FrontendDispatcher(FrontendRouter& frontendRouter) : m_frontendRouter(frontendRouter) { }
${eventDeclarations}
private:
    FrontendRouter& m_frontendRouter;
};""")

    ProtocolObjectBuilderDeclarationPrelude = (
"""    template<int STATE>
    class Builder {
    private:
        RefPtr<InspectorObject> m_result;

        template<int STEP> Builder<STATE | STEP>& castState()
        {
            return *reinterpret_cast<Builder<STATE | STEP>*>(this);
        }

        Builder(Ref</*${objectType}*/InspectorObject>&& object)
            : m_result(WTFMove(object))
        {
            COMPILE_ASSERT(STATE == NoFieldsSet, builder_created_in_non_init_state);
        }
        friend class ${objectType};
    public:""")

    ProtocolObjectBuilderDeclarationPostlude = (
"""
        Ref<${objectType}> release()
        {
            COMPILE_ASSERT(STATE == AllFieldsSet, result_is_not_ready);
            COMPILE_ASSERT(sizeof(${objectType}) == sizeof(InspectorObject), cannot_cast);

            Ref<InspectorObject> result = m_result.releaseNonNull();
            return WTFMove(*reinterpret_cast<Ref<${objectType}>*>(&result));
        }
    };

    /*
     * Synthetic constructor:
${constructorExample}
     */
    static Builder<NoFieldsSet> create()
    {
        return Builder<NoFieldsSet>(InspectorObject::create());
    }""")

    ProtocolObjectRuntimeCast = (
"""RefPtr<${objectType}> BindingTraits<${objectType}>::runtimeCast(RefPtr<InspectorValue>&& value)
{
    RefPtr<InspectorObject> result;
    bool castSucceeded = value->asObject(result);
    ASSERT_UNUSED(castSucceeded, castSucceeded);
#if !ASSERT_DISABLED
    BindingTraits<${objectType}>::assertValueHasExpectedType(result.get());
#endif  // !ASSERT_DISABLED
    COMPILE_ASSERT(sizeof(${objectType}) == sizeof(InspectorObjectBase), type_cast_problem);
    return static_cast<${objectType}*>(static_cast<InspectorObjectBase*>(result.get()));
}
""")
