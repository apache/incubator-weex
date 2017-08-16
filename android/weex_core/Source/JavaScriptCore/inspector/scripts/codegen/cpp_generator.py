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
import os.path
import re

from generator import ucfirst, Generator
from models import PrimitiveType, ObjectType, ArrayType, EnumType, AliasedType, Frameworks

log = logging.getLogger('global')

_PRIMITIVE_TO_CPP_NAME_MAP = {
    'boolean': 'bool',
    'integer': 'int',
    'number': 'double',
    'string': 'String',
    'object': 'Inspector::InspectorObject',
    'array': 'Inspector::InspectorArray',
    'any': 'Inspector::InspectorValue'
}

class CppGenerator(Generator):
    def __init__(self, *args, **kwargs):
        Generator.__init__(self, *args, **kwargs)

    def protocol_name(self):
        return self.model().framework.setting('cpp_protocol_group', '')

    def helpers_namespace(self):
        return '%sHelpers' % self.protocol_name()

    # Miscellaneous text manipulation routines.
    @staticmethod
    def cpp_getter_method_for_type(_type):
        if isinstance(_type, ObjectType):
            return 'getObject'
        if isinstance(_type, ArrayType):
            return 'getArray'
        if isinstance(_type, PrimitiveType):
            if _type.raw_name() is 'integer':
                return 'getInteger'
            elif _type.raw_name() is 'number':
                return 'getDouble'
            elif _type.raw_name() is 'any':
                return 'getValue'
            else:
                return 'get' + ucfirst(_type.raw_name())
        if isinstance(_type, AliasedType):
            return CppGenerator.cpp_getter_method_for_type(_type.aliased_type)
        if isinstance(_type, EnumType):
            return CppGenerator.cpp_getter_method_for_type(_type.primitive_type)

    @staticmethod
    def cpp_setter_method_for_type(_type):
        if isinstance(_type, ObjectType):
            return 'setObject'
        if isinstance(_type, ArrayType):
            return 'setArray'
        if isinstance(_type, PrimitiveType):
            if _type.raw_name() is 'integer':
                return 'setInteger'
            elif _type.raw_name() is 'number':
                return 'setDouble'
            elif _type.raw_name() is 'any':
                return 'setValue'
            else:
                return 'set' + ucfirst(_type.raw_name())
        if isinstance(_type, AliasedType):
            return CppGenerator.cpp_setter_method_for_type(_type.aliased_type)
        if isinstance(_type, EnumType):
            return CppGenerator.cpp_setter_method_for_type(_type.primitive_type)

    # Generate type representations for various situations.
    @staticmethod
    def cpp_protocol_type_for_type(_type):
        if isinstance(_type, ObjectType) and len(_type.members) == 0:
            return 'Inspector::InspectorObject'
        if isinstance(_type, ArrayType):
            if _type.raw_name() is None:  # Otherwise, fall through and use typedef'd name.
                return 'Inspector::Protocol::Array<%s>' % CppGenerator.cpp_protocol_type_for_type(_type.element_type)
        if isinstance(_type, (ObjectType, AliasedType, EnumType, ArrayType)):
            return 'Inspector::Protocol::%s::%s' % (_type.type_domain().domain_name, _type.raw_name())
        if isinstance(_type, PrimitiveType):
            return CppGenerator.cpp_name_for_primitive_type(_type)

    @staticmethod
    def cpp_protocol_type_for_type_member(type_member, object_declaration):
        if isinstance(type_member.type, EnumType) and type_member.type.is_anonymous:
            return '::'.join([CppGenerator.cpp_protocol_type_for_type(object_declaration.type), ucfirst(type_member.member_name)])
        else:
            return CppGenerator.cpp_protocol_type_for_type(type_member.type)

    @staticmethod
    def cpp_type_for_unchecked_formal_in_parameter(parameter):
        _type = parameter.type
        if isinstance(_type, AliasedType):
            _type = _type.aliased_type  # Fall through to enum or primitive.

        if isinstance(_type, EnumType):
            _type = _type.primitive_type  # Fall through to primitive.

        # This handles the 'any' type and objects with defined properties.
        if isinstance(_type, ObjectType) or _type.qualified_name() is 'object':
            cpp_name = 'Inspector::InspectorObject'
            if parameter.is_optional:
                return 'const %s*' % cpp_name
            else:
                return 'const %s&' % cpp_name
        if isinstance(_type, ArrayType):
            cpp_name = 'Inspector::InspectorArray'
            if parameter.is_optional:
                return 'const %s*' % cpp_name
            else:
                return 'const %s&' % cpp_name
        if isinstance(_type, PrimitiveType):
            cpp_name = CppGenerator.cpp_name_for_primitive_type(_type)
            if parameter.is_optional:
                return 'const %s* const' % cpp_name
            elif _type.raw_name() in ['string']:
                return 'const %s&' % cpp_name
            else:
                return cpp_name

        return "unknown_unchecked_formal_in_parameter_type"

    @staticmethod
    def cpp_type_for_checked_formal_event_parameter(parameter):
        return CppGenerator.cpp_type_for_type_with_name(parameter.type, parameter.parameter_name, parameter.is_optional)

    @staticmethod
    def cpp_type_for_type_member(member):
        return CppGenerator.cpp_type_for_type_with_name(member.type, member.member_name, False)

    @staticmethod
    def cpp_type_for_type_with_name(_type, type_name, is_optional):
        if isinstance(_type, (ArrayType, ObjectType)):
            return 'RefPtr<%s>' % CppGenerator.cpp_protocol_type_for_type(_type)
        if isinstance(_type, AliasedType):
            builder_type = CppGenerator.cpp_protocol_type_for_type(_type)
            if is_optional:
                return 'const %s* const' % builder_type
            elif _type.aliased_type.qualified_name() in ['integer', 'number']:
                return CppGenerator.cpp_name_for_primitive_type(_type.aliased_type)
            elif _type.aliased_type.qualified_name() in ['string']:
                return 'const %s&' % builder_type
            else:
                return builder_type
        if isinstance(_type, PrimitiveType):
            cpp_name = CppGenerator.cpp_name_for_primitive_type(_type)
            if _type.qualified_name() in ['object']:
                return 'RefPtr<Inspector::InspectorObject>'
            elif _type.qualified_name() in ['any']:
                return 'RefPtr<Inspector::InspectorValue>'
            elif is_optional:
                return 'const %s* const' % cpp_name
            elif _type.qualified_name() in ['string']:
                return 'const %s&' % cpp_name
            else:
                return cpp_name
        if isinstance(_type, EnumType):
            if _type.is_anonymous:
                enum_type_name = ucfirst(type_name)
            else:
                enum_type_name = 'Inspector::Protocol::%s::%s' % (_type.type_domain().domain_name, _type.raw_name())

            if is_optional:
                return '%s*' % enum_type_name
            else:
                return '%s' % enum_type_name

    @staticmethod
    def cpp_type_for_formal_out_parameter(parameter):
        _type = parameter.type

        if isinstance(_type, AliasedType):
            _type = _type.aliased_type  # Fall through.

        if isinstance(_type, (ObjectType, ArrayType)):
            return 'RefPtr<%s>&' % CppGenerator.cpp_protocol_type_for_type(_type)
        if isinstance(_type, PrimitiveType):
            cpp_name = CppGenerator.cpp_name_for_primitive_type(_type)
            if parameter.is_optional:
                return "Inspector::Protocol::OptOutput<%s>*" % cpp_name
            else:
                return '%s*' % cpp_name
        if isinstance(_type, EnumType):
            if _type.is_anonymous:
                return '%sBackendDispatcherHandler::%s*' % (_type.type_domain().domain_name, ucfirst(parameter.parameter_name))
            else:
                return 'Inspector::Protocol::%s::%s*' % (_type.type_domain().domain_name, _type.raw_name())

        raise ValueError("unknown formal out parameter type.")

    # FIXME: this is only slightly different from out parameters; they could be unified.
    @staticmethod
    def cpp_type_for_formal_async_parameter(parameter):
        _type = parameter.type
        if isinstance(_type, AliasedType):
            _type = _type.aliased_type  # Fall through.

        if isinstance(_type, EnumType):
            _type = _type.primitive_type  # Fall through.

        if isinstance(_type, (ObjectType, ArrayType)):
            return 'RefPtr<%s>&&' % CppGenerator.cpp_protocol_type_for_type(_type)
        if isinstance(_type, PrimitiveType):
            cpp_name = CppGenerator.cpp_name_for_primitive_type(_type)
            if parameter.is_optional:
                return "Inspector::Protocol::OptOutput<%s>*" % cpp_name
            elif _type.qualified_name() in ['integer', 'number']:
                return CppGenerator.cpp_name_for_primitive_type(_type)
            elif _type.qualified_name() in ['string']:
                return 'const %s&' % cpp_name
            else:
                return cpp_name

        raise ValueError("Unknown formal async parameter type.")

    # In-parameters don't use builder types, because they could be passed
    # "open types" that are manually constructed out of InspectorObjects.

    # FIXME: Only parameters that are actually open types should need non-builder parameter types.
    @staticmethod
    def cpp_type_for_stack_in_parameter(parameter):
        _type = parameter.type
        if isinstance(_type, AliasedType):
            _type = _type.aliased_type  # Fall through.

        if isinstance(_type, EnumType):
            _type = _type.primitive_type  # Fall through.

        if isinstance(_type, ObjectType):
            return "RefPtr<Inspector::InspectorObject>"
        if isinstance(_type, ArrayType):
            return "RefPtr<Inspector::InspectorArray>"
        if isinstance(_type, PrimitiveType):
            cpp_name = CppGenerator.cpp_name_for_primitive_type(_type)
            if _type.qualified_name() in ['any', 'object']:
                return "RefPtr<%s>" % CppGenerator.cpp_name_for_primitive_type(_type)
            elif parameter.is_optional and _type.qualified_name() not in ['boolean', 'string', 'integer']:
                return "Inspector::Protocol::OptOutput<%s>" % cpp_name
            else:
                return cpp_name

    @staticmethod
    def cpp_type_for_stack_out_parameter(parameter):
        _type = parameter.type
        if isinstance(_type, (ArrayType, ObjectType)):
            return 'RefPtr<%s>' % CppGenerator.cpp_protocol_type_for_type(_type)
        if isinstance(_type, AliasedType):
            builder_type = CppGenerator.cpp_protocol_type_for_type(_type)
            if parameter.is_optional:
                return "Inspector::Protocol::OptOutput<%s>" % builder_type
            return '%s' % builder_type
        if isinstance(_type, PrimitiveType):
            cpp_name = CppGenerator.cpp_name_for_primitive_type(_type)
            if parameter.is_optional:
                return "Inspector::Protocol::OptOutput<%s>" % cpp_name
            else:
                return cpp_name
        if isinstance(_type, EnumType):
            if _type.is_anonymous:
                return '%sBackendDispatcherHandler::%s' % (_type.type_domain().domain_name, ucfirst(parameter.parameter_name))
            else:
                return 'Inspector::Protocol::%s::%s' % (_type.type_domain().domain_name, _type.raw_name())

    @staticmethod
    def cpp_assertion_method_for_type_member(type_member, object_declaration):

        def assertion_method_for_type(_type):
            return 'BindingTraits<%s>::assertValueHasExpectedType' % CppGenerator.cpp_protocol_type_for_type(_type)

        if isinstance(type_member.type, AliasedType):
            return assertion_method_for_type(type_member.type.aliased_type)
        if isinstance(type_member.type, EnumType) and type_member.type.is_anonymous:
            return 'BindingTraits<%s>::assertValueHasExpectedType' % CppGenerator.cpp_protocol_type_for_type_member(type_member, object_declaration)

        return assertion_method_for_type(type_member.type)

    @staticmethod
    def cpp_name_for_primitive_type(_type):
        return _PRIMITIVE_TO_CPP_NAME_MAP.get(_type.raw_name())

    # Decide whether certain helpers are necessary in a situation.
    @staticmethod
    def should_use_wrapper_for_return_type(_type):
        return not isinstance(_type, (ArrayType, ObjectType))

    @staticmethod
    def should_use_references_for_type(_type):
        return isinstance(_type, (ArrayType, ObjectType)) or (isinstance(_type, (PrimitiveType)) and _type.qualified_name() in ["any", "object"])

    @staticmethod
    def should_pass_by_copy_for_return_type(_type):
        return isinstance(_type, (ArrayType, ObjectType)) or (isinstance(_type, (PrimitiveType)) and _type.qualified_name() == "object")
