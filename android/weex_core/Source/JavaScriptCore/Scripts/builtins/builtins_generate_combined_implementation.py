#!/usr/bin/env python
#
# Copyright (c) 2014, 2015 Apple Inc. All rights reserved.
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
import re
import string
from string import Template

from builtins_generator import BuiltinsGenerator
from builtins_model import Framework, Frameworks
from builtins_templates import BuiltinsGeneratorTemplates as Templates

log = logging.getLogger('global')


class BuiltinsCombinedImplementationGenerator(BuiltinsGenerator):
    def __init__(self, model):
        BuiltinsGenerator.__init__(self, model)

    def output_filename(self):
        return "%sBuiltins.cpp" % self.model().framework.setting('namespace')

    def generate_output(self):
        args = {
            'namespace': self.model().framework.setting('namespace'),
            'macroPrefix': self.model().framework.setting('macro_prefix'),
        }

        sections = []
        sections.append(self.generate_license())
        sections.append(Template(Templates.DoNotEditWarning).substitute(args))
        sections.append(self.generate_primary_header_includes())
        sections.append(self.generate_secondary_header_includes())
        sections.append(Template(Templates.NamespaceTop).substitute(args))
        for function in self.model().all_functions():
            sections.append(self.generate_embedded_code_string_section_for_function(function))
        if self.model().framework is Frameworks.JavaScriptCore:
            sections.append(Template(Templates.CombinedJSCImplementationStaticMacros).substitute(args))
        elif self.model().framework is Frameworks.WebCore:
            sections.append(Template(Templates.CombinedWebCoreImplementationStaticMacros).substitute(args))
        sections.append(Template(Templates.NamespaceBottom).substitute(args))

        return "\n\n".join(sections)

    def generate_secondary_header_includes(self):
        header_includes = [
            (["JavaScriptCore"],
                ("JavaScriptCore", "builtins/BuiltinExecutables.h"),
            ),
            (["JavaScriptCore", "WebCore"],
                ("JavaScriptCore", "heap/HeapInlines.h"),
            ),
            (["JavaScriptCore", "WebCore"],
                ("JavaScriptCore", "runtime/UnlinkedFunctionExecutable.h"),
            ),
            (["JavaScriptCore", "WebCore"],
                ("JavaScriptCore", "runtime/JSCellInlines.h"),
            ),
            (["WebCore"],
                ("JavaScriptCore", "runtime/StructureInlines.h"),
            ),
            (["WebCore"],
                ("JavaScriptCore", "runtime/JSCJSValueInlines.h"),
            ),
            (["JavaScriptCore", "WebCore"],
                ("JavaScriptCore", "runtime/VM.h"),
            ),
            (["JavaScriptCore", "WebCore"],
                ("JavaScriptCore", "runtime/Intrinsic.h"),
            ),
        ]

        return '\n'.join(self.generate_includes_from_entries(header_includes))
