#!/usr/bin/env python
#
# Copyright (c) 2016 Apple Inc. All rights reserved.
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
from string import Template

from builtins_generator import BuiltinsGenerator, WK_lcfirst, WK_ucfirst
from builtins_templates import BuiltinsGeneratorTemplates as Templates

log = logging.getLogger('global')


class BuiltinsInternalsWrapperImplementationGenerator(BuiltinsGenerator):
    def __init__(self, model):
        BuiltinsGenerator.__init__(self, model)
        self.internals = filter(lambda object: 'internal' in object.annotations, model.objects)

    def output_filename(self):
        return "%sJSBuiltinInternals.cpp" % self.model().framework.setting('namespace')

    def generate_output(self):
        args = {
            'namespace': self.model().framework.setting('namespace'),
        }

        sections = []
        sections.append(self.generate_license())
        sections.append(Template(Templates.DoNotEditWarning).substitute(args))
        sections.append(self.generate_primary_header_includes())
        sections.append(self.generate_secondary_header_includes())

        sections.append(Template(Templates.NamespaceTop).substitute(args))
        sections.append(self.generate_section_for_object())
        sections.append(Template(Templates.NamespaceBottom).substitute(args))

        return "\n\n".join(sections)

    def generate_secondary_header_includes(self):
        header_includes = [
            (["WebCore"],
                ("WebCore", "JSDOMGlobalObject.h"),
            ),
            (["WebCore"],
                ("WebCore", "WebCoreJSClientData.h"),
            ),
            (["WebCore"],
                ("JavaScriptCore", "heap/HeapInlines.h"),
            ),
            (["WebCore"],
                ("JavaScriptCore", "heap/SlotVisitorInlines.h"),
            ),
            (["WebCore"],
                ("JavaScriptCore", "runtime/JSCJSValueInlines.h"),
            ),
            (["WebCore"],
                ("JavaScriptCore", "runtime/StructureInlines.h"),
            ),
        ]
        return '\n'.join(self.generate_includes_from_entries(header_includes))

    def generate_section_for_object(self):
        lines = []

        lines.append(self.generate_constructor())
        lines.append(self.generate_visit_method())
        lines.append(self.generate_initialize_method())
        return '\n'.join(lines)

    def accessor_name(self, object):
        return WK_lcfirst(object.object_name)

    def member_name(self, object):
        return "m_" + self.accessor_name(object)

    def member_type(self, object):
        return WK_ucfirst(object.object_name) + "BuiltinFunctions"

    def generate_constructor(self):
        guards = set([object.annotations.get('conditional') for object in self.internals if 'conditional' in object.annotations])
        lines = ["JSBuiltinInternalFunctions::JSBuiltinInternalFunctions(JSC::VM& vm)",
                 "    : m_vm(vm)"]
        for object in self.internals:
            initializer = "    , %s(m_vm)" % self.member_name(object)
            lines.append(BuiltinsGenerator.wrap_with_guard(object.annotations.get('conditional'), initializer))
        lines.append("{")
        lines.append("    UNUSED_PARAM(vm);")
        lines.append("}\n")
        return '\n'.join(lines)

    def property_macro(self, object):
        lines = []
        lines.append("#define DECLARE_GLOBAL_STATIC(name) \\")
        lines.append("    JSDOMGlobalObject::GlobalPropertyInfo( \\")
        lines.append("        clientData.builtinFunctions().%sBuiltins().name##PrivateName(), %s().m_##name##Function.get() , JSC::DontDelete | JSC::ReadOnly)," % (self.accessor_name(object), self.accessor_name(object)))
        lines.append("    WEBCORE_FOREACH_%s_BUILTIN_FUNCTION_NAME(DECLARE_GLOBAL_STATIC)" % object.object_name.upper())
        lines.append("#undef DECLARE_GLOBAL_STATIC")
        return '\n'.join(lines)

    def generate_visit_method(self):
        lines = ["void JSBuiltinInternalFunctions::visit(JSC::SlotVisitor& visitor)",
                 "{"]
        for object in self.internals:
            visit = "    %s.visit(visitor);" % self.member_name(object)
            lines.append(BuiltinsGenerator.wrap_with_guard(object.annotations.get('conditional'), visit))
        lines.append("    UNUSED_PARAM(visitor);")
        lines.append("}\n")
        return '\n'.join(lines)

    def _generate_initialize_static_globals(self):
        lines = ["    JSVMClientData& clientData = *static_cast<JSVMClientData*>(m_vm.clientData);",
                 "    JSDOMGlobalObject::GlobalPropertyInfo staticGlobals[] = {"]
        for object in self.internals:
            lines.append(BuiltinsGenerator.wrap_with_guard(object.annotations.get('conditional'), self.property_macro(object)))
        lines.append("    };")
        lines.append("    globalObject.addStaticGlobals(staticGlobals, WTF_ARRAY_LENGTH(staticGlobals));")
        lines.append("    UNUSED_PARAM(clientData);")
        return '\n'.join(lines)

    def generate_initialize_method(self):
        lines = ["void JSBuiltinInternalFunctions::initialize(JSDOMGlobalObject& globalObject)",
                "{",
                "    UNUSED_PARAM(globalObject);"]

        for object in self.internals:
            init = "    %s.init(globalObject);" % self.member_name(object)
            lines.append(BuiltinsGenerator.wrap_with_guard(object.annotations.get('conditional'), init))
        lines.append("")

        guards = set([object.annotations.get('conditional') for object in self.internals if 'conditional' in object.annotations])
        lines.append(BuiltinsGenerator.wrap_with_guard(" || ".join(guards), self._generate_initialize_static_globals()))

        lines.append("}")
        return '\n'.join(lines)
