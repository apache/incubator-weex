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
from operator import methodcaller

from cpp_generator import CppGenerator
from cpp_generator_templates import CppGeneratorTemplates as CppTemplates
from generator import Generator, ucfirst
from models import AliasedType, ArrayType, EnumType, ObjectType

log = logging.getLogger('global')


class CppProtocolTypesImplementationGenerator(CppGenerator):
    def __init__(self, *args, **kwargs):
        CppGenerator.__init__(self, *args, **kwargs)

    def output_filename(self):
        return "%sProtocolObjects.cpp" % self.protocol_name()

    def generate_output(self):
        domains = self.domains_to_generate()
        self.calculate_types_requiring_shape_assertions(domains)

        secondary_headers = [
            '<wtf/Optional.h>',
            '<wtf/text/CString.h>',
        ]

        header_args = {
            'primaryInclude': '"%sProtocolObjects.h"' % self.protocol_name(),
            'secondaryIncludes': "\n".join(['#include %s' % header for header in secondary_headers]),
        }

        sections = []
        sections.append(self.generate_license())
        sections.append(Template(CppTemplates.ImplementationPrelude).substitute(None, **header_args))
        sections.append('namespace Protocol {')
        sections.extend(self._generate_enum_mapping_and_conversion_methods(domains))
        sections.append(self._generate_open_field_names())
        builder_sections = map(self._generate_builders_for_domain, domains)
        sections.extend(filter(lambda section: len(section) > 0, builder_sections))
        sections.append('} // namespace Protocol')
        sections.append(Template(CppTemplates.ImplementationPostlude).substitute(None, **header_args))

        return "\n\n".join(sections)

    # Private methods.

    def _generate_enum_mapping(self):
        if not self.assigned_enum_values():
            return []

        lines = []
        lines.append('static const char* const enum_constant_values[] = {')
        lines.extend(['    "%s",' % enum_value for enum_value in self.assigned_enum_values()])
        lines.append('};')
        lines.append('')
        lines.append('String getEnumConstantValue(int code) {')
        lines.append('    return enum_constant_values[code];')
        lines.append('}')
        return ['\n'.join(lines)]

    def _generate_enum_conversion_methods_for_domain(self, domain):

        def type_member_is_anonymous_enum_type(type_member):
            return isinstance(type_member.type, EnumType) and type_member.type.is_anonymous

        def generate_conversion_method_body(enum_type, cpp_protocol_type):
            body_lines = []
            body_lines.extend([
                'template<>',
                'std::optional<%s> parseEnumValueFromString<%s>(const String& protocolString)' % (cpp_protocol_type, cpp_protocol_type),
                '{',
                '    static const size_t constantValues[] = {',
            ])

            enum_values = enum_type.enum_values()
            for enum_value in enum_values:
                body_lines.append('        (size_t)%s::%s,' % (cpp_protocol_type, Generator.stylized_name_for_enum_value(enum_value)))

            body_lines.extend([
                '    };',
                '    for (size_t i = 0; i < %d; ++i)' % len(enum_values),
                '        if (protocolString == enum_constant_values[constantValues[i]])',
                '            return (%s)constantValues[i];' % cpp_protocol_type,
                '',
                '    return std::nullopt;',
                '}',
                '',
            ])
            return body_lines

        type_declarations = self.type_declarations_for_domain(domain)
        declaration_types = [decl.type for decl in type_declarations]
        object_types = filter(lambda _type: isinstance(_type, ObjectType), declaration_types)
        enum_types = filter(lambda _type: isinstance(_type, EnumType), declaration_types)
        if len(object_types) + len(enum_types) == 0:
            return ''

        sorted(object_types, key=methodcaller('raw_name'))
        sorted(enum_types, key=methodcaller('raw_name'))

        lines = []
        lines.append("// Enums in the '%s' Domain" % domain.domain_name)
        for enum_type in enum_types:
            cpp_protocol_type = CppGenerator.cpp_protocol_type_for_type(enum_type)
            lines.extend(generate_conversion_method_body(enum_type, cpp_protocol_type))

        for object_type in object_types:
            for enum_member in filter(type_member_is_anonymous_enum_type, object_type.members):
                cpp_protocol_type = CppGenerator.cpp_protocol_type_for_type_member(enum_member, object_type.declaration())
                lines.extend(generate_conversion_method_body(enum_member.type, cpp_protocol_type))

        if len(lines) == 1:
            return ''  # No real declarations to emit, just the domain comment.

        return self.wrap_with_guard_for_domain(domain, '\n'.join(lines))

    def _generate_enum_mapping_and_conversion_methods(self, domains):
        sections = []
        sections.append('namespace %s {' % self.helpers_namespace())
        sections.extend(self._generate_enum_mapping())
        enum_parser_sections = map(self._generate_enum_conversion_methods_for_domain, domains)
        sections.extend(filter(lambda section: len(section) > 0, enum_parser_sections))
        if len(sections) == 1:
            return []  # No declarations to emit, just the namespace.

        sections.append('} // namespace %s' % self.helpers_namespace())
        return sections

    def _generate_open_field_names(self):
        lines = []
        for domain in self.domains_to_generate():
            type_declarations = self.type_declarations_for_domain(domain)
            for type_declaration in filter(lambda decl: Generator.type_has_open_fields(decl.type), type_declarations):
                open_members = Generator.open_fields(type_declaration)
                for type_member in sorted(open_members, key=lambda member: member.member_name):
                    field_name = '::'.join(['Inspector', 'Protocol', domain.domain_name, ucfirst(type_declaration.type_name), ucfirst(type_member.member_name)])
                    lines.append('const char* %s = "%s";' % (field_name, type_member.member_name))

        return '\n'.join(lines)

    def _generate_builders_for_domain(self, domain):
        sections = []
        type_declarations = self.type_declarations_for_domain(domain)
        declarations_to_generate = filter(lambda decl: self.type_needs_shape_assertions(decl.type), type_declarations)

        for type_declaration in declarations_to_generate:
            for type_member in type_declaration.type_members:
                if isinstance(type_member.type, EnumType):
                    sections.append(self._generate_assertion_for_enum(type_member, type_declaration))

            if isinstance(type_declaration.type, ObjectType):
                sections.append(self._generate_assertion_for_object_declaration(type_declaration))
                if Generator.type_needs_runtime_casts(type_declaration.type):
                    sections.append(self._generate_runtime_cast_for_object_declaration(type_declaration))

        return '\n\n'.join(sections)

    def _generate_runtime_cast_for_object_declaration(self, object_declaration):
        args = {
            'objectType': CppGenerator.cpp_protocol_type_for_type(object_declaration.type)
        }
        return Template(CppTemplates.ProtocolObjectRuntimeCast).substitute(None, **args)

    def _generate_assertion_for_object_declaration(self, object_declaration):
        required_members = filter(lambda member: not member.is_optional, object_declaration.type_members)
        optional_members = filter(lambda member: member.is_optional, object_declaration.type_members)
        should_count_properties = not Generator.type_has_open_fields(object_declaration.type)
        lines = []

        lines.append('#if !ASSERT_DISABLED')
        lines.append('void BindingTraits<%s>::assertValueHasExpectedType(Inspector::InspectorValue* value)' % (CppGenerator.cpp_protocol_type_for_type(object_declaration.type)))
        lines.append("""{
    ASSERT_ARG(value, value);
    RefPtr<InspectorObject> object;
    bool castSucceeded = value->asObject(object);
    ASSERT_UNUSED(castSucceeded, castSucceeded);""")
        for type_member in required_members:
            args = {
                'memberName': type_member.member_name,
                'assertMethod': CppGenerator.cpp_assertion_method_for_type_member(type_member, object_declaration)
            }

            lines.append("""    {
        InspectorObject::iterator %(memberName)sPos = object->find(ASCIILiteral("%(memberName)s"));
        ASSERT(%(memberName)sPos != object->end());
        %(assertMethod)s(%(memberName)sPos->value.get());
    }""" % args)

        if should_count_properties:
            lines.append('')
            lines.append('    int foundPropertiesCount = %s;' % len(required_members))

        for type_member in optional_members:
            args = {
                'memberName': type_member.member_name,
                'assertMethod': CppGenerator.cpp_assertion_method_for_type_member(type_member, object_declaration)
            }

            lines.append("""    {
        InspectorObject::iterator %(memberName)sPos = object->find(ASCIILiteral("%(memberName)s"));
        if (%(memberName)sPos != object->end()) {
            %(assertMethod)s(%(memberName)sPos->value.get());""" % args)

            if should_count_properties:
                lines.append('            ++foundPropertiesCount;')
            lines.append('        }')
            lines.append('    }')

        if should_count_properties:
            lines.append('    if (foundPropertiesCount != object->size())')
            lines.append('        FATAL("Unexpected properties in object: %s\\n", object->toJSONString().ascii().data());')
        lines.append('}')
        lines.append('#endif // !ASSERT_DISABLED')
        return '\n'.join(lines)

    def _generate_assertion_for_enum(self, enum_member, object_declaration):
        lines = []
        lines.append('#if !ASSERT_DISABLED')
        lines.append('void %s(Inspector::InspectorValue* value)' % CppGenerator.cpp_assertion_method_for_type_member(enum_member, object_declaration))
        lines.append('{')
        lines.append('    ASSERT_ARG(value, value);')
        lines.append('    String result;')
        lines.append('    bool castSucceeded = value->asString(result);')
        lines.append('    ASSERT(castSucceeded);')

        assert_condition = ' || '.join(['result == "%s"' % enum_value for enum_value in enum_member.type.enum_values()])
        lines.append('    ASSERT(%s);' % assert_condition)
        lines.append('}')
        lines.append('#endif // !ASSERT_DISABLED')

        return '\n'.join(lines)
