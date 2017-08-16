# Copyright (C) 2012, 2014-2016 Apple Inc. All rights reserved.
# Copyright (C) 2013 Digia Plc. and/or its subsidiary(-ies)
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

# GPR conventions, to match the baseline JIT:
#
#
# On x86-32 bits (windows and non-windows)
# a0, a1, a2, a3 are only there for ease-of-use of offlineasm; they are not
# actually considered as such by the ABI and we need to push/pop our arguments
# on the stack. a0 and a1 are ecx and edx to follow fastcall.
#
# eax => t0, a2, r0
# edx => t1, a1, r1
# ecx => t2, a0
# ebx => t3, a3     (callee-save)
# esi => t4         (callee-save)
# edi => t5         (callee-save)
# ebp => cfr
# esp => sp
#
# On x86-64 non-windows
#
# rax => t0,     r0
# rdi =>     a0
# rsi => t1, a1
# rdx => t2, a2, r1
# rcx => t3, a3
#  r8 => t4
# r10 => t5
# rbx =>             csr0 (callee-save, PB, unused in baseline)
# r12 =>             csr1 (callee-save)
# r13 =>             csr2 (callee-save)
# r14 =>             csr3 (callee-save, tagTypeNumber)
# r15 =>             csr4 (callee-save, tagMask)
# rsp => sp
# rbp => cfr
# r11 =>                  (scratch)
#
# On x86-64 windows
# Arguments need to be push/pop'd on the stack in addition to being stored in
# the registers. Also, >8 return types are returned in a weird way.
#
# rax => t0,     r0
# rcx =>     a0
# rdx => t1, a1, r1
#  r8 => t2, a2
#  r9 => t3, a3
# r10 => t4
# rbx =>             csr0 (callee-save, PB, unused in baseline)
# rsi =>             csr1 (callee-save)
# rdi =>             csr2 (callee-save)
# r12 =>             csr3 (callee-save)
# r13 =>             csr4 (callee-save)
# r14 =>             csr5 (callee-save, tagTypeNumber)
# r15 =>             csr6 (callee-save, tagMask)
# rsp => sp
# rbp => cfr
# r11 =>                  (scratch)

def isX64
    case $activeBackend
    when "X86"
        false
    when "X86_WIN"
        false
    when "X86_64"
        true
    when "X86_64_WIN"
        true
    else
        raise "bad value for $activeBackend: #{$activeBackend}"
    end
end

def isWin
    case $activeBackend
    when "X86"
        false
    when "X86_WIN"
        true
    when "X86_64"
        false
    when "X86_64_WIN"
        true
    else
        raise "bad value for $activeBackend: #{$activeBackend}"
    end
end

def useX87
    case $activeBackend
    when "X86"
        true
    when "X86_WIN"
        true
    when "X86_64"
        false
    when "X86_64_WIN"
        false
    else
        raise "bad value for $activeBackend: #{$activeBackend}"
    end
end

def isMSVC
    $options.has_key?(:assembler) && $options[:assembler] == "MASM"
end

def isIntelSyntax
    $options.has_key?(:assembler) && $options[:assembler] == "MASM"
end

def register(name)
    isIntelSyntax ? name : "%" + name
end

def offsetRegister(off, register)
    isIntelSyntax ? "[#{off} + #{register}]" : "#{off}(#{register})"
end

def callPrefix
    isIntelSyntax ? "" : "*"
end

def orderOperands(opA, opB)
    isIntelSyntax ? "#{opB}, #{opA}" : "#{opA}, #{opB}"
end

def const(c)
    isIntelSyntax ? "#{c}" : "$#{c}"
end

def getSizeString(kind)
    if !isIntelSyntax
        return ""
    end

    size = ""
    case kind
    when :byte
        size = "byte"
    when :half
        size = "word"
    when :int
        size = "dword"
    when :ptr
        size =  isX64 ? "qword" : "dword"
    when :double
        size = "qword"
    when :quad
        size = "qword"
    else
        raise "Invalid kind #{kind}"
    end

    return size + " " + "ptr" + " ";
end

class SpecialRegister < NoChildren
    def x86Operand(kind)
        raise unless @name =~ /^r/
        raise unless isX64
        case kind
        when :half
            register(@name + "w")
        when :int
            register(@name + "d")
        when :ptr
            register(@name)
        when :quad
            register(@name)
        else
            raise
        end
    end
    def x86CallOperand(kind)
        # Call operands are not allowed to be partial registers.
        "#{callPrefix}#{x86Operand(:quad)}"
    end
end

X64_SCRATCH_REGISTER = SpecialRegister.new("r11")

def x86GPRName(name, kind)
    case name
    when "eax", "ebx", "ecx", "edx"
        name8 = name[1] + 'l'
        name16 = name[1..2]
    when "esi", "edi", "ebp", "esp"
        name16 = name[1..2]
        name8 = name16 + 'l'
    when "rax", "rbx", "rcx", "rdx"
        raise "bad GPR name #{name} in 32-bit X86" unless isX64
        name8 = name[1] + 'l'
        name16 = name[1..2]
    when "r8", "r9", "r10", "r12", "r13", "r14", "r15"
        raise "bad GPR name #{name} in 32-bit X86" unless isX64
        case kind
        when :half
            return register(name + "w")
        when :int
            return register(name + "d")
        when :ptr
            return register(name)
        when :quad
            return register(name)
        end
    else
        raise "bad GPR name #{name}"
    end
    case kind
    when :byte
        register(name8)
    when :half
        register(name16)
    when :int
        register("e" + name16)
    when :ptr
        register((isX64 ? "r" : "e") + name16)
    when :quad
        isX64 ? register("r" + name16) : raise
    else
        raise "invalid kind #{kind} for GPR #{name} in X86"
    end
end

class RegisterID
    def supports8BitOnX86
        case x86GPR
        when "eax", "ebx", "ecx", "edx", "edi", "esi", "ebp", "esp"
            true
        when "r8", "r9", "r10", "r12", "r13", "r14", "r15"
            false
        else
            raise
        end
    end

    def x86GPR
        if isX64
            case name
            when "t0", "r0"
                "eax"
            when "r1"
                "edx" # t1 = a1 when isWin, t2 = a2 otherwise
            when "a0"
                isWin ? "ecx" : "edi"
            when "t1", "a1"
                isWin ? "edx" : "esi"
            when "t2", "a2"
                isWin ? "r8" : "edx"
            when "t3", "a3"
                isWin ? "r9" : "ecx"
            when "t4"
                isWin ? "r10" : "r8"
            when "t5"
                raise "cannot use register #{name} on X86-64 Windows" unless not isWin
                "r10"
            when "csr0"
                "ebx"
            when "csr1"
                isWin ? "esi" : "r12"
            when "csr2"
                isWin ? "edi" : "r13"
            when "csr3"
                isWin ? "r12" : "r14"
            when "csr4"
                isWin ? "r13" : "r15"
            when "csr5"
                raise "cannot use register #{name} on X86-64" unless isWin
                "r14"
            when "csr6"
                raise "cannot use register #{name} on X86-64" unless isWin
                "r15"
            when "cfr"
                "ebp"
            when "sp"
                "esp"
            else
                raise "cannot use register #{name} on X86"
            end
        else
            case name
            when "t0", "r0", "a2"
                "eax"
            when "t1", "r1", "a1"
                "edx"
            when "t2", "a0"
                "ecx"
            when "t3", "a3"
                "ebx"
            when "t4"
                "esi"
            when "t5"
                "edi"
            when "cfr"
                "ebp"
            when "sp"
                "esp"
            end
        end
    end

    def x86Operand(kind)
        x86GPRName(x86GPR, kind)
    end

    def x86CallOperand(kind)
        "#{callPrefix}#{x86Operand(:ptr)}"
    end
end

class FPRegisterID
    def x86Operand(kind)
        raise unless kind == :double
        raise if useX87
        case name
        when "ft0", "fa0", "fr"
            register("xmm0")
        when "ft1", "fa1"
            register("xmm1")
        when "ft2", "fa2"
            register("xmm2")
        when "ft3", "fa3"
            register("xmm3")
        when "ft4"
            register("xmm4")
        when "ft5"
            register("xmm5")
        else
            raise "Bad register #{name} for X86 at #{codeOriginString}"
        end
    end
    def x87DefaultStackPosition
        case name
        when "ft0", "fr"
            0
        when "ft1"
            1
        when "ft2", "ft3", "ft4", "ft5"
            raise "Unimplemented register #{name} for X86 at #{codeOriginString}"
        else
            raise "Bad register #{name} for X86 at #{codeOriginString}"
        end
    end
    def x87Operand(offset)
        raise unless useX87
        raise unless offset == 0 or offset == 1
        "#{register("st")}(#{x87DefaultStackPosition + offset})"
    end
    def x86CallOperand(kind)
        "#{callPrefix}#{x86Operand(kind)}"
    end
end

class Immediate
    def validX86Immediate?
        if isX64
            value >= -0x80000000 and value <= 0x7fffffff
        else
            true
        end
    end
    def x86Operand(kind)
        "#{const(value)}"
    end
    def x86CallOperand(kind)
        "#{value}"
    end
end

class Address
    def supports8BitOnX86
        true
    end
    
    def x86AddressOperand(addressKind)
        "#{offsetRegister(offset.value, base.x86Operand(addressKind))}"
    end
    def x86Operand(kind)
        "#{getSizeString(kind)}#{x86AddressOperand(:ptr)}"
    end
    def x86CallOperand(kind)
        "#{callPrefix}#{x86Operand(kind)}"
    end
end

class BaseIndex
    def supports8BitOnX86
        true
    end
    
    def x86AddressOperand(addressKind)
        if !isIntelSyntax
            "#{offset.value}(#{base.x86Operand(addressKind)}, #{index.x86Operand(addressKind)}, #{scale})"
        else
            "#{getSizeString(addressKind)}[#{offset.value} + #{base.x86Operand(addressKind)} + #{index.x86Operand(addressKind)} * #{scale}]"
        end
    end
    
    def x86Operand(kind)
        if !isIntelSyntax
            x86AddressOperand(:ptr)
        else
            "#{getSizeString(kind)}[#{offset.value} + #{base.x86Operand(:ptr)} + #{index.x86Operand(:ptr)} * #{scale}]"
        end
    end

    def x86CallOperand(kind)
        "#{callPrefix}#{x86Operand(kind)}"
    end
end

class AbsoluteAddress
    def supports8BitOnX86
        true
    end
    
    def x86AddressOperand(addressKind)
        "#{address.value}"
    end
    
    def x86Operand(kind)
        "#{address.value}"
    end

    def x86CallOperand(kind)
        "#{callPrefix}#{address.value}"
    end
end

class LabelReference
    def x86CallOperand(kind)
        asmLabel
    end
end

class LocalLabelReference
    def x86Operand(kind)
        asmLabel
    end
    def x86CallOperand(kind)
        asmLabel
    end
end

class Sequence
    def getModifiedListX86_64
        newList = []
        
        @list.each {
            | node |
            newNode = node
            if node.is_a? Instruction
                unless node.opcode == "move"
                    usedScratch = false
                    newOperands = node.operands.map {
                        | operand |
                        if operand.immediate? and not operand.validX86Immediate?
                            if usedScratch
                                raise "Attempt to use scratch register twice at #{operand.codeOriginString}"
                            end
                            newList << Instruction.new(operand.codeOrigin, "move", [operand, X64_SCRATCH_REGISTER])
                            usedScratch = true
                            X64_SCRATCH_REGISTER
                        else
                            operand
                        end
                    }
                    newNode = Instruction.new(node.codeOrigin, node.opcode, newOperands, node.annotation)
                end
            else
                unless node.is_a? Label or
                        node.is_a? LocalLabel or
                        node.is_a? Skip
                    raise "Unexpected #{node.inspect} at #{node.codeOrigin}" 
                end
            end
            if newNode
                newList << newNode
            end
        }
        
        return newList
    end
    def getModifiedListX86_64_WIN
        getModifiedListX86_64
    end
end

class Instruction
    
    def x86Operands(*kinds)
        raise unless kinds.size == operands.size
        result = []
        kinds.size.times {
            | idx |
            i = isIntelSyntax ? (kinds.size - idx - 1) : idx
            result << operands[i].x86Operand(kinds[i])
        }
        result.join(", ")
    end

    def x86Suffix(kind)
        if isIntelSyntax
            return ""
        end

        case kind
        when :byte
            "b"
        when :half
            "w"
        when :int
            "l"
        when :ptr
            isX64 ? "q" : "l"
        when :quad
            isX64 ? "q" : raise
        when :double
            not useX87 ? "sd" : raise
        else
            raise
        end
    end
    
    def x86Bytes(kind)
        case kind
        when :byte
            1
        when :half
            2
        when :int
            4
        when :ptr
            isX64 ? 8 : 4
        when :quad
            isX64 ? 8 : raise
        when :double
            8
        else
            raise
        end
    end

    def getImplicitOperandString
        isIntelSyntax ? "st(0), " : ""
    end
    
    def handleX86OpWithNumOperands(opcode, kind, numOperands)
        if numOperands == 3
            if operands[0] == operands[2]
                $asm.puts "#{opcode} #{orderOperands(operands[1].x86Operand(kind), operands[2].x86Operand(kind))}"
            elsif operands[1] == operands[2]
                $asm.puts "#{opcode} #{orderOperands(operands[0].x86Operand(kind), operands[2].x86Operand(kind))}"
            else
                $asm.puts "mov#{x86Suffix(kind)} #{orderOperands(operands[0].x86Operand(kind), operands[2].x86Operand(kind))}"
                $asm.puts "#{opcode} #{orderOperands(operands[1].x86Operand(kind), operands[2].x86Operand(kind))}"
            end
        else
            $asm.puts "#{opcode} #{orderOperands(operands[0].x86Operand(kind), operands[1].x86Operand(kind))}"
        end
    end
    
    def handleX86Op(opcode, kind)
        handleX86OpWithNumOperands(opcode, kind, operands.size)
    end
    
    def handleX86Shift(opcode, kind)
        if operands[0].is_a? Immediate or operands[0].x86GPR == "ecx"
            $asm.puts "#{opcode} #{orderOperands(operands[0].x86Operand(:byte), operands[1].x86Operand(kind))}"
        else
            $asm.puts "xchg#{x86Suffix(:ptr)} #{operands[0].x86Operand(:ptr)}, #{x86GPRName("ecx", :ptr)}"
            $asm.puts "#{opcode} #{orderOperands(register("cl"), operands[1].x86Operand(kind))}"
            $asm.puts "xchg#{x86Suffix(:ptr)} #{operands[0].x86Operand(:ptr)}, #{x86GPRName("ecx", :ptr)}"
        end
    end
    
    def handleX86DoubleBranch(branchOpcode, mode)
        if useX87
            handleX87Compare(mode)
        else
            case mode
            when :normal
                $asm.puts "ucomisd #{orderOperands(operands[1].x86Operand(:double), operands[0].x86Operand(:double))}"
            when :reverse
                $asm.puts "ucomisd #{orderOperands(operands[0].x86Operand(:double), operands[1].x86Operand(:double))}"
            else
                raise mode.inspect
            end
        end
        $asm.puts "#{branchOpcode} #{operands[2].asmLabel}"
    end
    
    def handleX86IntCompare(opcodeSuffix, kind)
        if operands[0].is_a? Immediate and operands[0].value == 0 and operands[1].is_a? RegisterID and (opcodeSuffix == "e" or opcodeSuffix == "ne")
            $asm.puts "test#{x86Suffix(kind)} #{orderOperands(operands[1].x86Operand(kind), operands[1].x86Operand(kind))}"
        elsif operands[1].is_a? Immediate and operands[1].value == 0 and operands[0].is_a? RegisterID and (opcodeSuffix == "e" or opcodeSuffix == "ne")
            $asm.puts "test#{x86Suffix(kind)}  #{orderOperands(operands[0].x86Operand(kind), operands[0].x86Operand(kind))}"
        else
            $asm.puts "cmp#{x86Suffix(kind)} #{orderOperands(operands[1].x86Operand(kind), operands[0].x86Operand(kind))}"
        end
    end
    
    def handleX86IntBranch(branchOpcode, kind)
        handleX86IntCompare(branchOpcode[1..-1], kind)
        $asm.puts "#{branchOpcode} #{operands[2].asmLabel}"
    end
    
    def handleX86Set(setOpcode, operand)
        if operand.supports8BitOnX86
            $asm.puts "#{setOpcode} #{operand.x86Operand(:byte)}"
            if !isIntelSyntax
                $asm.puts "movzbl #{orderOperands(operand.x86Operand(:byte), operand.x86Operand(:int))}"
            else
                $asm.puts "movzx #{orderOperands(operand.x86Operand(:byte), operand.x86Operand(:int))}"
            end
        else
            ax = RegisterID.new(nil, "r0")
            $asm.puts "xchg#{x86Suffix(:ptr)} #{operand.x86Operand(:ptr)}, #{ax.x86Operand(:ptr)}"
            $asm.puts "#{setOpcode} #{ax.x86Operand(:byte)}"
            if !isIntelSyntax
                $asm.puts "movzbl #{ax.x86Operand(:byte)}, #{ax.x86Operand(:int)}"
            else
                $asm.puts "movzx #{ax.x86Operand(:int)}, #{ax.x86Operand(:byte)}"
            end
            $asm.puts "xchg#{x86Suffix(:ptr)} #{operand.x86Operand(:ptr)}, #{ax.x86Operand(:ptr)}"
        end
    end
    
    def handleX86IntCompareSet(setOpcode, kind)
        handleX86IntCompare(setOpcode[3..-1], kind)
        handleX86Set(setOpcode, operands[2])
    end
    
    def handleX86Test(kind)
        value = operands[0]
        case operands.size
        when 2
            mask = Immediate.new(codeOrigin, -1)
        when 3
            mask = operands[1]
        else
            raise "Expected 2 or 3 operands, but got #{operands.size} at #{codeOriginString}"
        end
        
        if mask.is_a? Immediate and mask.value == -1
            if value.is_a? RegisterID
                $asm.puts "test#{x86Suffix(kind)} #{value.x86Operand(kind)}, #{value.x86Operand(kind)}"
            else
                $asm.puts "cmp#{x86Suffix(kind)} #{orderOperands(const(0), value.x86Operand(kind))}"
            end
        else
            $asm.puts "test#{x86Suffix(kind)} #{orderOperands(mask.x86Operand(kind), value.x86Operand(kind))}"
        end
    end
    
    def handleX86BranchTest(branchOpcode, kind)
        handleX86Test(kind)
        $asm.puts "#{branchOpcode} #{operands.last.asmLabel}"
    end
    
    def handleX86SetTest(setOpcode, kind)
        handleX86Test(kind)
        handleX86Set(setOpcode, operands.last)
    end
    
    def handleX86OpBranch(opcode, branchOpcode, kind)
        handleX86OpWithNumOperands(opcode, kind, operands.size - 1)
        case operands.size
        when 4
            jumpTarget = operands[3]
        when 3
            jumpTarget = operands[2]
        else
            raise self.inspect
        end
        $asm.puts "#{branchOpcode} #{jumpTarget.asmLabel}"
    end
    
    def handleX86SubBranch(branchOpcode, kind)
        if operands.size == 4 and operands[1] == operands[2]
            $asm.puts "neg#{x86Suffix(kind)} #{operands[2].x86Operand(kind)}"
            $asm.puts "add#{x86Suffix(kind)} #{orderOperands(operands[0].x86Operand(kind), operands[2].x86Operand(kind))}"
        else
            handleX86OpWithNumOperands("sub#{x86Suffix(kind)}", kind, operands.size - 1)
        end
        case operands.size
        when 4
            jumpTarget = operands[3]
        when 3
            jumpTarget = operands[2]
        else
            raise self.inspect
        end
        $asm.puts "#{branchOpcode} #{jumpTarget.asmLabel}"
    end

    def handleX86Add(kind)
        if operands.size == 3 and operands[1] == operands[2]
            unless Immediate.new(nil, 0) == operands[0]
                $asm.puts "add#{x86Suffix(kind)} #{orderOperands(operands[0].x86Operand(kind), operands[2].x86Operand(kind))}"
            end
        elsif operands.size == 3 and operands[0].is_a? Immediate
            raise unless operands[1].is_a? RegisterID
            raise unless operands[2].is_a? RegisterID
            if operands[0].value == 0
                unless operands[1] == operands[2]
                    $asm.puts "mov#{x86Suffix(kind)} #{orderOperands(operands[1].x86Operand(kind), operands[2].x86Operand(kind))}"
                end
            else
                $asm.puts "lea#{x86Suffix(kind)} #{orderOperands(offsetRegister(operands[0].value, operands[1].x86Operand(kind)), operands[2].x86Operand(kind))}"
            end
        elsif operands.size == 3 and operands[0].is_a? RegisterID
            raise unless operands[1].is_a? RegisterID
            raise unless operands[2].is_a? RegisterID
            if operands[0] == operands[2]
                $asm.puts "add#{x86Suffix(kind)} #{orderOperands(operands[1].x86Operand(kind), operands[2].x86Operand(kind))}"
            else
                if !isIntelSyntax
                    $asm.puts "lea#{x86Suffix(kind)} (#{operands[0].x86Operand(kind)}, #{operands[1].x86Operand(kind)}), #{operands[2].x86Operand(kind)}"
                else
                    $asm.puts "lea#{x86Suffix(kind)} #{operands[2].x86Operand(kind)}, [#{operands[0].x86Operand(kind)} + #{operands[1].x86Operand(kind)}]"
                end
            end
        else
            unless Immediate.new(nil, 0) == operands[0]
                $asm.puts "add#{x86Suffix(kind)} #{x86Operands(kind, kind)}"
            end
        end
    end
    
    def handleX86Sub(kind)
        if operands.size == 3 and operands[1] == operands[2]
            $asm.puts "neg#{x86Suffix(kind)} #{operands[2].x86Operand(kind)}"
            $asm.puts "add#{x86Suffix(kind)} #{orderOperands(operands[0].x86Operand(kind), operands[2].x86Operand(kind))}"
        else
            handleX86Op("sub#{x86Suffix(kind)}", kind)
        end
    end
    
    def handleX86Mul(kind)
        if operands.size == 3 and operands[0].is_a? Immediate
            $asm.puts "imul#{x86Suffix(kind)} #{x86Operands(kind, kind, kind)}"
        else
            # FIXME: could do some peephole in case the left operand is immediate and it's
            # a power of two.
            handleX86Op("imul#{x86Suffix(kind)}", kind)
        end
    end
    
    def handleX86Peek()
        sp = RegisterID.new(nil, "sp")
        opA = offsetRegister(operands[0].value * x86Bytes(:ptr), sp.x86Operand(:ptr))
        opB = operands[1].x86Operand(:ptr)
        $asm.puts "mov#{x86Suffix(:ptr)} #{orderOperands(opA, opB)}"
    end

    def handleX86Poke()
        sp = RegisterID.new(nil, "sp")
        opA = operands[0].x86Operand(:ptr)
        opB = offsetRegister(operands[1].value * x86Bytes(:ptr), sp.x86Operand(:ptr))
        $asm.puts "mov#{x86Suffix(:ptr)} #{orderOperands(opA, opB)}"
    end

    def handleMove
        if Immediate.new(nil, 0) == operands[0] and operands[1].is_a? RegisterID
            if isX64
                $asm.puts "xor#{x86Suffix(:quad)} #{operands[1].x86Operand(:quad)}, #{operands[1].x86Operand(:quad)}"
            else
                $asm.puts "xor#{x86Suffix(:ptr)} #{operands[1].x86Operand(:ptr)}, #{operands[1].x86Operand(:ptr)}"
            end
        elsif operands[0] != operands[1]
            if isX64
                $asm.puts "mov#{x86Suffix(:quad)} #{x86Operands(:quad, :quad)}"
            else
                $asm.puts "mov#{x86Suffix(:ptr)} #{x86Operands(:ptr, :ptr)}"
            end
        end
    end

    def handleX87Compare(mode)
        floatingPointCompareImplicitOperand = getImplicitOperandString
        case mode
        when :normal
            if (operands[0].x87DefaultStackPosition == 0)
                $asm.puts "fucomi #{floatingPointCompareImplicitOperand}#{operands[1].x87Operand(0)}"
            else
                $asm.puts "fld #{operands[0].x87Operand(0)}"
                $asm.puts "fucomip #{floatingPointCompareImplicitOperand}#{operands[1].x87Operand(1)}"
            end
        when :reverse
            if (operands[1].x87DefaultStackPosition == 0)
                $asm.puts "fucomi #{floatingPointCompareImplicitOperand}#{operands[0].x87Operand(0)}"
            else
                $asm.puts "fld #{operands[1].x87Operand(0)}"
                $asm.puts "fucomip #{floatingPointCompareImplicitOperand}#{operands[0].x87Operand(1)}"
            end
        else
            raise mode.inspect
        end
    end

    def handleX87BinOp(opcode, opcodereverse)
        if (operands[1].x87DefaultStackPosition == 0)
            $asm.puts "#{opcode} #{orderOperands(operands[0].x87Operand(0), register("st"))}"
        elsif (operands[0].x87DefaultStackPosition == 0)
            if !isIntelSyntax
                $asm.puts "#{opcodereverse} #{register("st")}, #{operands[1].x87Operand(0)}"
            else
                $asm.puts "#{opcode} #{operands[1].x87Operand(0)}, #{register("st")}"
            end
        else
            $asm.puts "fld #{operands[0].x87Operand(0)}"
            $asm.puts "#{opcodereverse}p #{orderOperands(register("st"), operands[1].x87Operand(1))}"
        end
    end

    def lowerX86
        raise unless $activeBackend == "X86"
        lowerX86Common
    end

    def lowerX86_WIN
        raise unless $activeBackend == "X86_WIN" 
        lowerX86Common
    end
    
    def lowerX86_64
        raise unless $activeBackend == "X86_64"
        lowerX86Common
    end

    def lowerX86_64_WIN
        raise unless $activeBackend == "X86_64_WIN"
        lowerX86Common
    end

    def lowerX86Common
        $asm.codeOrigin codeOriginString if $enableCodeOriginComments
        $asm.annotation annotation if $enableInstrAnnotations
        $asm.debugAnnotation codeOrigin.debugDirective if $enableDebugAnnotations

        case opcode
        when "addi"
            handleX86Add(:int)
        when "addp"
            handleX86Add(:ptr)
        when "addq"
            handleX86Add(:quad)
        when "andi"
            handleX86Op("and#{x86Suffix(:int)}", :int)
        when "andp"
            handleX86Op("and#{x86Suffix(:ptr)}", :ptr)
        when "andq"
            handleX86Op("and#{x86Suffix(:quad)}", :quad)
        when "lshifti"
            handleX86Shift("sal#{x86Suffix(:int)}", :int)
        when "lshiftp"
            handleX86Shift("sal#{x86Suffix(:ptr)}", :ptr)
        when "lshiftq"
            handleX86Shift("sal#{x86Suffix(:quad)}", :quad)
        when "muli"
            handleX86Mul(:int)
        when "mulp"
            handleX86Mul(:ptr)
        when "mulq"
            handleX86Mul(:quad)
        when "negi"
            $asm.puts "neg#{x86Suffix(:int)} #{x86Operands(:int)}"
        when "negp"
            $asm.puts "neg#{x86Suffix(:ptr)} #{x86Operands(:ptr)}"
        when "negq"
            $asm.puts "neg#{x86Suffix(:quad)} #{x86Operands(:quad)}"
        when "noti"
            $asm.puts "not#{x86Suffix(:int)} #{x86Operands(:int)}"
        when "ori"
            handleX86Op("or#{x86Suffix(:int)}", :int)
        when "orp"
            handleX86Op("or#{x86Suffix(:ptr)}", :ptr)
        when "orq"
            handleX86Op("or#{x86Suffix(:quad)}", :quad)
        when "rshifti"
            handleX86Shift("sar#{x86Suffix(:int)}", :int)
        when "rshiftp"
            handleX86Shift("sar#{x86Suffix(:ptr)}", :ptr)
        when "rshiftq"
            handleX86Shift("sar#{x86Suffix(:quad)}", :quad)
        when "urshifti"
            handleX86Shift("shr#{x86Suffix(:int)}", :int)
        when "urshiftp"
            handleX86Shift("shr#{x86Suffix(:ptr)}", :ptr)
        when "urshiftq"
            handleX86Shift("shr#{x86Suffix(:quad)}", :quad)
        when "subi"
            handleX86Sub(:int)
        when "subp"
            handleX86Sub(:ptr)
        when "subq"
            handleX86Sub(:quad)
        when "xori"
            handleX86Op("xor#{x86Suffix(:int)}", :int)
        when "xorp"
            handleX86Op("xor#{x86Suffix(:ptr)}", :ptr)
        when "xorq"
            handleX86Op("xor#{x86Suffix(:quad)}", :quad)
        when "loadi", "storei"
            $asm.puts "mov#{x86Suffix(:int)} #{x86Operands(:int, :int)}"
        when "loadis"
            if isX64
                if !isIntelSyntax
                    $asm.puts "movslq #{x86Operands(:int, :quad)}"
                else
                    $asm.puts "movsxd #{x86Operands(:int, :quad)}"
                end
            else
                $asm.puts "mov#{x86Suffix(:int)} #{x86Operands(:int, :int)}"
            end
        when "loadp", "storep"
            $asm.puts "mov#{x86Suffix(:ptr)} #{x86Operands(:ptr, :ptr)}"
        when "loadq", "storeq"
            $asm.puts "mov#{x86Suffix(:quad)} #{x86Operands(:quad, :quad)}"
        when "loadb"
            if !isIntelSyntax
                $asm.puts "movzbl #{orderOperands(operands[0].x86Operand(:byte), operands[1].x86Operand(:int))}"
            else
                $asm.puts "movzx #{orderOperands(operands[0].x86Operand(:byte), operands[1].x86Operand(:int))}"
            end
        when "loadbs"
            if !isIntelSyntax
                $asm.puts "movsbl #{orderOperands(operands[0].x86Operand(:byte), operands[1].x86Operand(:int))}"
            else
                $asm.puts "movsx #{orderOperands(operands[0].x86Operand(:byte), operands[1].x86Operand(:int))}"
            end
        when "loadh"
            if !isIntelSyntax
                $asm.puts "movzwl #{orderOperands(operands[0].x86Operand(:half), operands[1].x86Operand(:int))}"
            else
                $asm.puts "movzx #{orderOperands(operands[0].x86Operand(:half), operands[1].x86Operand(:int))}"
            end
        when "loadhs"
            if !isIntelSyntax
                $asm.puts "movswl #{orderOperands(operands[0].x86Operand(:half), operands[1].x86Operand(:int))}"
            else
                $asm.puts "movsx #{orderOperands(operands[0].x86Operand(:half), operands[1].x86Operand(:int))}"
            end
        when "storeb"
            $asm.puts "mov#{x86Suffix(:byte)} #{x86Operands(:byte, :byte)}"
        when "loadd"
            if useX87
                if !isIntelSyntax
                    $asm.puts "fldl #{operands[0].x86Operand(:double)}"
                else
                    $asm.puts "fld #{operands[0].x86Operand(:double)}"
                end
                $asm.puts "fstp #{operands[1].x87Operand(1)}"
            else
                $asm.puts "movsd #{x86Operands(:double, :double)}"
            end
        when "moved"
            if useX87
                if (operands[0].x87DefaultStackPosition == 0)
                    $asm.puts "fst #{operands[1].x87Operand(0)}"
                else
                    $asm.puts "fld #{operands[0].x87Operand(0)}"
                    $asm.puts "fstp #{operands[1].x87Operand(1)}"
                end
            else
                $asm.puts "movsd #{x86Operands(:double, :double)}"
            end
        when "stored"
            if useX87
                if (operands[0].x87DefaultStackPosition == 0)
                    $asm.puts "fst#{x86Suffix(:int)} #{operands[1].x86Operand(:double)}"
                else
                    $asm.puts "fld #{operands[0].x87Operand(0)}"
                    if !isIntelSyntax
                        $asm.puts "fstpl #{operands[1].x86Operand(:double)}"
                    else
                        $asm.puts "fstp #{operands[1].x86Operand(:double)}"
                    end
                end
            else
                $asm.puts "movsd #{x86Operands(:double, :double)}"
            end
        when "addd"
            if useX87
                handleX87BinOp("fadd", "fadd")
            else
                $asm.puts "addsd #{x86Operands(:double, :double)}"
            end
        when "muld"
            if useX87
                handleX87BinOp("fmul", "fmul")
            else
                $asm.puts "mulsd #{x86Operands(:double, :double)}"
            end
        when "subd"
            if useX87
                handleX87BinOp("fsub", "fsubr")
            else
                $asm.puts "subsd #{x86Operands(:double, :double)}"
            end
        when "divd"
            if useX87
                handleX87BinOp("fdiv", "fdivr")
            else
                $asm.puts "divsd #{x86Operands(:double, :double)}"
            end
        when "sqrtd"
            if useX87
                $asm.puts "fld #{operands[0].x87Operand(0)}"
                $asm.puts "fsqrtl"
                $asm.puts "fstp #{operands[1].x87Operand(1)}"
            else
                $asm.puts "sqrtsd #{operands[0].x86Operand(:double)}, #{operands[1].x86Operand(:double)}"
            end
        when "ci2d"
            if useX87
                sp = RegisterID.new(nil, "sp")
                $asm.puts "mov#{x86Suffix(:int)} #{orderOperands(operands[0].x86Operand(:int), offsetRegister(-4, sp.x86Operand(:ptr)))}"
                $asm.puts "fild#{x86Suffix(:ptr)} #{getSizeString(:ptr)}#{offsetRegister(-4, sp.x86Operand(:ptr))}"
                $asm.puts "fstp #{operands[1].x87Operand(1)}"
            else
                $asm.puts "cvtsi2sd #{orderOperands(operands[0].x86Operand(:int), operands[1].x86Operand(:double))}"
            end
        when "bdeq"
            if useX87
                handleX87Compare(:normal)
            else
                $asm.puts "ucomisd #{orderOperands(operands[0].x86Operand(:double), operands[1].x86Operand(:double))}"
            end
            if operands[0] == operands[1]
                # This is just a jump ordered, which is a jnp.
                $asm.puts "jnp #{operands[2].asmLabel}"
            else
                isUnordered = LocalLabel.unique("bdeq")
                $asm.puts "jp #{LabelReference.new(codeOrigin, isUnordered).asmLabel}"
                $asm.puts "je #{LabelReference.new(codeOrigin, operands[2]).asmLabel}"
                isUnordered.lower("X86")
            end
        when "bdneq"
            handleX86DoubleBranch("jne", :normal)
        when "bdgt"
            handleX86DoubleBranch("ja", :normal)
        when "bdgteq"
            handleX86DoubleBranch("jae", :normal)
        when "bdlt"
            handleX86DoubleBranch("ja", :reverse)
        when "bdlteq"
            handleX86DoubleBranch("jae", :reverse)
        when "bdequn"
            handleX86DoubleBranch("je", :normal)
        when "bdnequn"
            if useX87
                handleX87Compare(:normal)
            else
                $asm.puts "ucomisd #{orderOperands(operands[0].x86Operand(:double), operands[1].x86Operand(:double))}"
            end
            if operands[0] == operands[1]
                # This is just a jump unordered, which is a jp.
                $asm.puts "jp #{operands[2].asmLabel}"
            else
                isUnordered = LocalLabel.unique("bdnequn")
                isEqual = LocalLabel.unique("bdnequn")
                $asm.puts "jp #{LabelReference.new(codeOrigin, isUnordered).asmLabel}"
                $asm.puts "je #{LabelReference.new(codeOrigin, isEqual).asmLabel}"
                isUnordered.lower("X86")
                $asm.puts "jmp #{operands[2].asmLabel}"
                isEqual.lower("X86")
            end
        when "bdgtun"
            handleX86DoubleBranch("jb", :reverse)
        when "bdgtequn"
            handleX86DoubleBranch("jbe", :reverse)
        when "bdltun"
            handleX86DoubleBranch("jb", :normal)
        when "bdltequn"
            handleX86DoubleBranch("jbe", :normal)
        when "btd2i"
            # FIXME: unused and unimplemented for x87
            raise if useX87
            $asm.puts "cvttsd2si #{operands[0].x86Operand(:double)}, #{operands[1].x86Operand(:int)}"
            $asm.puts "cmpl $0x80000000 #{operands[1].x86Operand(:int)}"
            $asm.puts "je #{operands[2].asmLabel}"
        when "td2i"
            # FIXME: unused and unimplemented for x87
            raise if useX87
            $asm.puts "cvttsd2si #{operands[0].x86Operand(:double)}, #{operands[1].x86Operand(:int)}"
        when "bcd2i"
            if useX87
                floatingPointCompareImplicitOperand = getImplicitOperandString
                sp = RegisterID.new(nil, "sp")
                if (operands[0].x87DefaultStackPosition == 0)
                    $asm.puts "fistl -4(#{sp.x86Operand(:ptr)})"
                else
                    $asm.puts "fld #{operands[0].x87Operand(0)}"
                    $asm.puts "fistp#{x86Suffix(:ptr)} #{getSizeString(:ptr)}#{offsetRegister(-4, sp.x86Operand(:ptr))}"
                end
                $asm.puts "mov#{x86Suffix(:int)} #{orderOperands(offsetRegister(-4, sp.x86Operand(:ptr)), operands[1].x86Operand(:int))}"
                $asm.puts "test#{x86Suffix(:int)} #{operands[1].x86Operand(:int)}, #{operands[1].x86Operand(:int)}"
                $asm.puts "je #{operands[2].asmLabel}"
                $asm.puts "fild#{x86Suffix(:int)} #{getSizeString(:int)}#{offsetRegister(-4, sp.x86Operand(:ptr))}"
                $asm.puts "fucomip #{floatingPointCompareImplicitOperand}#{operands[0].x87Operand(1)}"
                $asm.puts "jp #{operands[2].asmLabel}"
                $asm.puts "jne #{operands[2].asmLabel}"
            else
                $asm.puts "cvttsd2si #{operands[0].x86Operand(:double)}, #{operands[1].x86Operand(:int)}"
                $asm.puts "test#{x86Suffix(:int)} #{operands[1].x86Operand(:int)}, #{operands[1].x86Operand(:int)}"
                $asm.puts "je #{operands[2].asmLabel}"
                $asm.puts "cvtsi2sd #{operands[1].x86Operand(:int)}, %xmm7"
                $asm.puts "ucomisd #{operands[0].x86Operand(:double)}, %xmm7"
                $asm.puts "jp #{operands[2].asmLabel}"
                $asm.puts "jne #{operands[2].asmLabel}"
            end
        when "movdz"
            if useX87
                $asm.puts "fldzl"
                $asm.puts "fstp #{operands[0].x87Operand(1)}"
            else
                $asm.puts "xorpd #{operands[0].x86Operand(:double)}, #{operands[0].x86Operand(:double)}"
            end
        when "pop"
            operands.each {
                | op |
                $asm.puts "pop #{op.x86Operand(:ptr)}"
            }
        when "push"
            operands.each {
                | op |
                $asm.puts "push #{op.x86Operand(:ptr)}"
            }
        when "move"
            handleMove
        when "sxi2q"
            if !isIntelSyntax
                $asm.puts "movslq #{operands[0].x86Operand(:int)}, #{operands[1].x86Operand(:quad)}"
            else
                $asm.puts "movsxd #{orderOperands(operands[0].x86Operand(:int), operands[1].x86Operand(:quad))}"
            end
        when "zxi2q"
            $asm.puts "mov#{x86Suffix(:int)} #{orderOperands(operands[0].x86Operand(:int), operands[1].x86Operand(:int))}"
        when "nop"
            $asm.puts "nop"
        when "bieq"
            handleX86IntBranch("je", :int)
        when "bpeq"
            handleX86IntBranch("je", :ptr)
        when "bqeq"
            handleX86IntBranch("je", :quad)
        when "bineq"
            handleX86IntBranch("jne", :int)
        when "bpneq"
            handleX86IntBranch("jne", :ptr)
        when "bqneq"
            handleX86IntBranch("jne", :quad)
        when "bia"
            handleX86IntBranch("ja", :int)
        when "bpa"
            handleX86IntBranch("ja", :ptr)
        when "bqa"
            handleX86IntBranch("ja", :quad)
        when "biaeq"
            handleX86IntBranch("jae", :int)
        when "bpaeq"
            handleX86IntBranch("jae", :ptr)
        when "bqaeq"
            handleX86IntBranch("jae", :quad)
        when "bib"
            handleX86IntBranch("jb", :int)
        when "bpb"
            handleX86IntBranch("jb", :ptr)
        when "bqb"
            handleX86IntBranch("jb", :quad)
        when "bibeq"
            handleX86IntBranch("jbe", :int)
        when "bpbeq"
            handleX86IntBranch("jbe", :ptr)
        when "bqbeq"
            handleX86IntBranch("jbe", :quad)
        when "bigt"
            handleX86IntBranch("jg", :int)
        when "bpgt"
            handleX86IntBranch("jg", :ptr)
        when "bqgt"
            handleX86IntBranch("jg", :quad)
        when "bigteq"
            handleX86IntBranch("jge", :int)
        when "bpgteq"
            handleX86IntBranch("jge", :ptr)
        when "bqgteq"
            handleX86IntBranch("jge", :quad)
        when "bilt"
            handleX86IntBranch("jl", :int)
        when "bplt"
            handleX86IntBranch("jl", :ptr)
        when "bqlt"
            handleX86IntBranch("jl", :quad)
        when "bilteq"
            handleX86IntBranch("jle", :int)
        when "bplteq"
            handleX86IntBranch("jle", :ptr)
        when "bqlteq"
            handleX86IntBranch("jle", :quad)
        when "bbeq"
            handleX86IntBranch("je", :byte)
        when "bbneq"
            handleX86IntBranch("jne", :byte)
        when "bba"
            handleX86IntBranch("ja", :byte)
        when "bbaeq"
            handleX86IntBranch("jae", :byte)
        when "bbb"
            handleX86IntBranch("jb", :byte)
        when "bbbeq"
            handleX86IntBranch("jbe", :byte)
        when "bbgt"
            handleX86IntBranch("jg", :byte)
        when "bbgteq"
            handleX86IntBranch("jge", :byte)
        when "bblt"
            handleX86IntBranch("jl", :byte)
        when "bblteq"
            handleX86IntBranch("jlteq", :byte)
        when "btis"
            handleX86BranchTest("js", :int)
        when "btps"
            handleX86BranchTest("js", :ptr)
        when "btqs"
            handleX86BranchTest("js", :quad)
        when "btiz"
            handleX86BranchTest("jz", :int)
        when "btpz"
            handleX86BranchTest("jz", :ptr)
        when "btqz"
            handleX86BranchTest("jz", :quad)
        when "btinz"
            handleX86BranchTest("jnz", :int)
        when "btpnz"
            handleX86BranchTest("jnz", :ptr)
        when "btqnz"
            handleX86BranchTest("jnz", :quad)
        when "btbs"
            handleX86BranchTest("js", :byte)
        when "btbz"
            handleX86BranchTest("jz", :byte)
        when "btbnz"
            handleX86BranchTest("jnz", :byte)
        when "jmp"
            $asm.puts "jmp #{operands[0].x86CallOperand(:ptr)}"
        when "baddio"
            handleX86OpBranch("add#{x86Suffix(:int)}", "jo", :int)
        when "baddpo"
            handleX86OpBranch("add#{x86Suffix(:ptr)}", "jo", :ptr)
        when "baddqo"
            handleX86OpBranch("add#{x86Suffix(:quad)}", "jo", :quad)
        when "baddis"
            handleX86OpBranch("add#{x86Suffix(:int)}", "js", :int)
        when "baddps"
            handleX86OpBranch("add#{x86Suffix(:ptr)}", "js", :ptr)
        when "baddqs"
            handleX86OpBranch("add#{x86Suffix(:quad)}", "js", :quad)
        when "baddiz"
            handleX86OpBranch("add#{x86Suffix(:int)}", "jz", :int)
        when "baddpz"
            handleX86OpBranch("add#{x86Suffix(:ptr)}", "jz", :ptr)
        when "baddqz"
            handleX86OpBranch("add#{x86Suffix(:quad)}", "jz", :quad)
        when "baddinz"
            handleX86OpBranch("add#{x86Suffix(:int)}", "jnz", :int)
        when "baddpnz"
            handleX86OpBranch("add#{x86Suffix(:ptr)}", "jnz", :ptr)
        when "baddqnz"
            handleX86OpBranch("add#{x86Suffix(:quad)}", "jnz", :quad)
        when "bsubio"
            handleX86SubBranch("jo", :int)
        when "bsubis"
            handleX86SubBranch("js", :int)
        when "bsubiz"
            handleX86SubBranch("jz", :int)
        when "bsubinz"
            handleX86SubBranch("jnz", :int)
        when "bmulio"
            handleX86OpBranch("imul#{x86Suffix(:int)}", "jo", :int)
        when "bmulis"
            handleX86OpBranch("imul#{x86Suffix(:int)}", "js", :int)
        when "bmuliz"
            handleX86OpBranch("imul#{x86Suffix(:int)}", "jz", :int)
        when "bmulinz"
            handleX86OpBranch("imul#{x86Suffix(:int)}", "jnz", :int)
        when "borio"
            handleX86OpBranch("orl", "jo", :int)
        when "boris"
            handleX86OpBranch("orl", "js", :int)
        when "boriz"
            handleX86OpBranch("orl", "jz", :int)
        when "borinz"
            handleX86OpBranch("orl", "jnz", :int)
        when "break"
            $asm.puts "int #{const(3)}"
        when "call"
            if useX87
                2.times {
                    | offset |
                    $asm.puts "ffree #{register("st")}(#{offset})"
                }
            end
            op = operands[0].x86CallOperand(:ptr)
            if operands[0].is_a? LabelReference
                operands[0].used
            end
            $asm.puts "call #{op}"
        when "ret"
            $asm.puts "ret"
        when "cieq"
            handleX86IntCompareSet("sete", :int)
        when "cbeq"
            handleX86IntCompareSet("sete", :byte)
        when "cpeq"
            handleX86IntCompareSet("sete", :ptr)
        when "cqeq"
            handleX86IntCompareSet("sete", :quad)
        when "cineq"
            handleX86IntCompareSet("setne", :int)
        when "cbneq"
            handleX86IntCompareSet("setne", :byte)
        when "cpneq"
            handleX86IntCompareSet("setne", :ptr)
        when "cqneq"
            handleX86IntCompareSet("setne", :quad)
        when "cia"
            handleX86IntCompareSet("seta", :int)
        when "cba"
            handleX86IntCompareSet("seta", :byte)
        when "cpa"
            handleX86IntCompareSet("seta", :ptr)
        when "cqa"
            handleX86IntCompareSet("seta", :quad)
        when "ciaeq"
            handleX86IntCompareSet("setae", :int)
        when "cbaeq"
            handleX86IntCompareSet("setae", :byte)
        when "cpaeq"
            handleX86IntCompareSet("setae", :ptr)
        when "cqaeq"
            handleX86IntCompareSet("setae", :quad)
        when "cib"
            handleX86IntCompareSet("setb", :int)
        when "cbb"
            handleX86IntCompareSet("setb", :byte)
        when "cpb"
            handleX86IntCompareSet("setb", :ptr)
        when "cqb"
            handleX86IntCompareSet("setb", :quad)
        when "cibeq"
            handleX86IntCompareSet("setbe", :int)
        when "cbbeq"
            handleX86IntCompareSet("setbe", :byte)
        when "cpbeq"
            handleX86IntCompareSet("setbe", :ptr)
        when "cqbeq"
            handleX86IntCompareSet("setbe", :quad)
        when "cigt"
            handleX86IntCompareSet("setg", :int)
        when "cbgt"
            handleX86IntCompareSet("setg", :byte)
        when "cpgt"
            handleX86IntCompareSet("setg", :ptr)
        when "cqgt"
            handleX86IntCompareSet("setg", :quad)
        when "cigteq"
            handleX86IntCompareSet("setge", :int)
        when "cbgteq"
            handleX86IntCompareSet("setge", :byte)
        when "cpgteq"
            handleX86IntCompareSet("setge", :ptr)
        when "cqgteq"
            handleX86IntCompareSet("setge", :quad)
        when "cilt"
            handleX86IntCompareSet("setl", :int)
        when "cblt"
            handleX86IntCompareSet("setl", :byte)
        when "cplt"
            handleX86IntCompareSet("setl", :ptr)
        when "cqlt"
            handleX86IntCompareSet("setl", :quad)
        when "cilteq"
            handleX86IntCompareSet("setle", :int)
        when "cblteq"
            handleX86IntCompareSet("setle", :byte)
        when "cplteq"
            handleX86IntCompareSet("setle", :ptr)
        when "cqlteq"
            handleX86IntCompareSet("setle", :quad)
        when "tis"
            handleX86SetTest("sets", :int)
        when "tiz"
            handleX86SetTest("setz", :int)
        when "tinz"
            handleX86SetTest("setnz", :int)
        when "tps"
            handleX86SetTest("sets", :ptr)
        when "tpz"
            handleX86SetTest("setz", :ptr)
        when "tpnz"
            handleX86SetTest("setnz", :ptr)
        when "tqs"
            handleX86SetTest("sets", :quad)
        when "tqz"
            handleX86SetTest("setz", :quad)
        when "tqnz"
            handleX86SetTest("setnz", :quad)
        when "tbs"
            handleX86SetTest("sets", :byte)
        when "tbz"
            handleX86SetTest("setz", :byte)
        when "tbnz"
            handleX86SetTest("setnz", :byte)
        when "peek"
            handleX86Peek()
        when "poke"
            handleX86Poke()
        when "cdqi"
            $asm.puts "cdq"
        when "idivi"
            $asm.puts "idiv#{x86Suffix(:int)} #{operands[0].x86Operand(:int)}"
        when "fii2d"
            if useX87
                sp = RegisterID.new(nil, "sp")
                $asm.puts "mov#{x86Suffix(:int)} #{orderOperands(operands[0].x86Operand(:int), offsetRegister(-8, sp.x86Operand(:ptr)))}"
                $asm.puts "mov#{x86Suffix(:int)} #{orderOperands(operands[1].x86Operand(:int), offsetRegister(-4, sp.x86Operand(:ptr)))}"
                $asm.puts "fld#{x86Suffix(:ptr)} #{getSizeString(:double)}#{offsetRegister(-8, sp.x86Operand(:ptr))}"
                $asm.puts "fstp #{operands[2].x87Operand(1)}"
            else
                $asm.puts "movd #{operands[0].x86Operand(:int)}, #{operands[2].x86Operand(:double)}"
                $asm.puts "movd #{operands[1].x86Operand(:int)}, %xmm7"
                $asm.puts "psllq $32, %xmm7"
                $asm.puts "por %xmm7, #{operands[2].x86Operand(:double)}"
            end
        when "fd2ii"
            if useX87
                sp = RegisterID.new(nil, "sp")
                if (operands[0].x87DefaultStackPosition == 0)
                    $asm.puts "fst#{x86Suffix(:ptr)} #{getSizeString(:double)}#{offsetRegister(-8, sp.x86Operand(:ptr))}"
                else
                    $asm.puts "fld #{operands[0].x87Operand(0)}"
                    $asm.puts "fstpl -8(#{sp.x86Operand(:ptr)})"
                end
                $asm.puts "mov#{x86Suffix(:int)} #{orderOperands(offsetRegister(-8, sp.x86Operand(:ptr)), operands[1].x86Operand(:int))}"
                $asm.puts "mov#{x86Suffix(:int)} #{orderOperands(offsetRegister(-4, sp.x86Operand(:ptr)), operands[2].x86Operand(:int))}"
            else
                $asm.puts "movd #{operands[0].x86Operand(:double)}, #{operands[1].x86Operand(:int)}"
                $asm.puts "movsd #{operands[0].x86Operand(:double)}, %xmm7"
                $asm.puts "psrlq $32, %xmm7"
                $asm.puts "movd %xmm7, #{operands[2].x86Operand(:int)}"
            end
        when "fq2d"
            if useX87
                sp = RegisterID.new(nil, "sp")
                $asm.puts "movq #{operands[0].x86Operand(:quad)}, -8(#{sp.x86Operand(:ptr)})"
                $asm.puts "fldl -8(#{sp.x86Operand(:ptr)})"
                $asm.puts "fstp #{operands[1].x87Operand(1)}"
            else
                if !isIntelSyntax
                    $asm.puts "movq #{operands[0].x86Operand(:quad)}, #{operands[1].x86Operand(:double)}"
                else
                    # MASM does not accept register operands with movq.
                    # Debugging shows that movd actually moves a qword when using MASM.
                    $asm.puts "movd #{operands[1].x86Operand(:double)}, #{operands[0].x86Operand(:quad)}"
                end
            end
        when "fd2q"
            if useX87
                sp = RegisterID.new(nil, "sp")
                if (operands[0].x87DefaultStackPosition == 0)
                    $asm.puts "fst#{x86Suffix(:int)} #{getSizeString(:int)}#{offsetRegister(-8, sp.x86Operand(:ptr))}"
                else
                    $asm.puts "fld #{operands[0].x87Operand(0)}"
                    $asm.puts "fstpl -8(#{sp.x86Operand(:ptr)})"
                end
                $asm.puts "movq -8(#{sp.x86Operand(:ptr)}), #{operands[1].x86Operand(:quad)}"
            else
                if !isIntelSyntax
                    $asm.puts "movq #{operands[0].x86Operand(:double)}, #{operands[1].x86Operand(:quad)}"
                else
                    # MASM does not accept register operands with movq.
                    # Debugging shows that movd actually moves a qword when using MASM.
                    $asm.puts "movd #{operands[1].x86Operand(:quad)}, #{operands[0].x86Operand(:double)}"
                end
            end
        when "bo"
            $asm.puts "jo #{operands[0].asmLabel}"
        when "bs"
            $asm.puts "js #{operands[0].asmLabel}"
        when "bz"
            $asm.puts "jz #{operands[0].asmLabel}"
        when "bnz"
            $asm.puts "jnz #{operands[0].asmLabel}"
        when "leai"
            $asm.puts "lea#{x86Suffix(:int)} #{orderOperands(operands[0].x86AddressOperand(:int), operands[1].x86Operand(:int))}"
        when "leap"
            $asm.puts "lea#{x86Suffix(:ptr)} #{orderOperands(operands[0].x86AddressOperand(:ptr), operands[1].x86Operand(:ptr))}"
        when "memfence"
            sp = RegisterID.new(nil, "sp")
            if isIntelSyntax
                $asm.puts "mfence"
            else
                $asm.puts "lock; orl $0, (#{sp.x86Operand(:ptr)})"
            end
        else
            lowerDefault
        end
    end
end

