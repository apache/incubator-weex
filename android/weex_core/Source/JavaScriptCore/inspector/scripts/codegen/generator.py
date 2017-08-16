#!/usr/bin/env python
#
# Copyright (c) 2014 Apple Inc. All rights reserved.
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
import os.path
import re
from string import Template

from generator_templates import GeneratorTemplates as Templates
from models import PrimitiveType, ObjectType, ArrayType, EnumType, AliasedType, Frameworks, Platforms

log = logging.getLogger('global')


def ucfirst(str):
    return str[:1].upper() + str[1:]

_ALWAYS_SPECIALCASED_ENUM_VALUE_SUBSTRINGS = set(['API', 'CSS', 'DOM', 'HTML', 'JIT', 'XHR', 'XML', 'IOS', 'MacOS'])
_ALWAYS_SPECIALCASED_ENUM_VALUE_LOOKUP_TABLE = dict([(s.upper(), s) for s in _ALWAYS_SPECIALCASED_ENUM_VALUE_SUBSTRINGS])

# These objects are built manually by creating and setting InspectorValues.
# Before sending these over the protocol, their shapes are checked against the specification.
# So, any types referenced by these types require debug-only assertions that check values.
# Calculating necessary assertions is annoying, and adds a lot of complexity to the generator.

# FIXME: This should be converted into a property in JSON.
_TYPES_NEEDING_RUNTIME_CASTS = set([
    "Runtime.ObjectPreview",
    "Runtime.RemoteObject",
    "Runtime.PropertyDescriptor",
    "Runtime.InternalPropertyDescriptor",
    "Runtime.CollectionEntry",
    "Debugger.FunctionDetails",
    "Debugger.CallFrame",
    "Canvas.TraceLog",
    "Canvas.ResourceInfo",
    "Canvas.ResourceState",
    # This should be a temporary hack. TimelineEvent should be created via generated C++ API.
    "Timeline.TimelineEvent",
    # For testing purposes only.
    "Test.TypeNeedingCast"
])

# FIXME: This should be converted into a property in JSON.
_TYPES_WITH_OPEN_FIELDS = {
    "Timeline.TimelineEvent": [],
    # InspectorStyleSheet not only creates this property but wants to read it and modify it.
    "CSS.CSSProperty": [],
    # InspectorNetworkAgent needs to update mime-type.
    "Network.Response": ["mimeType"],
    # For testing purposes only.
    "Test.OpenParameters": ["alpha"],
}

class Generator:
    def __init__(self, model, platform, input_filepath):
        self._model = model
        self._platform = platform
        self._input_filepath = input_filepath
        self._settings = {}

    def model(self):
        return self._model

    def platform(self):
        return self._platform

    def set_generator_setting(self, key, value):
        self._settings[key] = value

    def can_generate_platform(self, model_platform):
        return model_platform is Platforms.Generic or self._platform is Platforms.All or model_platform is self._platform

    def type_declarations_for_domain(self, domain):
        return [type_declaration for type_declaration in domain.all_type_declarations() if self.can_generate_platform(type_declaration.platform)]

    def commands_for_domain(self, domain):
        return [command for command in domain.all_commands() if self.can_generate_platform(command.platform)]

    def events_for_domain(self, domain):
        return [event for event in domain.all_events() if self.can_generate_platform(event.platform)]

    # The goofy name is to disambiguate generator settings from framework settings.
    def get_generator_setting(self, key, default=None):
        return self._settings.get(key, default)

    def generate_license(self):
        return Template(Templates.CopyrightBlock).substitute(None, inputFilename=os.path.basename(self._input_filepath))

    # These methods are overridden by subclasses.
    def non_supplemental_domains(self):
        return filter(lambda domain: not domain.is_supplemental, self.model().domains)

    def domains_to_generate(self):
        return self.non_supplemental_domains()

    def generate_output(self):
        pass

    def output_filename(self):
        pass

    def encoding_for_enum_value(self, enum_value):
        if not hasattr(self, "_assigned_enum_values"):
            self._traverse_and_assign_enum_values()

        return self._enum_value_encodings[enum_value]

    def assigned_enum_values(self):
        if not hasattr(self, "_assigned_enum_values"):
            self._traverse_and_assign_enum_values()

        return self._assigned_enum_values[:]  # Slice.

    @staticmethod
    def type_needs_runtime_casts(_type):
        return _type.qualified_name() in _TYPES_NEEDING_RUNTIME_CASTS

    @staticmethod
    def type_has_open_fields(_type):
        return _type.qualified_name() in _TYPES_WITH_OPEN_FIELDS

    @staticmethod
    def open_fields(type_declaration):
        fields = set(_TYPES_WITH_OPEN_FIELDS.get(type_declaration.type.qualified_name(), []))
        if not fields:
            return type_declaration.type_members
        return filter(lambda member: member.member_name in fields, type_declaration.type_members)

    def type_needs_shape_assertions(self, _type):
        if not hasattr(self, "_types_needing_shape_assertions"):
            self.calculate_types_requiring_shape_assertions(self.model().domains)

        return _type in self._types_needing_shape_assertions

    # To restrict the domains over which we compute types needing assertions, call this method
    # before generating any output with the desired domains parameter. The computed
    # set of types will not be automatically regenerated on subsequent calls to
    # Generator.types_needing_shape_assertions().
    def calculate_types_requiring_shape_assertions(self, domains):
        domain_names = map(lambda domain: domain.domain_name, domains)
        log.debug("> Calculating types that need shape assertions (eligible domains: %s)" % ", ".join(domain_names))

        # Mutates the passed-in set; this simplifies checks to prevent infinite recursion.
        def gather_transitively_referenced_types(_type, gathered_types):
            if _type in gathered_types:
                return

            if isinstance(_type, ObjectType):
                log.debug("> Adding type %s to list of types needing shape assertions." % _type.qualified_name())
                gathered_types.add(_type)
                for type_member in _type.members:
                    gather_transitively_referenced_types(type_member.type, gathered_types)
            elif isinstance(_type, EnumType):
                log.debug("> Adding type %s to list of types needing shape assertions." % _type.qualified_name())
                gathered_types.add(_type)
            elif isinstance(_type, AliasedType):
                gather_transitively_referenced_types(_type.aliased_type, gathered_types)
            elif isinstance(_type, ArrayType):
                gather_transitively_referenced_types(_type.element_type, gathered_types)

        found_types = set()
        for domain in domains:
            for declaration in self.type_declarations_for_domain(domain):
                if declaration.type.qualified_name() in _TYPES_NEEDING_RUNTIME_CASTS:
                    log.debug("> Gathering types referenced by %s to generate shape assertions." % declaration.type.qualified_name())
                    gather_transitively_referenced_types(declaration.type, found_types)

        self._types_needing_shape_assertions = found_types

    # Private helper instance methods.
    def _traverse_and_assign_enum_values(self):
        self._enum_value_encodings = {}
        self._assigned_enum_values = []
        all_types = []

        domains = self.non_supplemental_domains()

        for domain in domains:
            for type_declaration in self.type_declarations_for_domain(domain):
                all_types.append(type_declaration.type)
                for type_member in type_declaration.type_members:
                    all_types.append(type_member.type)

        for domain in domains:
            for event in self.events_for_domain(domain):
                all_types.extend([parameter.type for parameter in event.event_parameters])

        for domain in domains:
            for command in self.commands_for_domain(domain):
                all_types.extend([parameter.type for parameter in command.call_parameters])
                all_types.extend([parameter.type for parameter in command.return_parameters])

        for _type in all_types:
            if not isinstance(_type, EnumType):
                continue
            map(self._assign_encoding_for_enum_value, _type.enum_values())

    def _assign_encoding_for_enum_value(self, enum_value):
        if enum_value in self._enum_value_encodings:
            return

        self._enum_value_encodings[enum_value] = len(self._assigned_enum_values)
        self._assigned_enum_values.append(enum_value)

    # Miscellaneous text manipulation routines.
    def wrap_with_guard_for_domain(self, domain, text):
        if self.model().framework is Frameworks.WebInspector:
            return text
        guard = domain.feature_guard
        if guard:
            return Generator.wrap_with_guard(guard, text)
        return text

    @staticmethod
    def wrap_with_guard(guard, text):
        return '\n'.join([
            '#if %s' % guard,
            text,
            '#endif // %s' % guard,
        ])

    @staticmethod
    def stylized_name_for_enum_value(enum_value):
        regex = '(%s)' % "|".join(_ALWAYS_SPECIALCASED_ENUM_VALUE_SUBSTRINGS)

        def replaceCallback(match):
            return _ALWAYS_SPECIALCASED_ENUM_VALUE_LOOKUP_TABLE[match.group(1).upper()]

        # Split on hyphen, introduce camelcase, and force uppercasing of acronyms.
        subwords = map(ucfirst, enum_value.split('-'))
        return re.sub(re.compile(regex, re.IGNORECASE), replaceCallback, "".join(subwords))

    @staticmethod
    def js_name_for_parameter_type(_type):
        _type = _type
        if isinstance(_type, AliasedType):
            _type = _type.aliased_type  # Fall through.
        if isinstance(_type, EnumType):
            _type = _type.primitive_type  # Fall through.

        if isinstance(_type, (ArrayType, ObjectType)):
            return 'object'
        if isinstance(_type, PrimitiveType):
            if _type.qualified_name() in ['object', 'any']:
                return 'object'
            elif _type.qualified_name() in ['integer', 'number']:
                return 'number'
            else:
                return _type.qualified_name()

    @staticmethod
    def string_for_file_include(filename, file_framework, target_framework):
        if file_framework is target_framework:
            return '"%s"' % filename
        else:
            return '<%s/%s>' % (file_framework.name, filename)
