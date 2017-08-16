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
from models import ObjectType, EnumType, Platforms
from objc_generator import ObjCGenerator, join_type_and_name
from objc_generator_templates import ObjCGeneratorTemplates as ObjCTemplates

log = logging.getLogger('global')


def add_newline(lines):
    if lines and lines[-1] == '':
        return
    lines.append('')


class ObjCHeaderGenerator(ObjCGenerator):
    def __init__(self, *args, **kwargs):
        ObjCGenerator.__init__(self, *args, **kwargs)

    def output_filename(self):
        return '%s.h' % self.protocol_name()

    def generate_output(self):
        headers = set([
            '<WebInspector/%sJSONObject.h>' % ObjCGenerator.OBJC_STATIC_PREFIX,
        ])

        header_args = {
            'includes': '\n'.join(['#import ' + header for header in sorted(headers)]),
        }

        domains = self.domains_to_generate()
        type_domains = filter(self.should_generate_types_for_domain, domains)
        command_domains = filter(self.should_generate_commands_for_domain, domains)
        event_domains = filter(self.should_generate_events_for_domain, domains)

        # FIXME: <https://webkit.org/b/138222> Web Inspector: Reduce unnecessary enums/types generated in ObjC Protocol Interfaces
        # Currently we generate enums/types for all types in the type_domains. For the built-in
        # JSC domains (Debugger, Runtime) this produces extra unused types. We only need to
        # generate these types if they are referenced by the command_domains or event_domains.

        sections = []
        sections.append(self.generate_license())
        sections.append(Template(ObjCTemplates.HeaderPrelude).substitute(None, **header_args))
        sections.append('\n'.join(filter(None, map(self._generate_forward_declarations, type_domains))))
        sections.append(self._generate_enum_for_platforms())
        sections.append('\n'.join(filter(None, map(self._generate_enums, type_domains))))
        sections.append('\n'.join(filter(None, map(self._generate_types, type_domains))))

        if self.get_generator_setting('generate_backend', False):
            sections.append('\n\n'.join(filter(None, map(self._generate_command_protocols, command_domains))))
            sections.append('\n\n'.join(filter(None, map(self._generate_event_interfaces, event_domains))))

        sections.append(Template(ObjCTemplates.HeaderPostlude).substitute(None))
        return '\n\n'.join(sections)

    def _generate_forward_declarations(self, domain):
        lines = []
        for declaration in self.type_declarations_for_domain(domain):
            if (isinstance(declaration.type, ObjectType)):
                objc_name = self.objc_name_for_type(declaration.type)
                lines.append('@class %s;' % objc_name)
        return '\n'.join(lines)

    def _generate_enums(self, domain):
        lines = []

        # Type enums and member enums.
        for declaration in self.type_declarations_for_domain(domain):
            if isinstance(declaration.type, EnumType):
                add_newline(lines)
                lines.append(self._generate_anonymous_enum_for_declaration(domain, declaration))
            else:
                for member in declaration.type_members:
                    if isinstance(member.type, EnumType) and member.type.is_anonymous:
                        add_newline(lines)
                        lines.append(self._generate_anonymous_enum_for_member(domain, declaration, member))

        # Anonymous command enums.
        for command in self.commands_for_domain(domain):
            for parameter in command.call_parameters:
                if isinstance(parameter.type, EnumType) and parameter.type.is_anonymous:
                    add_newline(lines)
                    lines.append(self._generate_anonymous_enum_for_parameter(domain, command.command_name, parameter))
            for parameter in command.return_parameters:
                if isinstance(parameter.type, EnumType) and parameter.type.is_anonymous:
                    add_newline(lines)
                    lines.append(self._generate_anonymous_enum_for_parameter(domain, command.command_name, parameter))

        # Anonymous event enums.
        for event in self.events_for_domain(domain):
            for parameter in event.event_parameters:
                if isinstance(parameter.type, EnumType) and parameter.type.is_anonymous:
                    add_newline(lines)
                    lines.append(self._generate_anonymous_enum_for_parameter(domain, event.event_name, parameter))

        return '\n'.join(lines)

    def _generate_types(self, domain):
        lines = []
        # Type interfaces.
        for declaration in self.type_declarations_for_domain(domain):
            if isinstance(declaration.type, ObjectType):
                add_newline(lines)
                lines.append(self._generate_type_interface(domain, declaration))
        return '\n'.join(lines)

    def _generate_enum_for_platforms(self):
        objc_enum_name = '%sPlatform' % self.objc_prefix()
        enum_values = [platform.name for platform in Platforms]
        return self._generate_enum(objc_enum_name, enum_values)

    def _generate_anonymous_enum_for_declaration(self, domain, declaration):
        objc_enum_name = self.objc_enum_name_for_anonymous_enum_declaration(declaration)
        return self._generate_enum(objc_enum_name, declaration.type.enum_values())

    def _generate_anonymous_enum_for_member(self, domain, declaration, member):
        objc_enum_name = self.objc_enum_name_for_anonymous_enum_member(declaration, member)
        return self._generate_enum(objc_enum_name, member.type.enum_values())

    def _generate_anonymous_enum_for_parameter(self, domain, event_or_command_name, parameter):
        objc_enum_name = self.objc_enum_name_for_anonymous_enum_parameter(domain, event_or_command_name, parameter)
        return self._generate_enum(objc_enum_name, parameter.type.enum_values())

    def _generate_enum(self, enum_name, enum_values):
        lines = []
        lines.append('typedef NS_ENUM(NSInteger, %s) {' % enum_name)
        for enum_value in enum_values:
            lines.append('    %s%s,' % (enum_name, Generator.stylized_name_for_enum_value(enum_value)))
        lines.append('};')
        return '\n'.join(lines)

    def _generate_type_interface(self, domain, declaration):
        lines = []
        objc_name = self.objc_name_for_type(declaration.type)
        lines.append('__attribute__((visibility ("default")))')
        lines.append('@interface %s : %sJSONObject' % (objc_name, ObjCGenerator.OBJC_STATIC_PREFIX))

        # The initializers that take a payload or inspector object are only needed by the frontend.
        if self.get_generator_setting('generate_frontend', False):
            lines.append('- (instancetype)initWithPayload:(NSDictionary<NSString *, id> *)payload;')
            lines.append('- (instancetype)initWithJSONObject:(RWIProtocolJSONObject *)jsonObject;')

        required_members = filter(lambda member: not member.is_optional, declaration.type_members)
        optional_members = filter(lambda member: member.is_optional, declaration.type_members)
        if required_members:
            lines.append(self._generate_init_method_for_required_members(domain, declaration, required_members))
        for member in required_members:
            lines.append('/* required */ ' + self._generate_member_property(declaration, member))
        for member in optional_members:
            lines.append('/* optional */ ' + self._generate_member_property(declaration, member))
        lines.append('@end')
        return '\n'.join(lines)

    def _generate_init_method_for_required_members(self, domain, declaration, required_members):
        pairs = []
        for member in required_members:
            objc_type = self.objc_type_for_member(declaration, member)
            var_name = ObjCGenerator.identifier_to_objc_identifier(member.member_name)
            pairs.append('%s:(%s)%s' % (var_name, objc_type, var_name))
        pairs[0] = ucfirst(pairs[0])
        return '- (instancetype)initWith%s;' % ' '.join(pairs)

    def _generate_member_property(self, declaration, member):
        accessor_type = self.objc_accessor_type_for_member(member)
        objc_type = self.objc_type_for_member(declaration, member)
        return '@property (nonatomic, %s) %s;' % (accessor_type, join_type_and_name(objc_type, ObjCGenerator.identifier_to_objc_identifier(member.member_name)))

    def _generate_command_protocols(self, domain):
        lines = []
        if self.commands_for_domain(domain):
            objc_name = '%s%sDomainHandler' % (self.objc_prefix(), domain.domain_name)
            lines.append('@protocol %s <NSObject>' % objc_name)
            lines.append('@required')
            for command in self.commands_for_domain(domain):
                lines.append(self._generate_single_command_protocol(domain, command))
            lines.append('@end')
        return '\n'.join(lines)

    def _generate_single_command_protocol(self, domain, command):
        pairs = []
        pairs.append('ErrorCallback:(void(^)(NSString *error))errorCallback')
        pairs.append('successCallback:(%s)successCallback' % self._callback_block_for_command(domain, command))
        for parameter in command.call_parameters:
            param_name = parameter.parameter_name
            pairs.append('%s:(%s)%s' % (param_name, self.objc_type_for_param(domain, command.command_name, parameter), param_name))
        return '- (void)%sWith%s;' % (command.command_name, ' '.join(pairs))

    def _callback_block_for_command(self, domain, command):
        pairs = []
        for parameter in command.return_parameters:
            pairs.append(join_type_and_name(self.objc_type_for_param(domain, command.command_name, parameter), parameter.parameter_name))
        return 'void(^)(%s)' % ', '.join(pairs)

    def _generate_event_interfaces(self, domain):
        lines = []
        events = self.events_for_domain(domain)
        if len(events):
            objc_name = '%s%sDomainEventDispatcher' % (self.objc_prefix(), domain.domain_name)
            lines.append('__attribute__((visibility ("default")))')
            lines.append('@interface %s : NSObject' % objc_name)
            for event in events:
                lines.append(self._generate_single_event_interface(domain, event))
            lines.append('@end')
        return '\n'.join(lines)

    def _generate_single_event_interface(self, domain, event):
        if not event.event_parameters:
            return '- (void)%s;' % event.event_name
        pairs = []
        for parameter in event.event_parameters:
            param_name = parameter.parameter_name
            pairs.append('%s:(%s)%s' % (param_name, self.objc_type_for_param(domain, event.event_name, parameter), param_name))
        pairs[0] = ucfirst(pairs[0])
        return '- (void)%sWith%s;' % (event.event_name, ' '.join(pairs))
