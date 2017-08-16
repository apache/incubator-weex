#!/usr/bin/env python
#
# Copyright (c) 2014-2016 Apple Inc. All rights reserved.
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


import logging
import string
from string import Template

from cpp_generator import CppGenerator
from cpp_generator_templates import CppGeneratorTemplates as CppTemplates
from generator import Generator, ucfirst
from models import ObjectType, ArrayType

log = logging.getLogger('global')


class CppBackendDispatcherImplementationGenerator(CppGenerator):
    def __init__(self, *args, **kwargs):
        CppGenerator.__init__(self, *args, **kwargs)

    def output_filename(self):
        return "%sBackendDispatchers.cpp" % self.protocol_name()

    def domains_to_generate(self):
        return filter(lambda domain: len(self.commands_for_domain(domain)) > 0, Generator.domains_to_generate(self))

    def generate_output(self):
        secondary_headers = [
            '<inspector/InspectorFrontendRouter.h>',
            '<inspector/InspectorValues.h>',
            '<wtf/NeverDestroyed.h>',
            '<wtf/text/CString.h>']

        secondary_includes = ['#include %s' % header for header in secondary_headers]

        if self.model().framework.setting('alternate_dispatchers', False):
            secondary_includes.append('')
            secondary_includes.append('#if ENABLE(INSPECTOR_ALTERNATE_DISPATCHERS)')
            secondary_includes.append('#include "%sAlternateBackendDispatchers.h"' % self.protocol_name())
            secondary_includes.append('#endif')

        header_args = {
            'primaryInclude': '"%sBackendDispatchers.h"' % self.protocol_name(),
            'secondaryIncludes': '\n'.join(secondary_includes),
        }

        sections = []
        sections.append(self.generate_license())
        sections.append(Template(CppTemplates.ImplementationPrelude).substitute(None, **header_args))
        sections.append("\n".join(map(self._generate_handler_class_destructor_for_domain, self.domains_to_generate())))
        sections.extend(map(self._generate_dispatcher_implementations_for_domain, self.domains_to_generate()))
        sections.append(Template(CppTemplates.ImplementationPostlude).substitute(None, **header_args))
        return "\n\n".join(sections)

    # Private methods.

    def _generate_handler_class_destructor_for_domain(self, domain):
        destructor_args = {
            'domainName': domain.domain_name
        }
        destructor = '%(domainName)sBackendDispatcherHandler::~%(domainName)sBackendDispatcherHandler() { }' % destructor_args
        return self.wrap_with_guard_for_domain(domain, destructor)

    def _generate_dispatcher_implementations_for_domain(self, domain):
        implementations = []

        constructor_args = {
            'domainName': domain.domain_name,
        }
        implementations.append(Template(CppTemplates.BackendDispatcherImplementationDomainConstructor).substitute(None, **constructor_args))

        commands = self.commands_for_domain(domain)

        if len(commands) <= 5:
            implementations.append(self._generate_small_dispatcher_switch_implementation_for_domain(domain))
        else:
            implementations.append(self._generate_large_dispatcher_switch_implementation_for_domain(domain))

        for command in commands:
            if command.is_async:
                implementations.append(self._generate_async_dispatcher_class_for_domain(command, domain))
            implementations.append(self._generate_dispatcher_implementation_for_command(command, domain))

        return self.wrap_with_guard_for_domain(domain, '\n\n'.join(implementations))

    def _generate_small_dispatcher_switch_implementation_for_domain(self, domain):
        commands = self.commands_for_domain(domain)

        cases = []
        cases.append('    if (method == "%s")' % commands[0].command_name)
        cases.append('        %s(requestId, WTFMove(parameters));' % commands[0].command_name)
        for command in commands[1:]:
            cases.append('    else if (method == "%s")' % command.command_name)
            cases.append('        %s(requestId, WTFMove(parameters));' % command.command_name)

        switch_args = {
            'domainName': domain.domain_name,
            'dispatchCases': "\n".join(cases)
        }

        return Template(CppTemplates.BackendDispatcherImplementationSmallSwitch).substitute(None, **switch_args)

    def _generate_large_dispatcher_switch_implementation_for_domain(self, domain):
        commands = self.commands_for_domain(domain)

        cases = []
        for command in commands:
            args = {
                'domainName': domain.domain_name,
                'commandName': command.command_name
            }
            cases.append('            { "%(commandName)s", &%(domainName)sBackendDispatcher::%(commandName)s },' % args)

        switch_args = {
            'domainName': domain.domain_name,
            'dispatchCases': "\n".join(cases)
        }

        return Template(CppTemplates.BackendDispatcherImplementationLargeSwitch).substitute(None, **switch_args)

    def _generate_async_dispatcher_class_for_domain(self, command, domain):
        out_parameter_assignments = []
        formal_parameters = []

        for parameter in command.return_parameters:
            param_args = {
                'keyedSetMethod': CppGenerator.cpp_setter_method_for_type(parameter.type),
                'parameterKey': parameter.parameter_name,
                'parameterName': parameter.parameter_name,
                'parameterType': CppGenerator.cpp_type_for_stack_in_parameter(parameter),
                'helpersNamespace': self.helpers_namespace(),
            }

            formal_parameters.append('%s %s' % (CppGenerator.cpp_type_for_formal_async_parameter(parameter), parameter.parameter_name))

            if parameter.is_optional:
                if CppGenerator.should_use_wrapper_for_return_type(parameter.type):
                    out_parameter_assignments.append('    if (%(parameterName)s.isAssigned())' % param_args)
                    out_parameter_assignments.append('        jsonMessage->%(keyedSetMethod)s(ASCIILiteral("%(parameterKey)s"), %(parameterName)s.getValue());' % param_args)
                else:
                    out_parameter_assignments.append('    if (%(parameterName)s)' % param_args)
                    out_parameter_assignments.append('        jsonMessage->%(keyedSetMethod)s(ASCIILiteral("%(parameterKey)s"), %(parameterName)s);' % param_args)
            elif parameter.type.is_enum():
                out_parameter_assignments.append('    jsonMessage->%(keyedSetMethod)s(ASCIILiteral("%(parameterKey)s"), Inspector::Protocol::%(helpersNamespace)s::getEnumConstantValue(%(parameterName)s));' % param_args)
            else:
                out_parameter_assignments.append('    jsonMessage->%(keyedSetMethod)s(ASCIILiteral("%(parameterKey)s"), %(parameterName)s);' % param_args)

        async_args = {
            'domainName': domain.domain_name,
            'callbackName': ucfirst(command.command_name) + 'Callback',
            'formalParameters': ", ".join(formal_parameters),
            'outParameterAssignments': "\n".join(out_parameter_assignments)
        }
        return Template(CppTemplates.BackendDispatcherImplementationAsyncCommand).substitute(None, **async_args)

    def _generate_dispatcher_implementation_for_command(self, command, domain):
        in_parameter_declarations = []
        out_parameter_declarations = []
        out_parameter_assignments = []
        alternate_dispatcher_method_parameters = ['requestId']
        method_parameters = ['error']

        for parameter in command.call_parameters:
            parameter_name = 'in_' + parameter.parameter_name
            if parameter.is_optional:
                parameter_name = 'opt_' + parameter_name

            out_success_argument = 'nullptr'
            if parameter.is_optional:
                out_success_argument = '&%s_valueFound' % parameter_name
                in_parameter_declarations.append('    bool %s_valueFound = false;' % parameter_name)

            # Now add appropriate operators.
            parameter_expression = parameter_name

            if CppGenerator.should_use_references_for_type(parameter.type):
                if parameter.is_optional:
                    parameter_expression = '%s.get()' % parameter_expression
                else:
                    # This assumes that we have already proved the object is non-null.
                    # If a required property is missing, InspectorBackend::getObject will
                    # append a protocol error, and the method dispatcher will return without
                    # invoking the backend method (and dereferencing the object).
                    parameter_expression = '*%s' % parameter_expression
            elif parameter.is_optional:
                parameter_expression = '&%s' % parameter_expression

            param_args = {
                'parameterType': CppGenerator.cpp_type_for_stack_in_parameter(parameter),
                'parameterKey': parameter.parameter_name,
                'parameterName': parameter_name,
                'parameterExpression': parameter_expression,
                'keyedGetMethod': CppGenerator.cpp_getter_method_for_type(parameter.type),
                'successOutParam': out_success_argument
            }

            in_parameter_declarations.append('    %(parameterType)s %(parameterName)s = m_backendDispatcher->%(keyedGetMethod)s(parameters.get(), ASCIILiteral("%(parameterKey)s"), %(successOutParam)s);' % param_args)

            if parameter.is_optional:
                optional_in_parameter_string = '%(parameterName)s_valueFound ? %(parameterExpression)s : nullptr' % param_args
                alternate_dispatcher_method_parameters.append(optional_in_parameter_string)
                method_parameters.append(optional_in_parameter_string)
            else:
                alternate_dispatcher_method_parameters.append(parameter_expression)
                method_parameters.append(parameter_expression)

        if command.is_async:
            async_args = {
                'domainName': domain.domain_name,
                'callbackName': ucfirst(command.command_name) + 'Callback'
            }

            out_parameter_assignments.append('        callback->disable();')
            out_parameter_assignments.append('        m_backendDispatcher->reportProtocolError(BackendDispatcher::ServerError, error);')
            out_parameter_assignments.append('        return;')
            method_parameters.append('callback.copyRef()')

        else:
            for parameter in command.return_parameters:
                param_args = {
                    'parameterType': CppGenerator.cpp_type_for_stack_out_parameter(parameter),
                    'parameterKey': parameter.parameter_name,
                    'parameterName': parameter.parameter_name,
                    'keyedSetMethod': CppGenerator.cpp_setter_method_for_type(parameter.type),
                    'helpersNamespace': self.helpers_namespace(),
                }

                out_parameter_declarations.append('    %(parameterType)s out_%(parameterName)s;' % param_args)
                if parameter.is_optional:
                    if CppGenerator.should_use_wrapper_for_return_type(parameter.type):
                        out_parameter_assignments.append('        if (out_%(parameterName)s.isAssigned())' % param_args)
                        out_parameter_assignments.append('            result->%(keyedSetMethod)s(ASCIILiteral("%(parameterKey)s"), out_%(parameterName)s.getValue());' % param_args)
                    else:
                        out_parameter_assignments.append('        if (out_%(parameterName)s)' % param_args)
                        out_parameter_assignments.append('            result->%(keyedSetMethod)s(ASCIILiteral("%(parameterKey)s"), out_%(parameterName)s);' % param_args)
                elif parameter.type.is_enum():
                    out_parameter_assignments.append('        result->%(keyedSetMethod)s(ASCIILiteral("%(parameterKey)s"), Inspector::Protocol::%(helpersNamespace)s::getEnumConstantValue(out_%(parameterName)s));' % param_args)
                else:
                    out_parameter_assignments.append('        result->%(keyedSetMethod)s(ASCIILiteral("%(parameterKey)s"), out_%(parameterName)s);' % param_args)

                if CppGenerator.should_pass_by_copy_for_return_type(parameter.type):
                    method_parameters.append('out_' + parameter.parameter_name)
                else:
                    method_parameters.append('&out_' + parameter.parameter_name)

        command_args = {
            'domainName': domain.domain_name,
            'callbackName': '%sCallback' % ucfirst(command.command_name),
            'commandName': command.command_name,
            'inParameterDeclarations': '\n'.join(in_parameter_declarations),
            'invocationParameters': ', '.join(method_parameters),
            'alternateInvocationParameters': ', '.join(alternate_dispatcher_method_parameters),
        }

        lines = []
        if len(command.call_parameters) == 0:
            lines.append('void %(domainName)sBackendDispatcher::%(commandName)s(long requestId, RefPtr<InspectorObject>&&)' % command_args)
        else:
            lines.append('void %(domainName)sBackendDispatcher::%(commandName)s(long requestId, RefPtr<InspectorObject>&& parameters)' % command_args)
        lines.append('{')

        if len(command.call_parameters) > 0:
            lines.append(Template(CppTemplates.BackendDispatcherImplementationPrepareCommandArguments).substitute(None, **command_args))

        if self.model().framework.setting('alternate_dispatchers', False):
            lines.append('#if ENABLE(INSPECTOR_ALTERNATE_DISPATCHERS)')
            lines.append('    if (m_alternateDispatcher) {')
            lines.append('        m_alternateDispatcher->%(commandName)s(%(alternateInvocationParameters)s);' % command_args)
            lines.append('        return;')
            lines.append('    }')
            lines.append('#endif')
            lines.append('')

        lines.append('    ErrorString error;')
        lines.append('    Ref<InspectorObject> result = InspectorObject::create();')
        if command.is_async:
            lines.append('    Ref<%(domainName)sBackendDispatcherHandler::%(callbackName)s> callback = adoptRef(*new %(domainName)sBackendDispatcherHandler::%(callbackName)s(m_backendDispatcher.copyRef(), requestId));' % command_args)
        if len(command.return_parameters) > 0:
            lines.extend(out_parameter_declarations)
        lines.append('    m_agent->%(commandName)s(%(invocationParameters)s);' % command_args)
        lines.append('')
        if command.is_async:
            lines.append('    if (error.length()) {')
            lines.extend(out_parameter_assignments)
            lines.append('    }')
        elif len(command.return_parameters) > 1:
            lines.append('    if (!error.length()) {')
            lines.extend(out_parameter_assignments)
            lines.append('    }')
        elif len(command.return_parameters) == 1:
            lines.append('    if (!error.length())')
            lines.extend(out_parameter_assignments)
            lines.append('')

        if not command.is_async:
            lines.append('    if (!error.length())')
            lines.append('        m_backendDispatcher->sendResponse(requestId, WTFMove(result));')
            lines.append('    else')
            lines.append('        m_backendDispatcher->reportProtocolError(BackendDispatcher::ServerError, WTFMove(error));')
        lines.append('}')
        return "\n".join(lines)
