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
import collections

log = logging.getLogger('global')


def ucfirst(str):
    return str[:1].upper() + str[1:]


def find_duplicates(l):
    return [key for key, count in collections.Counter(l).items() if count > 1]


_FRAMEWORK_CONFIG_MAP = {
    "Global": {
    },
    "JavaScriptCore": {
        "cpp_protocol_group": "Inspector",
        "export_macro": "JS_EXPORT_PRIVATE",
        "alternate_dispatchers": True,
    },
    "WebKit": {
        "cpp_protocol_group": "Automation",
        "objc_protocol_group": "WD",
        "objc_prefix": "WD",
    },
    "WebInspector": {
        "objc_protocol_group": "RWI",
        "objc_prefix": "RWI",
    },
    # Used for code generator tests.
    "Test": {
        "alternate_dispatchers": True,
        "cpp_protocol_group": "Test",
        "objc_protocol_group": "Test",
        "objc_prefix": "Test",
    }
}


class ParseException(Exception):
    pass


class TypecheckException(Exception):
    pass


class Framework:
    def __init__(self, name):
        self._settings = _FRAMEWORK_CONFIG_MAP[name]
        self.name = name

    def setting(self, key, default=''):
        return self._settings.get(key, default)

    @staticmethod
    def fromString(frameworkString):
        if frameworkString == "Global":
            return Frameworks.Global

        if frameworkString == "JavaScriptCore":
            return Frameworks.JavaScriptCore

        if frameworkString == "WebKit":
            return Frameworks.WebKit

        if frameworkString == "WebInspector":
            return Frameworks.WebInspector

        if frameworkString == "Test":
            return Frameworks.Test

        raise ParseException("Unknown framework: %s" % frameworkString)


class Frameworks:
    Global = Framework("Global")
    JavaScriptCore = Framework("JavaScriptCore")
    WebKit = Framework("WebKit")
    WebInspector = Framework("WebInspector")
    Test = Framework("Test")


class Platform:
    def __init__(self, name):
        self.name = name

    @staticmethod
    def fromString(platformString):
        platformString = platformString.lower()
        if platformString == "ios":
            return Platforms.iOS

        if platformString == "macos":
            return Platforms.macOS

        if platformString == "all":
            return Platforms.All

        if platformString == "generic" or not platformString:
            return Platforms.Generic

        raise ParseException("Unknown platform: %s" % platformString)


class Platforms:
    All = Platform("all")
    Generic = Platform("generic")
    iOS = Platform("ios")
    macOS = Platform("macos")

    # Allow iteration over all platforms. See <http://stackoverflow.com/questions/5434400/>.
    class __metaclass__(type):
        def __iter__(self):
            for attr in dir(Platforms):
                if not attr.startswith("__"):
                    yield getattr(Platforms, attr)

class TypeReference:
    def __init__(self, type_kind, referenced_type_name, enum_values, array_items):
        self.type_kind = type_kind
        self.referenced_type_name = referenced_type_name
        self.enum_values = enum_values
        if array_items is None:
            self.array_type_ref = None
        else:
            self.array_type_ref = TypeReference(array_items.get('type'), array_items.get('$ref'), array_items.get('enum'), array_items.get('items'))

        if type_kind is not None and referenced_type_name is not None:
            raise ParseException("Type reference cannot have both 'type' and '$ref' keys.")

        all_primitive_types = ["integer", "number", "string", "boolean", "enum", "object", "array", "any"]
        if type_kind is not None and type_kind not in all_primitive_types:
            raise ParseException("Type reference '%s' is not a primitive type. Allowed values: %s" % (type_kind, ', '.join(all_primitive_types)))

        if type_kind == "array" and array_items is None:
            raise ParseException("Type reference with type 'array' must have key 'items' to define array element type.")

        if enum_values is not None and len(enum_values) == 0:
            raise ParseException("Type reference with enum values must have at least one enum value.")

    def referenced_name(self):
        if self.referenced_type_name is not None:
            return self.referenced_type_name
        else:
            return self.type_kind  # one of all_primitive_types


class Type:
    def __init__(self):
        pass

    def __eq__(self, other):
        return self.qualified_name() == other.qualified_name()

    def __hash__(self):
        return self.qualified_name().__hash__()

    def raw_name(self):
        return self._name

    # These methods should be overridden by subclasses.
    def is_enum(self):
        return False

    def type_domain(self):
        pass

    def qualified_name(self):
        pass

    # This is used to resolve nested types after instances are created.
    def resolve_type_references(self, protocol):
        pass


class PrimitiveType(Type):
    def __init__(self, name):
        self._name = name

    def __repr__(self):
        return 'PrimitiveType[%s]' % self.qualified_name()

    def type_domain(self):
        return None

    def qualified_name(self):
        return self.raw_name()


class AliasedType(Type):
    def __init__(self, declaration, domain, aliased_type_ref):
        self._name = declaration.type_name
        self._declaration = declaration
        self._domain = domain
        self._aliased_type_ref = aliased_type_ref
        self.aliased_type = None

    def __repr__(self):
        if self.aliased_type is not None:
            return 'AliasedType[%s -> %r]' % (self.qualified_name(), self.aliased_type)
        else:
            return 'AliasedType[%s -> (unresolved)]' % self.qualified_name()

    def is_enum(self):
        return self.aliased_type.is_enum()

    def type_domain(self):
        return self._domain

    def qualified_name(self):
        return  ".".join([self.type_domain().domain_name, self.raw_name()])

    def resolve_type_references(self, protocol):
        if self.aliased_type is not None:
            return

        self.aliased_type = protocol.lookup_type_reference(self._aliased_type_ref, self.type_domain())
        log.debug("< Resolved type reference for aliased type in %s: %s" % (self.qualified_name(), self.aliased_type.qualified_name()))


class EnumType(Type):
    def __init__(self, declaration, domain, values, primitive_type_ref, is_anonymous=False):
        self._name = "(anonymous)" if declaration is None else declaration.type_name
        self._declaration = declaration
        self._domain = domain
        self._values = values
        self._primitive_type_ref = primitive_type_ref
        self.primitive_type = None
        self.is_anonymous = is_anonymous

    def __repr__(self):
        return 'EnumType[primitive_type=%s; enum_values=%s]' % (self.qualified_name(), ', '.join(map(str, self.enum_values())))

    def is_enum(self):
        return True

    def enum_values(self):
        return self._values

    def type_domain(self):
        return self._domain

    def declaration(self):
        return self._declaration

    def qualified_name(self):
        return  ".".join([self.type_domain().domain_name, self.raw_name()])

    def resolve_type_references(self, protocol):
        if self.primitive_type is not None:
            return

        self.primitive_type = protocol.lookup_type_reference(self._primitive_type_ref, Domains.GLOBAL)
        log.debug("< Resolved type reference for enum type in %s: %s" % (self.qualified_name(), self.primitive_type.qualified_name()))
        log.debug("<< enum values: %s" % self.enum_values())


class ArrayType(Type):
    def __init__(self, declaration, element_type_ref, domain):
        self._name = None if declaration is None else declaration.type_name
        self._declaration = declaration
        self._domain = domain
        self._element_type_ref = element_type_ref
        self.element_type = None

    def __repr__(self):
        if self.element_type is not None:
            return 'ArrayType[element_type=%r]' % self.element_type
        else:
            return 'ArrayType[element_type=(unresolved)]'

    def declaration(self):
        return self._declaration

    def type_domain(self):
        return self._domain

    def qualified_name(self):
        return  ".".join(["array", self.element_type.qualified_name()])

    def resolve_type_references(self, protocol):
        if self.element_type is not None:
            return

        self.element_type = protocol.lookup_type_reference(self._element_type_ref, self.type_domain())
        log.debug("< Resolved type reference for element type in %s: %s" % (self.qualified_name(), self.element_type.qualified_name()))


class ObjectType(Type):
    def __init__(self, declaration, domain):
        self._name = declaration.type_name
        self._declaration = declaration
        self._domain = domain
        self.members = declaration.type_members

    def __repr__(self):
        return 'ObjectType[%s]' % self.qualified_name()

    def declaration(self):
        return self._declaration

    def type_domain(self):
        return self._domain

    def qualified_name(self):
        return  ".".join([self.type_domain().domain_name, self.raw_name()])


def check_for_required_properties(props, obj, what):
    for prop in props:
        if prop not in obj:
            raise ParseException("When parsing %s, required property missing: %s" % (what, prop))


class Protocol:
    def __init__(self, framework_name):
        self.domains = []
        self.types_by_name = {}
        self.framework = Framework.fromString(framework_name)

    def parse_specification(self, json, isSupplemental):
        log.debug("parse toplevel")

        if isinstance(json, dict) and 'domains' in json:
            json = json['domains']
        if not isinstance(json, list):
            json = [json]

        for domain in json:
            self.parse_domain(domain, isSupplemental)

    def parse_domain(self, json, isSupplemental):
        check_for_required_properties(['domain'], json, "domain")
        log.debug("parse domain " + json['domain'])

        types = []
        commands = []
        events = []

        if 'types' in json:
            if not isinstance(json['types'], list):
                raise ParseException("Malformed domain specification: types is not an array")
            types.extend([self.parse_type_declaration(declaration) for declaration in json['types']])

        if 'commands' in json:
            if not isinstance(json['commands'], list):
                raise ParseException("Malformed domain specification: commands is not an array")
            commands.extend([self.parse_command(command) for command in json['commands']])

        if 'events' in json:
            if not isinstance(json['events'], list):
                raise ParseException("Malformed domain specification: events is not an array")
            events.extend([self.parse_event(event) for event in json['events']])

        if 'availability' in json:
            if not commands and not events:
                raise ParseException("Malformed domain specification: availability should only be included if there are commands or events.")
            allowed_activation_strings = set(['web'])
            if json['availability'] not in allowed_activation_strings:
                raise ParseException('Malformed domain specification: availability is an unsupported string. Was: "%s", Allowed values: %s' % (json['availability'], ', '.join(allowed_activation_strings)))

        if 'workerSupported' in json:
            if not isinstance(json['workerSupported'], bool):
                raise ParseException('Malformed domain specification: workerSupported is not a boolean. Was: "%s"' % json['availability'])

        self.domains.append(Domain(json['domain'], json.get('description', ''), json.get('featureGuard'), json.get('availability'), json.get('workerSupported', False), isSupplemental, types, commands, events))

    def parse_type_declaration(self, json):
        check_for_required_properties(['id', 'type'], json, "type")
        log.debug("parse type %s" % json['id'])

        type_members = []

        if 'properties' in json:
            if not isinstance(json['properties'], list):
                raise ParseException("Malformed type specification: properties is not an array")
            type_members.extend([self.parse_type_member(member) for member in json['properties']])

        duplicate_names = find_duplicates([member.member_name for member in type_members])
        if len(duplicate_names) > 0:
            raise ParseException("Malformed domain specification: type declaration for %s has duplicate member names" % json['id'])

        type_ref = TypeReference(json['type'], json.get('$ref'), json.get('enum'), json.get('items'))
        platform = Platform.fromString(json.get('platform', 'generic'))
        return TypeDeclaration(json['id'], type_ref, json.get("description", ""), platform, type_members)

    def parse_type_member(self, json):
        check_for_required_properties(['name'], json, "type member")
        log.debug("parse type member %s" % json['name'])

        type_ref = TypeReference(json.get('type'), json.get('$ref'), json.get('enum'), json.get('items'))
        return TypeMember(json['name'], type_ref, json.get('optional', False), json.get('description', ""))

    def parse_command(self, json):
        check_for_required_properties(['name'], json, "command")
        log.debug("parse command %s" % json['name'])

        call_parameters = []
        return_parameters = []

        if 'parameters' in json:
            if not isinstance(json['parameters'], list):
                raise ParseException("Malformed command specification: parameters is not an array")
            call_parameters.extend([self.parse_call_or_return_parameter(parameter) for parameter in json['parameters']])

            duplicate_names = find_duplicates([param.parameter_name for param in call_parameters])
            if len(duplicate_names) > 0:
                raise ParseException("Malformed domain specification: call parameter list for command %s has duplicate parameter names" % json['name'])

        if 'returns' in json:
            if not isinstance(json['returns'], list):
                raise ParseException("Malformed command specification: returns is not an array")
            return_parameters.extend([self.parse_call_or_return_parameter(parameter) for parameter in json['returns']])

            duplicate_names = find_duplicates([param.parameter_name for param in return_parameters])
            if len(duplicate_names) > 0:
                raise ParseException("Malformed domain specification: return parameter list for command %s has duplicate parameter names" % json['name'])

        platform = Platform.fromString(json.get('platform', 'generic'))
        return Command(json['name'], call_parameters, return_parameters, json.get('description', ""), platform, json.get('async', False))

    def parse_event(self, json):
        check_for_required_properties(['name'], json, "event")
        log.debug("parse event %s" % json['name'])

        event_parameters = []

        if 'parameters' in json:
            if not isinstance(json['parameters'], list):
                raise ParseException("Malformed event specification: parameters is not an array")
            event_parameters.extend([self.parse_call_or_return_parameter(parameter) for parameter in json['parameters']])

            duplicate_names = find_duplicates([param.parameter_name for param in event_parameters])
            if len(duplicate_names) > 0:
                raise ParseException("Malformed domain specification: parameter list for event %s has duplicate parameter names" % json['name'])

        platform = Platform.fromString(json.get('platform', 'generic'))
        return Event(json['name'], event_parameters, json.get('description', ""), platform)

    def parse_call_or_return_parameter(self, json):
        check_for_required_properties(['name'], json, "parameter")
        log.debug("parse parameter %s" % json['name'])

        type_ref = TypeReference(json.get('type'), json.get('$ref'), json.get('enum'), json.get('items'))
        return Parameter(json['name'], type_ref, json.get('optional', False), json.get('description', ""))

    def resolve_types(self):
        qualified_declared_type_names = set(['boolean', 'string', 'integer', 'number', 'enum', 'array', 'object', 'any'])

        self.types_by_name['string'] = PrimitiveType('string')
        for _primitive_type in ['boolean', 'integer', 'number']:
            self.types_by_name[_primitive_type] = PrimitiveType(_primitive_type)
        for _object_type in ['any', 'object']:
            self.types_by_name[_object_type] = PrimitiveType(_object_type)

        # Gather qualified type names from type declarations in each domain.
        for domain in self.domains:
            for declaration in domain.all_type_declarations():
                # Basic sanity checking.
                if declaration.type_ref.referenced_type_name is not None:
                    raise TypecheckException("Type declarations must name a base type, not a type reference.")

                # Find duplicate qualified type names.
                qualified_type_name = ".".join([domain.domain_name, declaration.type_name])
                if qualified_type_name in qualified_declared_type_names:
                    raise TypecheckException("Duplicate type declaration: %s" % qualified_type_name)

                qualified_declared_type_names.add(qualified_type_name)

                type_instance = None

                kind = declaration.type_ref.type_kind
                if declaration.type_ref.enum_values is not None:
                    primitive_type_ref = TypeReference(declaration.type_ref.type_kind, None, None, None)
                    type_instance = EnumType(declaration, domain, declaration.type_ref.enum_values, primitive_type_ref)
                elif kind == "array":
                    type_instance = ArrayType(declaration, declaration.type_ref.array_type_ref, domain)
                elif kind == "object":
                    type_instance = ObjectType(declaration, domain)
                else:
                    type_instance = AliasedType(declaration, domain, declaration.type_ref)

                log.debug("< Created fresh type %r for declaration %s" % (type_instance, qualified_type_name))
                self.types_by_name[qualified_type_name] = type_instance

        # Resolve all type references recursively.
        for domain in self.domains:
            domain.resolve_type_references(self)

    def lookup_type_for_declaration(self, declaration, domain):
        # This will only match a type defined in the same domain, where prefixes aren't required.
        qualified_name = ".".join([domain.domain_name, declaration.type_name])
        if qualified_name in self.types_by_name:
            found_type = self.types_by_name[qualified_name]
            found_type.resolve_type_references(self)
            return found_type

        raise TypecheckException("Lookup failed for type declaration: %s (referenced from domain: %s)" % (declaration.type_name, domain.domain_name))

    def lookup_type_reference(self, type_ref, domain):
        # If reference is to an anonymous array type, create a fresh instance.
        if type_ref.type_kind == "array":
            type_instance = ArrayType(None, type_ref.array_type_ref, domain)
            type_instance.resolve_type_references(self)
            log.debug("< Created fresh type instance for anonymous array type: %s" % type_instance.qualified_name())
            return type_instance

        # If reference is to an anonymous enum type, create a fresh instance.
        if type_ref.enum_values is not None:
            # We need to create a type reference without enum values as the enum's nested type.
            primitive_type_ref = TypeReference(type_ref.type_kind, None, None, None)
            type_instance = EnumType(None, domain, type_ref.enum_values, primitive_type_ref, True)
            type_instance.resolve_type_references(self)
            log.debug("< Created fresh type instance for anonymous enum type: %s" % type_instance.qualified_name())
            return type_instance

        # This will match when referencing a type defined in the same domain, where prefixes aren't required.
        qualified_name = ".".join([domain.domain_name, type_ref.referenced_name()])
        if qualified_name in self.types_by_name:
            found_type = self.types_by_name[qualified_name]
            found_type.resolve_type_references(self)
            log.debug("< Lookup succeeded for unqualified type: %s" % found_type.qualified_name())
            return found_type

        # This will match primitive types and fully-qualified types from a different domain.
        if type_ref.referenced_name() in self.types_by_name:
            found_type = self.types_by_name[type_ref.referenced_name()]
            found_type.resolve_type_references(self)
            log.debug("< Lookup succeeded for primitive or qualified type: %s" % found_type.qualified_name())
            return found_type

        raise TypecheckException("Lookup failed for type reference: %s (referenced from domain: %s)" % (type_ref.referenced_name(), domain.domain_name))


class Domain:
    def __init__(self, domain_name, description, feature_guard, availability, workerSupported, isSupplemental, type_declarations, commands, events):
        self.domain_name = domain_name
        self.description = description
        self.feature_guard = feature_guard
        self.availability = availability
        self.workerSupported = workerSupported
        self.is_supplemental = isSupplemental
        self._type_declarations = type_declarations
        self._commands = commands
        self._events = events

    def all_type_declarations(self):
        return self._type_declarations

    def all_commands(self):
        return self._commands

    def all_events(self):
        return self._events

    def resolve_type_references(self, protocol):
        log.debug("> Resolving type declarations for domain: %s" % self.domain_name)
        for declaration in self._type_declarations:
            declaration.resolve_type_references(protocol, self)

        log.debug("> Resolving types in commands for domain: %s" % self.domain_name)
        for command in self._commands:
            command.resolve_type_references(protocol, self)

        log.debug("> Resolving types in events for domain: %s" % self.domain_name)
        for event in self._events:
            event.resolve_type_references(protocol, self)


class Domains:
    GLOBAL = Domain("", "The global domain, in which primitive types are implicitly declared.", None, None, True, False, [], [], [])


class TypeDeclaration:
    def __init__(self, type_name, type_ref, description, platform, type_members):
        self.type_name = type_name
        self.type_ref = type_ref
        self.description = description
        self.platform = platform
        self.type_members = type_members

        if self.type_name != ucfirst(self.type_name):
            raise ParseException("Types must begin with an uppercase character.")

    def resolve_type_references(self, protocol, domain):
        log.debug(">> Resolving type references for type declaration: %s" % self.type_name)
        self.type = protocol.lookup_type_for_declaration(self, domain)
        for member in self.type_members:
            member.resolve_type_references(protocol, domain)


class TypeMember:
    def __init__(self, member_name, type_ref, is_optional, description):
        self.member_name = member_name
        self.type_ref = type_ref
        self.is_optional = is_optional
        self.description = description

        if not isinstance(self.is_optional, bool):
            raise ParseException("The 'optional' flag for a type member must be a boolean literal.")

    def resolve_type_references(self, protocol, domain):
        log.debug(">>> Resolving type references for type member: %s" % self.member_name)
        self.type = protocol.lookup_type_reference(self.type_ref, domain)


class Parameter:
    def __init__(self, parameter_name, type_ref, is_optional, description):
        self.parameter_name = parameter_name
        self.type_ref = type_ref
        self.is_optional = is_optional
        self.description = description

        if not isinstance(self.is_optional, bool):
            raise ParseException("The 'optional' flag for a parameter must be a boolean literal.")

    def resolve_type_references(self, protocol, domain):
        log.debug(">>> Resolving type references for parameter: %s" % self.parameter_name)
        self.type = protocol.lookup_type_reference(self.type_ref, domain)


class Command:
    def __init__(self, command_name, call_parameters, return_parameters, description, platform, is_async):
        self.command_name = command_name
        self.call_parameters = call_parameters
        self.return_parameters = return_parameters
        self.description = description
        self.platform = platform
        self.is_async = is_async

    def resolve_type_references(self, protocol, domain):
        log.debug(">> Resolving type references for call parameters in command: %s" % self.command_name)
        for parameter in self.call_parameters:
            parameter.resolve_type_references(protocol, domain)

        log.debug(">> Resolving type references for return parameters in command: %s" % self.command_name)
        for parameter in self.return_parameters:
            parameter.resolve_type_references(protocol, domain)


class Event:
    def __init__(self, event_name, event_parameters, description, platform):
        self.event_name = event_name
        self.event_parameters = event_parameters
        self.description = description
        self.platform = platform

    def resolve_type_references(self, protocol, domain):
        log.debug(">> Resolving type references for parameters in event: %s" % self.event_name)
        for parameter in self.event_parameters:
            parameter.resolve_type_references(protocol, domain)
