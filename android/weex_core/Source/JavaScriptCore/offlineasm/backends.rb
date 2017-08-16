# Copyright (C) 2011, 2016 Apple Inc. All rights reserved.
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

require "config"
require "arm"
require "arm64"
require "ast"
require "x86"
require "mips"
require "cloop"

BACKENDS =
    [
     "X86",
     "X86_WIN",
     "X86_64",
     "X86_64_WIN",
     "ARM",
     "ARMv7",
     "ARMv7_TRADITIONAL",
     "ARM64",
     "MIPS",
     "C_LOOP"
    ]

# Keep the set of working backends separate from the set of backends that might be
# supported. This is great because the BACKENDS list is almost like a reserved
# words list, in that it causes settings resolution to treat those words specially.
# Hence this lets us set aside the name of a backend we might want to support in
# the future while not actually supporting the backend yet.
WORKING_BACKENDS =
    [
     "X86",
     "X86_WIN",
     "X86_64",
     "X86_64_WIN",
     "ARM",
     "ARMv7",
     "ARMv7_TRADITIONAL",
     "ARM64",
     "MIPS",
     "C_LOOP"
    ]

BACKEND_PATTERN = Regexp.new('\\A(' + BACKENDS.join(')|(') + ')\\Z')

$allBackends = {}
$validBackends = {}
BACKENDS.each {
    | backend |
    $validBackends[backend] = true
    $allBackends[backend] = true
}

def includeOnlyBackends(list)
    newValidBackends = {}
    list.each {
        | backend |
        if $validBackends[backend]
            newValidBackends[backend] = true
        end
    }
    $validBackends = newValidBackends
end

def isBackend?(backend)
    $allBackends[backend]
end

def isValidBackend?(backend)
    $validBackends[backend]
end

def validBackends
    $validBackends.keys
end

class LoweringError < StandardError
    attr_reader :originString
    
    def initialize(e, originString)
        super "#{e} (due to #{originString})"
        @originString = originString
        set_backtrace e.backtrace
    end
end

class Node
    def lower(name)
        begin
            $activeBackend = name
            send("lower" + name)
        rescue => e
            raise LoweringError.new(e, codeOriginString)
        end
    end
end

# Overrides for lower() for those nodes that are backend-agnostic

class Label
    def lower(name)
        $asm.debugAnnotation codeOrigin.debugDirective if $enableDebugAnnotations
        $asm.putsLabel(self.name[1..-1], @global)
    end
end

class LocalLabel
    def lower(name)
        $asm.putsLocalLabel "_offlineasm_#{self.name[1..-1]}"
    end
end

class LabelReference
    def asmLabel
        if extern?
            Assembler.externLabelReference(name[1..-1])
        else
            Assembler.labelReference(name[1..-1])
        end
    end

    def cLabel
        Assembler.cLabelReference(name[1..-1])
    end
end

class LocalLabelReference
    def asmLabel
        Assembler.localLabelReference("_offlineasm_"+name[1..-1])
    end

    def cLabel
        Assembler.cLocalLabelReference("_offlineasm_"+name[1..-1])
    end
end

class Skip
    def lower(name)
    end
end

class Sequence
    def lower(name)
        $activeBackend = name
        if respond_to? "getModifiedList#{name}"
            newList = send("getModifiedList#{name}")
            newList.each {
                | node |
                node.lower(name)
            }
        elsif respond_to? "lower#{name}"
            send("lower#{name}")
        else
            @list.each {
                | node |
                node.lower(name)
            }
        end
    end
end

