#!/usr/bin/env python
#
# Copyright (c) 2014, 2016 Apple Inc. All rights reserved.
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

from generator import Generator, ucfirst
from generator_templates import GeneratorTemplates as Templates
from models import EnumType

log = logging.getLogger('global')


class JSBackendCommandsGenerator(Generator):
    def __init__(self, *args, **kwargs):
        Generator.__init__(self, *args, **kwargs)

    def output_filename(self):
        return "InspectorBackendCommands.js"

    def should_generate_domain(self, domain):
        type_declarations = self.type_declarations_for_domain(domain)
        domain_enum_types = filter(lambda declaration: isinstance(declaration.type, EnumType), type_declarations)
        return len(self.commands_for_domain(domain)) > 0 or len(self.events_for_domain(domain)) > 0 or len(domain_enum_types) > 0

    def domains_to_generate(self):
        return filter(self.should_generate_domain, Generator.domains_to_generate(self))

    def generate_output(self):
        sections = []
        sections.append(self.generate_license())
        sections.extend(map(self.generate_domain, self.domains_to_generate()))
        return "\n\n".join(sections)

    def generate_domain(self, domain):
        lines = []
        args = {
            'domain': domain.domain_name
        }

        lines.append('// %(domain)s.' % args)

        type_declarations = self.type_declarations_for_domain(domain)
        commands = self.commands_for_domain(domain)
        events = self.events_for_domain(domain)

        has_async_commands = any(map(lambda command: command.is_async, commands))
        if len(events) > 0 or has_async_commands:
            lines.append('InspectorBackend.register%(domain)sDispatcher = InspectorBackend.registerDomainDispatcher.bind(InspectorBackend, "%(domain)s");' % args)

        for declaration in type_declarations:
            if declaration.type.is_enum():
                enum_args = {
                    'domain': domain.domain_name,
                    'enumName': declaration.type_name,
                    'enumMap': ", ".join(['%s: "%s"' % (Generator.stylized_name_for_enum_value(enum_value), enum_value) for enum_value in declaration.type.enum_values()])
                }
                lines.append('InspectorBackend.registerEnum("%(domain)s.%(enumName)s", {%(enumMap)s});' % enum_args)

            def is_anonymous_enum_member(type_member):
                return isinstance(type_member.type, EnumType) and type_member.type.is_anonymous

            for _member in filter(is_anonymous_enum_member, declaration.type_members):
                enum_args = {
                    'domain': domain.domain_name,
                    'enumName': '%s%s' % (declaration.type_name, ucfirst(_member.member_name)),
                    'enumMap': ", ".join(['%s: "%s"' % (Generator.stylized_name_for_enum_value(enum_value), enum_value) for enum_value in _member.type.enum_values()])
                }
                lines.append('InspectorBackend.registerEnum("%(domain)s.%(enumName)s", {%(enumMap)s});' % enum_args)

        def is_anonymous_enum_param(param):
            return isinstance(param.type, EnumType) and param.type.is_anonymous

        for event in events:
            for param in filter(is_anonymous_enum_param, event.event_parameters):
                enum_args = {
                    'domain': domain.domain_name,
                    'enumName': '%s%s' % (ucfirst(event.event_name), ucfirst(param.parameter_name)),
                    'enumMap': ", ".join(['%s: "%s"' % (Generator.stylized_name_for_enum_value(enum_value), enum_value) for enum_value in param.type.enum_values()])
                }
                lines.append('InspectorBackend.registerEnum("%(domain)s.%(enumName)s", {%(enumMap)s});' % enum_args)

            event_args = {
                'domain': domain.domain_name,
                'eventName': event.event_name,
                'params': ", ".join(['"%s"' % parameter.parameter_name for parameter in event.event_parameters])
            }
            lines.append('InspectorBackend.registerEvent("%(domain)s.%(eventName)s", [%(params)s]);' % event_args)

        for command in commands:
            def generate_parameter_object(parameter):
                optional_string = "true" if parameter.is_optional else "false"
                pairs = []
                pairs.append('"name": "%s"' % parameter.parameter_name)
                pairs.append('"type": "%s"' % Generator.js_name_for_parameter_type(parameter.type))
                pairs.append('"optional": %s' % optional_string)
                return "{%s}" % ", ".join(pairs)

            command_args = {
                'domain': domain.domain_name,
                'commandName': command.command_name,
                'callParams': ", ".join([generate_parameter_object(parameter) for parameter in command.call_parameters]),
                'returnParams': ", ".join(['"%s"' % parameter.parameter_name for parameter in command.return_parameters]),
            }
            lines.append('InspectorBackend.registerCommand("%(domain)s.%(commandName)s", [%(callParams)s], [%(returnParams)s]);' % command_args)

        if commands or events:
            activate_args = {
                'domain': domain.domain_name,
                'availability': domain.availability,
            }
            if domain.availability:
                lines.append('InspectorBackend.activateDomain("%(domain)s", "%(availability)s");' % activate_args)
            else:
                lines.append('InspectorBackend.activateDomain("%(domain)s");' % activate_args)

        if domain.workerSupported:
            lines.append('InspectorBackend.workerSupportedDomain("%s");' % domain.domain_name)

        return "\n".join(lines)
