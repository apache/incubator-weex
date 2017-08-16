#!/usr/bin/env ruby

# Copyright (C) 2015-2017 Apple Inc. All rights reserved.
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

require "pathname"

class Opcode
    attr_reader :name, :custom, :overloads
    attr_reader :attributes

    def initialize(name, custom)
        @name = name
        @custom = custom
        @attributes = {}
        unless custom
            @overloads = []
        end
    end

    def masmName
        name[0].downcase + name[1..-1]
    end
end

class Arg
    attr_reader :role, :bank, :width

    def initialize(role, bank, width)
        @role = role
        @bank = bank
        @width = width
    end
    
    def self.widthCode(width)
        if width == "Ptr"
            "pointerWidth()"
        else
            "Width#{width}"
        end
    end

    def widthCode
        Arg.widthCode(width)
    end
    
    def self.roleCode(role)
        case role
        when "U"
            "Use"
        when "D"
            "Def"
        when "ZD"
            "ZDef"
        when "UD"
            "UseDef"
        when "UZD"
            "UseZDef"
        when "UA"
            "UseAddr"
        when "S"
            "Scratch"
        when "ED"
            "EarlyDef"
        when "EZD"
            "EarlyZDef"
        when "LU"
            "LateUse"
        else
            raise
        end
    end
    
    def roleCode
        Arg.roleCode(role)
    end
end

class Overload
    attr_reader :signature, :forms

    def initialize(signature, forms)
        @signature = signature
        @forms = forms
    end
end

class Kind
    attr_reader :name
    attr_accessor :custom

    def initialize(name)
        @name = name
        @custom = false
    end

    def ==(other)
        if other.is_a? String
            @name == other
        else
            @name == other.name and @custom == other.custom
        end
    end

    def Kind.argKinds(kind)
        if kind == "Addr"
            ["Addr", "Stack", "CallArg"]
        else
            [kind]
        end
    end

    def argKinds
        Kind.argKinds(kind)
    end
end

class Form
    attr_reader :kinds, :altName, :archs

    def initialize(kinds, altName, archs)
        @kinds = kinds
        @altName = altName
        @archs = archs
    end
end

class Origin
    attr_reader :fileName, :lineNumber
    
    def initialize(fileName, lineNumber)
        @fileName = fileName
        @lineNumber = lineNumber
    end
    
    def to_s
        "#{fileName}:#{lineNumber}"
    end
end

class Token
    attr_reader :origin, :string
    
    def initialize(origin, string)
        @origin = origin
        @string = string
    end
    
    def ==(other)
        if other.is_a? Token
            @string == other.string
        else
            @string == other
        end
    end
    
    def =~(other)
        @string =~ other
    end
    
    def to_s
        "#{@string.inspect} at #{origin}"
    end
    
    def parseError(*comment)
        if comment.empty?
            raise "Parse error: #{to_s}"
        else
            raise "Parse error: #{to_s}: #{comment[0]}"
        end
    end
end

def lex(str, fileName)
    fileName = Pathname.new(fileName)
    result = []
    lineNumber = 1
    while not str.empty?
        case str
        when /\A\#([^\n]*)/
            # comment, ignore
        when /\A\n/
            # newline, ignore
            lineNumber += 1
        when /\A([a-zA-Z0-9_]+)/
            result << Token.new(Origin.new(fileName, lineNumber), $&)
        when /\A([ \t\r]+)/
            # whitespace, ignore
        when /\A[,:*\/]/
            result << Token.new(Origin.new(fileName, lineNumber), $&)
        else
            raise "Lexer error at #{Origin.new(fileName, lineNumber).to_s}, unexpected sequence #{str[0..20].inspect}"
        end
        str = $~.post_match
    end
    result
end

def isRole(token)
    token =~ /\A((U)|(D)|(UD)|(ZD)|(UZD)|(UA)|(S)|(ED)|(EZD)|(LU))\Z/
end

def isGF(token)
    token =~ /\A((G)|(F))\Z/
end

def isKind(token)
    token =~ /\A((Tmp)|(Imm)|(BigImm)|(BitImm)|(BitImm64)|(SimpleAddr)|(Addr)|(Index)|(RelCond)|(ResCond)|(DoubleCond)|(StatusCond))\Z/
end

def isArch(token)
    token =~ /\A((x86)|(x86_32)|(x86_64)|(arm)|(armv7)|(arm64)|(32)|(64))\Z/
end

def isWidth(token)
    token =~ /\A((8)|(16)|(32)|(64)|(Ptr))\Z/
end

def isKeyword(token)
    isRole(token) or isGF(token) or isKind(token) or isArch(token) or isWidth(token) or
        token == "custom" or token == "as"
end

def isIdentifier(token)
    token =~ /\A([a-zA-Z0-9_]+)\Z/ and not isKeyword(token)
end

class Parser
    def initialize(data, fileName)
        @tokens = lex(data, fileName)
        @idx = 0
    end

    def token
        @tokens[@idx]
    end

    def advance
        @idx += 1
    end

    def parseError(*comment)
        if token
            token.parseError(*comment)
        else
            if comment.empty?
                raise "Parse error at end of file"
            else
                raise "Parse error at end of file: #{comment[0]}"
            end
        end
    end

    def consume(string)
        parseError("Expected #{string}") unless token == string
        advance
    end

    def consumeIdentifier
        result = token.string
        parseError("Expected identifier") unless isIdentifier(result)
        advance
        result
    end

    def consumeRole
        result = token.string
        parseError("Expected role (U, D, UD, ZD, UZD, UA, or S)") unless isRole(result)
        advance
        result
    end

    def consumeBank
        result = token.string
        parseError("Expected bank (G or F)") unless isGF(result)
        advance
        result
    end

    def consumeKind
        result = token.string
        parseError("Expected kind (Imm, BigImm, BitImm, BitImm64, Tmp, SimpleAddr, Addr, Index, RelCond, ResCond, DoubleCond, or StatusCond)") unless isKind(result)
        advance
        result
    end

    def consumeWidth
        result = token.string
        parseError("Expected width (8, 16, 32, or 64)") unless isWidth(result)
        advance
        result
    end

    def parseArchs
        return nil unless isArch(token)

        result = []
        while isArch(token)
            case token.string
            when "x86"
                result << "X86"
                result << "X86_64"
            when "x86_32"
                result << "X86"
            when "x86_64"
                result << "X86_64"
            when "arm"
                result << "ARMv7"
                result << "ARM64"
            when "armv7"
                result << "ARMv7"
            when "arm64"
                result << "ARM64"
            when "32"
                result << "X86"
                result << "ARMv7"
            when "64"
                result << "X86_64"
                result << "ARM64"
            else
                raise token.string
            end
            advance
        end

        consume(":")
        @lastArchs = result
    end

    def consumeArchs
        result = @lastArchs
        @lastArchs = nil
        result
    end

    def parseAndConsumeArchs
        parseArchs
        consumeArchs
    end

    def intersectArchs(left, right)
        return left unless right
        return right unless left

        left.select {
            | value |
            right.find {
                | otherValue |
                value == otherValue
            }
        }
    end

    def parse
        result = {}
        
        loop {
            break if @idx >= @tokens.length

            if token == "custom"
                consume("custom")
                opcodeName = consumeIdentifier

                parseError("Cannot overload a custom opcode") if result[opcodeName]

                result[opcodeName] = Opcode.new(opcodeName, true)
            else
                opcodeArchs = parseAndConsumeArchs

                opcodeName = consumeIdentifier

                if result[opcodeName]
                    opcode = result[opcodeName]
                    parseError("Cannot overload a custom opcode") if opcode.custom
                else
                    opcode = Opcode.new(opcodeName, false)
                    result[opcodeName] = opcode
                end

                signature = []
                forms = []
                
                if isRole(token)
                    loop {
                        role = consumeRole
                        consume(":")
                        bank = consumeBank
                        consume(":")
                        width = consumeWidth
                        
                        signature << Arg.new(role, bank, width)
                        
                        break unless token == ","
                        consume(",")
                    }
                end

                while token == "/"
                    consume("/")
                    case token.string
                    when "branch"
                        opcode.attributes[:branch] = true
                        opcode.attributes[:terminal] = true
                    when "terminal"
                        opcode.attributes[:terminal] = true
                    when "effects"
                        opcode.attributes[:effects] = true
                    when "return"
                        opcode.attributes[:return] = true
                        opcode.attributes[:terminal] = true
                    else
                        parseError("Bad / directive")
                    end
                    advance
                end

                parseArchs
                if isKind(token)
                    loop {
                        kinds = []
                        altName = nil
                        formArchs = consumeArchs
                        loop {
                            kinds << Kind.new(consumeKind)

                            if token == "*"
                                parseError("Can only apply * to Tmp") unless kinds[-1].name == "Tmp"
                                kinds[-1].custom = true
                                consume("*")
                            end

                            break unless token == ","
                            consume(",")
                        }

                        if token == "as"
                            consume("as")
                            altName = consumeIdentifier
                        end

                        parseError("Form has wrong number of arguments for overload") unless kinds.length == signature.length
                        kinds.each_with_index {
                            | kind, index |
                            if kind.name == "Imm" or kind.name == "BigImm" or kind.name == "BitImm" or kind.name == "BitImm64"
                                if signature[index].role != "U"
                                    parseError("Form has an immediate for a non-use argument")
                                end
                                if signature[index].bank != "G"
                                    parseError("Form has an immediate for a non-general-purpose argument")
                                end
                            end
                        }
                        forms << Form.new(kinds, altName, intersectArchs(opcodeArchs, formArchs))

                        parseArchs
                        break unless isKind(token)
                    }
                end

                if signature.length == 0
                    raise unless forms.length == 0
                    forms << Form.new([], nil, opcodeArchs)
                end

                opcode.overloads << Overload.new(signature, forms)
            end
        }

        result
    end
end

$fileName = ARGV[0]

parser = Parser.new(IO::read($fileName), $fileName)
$opcodes = parser.parse

def writeH(filename)
    File.open("Air#{filename}.h", "w") {
        | outp |
        
        outp.puts "// Generated by opcode_generator.rb from #{$fileName} -- do not edit!"
        
        outp.puts "#ifndef Air#{filename}_h"
        outp.puts "#define Air#{filename}_h"

        yield outp
        
        outp.puts "#endif // Air#{filename}_h"
    }
end

writeH("Opcode") {
    | outp |
    outp.puts "namespace JSC { namespace B3 { namespace Air {"
    outp.puts "enum Opcode : int16_t {"
    $opcodes.keys.sort.each {
        | opcode |
        outp.puts "    #{opcode},"
    }
    outp.puts "};"

    outp.puts "static const unsigned numOpcodes = #{$opcodes.keys.size};"
    outp.puts "} } } // namespace JSC::B3::Air"
    
    outp.puts "namespace WTF {"
    outp.puts "class PrintStream;"
    outp.puts "JS_EXPORT_PRIVATE void printInternal(PrintStream&, JSC::B3::Air::Opcode);"
    outp.puts "} // namespace WTF"
}

# From here on, we don't try to emit properly indented code, since we're using a recursive pattern
# matcher.

def matchForms(outp, speed, forms, columnIndex, columnGetter, filter, callback)
    return if forms.length == 0

    if filter[forms]
        return
    end

    if columnIndex >= forms[0].kinds.length
        raise "Did not reduce to one form: #{forms.inspect}" unless forms.length == 1
        callback[forms[0]]
        outp.puts "break;"
        return
    end
    
    groups = {}
    forms.each {
        | form |
        kind = form.kinds[columnIndex].name
        if groups[kind]
            groups[kind] << form
        else
            groups[kind] = [form]
        end
    }

    if speed == :fast and groups.length == 1
        matchForms(outp, speed, forms, columnIndex + 1, columnGetter, filter, callback)
        return
    end

    outp.puts "switch (#{columnGetter[columnIndex]}) {"
    groups.each_pair {
        | key, value |
        outp.puts "#if USE(JSVALUE64)" if key == "BigImm" or key == "BitImm64"
        Kind.argKinds(key).each {
            | argKind |
            outp.puts "case Arg::#{argKind}:"
        }
        matchForms(outp, speed, value, columnIndex + 1, columnGetter, filter, callback)
        outp.puts "break;"
        outp.puts "#endif // USE(JSVALUE64)" if key == "BigImm" or key == "BitImm64"
    }
    outp.puts "default:"
    outp.puts "break;"
    outp.puts "}"
end

def matchInstOverload(outp, speed, inst)
    outp.puts "switch (#{inst}->kind.opcode) {"
    $opcodes.values.each {
        | opcode |
        outp.puts "case Opcode::#{opcode.name}:"
        if opcode.custom
            yield opcode, nil
        else
            needOverloadSwitch = ((opcode.overloads.size != 1) or speed == :safe)
            outp.puts "switch (#{inst}->args.size()) {" if needOverloadSwitch
            opcode.overloads.each {
                | overload |
                outp.puts "case #{overload.signature.length}:" if needOverloadSwitch
                yield opcode, overload
                outp.puts "break;" if needOverloadSwitch
            }
            if needOverloadSwitch
                outp.puts "default:"
                outp.puts "break;"
                outp.puts "}"
            end
        end
        outp.puts "break;"
    }
    outp.puts "default:"
    outp.puts "break;"
    outp.puts "}"
end
    
def matchInstOverloadForm(outp, speed, inst)
    matchInstOverload(outp, speed, inst) {
        | opcode, overload |
        if opcode.custom
            yield opcode, nil, nil
        else
            columnGetter = proc {
                | columnIndex |
                "#{inst}->args[#{columnIndex}].kind()"
            }
            filter = proc { false }
            callback = proc {
                | form |
                yield opcode, overload, form
            }
            matchForms(outp, speed, overload.forms, 0, columnGetter, filter, callback)
        end
    }
end

def beginArchs(outp, archs)
    return unless archs
    if archs.empty?
        outp.puts "#if 0"
        return
    end
    outp.puts("#if " + archs.map {
                  | arch |
                  "CPU(#{arch})"
              }.join(" || "))
end

def endArchs(outp, archs)
    return unless archs
    outp.puts "#endif"
end

writeH("OpcodeUtils") {
    | outp |
    outp.puts "#include \"AirCustom.h\""
    outp.puts "#include \"AirInst.h\""
    outp.puts "namespace JSC { namespace B3 { namespace Air {"
    
    outp.puts "inline bool opgenHiddenTruth() { return true; }"
    outp.puts "template<typename T>"
    outp.puts "inline T* opgenHiddenPtrIdentity(T* pointer) { return pointer; }"
    outp.puts "#define OPGEN_RETURN(value) do {\\"
    outp.puts "    if (opgenHiddenTruth())\\"
    outp.puts "        return value;\\"
    outp.puts "} while (false)"

    outp.puts "template<typename Functor>"
    outp.puts "void Inst::forEachArg(const Functor& functor)"
    outp.puts "{"
    matchInstOverload(outp, :fast, "this") {
        | opcode, overload |
        if opcode.custom
            outp.puts "#{opcode.name}Custom::forEachArg(*this, functor);"
        else
            overload.signature.each_with_index {
                | arg, index |
                outp.puts "functor(args[#{index}], Arg::#{arg.roleCode}, #{arg.bank}P, #{arg.widthCode});"
            }
        end
    }
    outp.puts "}"

    outp.puts "template<typename... Arguments>"
    outp.puts "ALWAYS_INLINE bool isValidForm(Opcode opcode, Arguments... arguments)"
    outp.puts "{"
    outp.puts "Arg::Kind kinds[sizeof...(Arguments)] = { arguments... };"
    outp.puts "switch (opcode) {"
    $opcodes.values.each {
        | opcode |
        outp.puts "case Opcode::#{opcode.name}:"
        if opcode.custom
            outp.puts "OPGEN_RETURN(#{opcode.name}Custom::isValidFormStatic(arguments...));"
        else
            outp.puts "switch (sizeof...(Arguments)) {"
            opcode.overloads.each {
                | overload |
                outp.puts "case #{overload.signature.length}:"
                columnGetter = proc { | columnIndex | "opgenHiddenPtrIdentity(kinds)[#{columnIndex}]" }
                filter = proc { false }
                callback = proc {
                    | form |
                    # This conservatively says that Stack is not a valid form for UseAddr,
                    # because it's only valid if it's not a spill slot. This is consistent with
                    # isValidForm() being conservative and it also happens to be practical since
                    # we don't really use isValidForm for deciding when Stack is safe.
                    overload.signature.length.times {
                        | index |
                        if overload.signature[index].role == "UA"
                            outp.puts "if (opgenHiddenPtrIdentity(kinds)[#{index}] == Arg::Stack)"
                            outp.puts "    return false;"
                        end
                    }
                    
                    notCustom = (not form.kinds.detect { | kind | kind.custom })
                    if notCustom
                        beginArchs(outp, form.archs)
                        outp.puts "OPGEN_RETURN(true);"
                        endArchs(outp, form.archs)
                    end
                }
                matchForms(outp, :safe, overload.forms, 0, columnGetter, filter, callback)
                outp.puts "break;"
            }
            outp.puts "default:"
            outp.puts "break;"
            outp.puts "}"
        end
        outp.puts "break;"
    }
    outp.puts "default:"
    outp.puts "break;"
    outp.puts "}"
    outp.puts "return false; "
    outp.puts "}"

    outp.puts "inline bool isDefinitelyTerminal(Opcode opcode)"
    outp.puts "{"
    outp.puts "switch (opcode) {"
    didFindTerminals = false
    $opcodes.values.each {
        | opcode |
        if opcode.attributes[:terminal]
            outp.puts "case Opcode::#{opcode.name}:"
            didFindTerminals = true
        end
    }
    if didFindTerminals
        outp.puts "return true;"
    end
    outp.puts "default:"
    outp.puts "return false;"
    outp.puts "}"
    outp.puts "}"

    outp.puts "inline bool isReturn(Opcode opcode)"
    outp.puts "{"
    outp.puts "switch (opcode) {"
    didFindReturns = false
    $opcodes.values.each {
        | opcode |
        if opcode.attributes[:return]
            outp.puts "case Opcode::#{opcode.name}:"
            didFindReturns = true
        end
    }
    if didFindReturns
        outp.puts "return true;"
    end
    outp.puts "default:"
    outp.puts "return false;"
    outp.puts "}"
    outp.puts "}"
    
    outp.puts "} } } // namespace JSC::B3::Air"
}

writeH("OpcodeGenerated") {
    | outp |
    outp.puts "#include \"AirInstInlines.h\""
    outp.puts "#include \"wtf/PrintStream.h\""
    outp.puts "namespace WTF {"
    outp.puts "using namespace JSC::B3::Air;"
    outp.puts "void printInternal(PrintStream& out, Opcode opcode)"
    outp.puts "{"
    outp.puts "    switch (opcode) {"
    $opcodes.keys.each {
        | opcode |
        outp.puts "    case Opcode::#{opcode}:"
        outp.puts "        out.print(\"#{opcode}\");"
        outp.puts "        return;"
    }
    outp.puts "    }"
    outp.puts "    RELEASE_ASSERT_NOT_REACHED();"
    outp.puts "}"
    outp.puts "} // namespace WTF"
    outp.puts "namespace JSC { namespace B3 { namespace Air {"
    outp.puts "bool Inst::isValidForm()"
    outp.puts "{"
    matchInstOverloadForm(outp, :safe, "this") {
        | opcode, overload, form |
        if opcode.custom
            outp.puts "OPGEN_RETURN(#{opcode.name}Custom::isValidForm(*this));"
        else
            beginArchs(outp, form.archs)
            needsMoreValidation = false
            overload.signature.length.times {
                | index |
                arg = overload.signature[index]
                kind = form.kinds[index]
                needsMoreValidation |= kind.custom

                # Some kinds of Args reqire additional validation.
                case kind.name
                when "Tmp"
                    outp.puts "if (!args[#{index}].tmp().is#{arg.bank}P())"
                    outp.puts "OPGEN_RETURN(false);"
                when "Imm"
                    outp.puts "if (!Arg::isValidImmForm(args[#{index}].value()))"
                    outp.puts "OPGEN_RETURN(false);"
                when "BitImm"
                    outp.puts "if (!Arg::isValidBitImmForm(args[#{index}].value()))"
                    outp.puts "OPGEN_RETURN(false);"
                when "BitImm64"
                    outp.puts "if (!Arg::isValidBitImm64Form(args[#{index}].value()))"
                    outp.puts "OPGEN_RETURN(false);"
                when "SimpleAddr"
                    outp.puts "if (!args[#{index}].tmp().isGP())"
                    outp.puts "OPGEN_RETURN(false);"
                when "Addr"
                    if arg.role == "UA"
                        outp.puts "if (args[#{index}].isStack() && args[#{index}].stackSlot()->isSpill())"
                        outp.puts "OPGEN_RETURN(false);"
                    end
                    
                    outp.puts "if (!Arg::isValidAddrForm(args[#{index}].offset()))"
                    outp.puts "OPGEN_RETURN(false);"
                when "Index"
                    outp.puts "if (!Arg::isValidIndexForm(args[#{index}].scale(), args[#{index}].offset(), #{arg.widthCode}))"
                    outp.puts "OPGEN_RETURN(false);"
                when "BigImm"
                when "RelCond"
                when "ResCond"
                when "DoubleCond"
                when "StatusCond"
                else
                    raise "Unexpected kind: #{kind.name}"
                end
            }
            if needsMoreValidation
                outp.puts "if (!is#{opcode.name}Valid(*this))"
                outp.puts "OPGEN_RETURN(false);"
            end
            outp.puts "OPGEN_RETURN(true);"
            endArchs(outp, form.archs)
        end
    }
    outp.puts "return false;"
    outp.puts "}"

    outp.puts "bool Inst::admitsStack(unsigned argIndex)"
    outp.puts "{"
    outp.puts "switch (kind.opcode) {"
    $opcodes.values.each {
        | opcode |
        outp.puts "case Opcode::#{opcode.name}:"

        if opcode.custom
            outp.puts "OPGEN_RETURN(#{opcode.name}Custom::admitsStack(*this, argIndex));"
        else
            # Switch on the argIndex.
            outp.puts "switch (argIndex) {"

            numArgs = opcode.overloads.map {
                | overload |
                overload.signature.length
            }.max
            
            numArgs.times {
                | argIndex |
                outp.puts "case #{argIndex}:"

                # Check if all of the forms of all of the overloads either do, or don't, admit an address
                # at this index. We expect this to be a very common case.
                numYes = 0
                numNo = 0
                opcode.overloads.each {
                    | overload |
                    useAddr = (overload.signature[argIndex] and
                               overload.signature[argIndex].role == "UA")
                    overload.forms.each {
                        | form |
                        if form.kinds[argIndex] == "Addr" and not useAddr
                            numYes += 1
                        else
                            numNo += 1
                        end
                    }
                }

                # Note that we deliberately test numYes first because if we end up with no forms, we want
                # to say that Address is inadmissible.
                if numYes == 0
                    outp.puts "OPGEN_RETURN(false);"
                elsif numNo == 0
                    outp.puts "OPGEN_RETURN(true);"
                else
                    # Now do the full test.

                    needOverloadSwitch = (opcode.overloads.size != 1)

                    outp.puts "switch (args.size()) {" if needOverloadSwitch
                    opcode.overloads.each {
                        | overload |

                        useAddr = (overload.signature[argIndex] and
                                   overload.signature[argIndex].role == "UA")
                        
                        # Again, check if all of them do what we want.
                        numYes = 0
                        numNo = 0
                        overload.forms.each {
                            | form |
                            if form.kinds[argIndex] == "Addr" and not useAddr
                                numYes += 1
                            else
                                numNo += 1
                            end
                        }

                        if numYes == 0
                            # Don't emit anything, just drop to default.
                        elsif numNo == 0
                            outp.puts "case #{overload.signature.length}:" if needOverloadSwitch
                            outp.puts "OPGEN_RETURN(true);"
                            outp.puts "break;" if needOverloadSwitch
                        else
                            outp.puts "case #{overload.signature.length}:" if needOverloadSwitch

                            # This is how we test the hypothesis that changing this argument to an
                            # address yields a valid form.
                            columnGetter = proc {
                                | columnIndex |
                                if columnIndex == argIndex
                                    "Arg::Addr"
                                else
                                    "args[#{columnIndex}].kind()"
                                end
                            }
                            filter = proc {
                                | forms |
                                numYes = 0

                                forms.each {
                                    | form |
                                    if form.kinds[argIndex] == "Addr"
                                        numYes += 1
                                    end
                                }

                                if numYes == 0
                                    # Drop down, emit no code, since we cannot match.
                                    true
                                else
                                    # Keep going.
                                    false
                                end
                            }
                            callback = proc {
                                | form |
                                beginArchs(outp, form.archs)
                                outp.puts "OPGEN_RETURN(true);"
                                endArchs(outp, form.archs)
                            }
                            matchForms(outp, :safe, overload.forms, 0, columnGetter, filter, callback)

                            outp.puts "break;" if needOverloadSwitch
                        end
                    }
                    if needOverloadSwitch
                        outp.puts "default:"
                        outp.puts "break;"
                        outp.puts "}"
                    end
                end
                
                outp.puts "break;"
            }
            
            outp.puts "default:"
            outp.puts "break;"
            outp.puts "}"
        end
        
        outp.puts "break;"
    }
    outp.puts "default:";
    outp.puts "break;"
    outp.puts "}"
    outp.puts "return false;"
    outp.puts "}"

    outp.puts "bool Inst::isTerminal()"
    outp.puts "{"
    outp.puts "switch (kind.opcode) {"
    foundTrue = false
    $opcodes.values.each {
        | opcode |
        if opcode.attributes[:terminal]
            outp.puts "case Opcode::#{opcode.name}:"
            foundTrue = true
        end
    }
    if foundTrue
        outp.puts "return true;"
    end
    $opcodes.values.each {
        | opcode |
        if opcode.custom
            outp.puts "case Opcode::#{opcode.name}:"
            outp.puts "return #{opcode.name}Custom::isTerminal(*this);"
        end
    }
    outp.puts "default:"
    outp.puts "return false;"
    outp.puts "}"
    outp.puts "}"
    
    outp.puts "bool Inst::hasNonArgNonControlEffects()"
    outp.puts "{"
    outp.puts "if (kind.traps)"
    outp.puts "return true;"
    outp.puts "switch (kind.opcode) {"
    foundTrue = false
    $opcodes.values.each {
        | opcode |
        if opcode.attributes[:effects]
            outp.puts "case Opcode::#{opcode.name}:"
            foundTrue = true
        end
    }
    if foundTrue
        outp.puts "return true;"
    end
    $opcodes.values.each {
        | opcode |
        if opcode.custom
            outp.puts "case Opcode::#{opcode.name}:"
            outp.puts "return #{opcode.name}Custom::hasNonArgNonControlEffects(*this);"
        end
    }
    outp.puts "default:"
    outp.puts "return false;"
    outp.puts "}"
    outp.puts "}"
    
    outp.puts "bool Inst::hasNonArgEffects()"
    outp.puts "{"
    outp.puts "if (kind.traps)"
    outp.puts "return true;"
    outp.puts "switch (kind.opcode) {"
    foundTrue = false
    $opcodes.values.each {
        | opcode |
        if opcode.attributes[:terminal] or opcode.attributes[:effects]
            outp.puts "case Opcode::#{opcode.name}:"
            foundTrue = true
        end
    }
    if foundTrue
        outp.puts "return true;"
    end
    $opcodes.values.each {
        | opcode |
        if opcode.custom
            outp.puts "case Opcode::#{opcode.name}:"
            outp.puts "return #{opcode.name}Custom::hasNonArgEffects(*this);"
        end
    }
    outp.puts "default:"
    outp.puts "return false;"
    outp.puts "}"
    outp.puts "}"
    
    outp.puts "CCallHelpers::Jump Inst::generate(CCallHelpers& jit, GenerationContext& context)"
    outp.puts "{"
    outp.puts "UNUSED_PARAM(jit);"
    outp.puts "UNUSED_PARAM(context);"
    outp.puts "CCallHelpers::Jump result;"
    matchInstOverloadForm(outp, :fast, "this") {
        | opcode, overload, form |
        if opcode.custom
            outp.puts "OPGEN_RETURN(#{opcode.name}Custom::generate(*this, jit, context));"
        else
            beginArchs(outp, form.archs)
            if form.altName
                methodName = form.altName
            else
                methodName = opcode.masmName
            end
            if opcode.attributes[:branch]
                outp.print "result = "
            end
            outp.print "jit.#{methodName}("

            form.kinds.each_with_index {
                | kind, index |
                if index != 0
                    outp.print ", "
                end
                case kind.name
                when "Tmp"
                    if overload.signature[index].bank == "G"
                        outp.print "args[#{index}].gpr()"
                    else
                        outp.print "args[#{index}].fpr()"
                    end
                when "Imm", "BitImm"
                    outp.print "args[#{index}].asTrustedImm32()"
                when "BigImm", "BitImm64"
                    outp.print "args[#{index}].asTrustedImm64()"
                when "SimpleAddr", "Addr"
                    outp.print "args[#{index}].asAddress()"
                when "Index"
                    outp.print "args[#{index}].asBaseIndex()"
                when "RelCond"
                    outp.print "args[#{index}].asRelationalCondition()"
                when "ResCond"
                    outp.print "args[#{index}].asResultCondition()"
                when "DoubleCond"
                    outp.print "args[#{index}].asDoubleCondition()"
                when "StatusCond"
                    outp.print "args[#{index}].asStatusCondition()"
                end
            }

            outp.puts ");"
            outp.puts "OPGEN_RETURN(result);"
            endArchs(outp, form.archs)
        end
    }
    outp.puts "RELEASE_ASSERT_NOT_REACHED();"
    outp.puts "return result;"
    outp.puts "}"

    outp.puts "} } } // namespace JSC::B3::Air"
}

# This is a hack for JSAir. It's a joke.
File.open("JSAir_opcode.js", "w") {
    | outp |
    outp.puts "\"use strict\";"
    outp.puts "// Generated by opcode_generator.rb from #{$fileName} -- do not edit!"
    
    $opcodes.values.each {
        | opcode |
        outp.puts "const #{opcode.name} = Symbol(#{opcode.name.inspect});"
    }
    
    outp.puts "function Inst_forEachArg(inst, func)"
    outp.puts "{"
    outp.puts "let replacement;"
    outp.puts "switch (inst.opcode) {"
    $opcodes.values.each {
        | opcode |
        outp.puts "case Opcode::#{opcode.name}:"
        if opcode.custom
            outp.puts "#{opcode.name}Custom.forEachArg(inst, func);"
        else
            needOverloadSwitch = opcode.overloads.size != 1
            outp.puts "switch (inst.args.length) {" if needOverloadSwitch
            opcode.overloads.each {
                | overload |
                outp.puts "case #{overload.signature.length}:" if needOverloadSwitch
                overload.signature.each_with_index {
                    | arg, index |
                    outp.puts "inst.visitArg(#{index}, func, Arg.#{arg.roleCode}, #{arg.bank}P, #{arg.width});"
                }
                outp.puts "break;"
            }
            if needOverloadSwitch
                outp.puts "default:"
                outp.puts "throw new Error(\"Bad overload\");"
                outp.puts "break;"
                outp.puts "}"
            end
        end
        outp.puts "break;"
    }
    outp.puts "default:"
    outp.puts "throw \"Bad opcode\";"
    outp.puts "}"
    outp.puts "}"
    
    outp.puts "function Inst_hasNonArgEffects(inst)"
    outp.puts "{"
    outp.puts "switch (inst.opcode) {"
    foundTrue = false
    $opcodes.values.each {
        | opcode |
        if opcode.attributes[:terminal] or opcode.attributes[:effects]
            outp.puts "case Opcode::#{opcode.name}:"
            foundTrue = true
        end
    }
    if foundTrue
        outp.puts "return true;"
    end
    $opcodes.values.each {
        | opcode |
        if opcode.custom
            outp.puts "case Opcode::#{opcode.name}:"
            outp.puts "return #{opcode.name}Custom.hasNonArgNonControlEffects(inst);"
        end
    }
    outp.puts "default:"
    outp.puts "return false;"
    outp.puts "}"
    outp.puts "}"
    
    outp.puts "function opcodeCode(opcode)"
    outp.puts "{"
    outp.puts "switch (opcode) {"
    $opcodes.keys.sort.each_with_index {
        | opcode, index |
        outp.puts "case Opcode::#{opcode}:"
        outp.puts "return #{index}"
    }
    outp.puts "default:"
    outp.puts "throw new Error(\"bad opcode\");"
    outp.puts "}"
    outp.puts "}"
}

