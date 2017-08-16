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

from generator import Generator
from models import EnumType, Frameworks, Platforms
from objc_generator import ObjCGenerator
from objc_generator_templates import ObjCGeneratorTemplates as ObjCTemplates

log = logging.getLogger('global')


def add_newline(lines):
    if lines and lines[-1] == '':
        return
    lines.append('')


class ObjCProtocolTypeConversionsHeaderGenerator(ObjCGenerator):
    def __init__(self, *args, **kwargs):
        ObjCGenerator.__init__(self, *args, **kwargs)

    def output_filename(self):
        return '%sTypeConversions.h' % self.protocol_name()

    def domains_to_generate(self):
        return filter(self.should_generate_types_for_domain, Generator.domains_to_generate(self))

    def generate_output(self):
        headers = [
            '"%s.h"' % self.protocol_name(),
            Generator.string_for_file_include('%sArrayConversions.h' % ObjCGenerator.OBJC_STATIC_PREFIX, Frameworks.WebInspector, self.model().framework),
        ]
        headers.sort()

        header_args = {
            'includes': '\n'.join(['#import ' + header for header in headers]),
        }

        domains = self.domains_to_generate()
        sections = []
        sections.append(self.generate_license())
        sections.append(Template(ObjCTemplates.TypeConversionsHeaderPrelude).substitute(None, **header_args))
        sections.append(Template(ObjCTemplates.TypeConversionsHeaderStandard).substitute(None))
        sections.append(self._generate_enum_conversion_for_platforms())
        sections.extend(map(self._generate_enum_conversion_functions, domains))
        sections.append(Template(ObjCTemplates.TypeConversionsHeaderPostlude).substitute(None, **header_args))
        return '\n\n'.join(sections)

    def _generate_enum_conversion_functions(self, domain):
        lines = []

        # Type enums and member enums.
        for declaration in self.type_declarations_for_domain(domain):
            if isinstance(declaration.type, EnumType):
                add_newline(lines)
                lines.append(self._generate_anonymous_enum_conversion_for_declaration(domain, declaration))
            else:
                for member in declaration.type_members:
                    if (isinstance(member.type, EnumType) and member.type.is_anonymous):
                        add_newline(lines)
                        lines.append(self._generate_anonymous_enum_conversion_for_member(domain, declaration, member))

        # Anonymous command enums.
        for command in self.commands_for_domain(domain):
            for parameter in command.call_parameters:
                if (isinstance(parameter.type, EnumType) and parameter.type.is_anonymous):
                    add_newline(lines)
                    lines.append(self._generate_anonymous_enum_conversion_for_parameter(domain, command.command_name, parameter))
            for parameter in command.return_parameters:
                if (isinstance(parameter.type, EnumType) and parameter.type.is_anonymous):
                    add_newline(lines)
                    lines.append(self._generate_anonymous_enum_conversion_for_parameter(domain, command.command_name, parameter))

        # Anonymous event enums.
        for event in self.events_for_domain(domain):
            for parameter in event.event_parameters:
                if (isinstance(parameter.type, EnumType) and parameter.type.is_anonymous):
                    add_newline(lines)
                    lines.append(self._generate_anonymous_enum_conversion_for_parameter(domain, event.event_name, parameter))

        return '\n'.join(lines)

    def _generate_enum_conversion_for_platforms(self):
        objc_enum_name = '%sPlatform' % self.objc_prefix()
        enum_values = [platform.name for platform in Platforms]
        lines = []
        lines.append(self._generate_enum_objc_to_protocol_string(objc_enum_name, enum_values))
        lines.append(self._generate_enum_from_protocol_string(objc_enum_name, enum_values))
        return '\n\n'.join(lines)

    def _generate_anonymous_enum_conversion_for_declaration(self, domain, declaration):
        objc_enum_name = self.objc_enum_name_for_anonymous_enum_declaration(declaration)
        enum_values = declaration.type.enum_values()
        lines = []
        lines.append(self._generate_enum_objc_to_protocol_string(objc_enum_name, enum_values))
        lines.append(self._generate_enum_from_protocol_string(objc_enum_name, enum_values))
        return '\n\n'.join(lines)

    def _generate_anonymous_enum_conversion_for_member(self, domain, declaration, member):
        objc_enum_name = self.objc_enum_name_for_anonymous_enum_member(declaration, member)
        enum_values = member.type.enum_values()
        lines = []
        lines.append(self._generate_enum_objc_to_protocol_string(objc_enum_name, enum_values))
        lines.append(self._generate_enum_from_protocol_string(objc_enum_name, enum_values))
        return '\n\n'.join(lines)

    def _generate_anonymous_enum_conversion_for_parameter(self, domain, event_or_command_name, parameter):
        objc_enum_name = self.objc_enum_name_for_anonymous_enum_parameter(domain, event_or_command_name, parameter)
        enum_values = parameter.type.enum_values()
        lines = []
        lines.append(self._generate_enum_objc_to_protocol_string(objc_enum_name, enum_values))
        lines.append(self._generate_enum_from_protocol_string(objc_enum_name, enum_values))
        return '\n\n'.join(lines)

    def _generate_enum_objc_to_protocol_string(self, objc_enum_name, enum_values):
        lines = []
        lines.append('inline String toProtocolString(%s value)' % objc_enum_name)
        lines.append('{')
        lines.append('    switch(value) {')
        for enum_value in enum_values:
            lines.append('    case %s%s:' % (objc_enum_name, Generator.stylized_name_for_enum_value(enum_value)))
            lines.append('        return ASCIILiteral("%s");' % enum_value)
        lines.append('    }')
        lines.append('}')
        return '\n'.join(lines)

    def _generate_enum_from_protocol_string(self, objc_enum_name, enum_values):
        lines = []
        lines.append('template<>')
        lines.append('inline std::optional<%s> fromProtocolString(const String& value)' % objc_enum_name)
        lines.append('{')
        for enum_value in enum_values:
            lines.append('    if (value == "%s")' % enum_value)
            lines.append('        return %s%s;' % (objc_enum_name, Generator.stylized_name_for_enum_value(enum_value)))
        lines.append('    return std::nullopt;')
        lines.append('}')
        return '\n'.join(lines)
