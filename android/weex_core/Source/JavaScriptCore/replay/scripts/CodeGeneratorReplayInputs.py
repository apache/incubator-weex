#!/usr/bin/env python
# Copyright (c) 2014 Apple Inc. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import os.path
import re
import sys
import string
from string import Template
import optparse
import logging
from CodeGeneratorReplayInputsTemplates import Templates

try:
    import json
except ImportError:
    import simplejson as json

# Configuration values are first looked up in the framework configuration,
# and then in the global configuration if there is no framework-specific value.
GLOBAL_CONFIG = {
    "baseFilename": "ReplayInputs",
    "guardCondition": "ENABLE(WEB_REPLAY)",
    "traitsFrameworkName": "JavaScriptCore",

    # These are formatted as ([allowed_frameworks], (framework, header_path)).
    # The generator can figure out how to format the includes.
    "headerIncludes": [
        (["WebCore"],
            ("WebCore", "replay/EventLoopInput.h")
        ),
        (["JavaScriptCore", "WebCore"],
            ("JavaScriptCore", "replay/EncodedValue.h")
        ),
        (["JavaScriptCore"],
            ("JavaScriptCore", "replay/NondeterministicInput.h")
        ),
        (["JavaScriptCore", "WebCore"],
            ("WTF", "wtf/TypeCasts.h")
        ),
        (["WebCore"],
            ("WTF", "wtf/text/WTFString.h")
        ),

        # Testing fixtures.
        (["Test"],
            ("WebCore", "platform/ExternalNamespaceHeaderIncludeDummy.h")
        ),
        (["Test"],
            ("Test", "platform/InternalNamespaceHeaderIncludeDummy.h")
        )
    ],

    "implIncludes": [
        (["WebCore"],
            ("WebCore", "replay/SerializationMethods.h")
        ),
        (["WebCore", "JavaScriptCore"],
            ("JavaScriptCore", "inspector/InspectorValues.h")
        ),
        (["WebCore", "JavaScriptCore"],
            ("WTF", "wtf/NeverDestroyed.h")
        ),

        # Testing fixtures.
        (["Test"],
            ("WebCore", "platform/ExternalNamespaceImplIncludeDummy.h")
        ),
        (["Test"],
            ("Test", "platform/InternalNamespaceImplIncludeDummy.h")
        )
    ],
}

FRAMEWORK_CONFIG_MAP = {
    "Global": {
        "prefix": "",
        "namespace": ""
    },

    "WTF": {
        "prefix": "WTF",
        "namespace": "WTF",
    },
    "JavaScriptCore": {
        "prefix": "JS",
        "namespace": "JSC",
        "exportMacro": "JS_EXPORT_PRIVATE",
    },
    "WebCore": {
        "prefix": "Web",
        "namespace": "WebCore",
        "exportMacro": "WEBCORE_EXPORT"
    },
    # Used for bindings tests.
    "Test": {
        "prefix": "Test",
        "namespace": "Test",
        "exportMacro": "TEST_EXPORT_MACRO"
    }
}

# These settings are specific to an input queue.
QUEUE_CONFIG_MAP = {
    "SCRIPT_MEMOIZED": {
        "enumValue": "ScriptMemoizedData",
        "baseClass": "NondeterministicInput<%s>",
    },
    "LOADER_MEMOIZED": {
        "enumValue": "LoaderMemoizedData",
        "baseClass": "NondeterministicInput<%s>",
    },
    "EVENT_LOOP": {
        "enumValue": "EventLoopInput",
        "baseClass": "EventLoopInput<%s>",
    },
}

# Use a global logger, which normally only logs errors.
# It can be configured to log debug messages from the CLI.
logging.basicConfig(format='%(levelname)s: %(message)s', level=logging.ERROR)
log = logging.getLogger('global')


# Model classes, which transliterate JSON input.
class ParseException(Exception):
    pass


class TypecheckException(Exception):
    pass


class Framework:
    def __init__(self, name):
        self._settings = FRAMEWORK_CONFIG_MAP[name]
        self.name = name

    def setting(self, key, default=''):
        return self._settings.get(key, default)

    @staticmethod
    def fromString(frameworkString):
        if frameworkString == "Global":
            return Frameworks.Global

        if frameworkString == "WTF":
            return Frameworks.WTF

        if frameworkString == "JavaScriptCore":
            return Frameworks.JavaScriptCore

        if frameworkString == "WebCore":
            return Frameworks.WebCore

        if frameworkString == "Test":
            return Frameworks.Test

        raise ParseException("Unknown framework: " + frameworkString)


class Frameworks:
    Global = Framework("Global")
    WTF = Framework("WTF")
    JavaScriptCore = Framework("JavaScriptCore")
    WebCore = Framework("WebCore")
    Test = Framework("Test")


class InputQueue:
    def __init__(self, settings):
        self._settings = settings

    def setting(self, key, default=''):
        return self._settings.get(key, default)

    @staticmethod
    def fromString(queueString):
        if queueString == "SCRIPT_MEMOIZED":
            return InputQueues.SCRIPT_MEMOIZED

        if queueString == "LOADER_MEMOIZED":
            return InputQueues.LOADER_MEMOIZED

        if queueString == "EVENT_LOOP":
            return InputQueues.EVENT_LOOP

        raise ParseException("Unknown input queue: " + queueString)


class InputQueues:
    SCRIPT_MEMOIZED = InputQueue(QUEUE_CONFIG_MAP["SCRIPT_MEMOIZED"])
    LOADER_MEMOIZED = InputQueue(QUEUE_CONFIG_MAP["LOADER_MEMOIZED"])
    EVENT_LOOP = InputQueue(QUEUE_CONFIG_MAP["EVENT_LOOP"])


class Input:
    def __init__(self, name, description, framework, queueString, flags, guard=None):
        self.name = name
        self.description = description
        self.framework = framework
        self.queue = InputQueue.fromString(queueString)
        self._flags = flags
        self.guard = guard
        self.members = []  # names should be unique, but ordered.

    def setting(self, key, default=''):
        if key in self._flags:
            return True

        return self.queue.setting(key, default)


class InputMember:
    def __init__(self, memberName, typeName, flags=[]):
        self.memberName = memberName
        self.typeName = typeName
        self._flags = flags

    def has_flag(self, key, default=''):
        return key in self._flags


class TypeMode:
    def __init__(self, name):
        self._name = name

    @staticmethod
    def fromString(modeString):
        modeString = modeString.upper()
        if modeString == 'SCALAR':
            return TypeModes.SCALAR
        if modeString == 'HEAVY_SCALAR':
            return TypeModes.HEAVY_SCALAR
        if modeString == 'OWNED':
            return TypeModes.OWNED
        if modeString == 'SHARED':
            return TypeModes.SHARED
        if modeString == 'VECTOR':
            return TypeModes.VECTOR

        raise ParseException("Unknown type mode: " + modeString)


class TypeModes:
    # Copy for assignment and for getter
    SCALAR = TypeMode("SCALAR")
    # Copy for assignment, pass by reference for getter
    HEAVY_SCALAR = TypeMode("HEAVY_SCALAR")
    # Move for assignment, pass by reference for getter
    OWNED = TypeMode("OWNED")
    # Copy a RefPtr for assignment and getter
    SHARED = TypeMode("SHARED")
    # Move operator for assignment, pass by reference for getter
    VECTOR = TypeMode("VECTOR")


class Type:
    def __init__(self, name, mode, framework, header, enclosing_class, values, guard_values_map, underlying_storage, flags, guard=None):
        self._name = name
        self.mode = mode
        self.framework = framework
        self.header = header
        self.enclosing_class = enclosing_class
        self.values = values
        self.guard_values_map = guard_values_map
        self.underlying_storage = underlying_storage
        self._flags = flags
        self.guard = guard

    def __eq__(self, other):
        return self.type_name() == other.type_name() and self.mode == other.mode

    def __hash__(self):
        return self._name.__hash__()

    def has_flag(self, flagString):
        return flagString in self._flags

    def is_struct(self):
        return self.has_flag("STRUCT")

    def is_enum_declaration(self):
        return self.has_flag("ENUM")

    def is_enum_class_declaration(self):
        return self.has_flag("ENUM_CLASS")

    def is_option_set(self):
        return self.has_flag("OPTION_SET")

    def is_enumerable(self):
        return self.is_enum_declaration() or self.is_enum_class_declaration() or self.is_option_set()

    def declaration_kind(self):
        if self.is_enum_declaration():
            return "enum"
        elif self.is_enum_class_declaration():
            return "enum class"
        # If the enumerable is an OptionSet<T>, then T must be forward declared as an enum class.
        elif self.is_option_set():
            return "enum class"
        elif self.is_struct():
            return "struct"
        else:
            return "class"

    def qualified_prefix(self):
        components = []
        if self.framework != Frameworks.Global:
            components.append(self.framework.setting('namespace'))
        if self.enclosing_class is not None:
            components.append(self.enclosing_class)
        components.append("")
        return "::".join(components)

    def type_name(self, qualified=False):
        if qualified:
            return "%s%s" % (self.qualified_prefix(), self._name)
        elif self.enclosing_class is not None:
            return "%s::%s" % (self.enclosing_class, self._name)
        else:
            return self._name

    def storage_type(self, qualified=False):
        if self.mode == TypeModes.OWNED:
            return "std::unique_ptr<%s>" % self.type_name(qualified)
        elif self.mode == TypeModes.SHARED:
            return "RefPtr<%s>" % self.type_name(qualified)
        else:
            return self.type_name(qualified)

    def borrow_type(self, qualified=False):
        if self.mode == TypeModes.SCALAR:
            return self.type_name(qualified)
        elif self.mode == TypeModes.SHARED:
            return "RefPtr<%s>" % self.type_name(qualified)
        else:
            return "const %s&" % self.type_name(qualified)

    def argument_type(self, qualified=False):
        if self.mode == TypeModes.SHARED:
            return "RefPtr<%s>&&" % self.type_name(qualified)
        else:
            return self.storage_type()

    def encoding_type_argument(self, qualified=False):
        return self.type_name(qualified=qualified)


def check_for_required_properties(props, obj, what):
    for prop in props:
        if prop not in obj:
            raise ParseException("When parsing %s, required property missing: %s" % (what, prop))


class VectorType(Type):
    def __init__(self, element_type):
        self._element_type = element_type
        self.mode = TypeModes.VECTOR
        self.framework = element_type.framework
        self.enclosing_class = None

    def has_flag(self):
        return False

    def is_struct(self):
        return False

    def is_enum_declaration(self):
        return False

    def is_enum_class_declaration(self):
        return False

    def is_option_set(self):
        return False

    def qualified_prefix(self):
        return ""

    def type_name(self, qualified=False):
        return "Vector<%s>" % self._element_type.storage_type(qualified=qualified)

    def encoding_type_argument(self, qualified=False):
        return "Vector<%s>" % self._element_type.type_name(qualified=qualified)

    def argument_type(self, qualified=False):
        return self.type_name(qualified=qualified) + "&"


class InputsModel:
    def __init__(self):
        self.inputs = []
        self.types = []

        # Types have associated frameworks and are in their namespace, but within the specification
        # file types are in a flat namespace. Types with the same name are not allowed.
        self.types_by_name = {}
        self.inputs_by_name = {}

    def enumerable_types(self):
        _enumerables = filter(lambda x: x.is_enumerable(), self.types)
        return sorted(_enumerables, key=lambda _enumerable: _enumerable.type_name())

    def get_type_for_member(self, member):
        if member.has_flag("VECTOR"):
            return VectorType(self.types_by_name.get(member.typeName))
        else:
            return self.types_by_name.get(member.typeName)

    def parse_specification(self, json):
        if 'types' in json:
            if not isinstance(json['types'], dict):
                raise ParseException("Malformed specification: types is not a dict of framework->type list")

            for framework_name, type_list in json['types'].iteritems():
                if not isinstance(type_list, list):
                    raise ParseException("Malformed specification: type list for framework %s is not a list" % framework_name)

                framework = Framework.fromString(framework_name)
                for _type in type_list:
                    self.parse_type_with_framework(_type, framework)

        if 'inputs' in json:
            if not isinstance(json['inputs'], dict):
                raise ParseException("Malformed specification: inputs is not a dict of framework->input list")

            for framework_name, input_list in json['inputs'].iteritems():
                if not isinstance(input_list, list):
                    raise ParseException("Malformed specification: input list for framework %s is not a list" % framework_name)

                framework = Framework.fromString(framework_name)
                for _input in input_list:
                    self.parse_input_with_framework(_input, framework)

    def parse_type_with_framework(self, json, framework):
        check_for_required_properties(['name', 'mode'], json, 'type')
        if framework is not Frameworks.Global:
            check_for_required_properties(['header'], json, 'non-global type')

        type_name = json['name']
        type_mode = TypeMode.fromString(json['mode'])
        header = json.get('header')
        enclosing_class = json.get('enclosing_class')
        enumerable_values = json.get('values')
        guarded_enumerable_values = json.get('guarded_values', {})
        type_storage = json.get('storage')
        type_flags = json.get('flags', [])
        guard = json.get('guard', None)
        _type = Type(type_name, type_mode, framework, header, enclosing_class, enumerable_values, guarded_enumerable_values, type_storage, type_flags, guard)
        if _type.is_enumerable():
            check_for_required_properties(['values'], json, 'enum')
            if not isinstance(json['values'], list) or len(_type.values) == 0:
                raise ParseException("Malformed specification: enumerable type %s does not supply a list of values" % type_name)

            if _type.is_enum_declaration() and enclosing_class is None and type_storage is None:
                raise ParseException("Could not parse enumerable type %s: C-style enum declarations not enclosed by a class must specify their storage type so they can be forward declared." % type_name)

        self.types.append(_type)

    def parse_input_with_framework(self, json, framework):
        check_for_required_properties(['name', 'description', 'queue', 'members'], json, 'input')
        _input = Input(json['name'], json['description'], framework, json['queue'], json.get('flags', []), json.get('guard'))
        if isinstance(json['members'], list):
            for member in json['members']:
                check_for_required_properties(['name', 'type'], member, 'member')
                _input.members.append(InputMember(member['name'], member['type'], member.get('flags', [])))

        self.inputs.append(_input)

    # Types cannot (yet) reference other types, so we can check references in one pass.
    def resolve_types(self):
        for _type in self.types:
            self.typecheck_type(_type)

        for _input in self.inputs:
            self.typecheck_input(_input)

    def typecheck_type(self, _type):
        log.debug("typecheck type " + _type.type_name())

        if _type.type_name() in self.types_by_name:
            raise TypecheckException("Duplicate type with name: " + _type.type_name())

        self.types_by_name[_type.type_name()] = _type

    def typecheck_input(self, _input):
        log.debug("typecheck input " + _input.name)

        if _input.name in self.inputs_by_name:
            raise TypecheckException("Duplicate input with name: " + _input.name)

        seen_members = {}

        for member in _input.members:
            if member.memberName in seen_members:
                raise TypecheckException("Duplicate input member with name: " + member.memberName)

            self.typecheck_input_member(member, _input)
            seen_members[member.memberName] = member

        self.inputs_by_name[_input.name] = _input

    def typecheck_input_member(self, input_member, _input):
        log.debug("typecheck member '%s' of '%s'" % (input_member.memberName, _input.name))

        if not input_member.typeName in self.types_by_name:
            raise TypecheckException("Unknown type '%s' referenced by member '%s' of input '%s'" % (input_member.typeName, input_member.memberName, _input.name))


# A writer that only updates file if it actually changed.
class IncrementalFileWriter:
    def __init__(self, filepath, force_output):
        self._filepath = filepath
        self._output = ""
        self.force_output = force_output

    def write(self, text):
        self._output += text

    def close(self):
        text_changed = True
        self._output = self._output.rstrip() + "\n"

        try:
            read_file = open(self._filepath, "r")
            old_text = read_file.read()
            read_file.close()
            text_changed = old_text != self._output
        except:
            # Ignore, just overwrite by default
            pass

        if text_changed or self.force_output:
            out_file = open(self._filepath, "w")
            out_file.write(self._output)
            out_file.close()


def wrap_with_guard(contents, condition=None):
    if condition is None:
        return contents

    return "\n".join([
        "#if %s" % condition,
        contents,
        "#endif // %s" % condition
    ])


class Generator:
    def __init__(self, model, target_framework_name, input_filepath, output_prefix):
        self._model = model
        self.target_framework = Framework.fromString(target_framework_name)
        self.traits_framework = Framework.fromString(self.setting('traitsFrameworkName'))
        self._input_filepath = input_filepath
        self._output_prefix = output_prefix

    def setting(self, key, default=''):
        return self.target_framework.setting(key, GLOBAL_CONFIG.get(key, default))

    def should_generate_item(self, item):
        return item.framework is self.target_framework

    # This does not account for any filename mangling performed on behalf of the test harness.
    def output_filename(self, extension=None):
        components = []
        if len(self._output_prefix) > 0:
            components.extend([self._output_prefix, '-'])

        components.extend([self.setting('prefix'), self.setting('baseFilename')])

        if extension is not None:
            components.extend(['.', extension])

        return "".join(components)

    def write_output_files(self, _dir, force=False):
        header_file = IncrementalFileWriter(os.path.join(_dir, self.output_filename('h')), force)
        implementation_file = IncrementalFileWriter(os.path.join(_dir, self.output_filename('cpp')), force)

        header_file.write(self.generate_header())
        implementation_file.write(self.generate_implementation())

        header_file.close()
        implementation_file.close()

    def generate_header(self):
        enumerable_types_to_generate = filter(self.should_generate_item, self._model.enumerable_types())
        inputs_to_generate = filter(self.should_generate_item, self._model.inputs)

        template_arguments = {
            'licenseBlock': self.generate_license(),
            'filename': self.output_filename(),
            'guardCondition': self.setting('guardCondition'),
            'traitsNamespace': self.traits_framework.setting('namespace'),
            'inputsNamespace': self.target_framework.setting('namespace'),
            'includes': self.generate_includes(defaults=self.setting('headerIncludes')),
            'typeForwardDeclarations': self.generate_type_forward_declarations(),
            'inputForwardDeclarations': "\n".join([wrap_with_guard("class %s;", _input.guard) % _input.name for _input in inputs_to_generate]),
            'inputClassDeclarations': "\n\n".join([self.generate_class_declaration(_input) for _input in inputs_to_generate]),
            'inputTraitDeclarations': "\n\n".join([self.generate_input_trait_declaration(_input) for _input in inputs_to_generate]),
            'inputTypeTraitDeclarations': "\n\n".join([self.generate_input_type_trait_declaration(_input) for _input in inputs_to_generate]),
            'enumerableTypeTraitDeclarations': "\n\n".join([wrap_with_guard(self.generate_enumerable_type_trait_declaration(_type), _type.guard) for _type in enumerable_types_to_generate]),
            'forEachMacro': self.generate_for_each_macro(),
        }

        return Template(Templates.HeaderSkeleton).substitute(template_arguments)

    def generate_implementation(self):
        enumerable_types_to_generate = filter(self.should_generate_item, self._model.enumerable_types())
        inputs_to_generate = filter(self.should_generate_item, self._model.inputs)

        template_arguments = {
            'licenseBlock': self.generate_license(),
            'filename': self.output_filename(),
            'guardCondition': self.setting('guardCondition'),
            'traitsNamespace': self.traits_framework.setting('namespace'),
            'inputsNamespace': self.target_framework.setting('namespace'),
            'includes': self.generate_includes(defaults=self.setting('implIncludes'), includes_for_types=True),
            'inputClassImplementations': "\n\n".join([self.generate_class_implementation(_input) for _input in inputs_to_generate]),
            'inputTraitImplementations': "\n\n".join([self.generate_input_trait_implementation(_input) for _input in inputs_to_generate]),
            'enumerableTypeTraitImplementations': "\n\n".join([wrap_with_guard(self.generate_enumerable_type_trait_implementation(_type), _type.guard) for _type in enumerable_types_to_generate]),
        }

        return Template(Templates.ImplementationSkeleton).substitute(template_arguments)

    def generate_license(self):
        return Template(Templates.CopyrightBlock).substitute(None, inputFilename=os.path.basename(self._input_filepath))

    def generate_includes(self, defaults=[], includes_for_types=False):
        lines = set()

        for _type in self._model.types:
            # Types in the "global" framework are implicitly declared and available in all namespaces.
            if _type.framework is Frameworks.Global:
                continue
            # For RefCounted types, we reverse when to include the header so that the destructor can be
            # used in the header file.
            include_for_destructor = _type.mode is TypeModes.SHARED
            # Enum declarations within classes cannot be forward declared, so we include
            # headers with the relevant class declaration.
            include_for_enclosing_class = _type.enclosing_class is not None
            # Include headers for types like URL and String which are copied, not owned or shared.
            include_for_copyable_member = _type.mode is TypeModes.HEAVY_SCALAR
            if (not includes_for_types) ^ (include_for_destructor or include_for_enclosing_class or include_for_copyable_member):
                continue

            if self.target_framework != _type.framework:
                lines.add("#include <%s>" % _type.header)
            else:
                lines.add("#include \"%s\"" % os.path.basename(_type.header))

        for entry in defaults:
            (allowed_framework_names, data) = entry
            (framework_name, header_path) = data

            if self.target_framework.name not in allowed_framework_names:
                continue
            if self.target_framework.name != framework_name:
                lines.add("#include <%s>" % header_path)
            else:
                lines.add("#include \"%s\"" % os.path.basename(header_path))

        return "\n".join(sorted(list(lines)))

    def generate_type_forward_declarations(self):
        lines = []

        decls_by_framework = {}
        frameworks = [Framework.fromString(s) for s in FRAMEWORK_CONFIG_MAP.keys() if s != Frameworks.Global.name]
        for framework in frameworks:
            decls_by_framework[framework] = []

        for _type in self._model.types:
            if _type.framework not in frameworks:
                continue
            if _type.enclosing_class is not None:
                continue
            if _type.mode == TypeModes.HEAVY_SCALAR:
                continue
            if _type.mode == TypeModes.SCALAR and not _type.is_enumerable():
                continue
            if _type.is_enum_declaration():
                declaration = "enum %s : %s;" % (_type.type_name(), _type.underlying_storage)
            else:
                declaration = "%s %s;" % (_type.declaration_kind(), _type.type_name())
            decls_by_framework[_type.framework].append(declaration)

        # Declare all namespaces explicitly, even if it's the main namespace.
        for framework in frameworks:
            if len(decls_by_framework[framework]) == 0:
                continue

            decls_by_framework[framework].sort()
            lines.append("namespace %s {" % framework.setting('namespace'))
            lines.extend(decls_by_framework[framework])
            lines.append("}")
            lines.append("")

        return "\n".join(lines)

    def generate_class_declaration(self, _input):
        extra_declarations = []
        if _input.queue == InputQueues.EVENT_LOOP:
            extra_declarations.extend([
                "",
                "    // EventLoopInput API",
                "    virtual void dispatch(ReplayController&) final;",
            ])

            if _input.setting('CREATE_FROM_PAGE'):
                extra_declarations.extend([
                    "    static std::unique_ptr<%s> createFromPage(const Page&);" % _input.name
                ])

        member_getters = [self.generate_input_member_getter(_member) for _member in _input.members]

        member_declarations = [self.generate_input_member_declaration(_member) for _member in _input.members]
        if len(member_declarations) > 0:
            member_declarations.insert(0, "private:")

        template_arguments = {
            'inputConstructor': self.generate_input_constructor_declaration(_input),
            'inputDestructor': self.generate_input_destructor_declaration(_input),
            'inputName': _input.name,
            'inputQueue': _input.setting('enumValue'),
            'baseClass': _input.setting('baseClass') % _input.name,
            'extraDeclarations': "\n".join(extra_declarations),
            'memberGetters': "\n".join(member_getters),
            'memberDeclarations': "\n".join(member_declarations),
        }

        return wrap_with_guard(Template(Templates.InputClassDeclaration).substitute(template_arguments), _input.guard)

    def generate_input_constructor_declaration(self, _input):
        formals_list = self.generate_constructor_formals_list(_input)
        terms = []
        if self.setting('exportMacro'):
            terms.append(self.setting('exportMacro'))
        terms.append("%s(%s)" % (_input.name, formals_list))
        return "    %s;" % " ".join(terms)

    def generate_input_destructor_declaration(self, _input):
        return "    virtual ~%s();" % _input.name

    def generate_input_member_getter(self, _member):
        member_type = self._model.get_type_for_member(_member)
        return "    %s %s() const { return %s; }" % (member_type.borrow_type(), _member.memberName, self.generate_member_borrow_expression(_member))

    def generate_input_member_declaration(self, _member):
        member_type = self._model.get_type_for_member(_member)
        return "    %s m_%s;" % (member_type.storage_type(), _member.memberName)

    def generate_input_member_tuples(self, _input):
        return [(_member, self._model.get_type_for_member(_member)) for _member in _input.members]

    def qualified_input_name(self, _input, forceQualified=False):
        if forceQualified or self.target_framework != self.traits_framework:
            return "%s::%s" % (self.target_framework.setting('namespace'), _input.name)
        else:
            return _input.name

    def generate_input_trait_declaration(self, _input):
        decl_type = ['struct']
        if len(self.setting('exportMacro')) > 0:
            decl_type.append(self.setting('exportMacro'))

        template_arguments = {
            'structOrClass': " ".join(decl_type),
            'queueType': _input.queue.setting('enumValue'),
            'qualifiedInputName': self.qualified_input_name(_input),
        }

        return wrap_with_guard(Template(Templates.InputTraitsDeclaration).substitute(template_arguments), _input.guard)

    def generate_input_type_trait_declaration(self, _input):
        template_arguments = {
            'qualifiedInputName': self.qualified_input_name(_input, forceQualified=True),
        }

        return wrap_with_guard(Template(Templates.InputTypeTraitsDeclaration).substitute(template_arguments), _input.guard)

    def generate_enumerable_type_trait_declaration(self, _type):
        decl_type = ['struct']
        if len(self.setting('exportMacro')) > 0:
            decl_type.append(self.setting('exportMacro'))

        should_qualify_type = _type.framework != self.traits_framework
        decoded_type = _type.type_name(qualified=should_qualify_type)
        if _type.is_option_set():
            decoded_type = "OptionSet<%s>" % decoded_type

        template_arguments = {
            'encodingTypeArgument': _type.encoding_type_argument(qualified=should_qualify_type),
            'enumerableType': decoded_type,
            'structOrClass': " ".join(decl_type)
        }
        return Template(Templates.EnumerableTypeTraitDeclaration).substitute(template_arguments)

    def generate_for_each_macro(self):
        inputs_to_generate = filter(self.should_generate_item, self._model.inputs)

        macro_name = "%s_REPLAY_INPUT_NAMES_FOR_EACH" % self.setting('prefix').upper()
        lines = []
        lines.append("#define %s(macro) \\" % macro_name)
        lines.extend(["    macro(%s) \\" % _input.name for _input in inputs_to_generate])
        lines.append("    \\")
        lines.append("// end of %s" % macro_name)
        return "\n".join(lines)

    def generate_class_implementation(self, _input):
        template_arguments = {
            'inputName': _input.name,
            'inputsNamespace': self.target_framework.setting('namespace'),
            'initializerList': self.generate_constructor_initializer_list(_input),
            'constructorFormalsList': self.generate_constructor_formals_list(_input),
        }

        return wrap_with_guard(Template(Templates.InputClassImplementation).substitute(template_arguments), _input.guard)

    def generate_enumerable_type_trait_implementation(self, _type):
        should_qualify_type = _type.framework != self.traits_framework
        prefix_components = []
        if should_qualify_type:
            prefix_components.append(_type.framework.setting('namespace'))
        if _type.is_enum_declaration() and _type.enclosing_class is not None:
            prefix_components.append(_type.enclosing_class)
        elif _type.is_enum_class_declaration() or _type.is_option_set():
            prefix_components.append(_type.type_name(qualified=False))
        prefix_components.append("")
        enumerable_type_prefix = "::".join(prefix_components)
        encodeLines = []

        if _type.is_enum_declaration():
            encode_template = Templates.EnumTypeEncodeCase
            decode_template = Templates.EnumTypeDecodeCase
            enumerable_type_trait_template = Templates.EnumTypeTraitImplementation
        elif _type.is_enum_class_declaration():
            encode_template = Templates.EnumClassTypeEncodeCase
            decode_template = Templates.EnumClassTypeDecodeCase
            enumerable_type_trait_template = Templates.EnumClassTypeTraitImplementation
        else:  # Thus, _type.is_option_set().
            encode_template = Templates.OptionSetTypeEncodeCase
            decode_template = Templates.OptionSetTypeDecodeCase
            enumerable_type_trait_template = Templates.OptionSetTypeTraitImplementation

        # Generate body for encode.
        for _value in _type.values:
            template_arguments = {
                'enumStringValue': _value,
                'qualifiedEnumValue': "%s%s" % (enumerable_type_prefix, _value),
            }
            encodeLines.append(Template(encode_template).substitute(template_arguments))

        for guard, guard_values in _type.guard_values_map.iteritems():
            guardedLines = []
            for guard_value in guard_values:
                template_arguments = {
                    'enumStringValue': guard_value,
                    'qualifiedEnumValue': "%s%s" % (enumerable_type_prefix, guard_value),
                }
                guardedLines.append(Template(encode_template).substitute(template_arguments))
            encodeLines.append(wrap_with_guard("\n".join(guardedLines), guard))

        # Generate body for decode.
        decodeLines = []
        for i, _value in enumerate(_type.values):

            template_arguments = {
                'branchKeyword': "else if" if i > 0 else "if",
                'enumStringValue': _value,
                'qualifiedEnumValue': "%s%s" % (enumerable_type_prefix, _value),
                'qualifiedEnumName': _type.type_name(qualified=should_qualify_type)
            }
            decodeLines.append(Template(decode_template).substitute(template_arguments))

        for guard, guard_values in _type.guard_values_map.iteritems():
            guardedLines = []
            for i, guard_value in enumerate(guard_values):
                template_arguments = {
                    'branchKeyword': "else if" if i > 0 else "if",
                    'enumStringValue': guard_value,
                    'qualifiedEnumValue': "%s%s" % (enumerable_type_prefix, guard_value),
                    'qualifiedEnumName': _type.type_name(qualified=should_qualify_type)
                }
                guardedLines.append(Template(decode_template).substitute(template_arguments))
            decodeLines.append(wrap_with_guard("\n".join(guardedLines), guard))

        enumerable_type = _type.type_name(qualified=should_qualify_type)
        if _type.is_option_set():
            enumerable_type = "OptionSet<%s>" % enumerable_type

        template_arguments = {
            'encodingTypeArgument': _type.encoding_type_argument(qualified=should_qualify_type),
            'enumerableType': enumerable_type,
            'encodeCases': "\n".join(encodeLines),
            'decodeCases': "\n".join(decodeLines)
        }

        return Template(enumerable_type_trait_template).substitute(template_arguments)

    def generate_input_trait_implementation(self, _input):
        template_arguments = {
            'inputsNamespace': self.target_framework.setting('namespace'),
            'inputNameStringLiteral': '"%s"' % _input.name,
            'qualifiedInputName': self.qualified_input_name(_input),
            'constructorArguments': self.generate_constructor_arguments_list(_input),
            'constructorFormalsList': self.generate_constructor_formals_list(_input),
            'encodeSteps': self.generate_input_encode_implementation(_input),
            'decodeSteps': self.generate_input_decode_implementation(_input),
        }
        return wrap_with_guard(Template(Templates.InputTraitsImplementation).substitute(template_arguments), _input.guard)

    def generate_input_encode_implementation(self, _input):
        steps = []
        for (_member, _type) in self.generate_input_member_tuples(_input):
            should_qualify_type = _type.framework != self.traits_framework
            put_method = "put<%s>" % _type.encoding_type_argument(qualified=should_qualify_type)

            steps.extend([
                "    encodedValue.%s(ASCIILiteral(\"%s\"), input.%s());" % (put_method, _member.memberName, _member.memberName)
            ])

        if len(steps) == 0:
            steps.extend([
                "    UNUSED_PARAM(encodedValue);",
                "    UNUSED_PARAM(input);",
            ])

        return "\n".join(steps)

    def generate_input_decode_implementation(self, _input):
        steps = []
        for (_member, _type) in self.generate_input_member_tuples(_input):
            should_qualify_type = _type.framework != self.traits_framework
            get_method = "get<%s>" % _type.encoding_type_argument(qualified=should_qualify_type)

            lines = [
                "    %s %s;" % (_type.storage_type(qualified=should_qualify_type), _member.memberName),
                "    if (!encodedValue.%s(ASCIILiteral(\"%s\"), %s))" % (get_method, _member.memberName, _member.memberName),
                "        return false;",
                ""
            ]

            steps.append("\n".join(lines))

        if len(steps) == 0:
            steps.extend([
                "    UNUSED_PARAM(encodedValue);",
            ])

        return "\n".join(steps)

    def generate_constructor_initializer_list(self, _input):
        initializers = []
        initializers.append("    : %s()" % (_input.setting('baseClass') % _input.name))
        for _member in _input.members:
            initializers.append("    , m_%s(%s)" % (_member.memberName, self.generate_member_move_expression(_member)))

        return "\n".join(initializers)

    def generate_constructor_formals_list(self, _input):
        member_tuples = self.generate_input_member_tuples(_input)
        return ", ".join(["%s %s" % (_type.argument_type(), _member.memberName) for (_member, _type) in member_tuples])

    def generate_member_borrow_expression(self, _member):
        _type = self._model.get_type_for_member(_member)
        expression = "m_%s" % _member.memberName
        if _type.mode == TypeModes.OWNED:
            expression = "*" + expression

        return expression

    def generate_member_move_expression(self, _member):
        _type = self._model.get_type_for_member(_member)
        if _type.mode in [TypeModes.OWNED, TypeModes.SHARED]:
            return "WTFMove(%s)" % _member.memberName
        else:
            return _member.memberName

    def generate_constructor_arguments_list(self, _input):
        return ", ".join([self.generate_member_move_expression(_member) for _member in _input.members])


def generate_from_specifications(input_filepaths=[], output_prefix="", output_dirpath=None, framework_name=None, force_output=False):

    if not framework_name in FRAMEWORK_CONFIG_MAP:
        raise ParseException("Unknown or unsupported framework name supplied: " + framework_name)

    if len(input_filepaths) == 0:
        raise ParseException("Must provide at least one specification file, none were provided.")

    def parse_json_from_file(input_filepath):
        try:
            with open(input_filepath, "r") as input_file:
                return json.load(input_file)
        except ValueError as e:
            raise Exception("Error parsing valid JSON in file: " + input_filepath)

    specifications = map(parse_json_from_file, input_filepaths)

    model = InputsModel()
    for spec in specifications:
        model.parse_specification(spec)

    model.resolve_types()
    generator = Generator(model, framework_name, input_filepaths[0], output_prefix)

    generator.write_output_files(output_dirpath, force_output)


if __name__ == '__main__':
    allowed_framework_names = FRAMEWORK_CONFIG_MAP.keys()

    cli_parser = optparse.OptionParser(usage="usage: %prog [options] <Inputs.json> [, <MoreInputs.json> ]")
    cli_parser.add_option("-o", "--outputDir", help="Directory where generated files should be written.")
    cli_parser.add_option("--framework", type="choice", choices=allowed_framework_names, help="The framework these inputs belong to.")  # JavaScriptCore, WebCore
    cli_parser.add_option("--force", action="store_true", help="Force output of generated scripts, even if nothing changed.")
    cli_parser.add_option("-v", "--debug", action="store_true", help="Log extra output for debugging the generator itself.")
    cli_parser.add_option("-t", "--test", action="store_true", help="Enable test mode. Use unique output filenames created by prepending the input filename.")

    options = None

    arg_options, arg_values = cli_parser.parse_args()

    if not arg_options.outputDir:
        raise ParseException("Missing output directory")

    if arg_options.debug:
        log.setLevel(logging.DEBUG)

    options = {
        'input_filepaths': arg_values,
        'output_dirpath': arg_options.outputDir,
        'output_prefix': os.path.basename(arg_values[0]) if arg_options.test else "",
        'framework_name': arg_options.framework,
        'force_output': arg_options.force
    }

    try:
        generate_from_specifications(**options)
    except (ParseException, TypecheckException) as e:
        if arg_options.test:
            log.error(e.message)
        else:
            raise e  # Force the build to fail.
