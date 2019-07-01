/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#if ENABLE(MASM_PROBE)

#include "MacroAssembler.h"

namespace JSC {

// What is MacroAssembler::print()?
// ===============================
// The MacroAsssembler::print() makes it easy to add print logging
// from JIT compiled code, and can be used to print all types of values
// at runtime e.g. CPU register values being operated on by the compiled
// code.
//
// print() is built on top of MacroAsssembler::probe(), and hence
// inserting logging in JIT compiled code will not perturb register values.
// The only register value that is perturbed is the PC (program counter)
// since there is now more compiled code to do the printing.
//
// How to use the MacroAssembler print()?
// =====================================
// 1. #include "MacroAssemblerPrinter.h" in the JIT file where you want to use print().
//
// 2. Add print() calls like these in your JIT code:
//
//      jit.print("Hello world\n"); // Emits code to print the string.
//
//      CodeBlock* cb = ...;
//      jit.print(cb, "\n");        // Emits code to print the pointer value.
//
//      RegisterID regID = ...;
//      jit.print(regID, "\n");     // Emits code to print the register value (not the id).
//
//      // Emits code to print all registers. Unlike other items, this prints
//      // multiple lines as follows:
//      //      cpu {
//      //          eax: 0x123456789
//      //          ebx: 0x000000abc
//      //          ...
//      //      }
//      jit.print(AllRegisters());
//
//      jit.print(MemWord<uint8_t>(regID), "\n");   // Emits code to print a byte pointed to by the register.
//      jit.print(MemWord<uint32_t>(regID), "\n");  // Emits code to print a 32-bit word pointed to by the register.
//
//      jit.print(MemWord<uint8_t>(Address(regID, 23), "\n");     // Emits code to print a byte at the address.
//      jit.print(MemWord<intptr_t>(AbsoluteAddress(&cb), "\n");  // Emits code to print an intptr_t sized word at the address.
//
//      jit.print(Memory(reg, 100), "\n");              // Emits code to print a 100 bytes at the address pointed by the register.
//      jit.print(Memory(Address(reg, 4), 100), "\n");  // Emits code to print a 100 bytes at the address.
//
//      // Print multiple things at once. This incurs the probe overhead only once
//      // to print all the items.
//      jit.print("cb:", cb, " regID:", regID, " cpu:\n", AllRegisters());
//
//   The type of values that can be printed is encapsulated in the PrintArg struct below.
//
//   Note: print() does not automatically insert a '\n' at the end of the line.
//   If you want a '\n', you'll have to add it explicitly (as in the examples above).


// This is a marker type only used with MacroAssemblerPrinter::print().
// See MacroAssemblerPrinter::print() below for details.
struct AllRegisters { };
struct PCRegister { };

struct Memory {
    using Address = MacroAssembler::Address;
    using AbsoluteAddress = MacroAssembler::AbsoluteAddress;
    using RegisterID = MacroAssembler::RegisterID;

    enum class AddressType {
        Address,
        AbsoluteAddress,
    };

    enum DumpStyle {
        SingleWordDump,
        GenericDump,
    };

    Memory(RegisterID& reg, size_t bytes, DumpStyle style = GenericDump)
        : addressType(AddressType::Address)
        , dumpStyle(style)
        , numBytes(bytes)
    {
        u.address = Address(reg, 0);
    }

    Memory(const Address& address, size_t bytes, DumpStyle style = GenericDump)
        : addressType(AddressType::Address)
        , dumpStyle(style)
        , numBytes(bytes)
    {
        u.address = address;
    }

    Memory(const AbsoluteAddress& address, size_t bytes, DumpStyle style = GenericDump)
        : addressType(AddressType::AbsoluteAddress)
        , dumpStyle(style)
        , numBytes(bytes)
    {
        u.absoluteAddress = address;
    }

    AddressType addressType;
    DumpStyle dumpStyle;
    size_t numBytes;
    union UnionedAddress {
        UnionedAddress() { }

        Address address;
        AbsoluteAddress absoluteAddress;
    } u;
};

template <typename IntType>
struct MemWord : public Memory {
    MemWord(RegisterID& reg)
        : Memory(reg, sizeof(IntType), Memory::SingleWordDump)
    { }

    MemWord(const Address& address)
        : Memory(address, sizeof(IntType), Memory::SingleWordDump)
    { }

    MemWord(const AbsoluteAddress& address)
        : Memory(address, sizeof(IntType), Memory::SingleWordDump)
    { }
};


class MacroAssemblerPrinter {
    using CPUState = MacroAssembler::CPUState;
    using ProbeContext = MacroAssembler::ProbeContext;
    using RegisterID = MacroAssembler::RegisterID;
    using FPRegisterID = MacroAssembler::FPRegisterID;
    
public:
    template<typename... Arguments>
    static void print(MacroAssembler* masm, Arguments... args)
    {
        auto argsList = std::make_unique<PrintArgsList>();
        appendPrintArg(argsList.get(), args...);
        masm->probe(printCallback, argsList.release(), 0);
    }
    
private:
    struct PrintArg {

        enum class Type {
            AllRegisters,
            PCRegister,
            RegisterID,
            FPRegisterID,
            Memory,
            ConstCharPtr,
            ConstVoidPtr,
            IntptrValue,
            UintptrValue,
        };
        
        PrintArg(AllRegisters&)
            : type(Type::AllRegisters)
        {
        }
        
        PrintArg(PCRegister&)
            : type(Type::PCRegister)
        {
        }
        
        PrintArg(RegisterID regID)
            : type(Type::RegisterID)
        {
            u.gpRegisterID = regID;
        }
        
        PrintArg(FPRegisterID regID)
            : type(Type::FPRegisterID)
        {
            u.fpRegisterID = regID;
        }

        PrintArg(const Memory& memory)
            : type(Type::Memory)
        {
            u.memory = memory;
        }

        PrintArg(const char* ptr)
            : type(Type::ConstCharPtr)
        {
            u.constCharPtr = ptr;
        }
        
        PrintArg(const void* ptr)
            : type(Type::ConstVoidPtr)
        {
            u.constVoidPtr = ptr;
        }
        
        PrintArg(int value)
            : type(Type::IntptrValue)
        {
            u.intptrValue = value;
        }
        
        PrintArg(unsigned value)
            : type(Type::UintptrValue)
        {
            u.intptrValue = value;
        }
        
        PrintArg(intptr_t value)
            : type(Type::IntptrValue)
        {
            u.intptrValue = value;
        }
        
        PrintArg(uintptr_t value)
            : type(Type::UintptrValue)
        {
            u.uintptrValue = value;
        }
        
        Type type;
        union Value {
            Value() { }

            RegisterID gpRegisterID;
            FPRegisterID fpRegisterID;
            Memory memory;
            const char* constCharPtr;
            const void* constVoidPtr;
            intptr_t intptrValue;
            uintptr_t uintptrValue;
        } u;
    };

    typedef Vector<PrintArg> PrintArgsList;
    
    template<typename FirstArg, typename... Arguments>
    static void appendPrintArg(PrintArgsList* argsList, FirstArg& firstArg, Arguments... otherArgs)
    {
        argsList->append(PrintArg(firstArg));
        appendPrintArg(argsList, otherArgs...);
    }
    
    static void appendPrintArg(PrintArgsList*) { }

private:
    static void printCallback(ProbeContext*);
};

template<typename... Arguments>
void MacroAssembler::print(Arguments... args)
{
    MacroAssemblerPrinter::print(this, args...);
}


// These printers will print a block of information. That block may be
// indented with the specified indentation.
void printCPU(MacroAssembler::CPUState&, int indentation = 0);
void printCPURegisters(MacroAssembler::CPUState&, int indentation = 0);

// These printers will print the specified information in line in the
// print stream. Hence, no indentation will be applied.
void printRegister(MacroAssembler::CPUState&, MacroAssembler::RegisterID);
void printRegister(MacroAssembler::CPUState&, MacroAssembler::FPRegisterID);
void printMemory(MacroAssembler::CPUState&, const Memory&);

} // namespace JSC

#endif // ENABLE(MASM_PROBE)
