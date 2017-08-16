# Copyright (C) 2011, 2012, 2014-2016 Apple Inc. All rights reserved.
# Copyright (C) 2014 University of Szeged. All rights reserved.
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

require "ast"
require "opt"
require "risc"

# Naming conventions:
#
# x<number>  => GPR. This is both the generic name of the register, and the name used
#               to indicate that the register is used in 64-bit mode.
# w<number>  => GPR in 32-bit mode. This is the low 32-bits of the GPR. If it is
#               mutated then the high 32-bit part of the register is zero filled.
# q<number>  => FPR. This is the generic name of the register.
# d<number>  => FPR used as an IEEE 64-bit binary floating point number (i.e. double).
#
# GPR conventions, to match the baseline JIT:
#
#  x0  => t0, a0, r0
#  x1  => t1, a1, r1
#  x2  => t2, a2
#  x3  => t3, a3
#  x4  => t4
#  x5  => t5
# x13  =>                  (scratch)
# x16  =>                  (scratch)
# x17  =>                  (scratch)
# x26  =>             csr0 (PB)
# x27  =>             csr1 (tagTypeNumber)
# x28  =>             csr2 (tagMask)
# x29  => cfr
#  sp  => sp
#  lr  => lr
#
# FPR conventions, to match the baseline JIT:
#
#  q0  => ft0, fa0, fr
#  q1  => ft1, fa1
#  q2  => ft2, fa2
#  q3  => ft3, fa3
#  q4  => ft4          (unused in baseline)
#  q5  => ft5          (unused in baseline)
#  q8  => csfr0        (Only the lower 64 bits)
#  q9  => csfr1        (Only the lower 64 bits)
# q10  => csfr2        (Only the lower 64 bits)
# q11  => csfr3        (Only the lower 64 bits)
# q12  => csfr4        (Only the lower 64 bits)
# q13  => csfr5        (Only the lower 64 bits)
# q14  => csfr6        (Only the lower 64 bits)
# q15  => csfr7        (Only the lower 64 bits)
# q31  => scratch

def arm64GPRName(name, kind)
    raise "bad GPR name #{name}" unless name =~ /^x/
    number = name[1..-1]
    case kind
    when :int
        "w" + number
    when :ptr
        "x" + number
    else
        raise "Wrong kind: #{kind}"
    end
end

def arm64FPRName(name, kind)
    raise "bad FPR kind #{kind}" unless kind == :double
    raise "bad FPR name #{name}" unless name =~ /^q/
    "d" + name[1..-1]
end

class SpecialRegister
    def arm64Operand(kind)
        case @name
        when /^x/
            arm64GPRName(@name, kind)
        when /^q/
            arm64FPRName(@name, kind)
        else
            raise "Bad name: #{@name}"
        end
    end
end

ARM64_EXTRA_GPRS = [SpecialRegister.new("x16"), SpecialRegister.new("x17"), SpecialRegister.new("x13")]
ARM64_EXTRA_FPRS = [SpecialRegister.new("q31")]

class RegisterID
    def arm64Operand(kind)
        case @name
        when 't0', 'a0', 'r0'
            arm64GPRName('x0', kind)
        when 't1', 'a1', 'r1'
            arm64GPRName('x1', kind)
        when 't2', 'a2'
            arm64GPRName('x2', kind)
        when 't3', 'a3'
            arm64GPRName('x3', kind)
        when 't4'
            arm64GPRName('x4', kind)
        when 't5'
            arm64GPRName('x5', kind)
        when 'cfr'
            arm64GPRName('x29', kind)
        when 'csr0'
            arm64GPRName('x19', kind)
        when 'csr1'
            arm64GPRName('x20', kind)
        when 'csr2'
            arm64GPRName('x21', kind)
        when 'csr3'
            arm64GPRName('x22', kind)
        when 'csr4'
            arm64GPRName('x23', kind)
        when 'csr5'
            arm64GPRName('x24', kind)
        when 'csr6'
            arm64GPRName('x25', kind)
        when 'csr7'
            arm64GPRName('x26', kind)
        when 'csr8'
            arm64GPRName('x27', kind)
        when 'csr9'
            arm64GPRName('x28', kind)
        when 'sp'
            'sp'
        when 'lr'
            'x30'
        else
            raise "Bad register name #{@name} at #{codeOriginString}"
        end
    end
end

class FPRegisterID
    def arm64Operand(kind)
        case @name
        when 'ft0', 'fr', 'fa0'
            arm64FPRName('q0', kind)
        when 'ft1', 'fa1'
            arm64FPRName('q1', kind)
        when 'ft2', 'fa2'
            arm64FPRName('q2', kind)
        when 'ft3', 'fa3'
            arm64FPRName('q3', kind)
        when 'ft4'
            arm64FPRName('q4', kind)
        when 'ft5'
            arm64FPRName('q5', kind)
        when 'csfr0'
            arm64FPRName('q8', kind)
        when 'csfr1'
            arm64FPRName('q9', kind)
        when 'csfr2'
            arm64FPRName('q10', kind)
        when 'csfr3'
            arm64FPRName('q11', kind)
        when 'csfr4'
            arm64FPRName('q12', kind)
        when 'csfr5'
            arm64FPRName('q13', kind)
        when 'csfr6'
            arm64FPRName('q14', kind)
        when 'csfr7'
            arm64FPRName('q15', kind)
        else "Bad register name #{@name} at #{codeOriginString}"
        end
    end
end

class Immediate
    def arm64Operand(kind)
        raise "Invalid immediate #{value} at #{codeOriginString}" if value < 0 or value > 4095
        "\##{value}"
    end
end

class Address
    def arm64Operand(kind)
        raise "Invalid offset #{offset.value} at #{codeOriginString}" if offset.value < -255 or offset.value > 4095
        "[#{base.arm64Operand(:ptr)}, \##{offset.value}]"
    end
    
    def arm64EmitLea(destination, kind)
        $asm.puts "add #{destination.arm64Operand(kind)}, #{base.arm64Operand(kind)}, \##{offset.value}"
    end
end

class BaseIndex
    def arm64Operand(kind)
        raise "Invalid offset #{offset.value} at #{codeOriginString}" if offset.value != 0
        "[#{base.arm64Operand(:ptr)}, #{index.arm64Operand(:ptr)}, lsl \##{scaleShift}]"
    end

    def arm64EmitLea(destination, kind)
        $asm.puts "add #{destination.arm64Operand(kind)}, #{base.arm64Operand(kind)}, #{index.arm64Operand(kind)}, lsl \##{scaleShift}"
    end
end

class AbsoluteAddress
    def arm64Operand(kind)
        raise "Unconverted absolute address #{address.value} at #{codeOriginString}"
    end
end

# FIXME: We could support AbsoluteAddress for lea, but we don't.

#
# Actual lowering code follows.
#

def arm64LowerMalformedLoadStoreAddresses(list)
    newList = []

    def isAddressMalformed(operand)
        operand.is_a? Address and not (-255..4095).include? operand.offset.value
    end

    list.each {
        | node |
        if node.is_a? Instruction
            if node.opcode =~ /^store/ and isAddressMalformed(node.operands[1])
                address = node.operands[1]
                tmp = Tmp.new(codeOrigin, :gpr)
                newList << Instruction.new(node.codeOrigin, "move", [address.offset, tmp])
                newList << Instruction.new(node.codeOrigin, node.opcode, [node.operands[0], BaseIndex.new(node.codeOrigin, address.base, tmp, 1, Immediate.new(codeOrigin, 0))], node.annotation)
            elsif node.opcode =~ /^load/ and isAddressMalformed(node.operands[0])
                address = node.operands[0]
                tmp = Tmp.new(codeOrigin, :gpr)
                newList << Instruction.new(node.codeOrigin, "move", [address.offset, tmp])
                newList << Instruction.new(node.codeOrigin, node.opcode, [BaseIndex.new(node.codeOrigin, address.base, tmp, 1, Immediate.new(codeOrigin, 0)), node.operands[1]], node.annotation)
            else
                newList << node
            end
        else
            newList << node
        end
    }
    newList
end

# Workaround for Cortex-A53 erratum (835769)
def arm64CortexA53Fix835769(list)
    newList = []
    lastOpcodeUnsafe = false

    list.each {
        | node |
        if node.is_a? Instruction
            case node.opcode
            when /^store/, /^load/
                # List all macro instructions that can be lowered to a load, store or prefetch ARM64 assembly instruction
                lastOpcodeUnsafe = true
            when  "muli", "mulp", "mulq", "smulli"
                # List all macro instructions that can be lowered to a 64-bit multiply-accumulate ARM64 assembly instruction
                # (defined as one of MADD, MSUB, SMADDL, SMSUBL, UMADDL or UMSUBL).
                if lastOpcodeUnsafe
                    newList << Instruction.new(node.codeOrigin, "nopCortexA53Fix835769", [])
                end
                lastOpcodeUnsafe = false
            else
                lastOpcodeUnsafe = false
            end
        end
        newList << node
    }
    newList
end

class Sequence
    def getModifiedListARM64
        result = @list
        result = riscLowerNot(result)
        result = riscLowerSimpleBranchOps(result)
        result = riscLowerHardBranchOps64(result)
        result = riscLowerShiftOps(result)
        result = arm64LowerMalformedLoadStoreAddresses(result)
        result = riscLowerMalformedAddresses(result) {
            | node, address |
            case node.opcode
            when "loadb", "loadbs", "storeb", /^bb/, /^btb/, /^cb/, /^tb/
                size = 1
            when "loadh", "loadhs"
                size = 2
            when "loadi", "loadis", "storei", "addi", "andi", "lshifti", "muli", "negi",
                "noti", "ori", "rshifti", "urshifti", "subi", "xori", /^bi/, /^bti/,
                /^ci/, /^ti/, "addis", "subis", "mulis", "smulli", "leai"
                size = 4
            when "loadp", "storep", "loadq", "storeq", "loadd", "stored", "lshiftp", "lshiftq", "negp", "negq", "rshiftp", "rshiftq",
                "urshiftp", "urshiftq", "addp", "addq", "mulp", "mulq", "andp", "andq", "orp", "orq", "subp", "subq", "xorp", "xorq", "addd",
                "divd", "subd", "muld", "sqrtd", /^bp/, /^bq/, /^btp/, /^btq/, /^cp/, /^cq/, /^tp/, /^tq/, /^bd/,
                "jmp", "call", "leap", "leaq"
                size = 8
            else
                raise "Bad instruction #{node.opcode} for heap access at #{node.codeOriginString}"
            end
            
            if address.is_a? BaseIndex
                address.offset.value == 0 and
                    (node.opcode =~ /^lea/ or address.scale == 1 or address.scale == size)
            elsif address.is_a? Address
                (-255..4095).include? address.offset.value
            else
                false
            end
        }
        result = riscLowerMisplacedImmediates(result, ["storeb", "storei", "storep", "storeq"])
        result = riscLowerMalformedImmediates(result, 0..4095)
        result = riscLowerMisplacedAddresses(result)
        result = riscLowerMalformedAddresses(result) {
            | node, address |
            case node.opcode
            when /^load/
                true
            when /^store/
                not (address.is_a? Address and address.offset.value < 0)
            when /^lea/
                true
            else
                raise "Bad instruction #{node.opcode} for heap access at #{node.codeOriginString}"
            end
        }
        result = riscLowerTest(result)
        result = assignRegistersToTemporaries(result, :gpr, ARM64_EXTRA_GPRS)
        result = assignRegistersToTemporaries(result, :fpr, ARM64_EXTRA_FPRS)
        result = arm64CortexA53Fix835769(result)
        return result
    end
end

def arm64Operands(operands, kinds)
    if kinds.is_a? Array
        raise "Mismatched operand lists: #{operands.inspect} and #{kinds.inspect}" if operands.size != kinds.size
    else
        kinds = operands.map{ kinds }
    end
    (0...operands.size).map {
        | index |
        operands[index].arm64Operand(kinds[index])
    }.join(', ')
end

def arm64FlippedOperands(operands, kinds)
    if kinds.is_a? Array
        kinds = [kinds[-1]] + kinds[0..-2]
    end
    arm64Operands([operands[-1]] + operands[0..-2], kinds)
end

# TAC = three address code.
def arm64TACOperands(operands, kind)
    if operands.size == 3
        return arm64FlippedOperands(operands, kind)
    end
    
    raise unless operands.size == 2
    
    return operands[1].arm64Operand(kind) + ", " + arm64FlippedOperands(operands, kind)
end

def emitARM64Add(opcode, operands, kind)
    if operands.size == 3
        raise unless operands[1].register?
        raise unless operands[2].register?
        
        if operands[0].immediate?
            if operands[0].value == 0 and flag !~ /s$/
                unless operands[1] == operands[2]
                    $asm.puts "mov #{arm64FlippedOperands(operands[1..2], kind)}"
                end
            else
                $asm.puts "#{opcode} #{arm64Operands(operands.reverse, kind)}"
            end
            return
        end
        
        raise unless operands[0].register?
        $asm.puts "#{opcode} #{arm64FlippedOperands(operands, kind)}"
        return
    end
    
    raise unless operands.size == 2
    
    if operands[0].immediate? and operands[0].value == 0 and opcode !~ /s$/
        return
    end
    
    $asm.puts "#{opcode} #{arm64TACOperands(operands, kind)}"
end

def emitARM64Unflipped(opcode, operands, kind)
    $asm.puts "#{opcode} #{arm64Operands(operands, kind)}"
end

def emitARM64TAC(opcode, operands, kind)
    $asm.puts "#{opcode} #{arm64TACOperands(operands, kind)}"
end

def emitARM64(opcode, operands, kind)
    $asm.puts "#{opcode} #{arm64FlippedOperands(operands, kind)}"
end

def emitARM64Access(opcode, opcodeNegativeOffset, register, memory, kind)
    if memory.is_a? Address and memory.offset.value < 0
        $asm.puts "#{opcodeNegativeOffset} #{register.arm64Operand(kind)}, #{memory.arm64Operand(kind)}"
        return
    end
    
    $asm.puts "#{opcode} #{register.arm64Operand(kind)}, #{memory.arm64Operand(kind)}"
end

def emitARM64Shift(opcodeRegs, opcodeImmediate, operands, kind)
    if operands.size == 3 and operands[1].immediate?
        magicNumbers = yield operands[1].value
        $asm.puts "#{opcodeImmediate} #{operands[2].arm64Operand(kind)}, #{operands[0].arm64Operand(kind)}, \##{magicNumbers[0]}, \##{magicNumbers[1]}"
        return
    end
    
    if operands.size == 2 and operands[0].immediate?
        magicNumbers = yield operands[0].value
        $asm.puts "#{opcodeImmediate} #{operands[1].arm64Operand(kind)}, #{operands[1].arm64Operand(kind)}, \##{magicNumbers[0]}, \##{magicNumbers[1]}"
        return
    end
    
    emitARM64TAC(opcodeRegs, operands, kind)
end

def emitARM64Branch(opcode, operands, kind, branchOpcode)
    emitARM64Unflipped(opcode, operands[0..-2], kind)
    $asm.puts "#{branchOpcode} #{operands[-1].asmLabel}"
end

def emitARM64Compare(operands, kind, compareCode)
    emitARM64Unflipped("subs #{arm64GPRName('xzr', kind)}, ", operands[0..-2], kind)
    $asm.puts "csinc #{operands[-1].arm64Operand(:int)}, wzr, wzr, #{compareCode}"
end

def emitARM64MoveImmediate(value, target)
    first = true
    isNegative = value < 0
    [48, 32, 16, 0].each {
        | shift |
        currentValue = (value >> shift) & 0xffff
        next if currentValue == (isNegative ? 0xffff : 0) and (shift != 0 or !first)
        if first
            if isNegative
                $asm.puts "movn #{target.arm64Operand(:ptr)}, \##{(~currentValue) & 0xffff}, lsl \##{shift}"
            else
                $asm.puts "movz #{target.arm64Operand(:ptr)}, \##{currentValue}, lsl \##{shift}"
            end
            first = false
        else
            $asm.puts "movk #{target.arm64Operand(:ptr)}, \##{currentValue}, lsl \##{shift}"
        end
    }
end

class Instruction
    def lowerARM64
        $asm.comment codeOriginString
        $asm.annotation annotation if $enableInstrAnnotations
        $asm.debugAnnotation codeOrigin.debugDirective if $enableDebugAnnotations

        case opcode
        when 'addi'
            emitARM64Add("add", operands, :int)
        when 'addis'
            emitARM64Add("adds", operands, :int)
        when 'addp'
            emitARM64Add("add", operands, :ptr)
        when 'addps'
            emitARM64Add("adds", operands, :ptr)
        when 'addq'
            emitARM64Add("add", operands, :ptr)
        when "andi"
            emitARM64TAC("and", operands, :int)
        when "andp"
            emitARM64TAC("and", operands, :ptr)
        when "andq"
            emitARM64TAC("and", operands, :ptr)
        when "ori"
            emitARM64TAC("orr", operands, :int)
        when "orp"
            emitARM64TAC("orr", operands, :ptr)
        when "orq"
            emitARM64TAC("orr", operands, :ptr)
        when "xori"
            emitARM64TAC("eor", operands, :int)
        when "xorp"
            emitARM64TAC("eor", operands, :ptr)
        when "xorq"
            emitARM64TAC("eor", operands, :ptr)
        when "lshifti"
            emitARM64Shift("lslv", "ubfm", operands, :int) {
                | value |
                [32 - value, 31 - value]
            }
        when "lshiftp"
            emitARM64Shift("lslv", "ubfm", operands, :ptr) {
                | value |
                [64 - value, 63 - value]
            }
        when "lshiftq"
            emitARM64Shift("lslv", "ubfm", operands, :ptr) {
                | value |
                [64 - value, 63 - value]
            }
        when "rshifti"
            emitARM64Shift("asrv", "sbfm", operands, :int) {
                | value |
                [value, 31]
            }
        when "rshiftp"
            emitARM64Shift("asrv", "sbfm", operands, :ptr) {
                | value |
                [value, 63]
            }
        when "rshiftq"
            emitARM64Shift("asrv", "sbfm", operands, :ptr) {
                | value |
                [value, 63]
            }
        when "urshifti"
            emitARM64Shift("lsrv", "ubfm", operands, :int) {
                | value |
                [value, 31]
            }
        when "urshiftp"
            emitARM64Shift("lsrv", "ubfm", operands, :ptr) {
                | value |
                [value, 63]
            }
        when "urshiftq"
            emitARM64Shift("lsrv", "ubfm", operands, :ptr) {
                | value |
                [value, 63]
            }
        when "muli"
            $asm.puts "madd #{arm64TACOperands(operands, :int)}, wzr"
        when "mulp"
            $asm.puts "madd #{arm64TACOperands(operands, :ptr)}, xzr"
        when "mulq"
            $asm.puts "madd #{arm64TACOperands(operands, :ptr)}, xzr"
        when "subi"
            emitARM64TAC("sub", operands, :int)
        when "subp"
            emitARM64TAC("sub", operands, :ptr)
        when "subq"
            emitARM64TAC("sub", operands, :ptr)
        when "subis"
            emitARM64TAC("subs", operands, :int)
        when "negi"
            $asm.puts "sub #{operands[0].arm64Operand(:int)}, wzr, #{operands[0].arm64Operand(:int)}"
        when "negp"
            $asm.puts "sub #{operands[0].arm64Operand(:ptr)}, xzr, #{operands[0].arm64Operand(:ptr)}"
        when "negq"
            $asm.puts "sub #{operands[0].arm64Operand(:ptr)}, xzr, #{operands[0].arm64Operand(:ptr)}"
        when "loadi"
            emitARM64Access("ldr", "ldur", operands[1], operands[0], :int)
        when "loadis"
            emitARM64Access("ldrsw", "ldursw", operands[1], operands[0], :ptr)
        when "loadp"
            emitARM64Access("ldr", "ldur", operands[1], operands[0], :ptr)
        when "loadq"
            emitARM64Access("ldr", "ldur", operands[1], operands[0], :ptr)
        when "storei"
            emitARM64Unflipped("str", operands, :int)
        when "storep"
            emitARM64Unflipped("str", operands, :ptr)
        when "storeq"
            emitARM64Unflipped("str", operands, :ptr)
        when "loadb"
            emitARM64Access("ldrb", "ldurb", operands[1], operands[0], :int)
        when "loadbs"
            emitARM64Access("ldrsb", "ldursb", operands[1], operands[0], :int)
        when "storeb"
            emitARM64Unflipped("strb", operands, :int)
        when "loadh"
            emitARM64Access("ldrh", "ldurh", operands[1], operands[0], :int)
        when "loadhs"
            emitARM64Access("ldrsh", "ldursh", operands[1], operands[0], :int)
        when "storeh"
            emitARM64Unflipped("strh", operands, :int)
        when "loadd"
            emitARM64Access("ldr", "ldur", operands[1], operands[0], :double)
        when "stored"
            emitARM64Unflipped("str", operands, :double)
        when "addd"
            emitARM64TAC("fadd", operands, :double)
        when "divd"
            emitARM64TAC("fdiv", operands, :double)
        when "subd"
            emitARM64TAC("fsub", operands, :double)
        when "muld"
            emitARM64TAC("fmul", operands, :double)
        when "sqrtd"
            emitARM64("fsqrt", operands, :double)
        when "ci2d"
            emitARM64("scvtf", operands, [:int, :double])
        when "bdeq"
            emitARM64Branch("fcmp", operands, :double, "b.eq")
        when "bdneq"
            emitARM64Unflipped("fcmp", operands[0..1], :double)
            isUnordered = LocalLabel.unique("bdneq")
            $asm.puts "b.vs #{LocalLabelReference.new(codeOrigin, isUnordered).asmLabel}"
            $asm.puts "b.ne #{operands[2].asmLabel}"
            isUnordered.lower("ARM64")
        when "bdgt"
            emitARM64Branch("fcmp", operands, :double, "b.gt")
        when "bdgteq"
            emitARM64Branch("fcmp", operands, :double, "b.ge")
        when "bdlt"
            emitARM64Branch("fcmp", operands, :double, "b.mi")
        when "bdlteq"
            emitARM64Branch("fcmp", operands, :double, "b.ls")
        when "bdequn"
            emitARM64Unflipped("fcmp", operands[0..1], :double)
            $asm.puts "b.vs #{operands[2].asmLabel}"
            $asm.puts "b.eq #{operands[2].asmLabel}"
        when "bdnequn"
            emitARM64Branch("fcmp", operands, :double, "b.ne")
        when "bdgtun"
            emitARM64Branch("fcmp", operands, :double, "b.hi")
        when "bdgtequn"
            emitARM64Branch("fcmp", operands, :double, "b.pl")
        when "bdltun"
            emitARM64Branch("fcmp", operands, :double, "b.lt")
        when "bdltequn"
            emitARM64Branch("fcmp", operands, :double, "b.le")
        when "btd2i"
            # FIXME: May be a good idea to just get rid of this instruction, since the interpreter
            # currently does not use it.
            raise "ARM64 does not support this opcode yet, #{codeOriginString}"
        when "td2i"
            emitARM64("fcvtzs", operands, [:double, :int])
        when "bcd2i"
            # FIXME: Remove this instruction, or use it and implement it. Currently it's not
            # used.
            raise "ARM64 does not support this opcode yet, #{codeOriginString}"
        when "movdz"
            # FIXME: Remove it or support it.
            raise "ARM64 does not support this opcode yet, #{codeOriginString}"
        when "pop"
            operands.each_slice(2) {
                | ops |
                # Note that the operands are in the reverse order of the case for push.
                # This is due to the fact that order matters for pushing and popping, and 
                # on platforms that only push/pop one slot at a time they pop their 
                # arguments in the reverse order that they were pushed. In order to remain 
                # compatible with those platforms we assume here that that's what has been done.

                # So for example, if we did push(A, B, C, D), we would then pop(D, C, B, A).
                # But since the ordering of arguments doesn't change on arm64 between the stp and ldp 
                # instructions we need to flip flop the argument positions that were passed to us.
                $asm.puts "ldp #{ops[1].arm64Operand(:ptr)}, #{ops[0].arm64Operand(:ptr)}, [sp], #16"
            }
        when "push"
            operands.each_slice(2) {
                | ops |
                $asm.puts "stp #{ops[0].arm64Operand(:ptr)}, #{ops[1].arm64Operand(:ptr)}, [sp, #-16]!"
            }
        when "move"
            if operands[0].immediate?
                emitARM64MoveImmediate(operands[0].value, operands[1])
            else
                emitARM64("mov", operands, :ptr)
            end
        when "sxi2p"
            emitARM64("sxtw", operands, [:int, :ptr])
        when "sxi2q"
            emitARM64("sxtw", operands, [:int, :ptr])
        when "zxi2p"
            emitARM64("uxtw", operands, [:int, :ptr])
        when "zxi2q"
            emitARM64("uxtw", operands, [:int, :ptr])
        when "nop"
            $asm.puts "nop"
        when "bieq", "bbeq"
            if operands[0].immediate? and operands[0].value == 0
                $asm.puts "cbz #{operands[1].arm64Operand(:int)}, #{operands[2].asmLabel}"
            elsif operands[1].immediate? and operands[1].value == 0
                $asm.puts "cbz #{operands[0].arm64Operand(:int)}, #{operands[2].asmLabel}"
            else
                emitARM64Branch("subs wzr, ", operands, :int, "b.eq")
            end
        when "bpeq"
            if operands[0].immediate? and operands[0].value == 0
                $asm.puts "cbz #{operands[1].arm64Operand(:ptr)}, #{operands[2].asmLabel}"
            elsif operands[1].immediate? and operands[1].value == 0
                $asm.puts "cbz #{operands[0].arm64Operand(:ptr)}, #{operands[2].asmLabel}"
            else
                emitARM64Branch("subs xzr, ", operands, :ptr, "b.eq")
            end
        when "bqeq"
            if operands[0].immediate? and operands[0].value == 0
                $asm.puts "cbz #{operands[1].arm64Operand(:ptr)}, #{operands[2].asmLabel}"
            elsif operands[1].immediate? and operands[1].value == 0
                $asm.puts "cbz #{operands[0].arm64Operand(:ptr)}, #{operands[2].asmLabel}"
            else
                emitARM64Branch("subs xzr, ", operands, :ptr, "b.eq")
            end
        when "bineq", "bbneq"
            if operands[0].immediate? and operands[0].value == 0
                $asm.puts "cbnz #{operands[1].arm64Operand(:int)}, #{operands[2].asmLabel}"
            elsif operands[1].immediate? and operands[1].value == 0
                $asm.puts "cbnz #{operands[0].arm64Operand(:int)}, #{operands[2].asmLabel}"
            else
                emitARM64Branch("subs wzr, ", operands, :int, "b.ne")
            end
        when "bpneq"
            if operands[0].immediate? and operands[0].value == 0
                $asm.puts "cbnz #{operands[1].arm64Operand(:ptr)}, #{operands[2].asmLabel}"
            elsif operands[1].immediate? and operands[1].value == 0
                $asm.puts "cbnz #{operands[0].arm64Operand(:ptr)}, #{operands[2].asmLabel}"
            else
                emitARM64Branch("subs xzr, ", operands, :ptr, "b.ne")
            end
        when "bqneq"
            if operands[0].immediate? and operands[0].value == 0
                $asm.puts "cbnz #{operands[1].arm64Operand(:ptr)}, #{operands[2].asmLabel}"
            elsif operands[1].immediate? and operands[1].value == 0
                $asm.puts "cbnz #{operands[0].arm64Operand(:ptr)}, #{operands[2].asmLabel}"
            else
                emitARM64Branch("subs xzr, ", operands, :ptr, "b.ne")
            end
        when "bia", "bba"
            emitARM64Branch("subs wzr, ", operands, :int, "b.hi")
        when "bpa"
            emitARM64Branch("subs xzr, ", operands, :ptr, "b.hi")
        when "bqa"
            emitARM64Branch("subs xzr, ", operands, :ptr, "b.hi")
        when "biaeq", "bbaeq"
            emitARM64Branch("subs wzr, ", operands, :int, "b.hs")
        when "bpaeq"
            emitARM64Branch("subs xzr, ", operands, :ptr, "b.hs")
        when "bqaeq"
            emitARM64Branch("subs xzr, ", operands, :ptr, "b.hs")
        when "bib", "bbb"
            emitARM64Branch("subs wzr, ", operands, :int, "b.lo")
        when "bpb"
            emitARM64Branch("subs xzr, ", operands, :ptr, "b.lo")
        when "bqb"
            emitARM64Branch("subs xzr, ", operands, :ptr, "b.lo")
        when "bibeq", "bbbeq"
            emitARM64Branch("subs wzr, ", operands, :int, "b.ls")
        when "bpbeq"
            emitARM64Branch("subs xzr, ", operands, :ptr, "b.ls")
        when "bqbeq"
            emitARM64Branch("subs xzr, ", operands, :ptr, "b.ls")
        when "bigt", "bbgt"
            emitARM64Branch("subs wzr, ", operands, :int, "b.gt")
        when "bpgt"
            emitARM64Branch("subs xzr, ", operands, :ptr, "b.gt")
        when "bqgt"
            emitARM64Branch("subs xzr, ", operands, :ptr, "b.gt")
        when "bigteq", "bbgteq"
            emitARM64Branch("subs wzr, ", operands, :int, "b.ge")
        when "bpgteq"
            emitARM64Branch("subs xzr, ", operands, :ptr, "b.ge")
        when "bqgteq"
            emitARM64Branch("subs xzr, ", operands, :ptr, "b.ge")
        when "bilt", "bblt"
            emitARM64Branch("subs wzr, ", operands, :int, "b.lt")
        when "bplt"
            emitARM64Branch("subs xzr, ", operands, :ptr, "b.lt")
        when "bqlt"
            emitARM64Branch("subs xzr, ", operands, :ptr, "b.lt")
        when "bilteq", "bblteq"
            emitARM64Branch("subs wzr, ", operands, :int, "b.le")
        when "bplteq"
            emitARM64Branch("subs xzr, ", operands, :ptr, "b.le")
        when "bqlteq"
            emitARM64Branch("subs xzr, ", operands, :ptr, "b.le")
        when "jmp"
            if operands[0].label?
                $asm.puts "b #{operands[0].asmLabel}"
            else
                emitARM64Unflipped("br", operands, :ptr)
            end
        when "call"
            if operands[0].label?
                $asm.puts "bl #{operands[0].asmLabel}"
            else
                emitARM64Unflipped("blr", operands, :ptr)
            end
        when "break"
            $asm.puts "brk \#0"
        when "ret"
            $asm.puts "ret"
        when "cieq", "cbeq"
            emitARM64Compare(operands, :int, "ne")
        when "cpeq"
            emitARM64Compare(operands, :ptr, "ne")
        when "cqeq"
            emitARM64Compare(operands, :ptr, "ne")
        when "cineq", "cbneq"
            emitARM64Compare(operands, :int, "eq")
        when "cpneq"
            emitARM64Compare(operands, :ptr, "eq")
        when "cqneq"
            emitARM64Compare(operands, :ptr, "eq")
        when "cia", "cba"
            emitARM64Compare(operands, :int, "ls")
        when "cpa"
            emitARM64Compare(operands, :ptr, "ls")
        when "cqa"
            emitARM64Compare(operands, :ptr, "ls")
        when "ciaeq", "cbaeq"
            emitARM64Compare(operands, :int, "lo")
        when "cpaeq"
            emitARM64Compare(operands, :ptr, "lo")
        when "cqaeq"
            emitARM64Compare(operands, :ptr, "lo")
        when "cib", "cbb"
            emitARM64Compare(operands, :int, "hs")
        when "cpb"
            emitARM64Compare(operands, :ptr, "hs")
        when "cqb"
            emitARM64Compare(operands, :ptr, "hs")
        when "cibeq", "cbbeq"
            emitARM64Compare(operands, :int, "hi")
        when "cpbeq"
            emitARM64Compare(operands, :ptr, "hi")
        when "cqbeq"
            emitARM64Compare(operands, :ptr, "hi")
        when "cilt", "cblt"
            emitARM64Compare(operands, :int, "ge")
        when "cplt"
            emitARM64Compare(operands, :ptr, "ge")
        when "cqlt"
            emitARM64Compare(operands, :ptr, "ge")
        when "cilteq", "cblteq"
            emitARM64Compare(operands, :int, "gt")
        when "cplteq"
            emitARM64Compare(operands, :ptr, "gt")
        when "cqlteq"
            emitARM64Compare(operands, :ptr, "gt")
        when "cigt", "cbgt"
            emitARM64Compare(operands, :int, "le")
        when "cpgt"
            emitARM64Compare(operands, :ptr, "le")
        when "cqgt"
            emitARM64Compare(operands, :ptr, "le")
        when "cigteq", "cbgteq"
            emitARM64Compare(operands, :int, "lt")
        when "cpgteq"
            emitARM64Compare(operands, :ptr, "lt")
        when "cqgteq"
            emitARM64Compare(operands, :ptr, "lt")
        when "peek"
            $asm.puts "ldr #{operands[1].arm64Operand(:ptr)}, [sp, \##{operands[0].value * 8}]"
        when "poke"
            $asm.puts "str #{operands[1].arm64Operand(:ptr)}, [sp, \##{operands[0].value * 8}]"
        when "fp2d"
            emitARM64("fmov", operands, [:ptr, :double])
        when "fq2d"
            emitARM64("fmov", operands, [:ptr, :double])
        when "fd2p"
            emitARM64("fmov", operands, [:double, :ptr])
        when "fd2q"
            emitARM64("fmov", operands, [:double, :ptr])
        when "bo"
            $asm.puts "b.vs #{operands[0].asmLabel}"
        when "bs"
            $asm.puts "b.mi #{operands[0].asmLabel}"
        when "bz"
            $asm.puts "b.eq #{operands[0].asmLabel}"
        when "bnz"
            $asm.puts "b.ne #{operands[0].asmLabel}"
        when "leai"
            operands[0].arm64EmitLea(operands[1], :int)
        when "leap"
            operands[0].arm64EmitLea(operands[1], :ptr)
        when "leaq"
            operands[0].arm64EmitLea(operands[1], :ptr)
        when "smulli"
            $asm.puts "smaddl #{operands[2].arm64Operand(:ptr)}, #{operands[0].arm64Operand(:int)}, #{operands[1].arm64Operand(:int)}, xzr"
        when "memfence"
            $asm.puts "dmb sy"
        when "pcrtoaddr"
            $asm.puts "adr #{operands[1].arm64Operand(:ptr)}, #{operands[0].value}"
        when "nopCortexA53Fix835769"
            $asm.putStr("#if CPU(ARM64_CORTEXA53)")
            $asm.puts "nop"
            $asm.putStr("#endif")
        else
            lowerDefault
        end
    end
end

