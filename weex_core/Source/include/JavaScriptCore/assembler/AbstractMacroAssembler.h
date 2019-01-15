/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#pragma once

#include "AbortReason.h"
#include "AssemblerBuffer.h"
#include "AssemblerCommon.h"
#include "CPU.h"
#include "CodeLocation.h"
#include "MacroAssemblerCodeRef.h"
#include "MacroAssemblerHelpers.h"
#include "Options.h"
#include <wtf/CryptographicallyRandomNumber.h>
#include <wtf/Noncopyable.h>
#include <wtf/SharedTask.h>
#include <wtf/Vector.h>
#include <wtf/WeakRandom.h>

namespace JSC {

#if ENABLE(ASSEMBLER)

class AllowMacroScratchRegisterUsage;
class DisallowMacroScratchRegisterUsage;
class LinkBuffer;
class Watchpoint;
namespace DFG {
struct OSRExit;
}

class AbstractMacroAssemblerBase {
public:
    enum StatusCondition {
        Success,
        Failure
    };
    
    static StatusCondition invert(StatusCondition condition)
    {
        switch (condition) {
        case Success:
            return Failure;
        case Failure:
            return Success;
        }
        RELEASE_ASSERT_NOT_REACHED();
        return Success;
    }
};

template <class AssemblerType, class MacroAssemblerType>
class AbstractMacroAssembler : public AbstractMacroAssemblerBase {
public:
    typedef AbstractMacroAssembler<AssemblerType, MacroAssemblerType> AbstractMacroAssemblerType;
    typedef AssemblerType AssemblerType_T;

    typedef MacroAssemblerCodePtr CodePtr;
    typedef MacroAssemblerCodeRef CodeRef;

    class Jump;

    typedef typename AssemblerType::RegisterID RegisterID;
    typedef typename AssemblerType::FPRegisterID FPRegisterID;
    
    static constexpr RegisterID firstRegister() { return AssemblerType::firstRegister(); }
    static constexpr RegisterID lastRegister() { return AssemblerType::lastRegister(); }

    static constexpr FPRegisterID firstFPRegister() { return AssemblerType::firstFPRegister(); }
    static constexpr FPRegisterID lastFPRegister() { return AssemblerType::lastFPRegister(); }

    // Section 1: MacroAssembler operand types
    //
    // The following types are used as operands to MacroAssembler operations,
    // describing immediate  and memory operands to the instructions to be planted.

    enum Scale {
        TimesOne,
        TimesTwo,
        TimesFour,
        TimesEight,
    };
    
    static Scale timesPtr()
    {
        if (sizeof(void*) == 4)
            return TimesFour;
        return TimesEight;
    }
    
    struct BaseIndex;
    
    // Address:
    //
    // Describes a simple base-offset address.
    struct Address {
        explicit Address(RegisterID base, int32_t offset = 0)
            : base(base)
            , offset(offset)
        {
        }
        
        Address withOffset(int32_t additionalOffset)
        {
            return Address(base, offset + additionalOffset);
        }
        
        BaseIndex indexedBy(RegisterID index, Scale) const;
        
        RegisterID base;
        int32_t offset;
    };

    struct ExtendedAddress {
        explicit ExtendedAddress(RegisterID base, intptr_t offset = 0)
            : base(base)
            , offset(offset)
        {
        }
        
        RegisterID base;
        intptr_t offset;
    };

    // ImplicitAddress:
    //
    // This class is used for explicit 'load' and 'store' operations
    // (as opposed to situations in which a memory operand is provided
    // to a generic operation, such as an integer arithmetic instruction).
    //
    // In the case of a load (or store) operation we want to permit
    // addresses to be implicitly constructed, e.g. the two calls:
    //
    //     load32(Address(addrReg), destReg);
    //     load32(addrReg, destReg);
    //
    // Are equivalent, and the explicit wrapping of the Address in the former
    // is unnecessary.
    struct ImplicitAddress {
        ImplicitAddress(RegisterID base)
            : base(base)
            , offset(0)
        {
        }

        ImplicitAddress(Address address)
            : base(address.base)
            , offset(address.offset)
        {
        }

        RegisterID base;
        int32_t offset;
    };

    // BaseIndex:
    //
    // Describes a complex addressing mode.
    struct BaseIndex {
        BaseIndex(RegisterID base, RegisterID index, Scale scale, int32_t offset = 0)
            : base(base)
            , index(index)
            , scale(scale)
            , offset(offset)
        {
        }
        
        RegisterID base;
        RegisterID index;
        Scale scale;
        int32_t offset;
        
        BaseIndex withOffset(int32_t additionalOffset)
        {
            return BaseIndex(base, index, scale, offset + additionalOffset);
        }
    };

    // AbsoluteAddress:
    //
    // Describes an memory operand given by a pointer.  For regular load & store
    // operations an unwrapped void* will be used, rather than using this.
    struct AbsoluteAddress {
        explicit AbsoluteAddress(const void* ptr)
            : m_ptr(ptr)
        {
        }

        const void* m_ptr;
    };

    // TrustedImmPtr:
    //
    // A pointer sized immediate operand to an instruction - this is wrapped
    // in a class requiring explicit construction in order to differentiate
    // from pointers used as absolute addresses to memory operations
    struct TrustedImmPtr {
        TrustedImmPtr() { }
        
        explicit TrustedImmPtr(const void* value)
            : m_value(value)
        {
        }
        
        // This is only here so that TrustedImmPtr(0) does not confuse the C++
        // overload handling rules.
        explicit TrustedImmPtr(int value)
            : m_value(0)
        {
            ASSERT_UNUSED(value, !value);
        }

        explicit TrustedImmPtr(size_t value)
            : m_value(reinterpret_cast<void*>(value))
        {
        }

        intptr_t asIntptr()
        {
            return reinterpret_cast<intptr_t>(m_value);
        }

        const void* m_value;
    };

    struct ImmPtr : private TrustedImmPtr
    {
        explicit ImmPtr(const void* value)
            : TrustedImmPtr(value)
        {
        }

        TrustedImmPtr asTrustedImmPtr() { return *this; }
    };

    // TrustedImm32:
    //
    // A 32bit immediate operand to an instruction - this is wrapped in a
    // class requiring explicit construction in order to prevent RegisterIDs
    // (which are implemented as an enum) from accidentally being passed as
    // immediate values.
    struct TrustedImm32 {
        TrustedImm32() { }
        
        explicit TrustedImm32(int32_t value)
            : m_value(value)
        {
        }

#if !CPU(X86_64)
        explicit TrustedImm32(TrustedImmPtr ptr)
            : m_value(ptr.asIntptr())
        {
        }
#endif

        int32_t m_value;
    };


    struct Imm32 : private TrustedImm32 {
        explicit Imm32(int32_t value)
            : TrustedImm32(value)
        {
        }
#if !CPU(X86_64)
        explicit Imm32(TrustedImmPtr ptr)
            : TrustedImm32(ptr)
        {
        }
#endif
        const TrustedImm32& asTrustedImm32() const { return *this; }

    };
    
    // TrustedImm64:
    //
    // A 64bit immediate operand to an instruction - this is wrapped in a
    // class requiring explicit construction in order to prevent RegisterIDs
    // (which are implemented as an enum) from accidentally being passed as
    // immediate values.
    struct TrustedImm64 {
        TrustedImm64() { }
        
        explicit TrustedImm64(int64_t value)
            : m_value(value)
        {
        }

#if CPU(X86_64) || CPU(ARM64)
        explicit TrustedImm64(TrustedImmPtr ptr)
            : m_value(ptr.asIntptr())
        {
        }
#endif

        int64_t m_value;
    };

    struct Imm64 : private TrustedImm64
    {
        explicit Imm64(int64_t value)
            : TrustedImm64(value)
        {
        }
#if CPU(X86_64) || CPU(ARM64)
        explicit Imm64(TrustedImmPtr ptr)
            : TrustedImm64(ptr)
        {
        }
#endif
        const TrustedImm64& asTrustedImm64() const { return *this; }
    };
    
    // Section 2: MacroAssembler code buffer handles
    //
    // The following types are used to reference items in the code buffer
    // during JIT code generation.  For example, the type Jump is used to
    // track the location of a jump instruction so that it may later be
    // linked to a label marking its destination.


    // Label:
    //
    // A Label records a point in the generated instruction stream, typically such that
    // it may be used as a destination for a jump.
    class Label {
        template<class TemplateAssemblerType, class TemplateMacroAssemblerType>
        friend class AbstractMacroAssembler;
        friend struct DFG::OSRExit;
        friend class Jump;
        friend class MacroAssemblerCodeRef;
        friend class LinkBuffer;
        friend class Watchpoint;

    public:
        Label()
        {
        }

        Label(AbstractMacroAssemblerType* masm)
            : m_label(masm->m_assembler.label())
        {
            masm->invalidateAllTempRegisters();
        }

        bool operator==(const Label& other) const { return m_label == other.m_label; }

        bool isSet() const { return m_label.isSet(); }
    private:
        AssemblerLabel m_label;
    };
    
    // ConvertibleLoadLabel:
    //
    // A ConvertibleLoadLabel records a loadPtr instruction that can be patched to an addPtr
    // so that:
    //
    // loadPtr(Address(a, i), b)
    //
    // becomes:
    //
    // addPtr(TrustedImmPtr(i), a, b)
    class ConvertibleLoadLabel {
        template<class TemplateAssemblerType, class TemplateMacroAssemblerType>
        friend class AbstractMacroAssembler;
        friend class LinkBuffer;
        
    public:
        ConvertibleLoadLabel()
        {
        }
        
        ConvertibleLoadLabel(AbstractMacroAssemblerType* masm)
            : m_label(masm->m_assembler.labelIgnoringWatchpoints())
        {
        }
        
        bool isSet() const { return m_label.isSet(); }
    private:
        AssemblerLabel m_label;
    };

    // DataLabelPtr:
    //
    // A DataLabelPtr is used to refer to a location in the code containing a pointer to be
    // patched after the code has been generated.
    class DataLabelPtr {
        template<class TemplateAssemblerType, class TemplateMacroAssemblerType>
        friend class AbstractMacroAssembler;
        friend class LinkBuffer;
    public:
        DataLabelPtr()
        {
        }

        DataLabelPtr(AbstractMacroAssemblerType* masm)
            : m_label(masm->m_assembler.label())
        {
        }

        bool isSet() const { return m_label.isSet(); }
        
    private:
        AssemblerLabel m_label;
    };

    // DataLabel32:
    //
    // A DataLabel32 is used to refer to a location in the code containing a 32-bit constant to be
    // patched after the code has been generated.
    class DataLabel32 {
        template<class TemplateAssemblerType, class TemplateMacroAssemblerType>
        friend class AbstractMacroAssembler;
        friend class LinkBuffer;
    public:
        DataLabel32()
        {
        }

        DataLabel32(AbstractMacroAssemblerType* masm)
            : m_label(masm->m_assembler.label())
        {
        }

        AssemblerLabel label() const { return m_label; }

    private:
        AssemblerLabel m_label;
    };

    // DataLabelCompact:
    //
    // A DataLabelCompact is used to refer to a location in the code containing a
    // compact immediate to be patched after the code has been generated.
    class DataLabelCompact {
        template<class TemplateAssemblerType, class TemplateMacroAssemblerType>
        friend class AbstractMacroAssembler;
        friend class LinkBuffer;
    public:
        DataLabelCompact()
        {
        }
        
        DataLabelCompact(AbstractMacroAssemblerType* masm)
            : m_label(masm->m_assembler.label())
        {
        }

        DataLabelCompact(AssemblerLabel label)
            : m_label(label)
        {
        }

        AssemblerLabel label() const { return m_label; }

    private:
        AssemblerLabel m_label;
    };

    // Call:
    //
    // A Call object is a reference to a call instruction that has been planted
    // into the code buffer - it is typically used to link the call, setting the
    // relative offset such that when executed it will call to the desired
    // destination.
    class Call {
        template<class TemplateAssemblerType, class TemplateMacroAssemblerType>
        friend class AbstractMacroAssembler;

    public:
        enum Flags {
            None = 0x0,
            Linkable = 0x1,
            Near = 0x2,
            Tail = 0x4,
            LinkableNear = 0x3,
            LinkableNearTail = 0x7,
        };

        Call()
            : m_flags(None)
        {
        }
        
        Call(AssemblerLabel jmp, Flags flags)
            : m_label(jmp)
            , m_flags(flags)
        {
        }

        bool isFlagSet(Flags flag)
        {
            return m_flags & flag;
        }

        static Call fromTailJump(Jump jump)
        {
            return Call(jump.m_label, Linkable);
        }

        AssemblerLabel m_label;
    private:
        Flags m_flags;
    };

    // Jump:
    //
    // A jump object is a reference to a jump instruction that has been planted
    // into the code buffer - it is typically used to link the jump, setting the
    // relative offset such that when executed it will jump to the desired
    // destination.
    class Jump {
        template<class TemplateAssemblerType, class TemplateMacroAssemblerType>
        friend class AbstractMacroAssembler;
        friend class Call;
        friend struct DFG::OSRExit;
        friend class LinkBuffer;
    public:
        Jump()
        {
        }
        
#if CPU(ARM_THUMB2)
        // Fixme: this information should be stored in the instruction stream, not in the Jump object.
        Jump(AssemblerLabel jmp, ARMv7Assembler::JumpType type = ARMv7Assembler::JumpNoCondition, ARMv7Assembler::Condition condition = ARMv7Assembler::ConditionInvalid)
            : m_label(jmp)
            , m_type(type)
            , m_condition(condition)
        {
        }
#elif CPU(ARM64)
        Jump(AssemblerLabel jmp, ARM64Assembler::JumpType type = ARM64Assembler::JumpNoCondition, ARM64Assembler::Condition condition = ARM64Assembler::ConditionInvalid)
            : m_label(jmp)
            , m_type(type)
            , m_condition(condition)
        {
        }

        Jump(AssemblerLabel jmp, ARM64Assembler::JumpType type, ARM64Assembler::Condition condition, bool is64Bit, ARM64Assembler::RegisterID compareRegister)
            : m_label(jmp)
            , m_type(type)
            , m_condition(condition)
            , m_is64Bit(is64Bit)
            , m_compareRegister(compareRegister)
        {
            ASSERT((type == ARM64Assembler::JumpCompareAndBranch) || (type == ARM64Assembler::JumpCompareAndBranchFixedSize));
        }

        Jump(AssemblerLabel jmp, ARM64Assembler::JumpType type, ARM64Assembler::Condition condition, unsigned bitNumber, ARM64Assembler::RegisterID compareRegister)
            : m_label(jmp)
            , m_type(type)
            , m_condition(condition)
            , m_bitNumber(bitNumber)
            , m_compareRegister(compareRegister)
        {
            ASSERT((type == ARM64Assembler::JumpTestBit) || (type == ARM64Assembler::JumpTestBitFixedSize));
        }
#else
        Jump(AssemblerLabel jmp)    
            : m_label(jmp)
        {
        }
#endif
        
        Label label() const
        {
            Label result;
            result.m_label = m_label;
            return result;
        }

        void link(AbstractMacroAssemblerType* masm) const
        {
            masm->invalidateAllTempRegisters();

#if ENABLE(DFG_REGISTER_ALLOCATION_VALIDATION)
            masm->checkRegisterAllocationAgainstBranchRange(m_label.m_offset, masm->debugOffset());
#endif

#if CPU(ARM_THUMB2)
            masm->m_assembler.linkJump(m_label, masm->m_assembler.label(), m_type, m_condition);
#elif CPU(ARM64)
            if ((m_type == ARM64Assembler::JumpCompareAndBranch) || (m_type == ARM64Assembler::JumpCompareAndBranchFixedSize))
                masm->m_assembler.linkJump(m_label, masm->m_assembler.label(), m_type, m_condition, m_is64Bit, m_compareRegister);
            else if ((m_type == ARM64Assembler::JumpTestBit) || (m_type == ARM64Assembler::JumpTestBitFixedSize))
                masm->m_assembler.linkJump(m_label, masm->m_assembler.label(), m_type, m_condition, m_bitNumber, m_compareRegister);
            else
                masm->m_assembler.linkJump(m_label, masm->m_assembler.label(), m_type, m_condition);
#else
            masm->m_assembler.linkJump(m_label, masm->m_assembler.label());
#endif
        }
        
        void linkTo(Label label, AbstractMacroAssemblerType* masm) const
        {
#if ENABLE(DFG_REGISTER_ALLOCATION_VALIDATION)
            masm->checkRegisterAllocationAgainstBranchRange(label.m_label.m_offset, m_label.m_offset);
#endif

#if CPU(ARM_THUMB2)
            masm->m_assembler.linkJump(m_label, label.m_label, m_type, m_condition);
#elif CPU(ARM64)
            if ((m_type == ARM64Assembler::JumpCompareAndBranch) || (m_type == ARM64Assembler::JumpCompareAndBranchFixedSize))
                masm->m_assembler.linkJump(m_label, label.m_label, m_type, m_condition, m_is64Bit, m_compareRegister);
            else if ((m_type == ARM64Assembler::JumpTestBit) || (m_type == ARM64Assembler::JumpTestBitFixedSize))
                masm->m_assembler.linkJump(m_label, label.m_label, m_type, m_condition, m_bitNumber, m_compareRegister);
            else
                masm->m_assembler.linkJump(m_label, label.m_label, m_type, m_condition);
#else
            masm->m_assembler.linkJump(m_label, label.m_label);
#endif
        }

        bool isSet() const { return m_label.isSet(); }

    private:
        AssemblerLabel m_label;
#if CPU(ARM_THUMB2)
        ARMv7Assembler::JumpType m_type;
        ARMv7Assembler::Condition m_condition;
#elif CPU(ARM64)
        ARM64Assembler::JumpType m_type;
        ARM64Assembler::Condition m_condition;
        bool m_is64Bit;
        unsigned m_bitNumber;
        ARM64Assembler::RegisterID m_compareRegister;
#endif
    };

    struct PatchableJump {
        PatchableJump()
        {
        }

        explicit PatchableJump(Jump jump)
            : m_jump(jump)
        {
        }

        operator Jump&() { return m_jump; }

        Jump m_jump;
    };

    // JumpList:
    //
    // A JumpList is a set of Jump objects.
    // All jumps in the set will be linked to the same destination.
    class JumpList {
    public:
        typedef Vector<Jump, 2> JumpVector;
        
        JumpList() { }
        
        JumpList(Jump jump)
        {
            if (jump.isSet())
                append(jump);
        }

        void link(AbstractMacroAssemblerType* masm) const
        {
            size_t size = m_jumps.size();
            for (size_t i = 0; i < size; ++i)
                m_jumps[i].link(masm);
        }
        
        void linkTo(Label label, AbstractMacroAssemblerType* masm) const
        {
            size_t size = m_jumps.size();
            for (size_t i = 0; i < size; ++i)
                m_jumps[i].linkTo(label, masm);
        }
        
        void append(Jump jump)
        {
            m_jumps.append(jump);
        }
        
        void append(const JumpList& other)
        {
            m_jumps.append(other.m_jumps.begin(), other.m_jumps.size());
        }

        bool empty()
        {
            return !m_jumps.size();
        }
        
        void clear()
        {
            m_jumps.clear();
        }
        
        const JumpVector& jumps() const { return m_jumps; }

    private:
        JumpVector m_jumps;
    };


    // Section 3: Misc admin methods
#if ENABLE(DFG_JIT)
    Label labelIgnoringWatchpoints()
    {
        Label result;
        result.m_label = m_assembler.labelIgnoringWatchpoints();
        return result;
    }
#else
    Label labelIgnoringWatchpoints()
    {
        return label();
    }
#endif
    
    Label label()
    {
        return Label(this);
    }
    
    void padBeforePatch()
    {
        // Rely on the fact that asking for a label already does the padding.
        (void)label();
    }
    
    Label watchpointLabel()
    {
        Label result;
        result.m_label = m_assembler.labelForWatchpoint();
        return result;
    }
    
    Label align()
    {
        m_assembler.align(16);
        return Label(this);
    }

#if ENABLE(DFG_REGISTER_ALLOCATION_VALIDATION)
    class RegisterAllocationOffset {
    public:
        RegisterAllocationOffset(unsigned offset)
            : m_offset(offset)
        {
        }

        void checkOffsets(unsigned low, unsigned high)
        {
            RELEASE_ASSERT_WITH_MESSAGE(!(low <= m_offset && m_offset <= high), "Unsafe branch over register allocation at instruction offset %u in jump offset range %u..%u", m_offset, low, high);
        }

    private:
        unsigned m_offset;
    };

    void addRegisterAllocationAtOffset(unsigned offset)
    {
        m_registerAllocationForOffsets.append(RegisterAllocationOffset(offset));
    }

    void clearRegisterAllocationOffsets()
    {
        m_registerAllocationForOffsets.clear();
    }

    void checkRegisterAllocationAgainstBranchRange(unsigned offset1, unsigned offset2)
    {
        if (offset1 > offset2)
            std::swap(offset1, offset2);

        size_t size = m_registerAllocationForOffsets.size();
        for (size_t i = 0; i < size; ++i)
            m_registerAllocationForOffsets[i].checkOffsets(offset1, offset2);
    }
#endif

    template<typename T, typename U>
    static ptrdiff_t differenceBetween(T from, U to)
    {
        return AssemblerType::getDifferenceBetweenLabels(from.m_label, to.m_label);
    }

    static ptrdiff_t differenceBetweenCodePtr(const MacroAssemblerCodePtr& a, const MacroAssemblerCodePtr& b)
    {
        return reinterpret_cast<ptrdiff_t>(b.executableAddress()) - reinterpret_cast<ptrdiff_t>(a.executableAddress());
    }

    unsigned debugOffset() { return m_assembler.debugOffset(); }

    ALWAYS_INLINE static void cacheFlush(void* code, size_t size)
    {
        AssemblerType::cacheFlush(code, size);
    }

#if ENABLE(MASM_PROBE)

    struct CPUState {
        #define DECLARE_REGISTER(_type, _regName) \
            _type _regName;
        FOR_EACH_CPU_REGISTER(DECLARE_REGISTER)
        #undef DECLARE_REGISTER

        static const char* gprName(RegisterID regID)
        {
            switch (regID) {
                #define DECLARE_REGISTER(_type, _regName) \
                case RegisterID::_regName: \
                    return #_regName;
                FOR_EACH_CPU_GPREGISTER(DECLARE_REGISTER)
                #undef DECLARE_REGISTER
            default:
                RELEASE_ASSERT_NOT_REACHED();
            }
        }

        static const char* fprName(FPRegisterID regID)
        {
            switch (regID) {
                #define DECLARE_REGISTER(_type, _regName) \
                case FPRegisterID::_regName: \
                    return #_regName;
                FOR_EACH_CPU_FPREGISTER(DECLARE_REGISTER)
                #undef DECLARE_REGISTER
            default:
                RELEASE_ASSERT_NOT_REACHED();
            }
        }

        void*& gpr(RegisterID regID)
        {
            switch (regID) {
                #define DECLARE_REGISTER(_type, _regName) \
                case RegisterID::_regName: \
                    return _regName;
                FOR_EACH_CPU_GPREGISTER(DECLARE_REGISTER)
                #undef DECLARE_REGISTER
            default:
                RELEASE_ASSERT_NOT_REACHED();
            }
        }

        double& fpr(FPRegisterID regID)
        {
            switch (regID) {
                #define DECLARE_REGISTER(_type, _regName) \
                case FPRegisterID::_regName: \
                    return _regName;
                FOR_EACH_CPU_FPREGISTER(DECLARE_REGISTER)
                #undef DECLARE_REGISTER
            default:
                RELEASE_ASSERT_NOT_REACHED();
            }
        }
    };

    struct ProbeContext;
    typedef void (*ProbeFunction)(struct ProbeContext*);

    struct ProbeContext {
        ProbeFunction probeFunction;
        void* arg1;
        void* arg2;
        CPUState cpu;

        // Convenience methods:
        void*& gpr(RegisterID regID) { return cpu.gpr(regID); }
        double& fpr(FPRegisterID regID) { return cpu.fpr(regID); }
        const char* gprName(RegisterID regID) { return cpu.gprName(regID); }
        const char* fprName(FPRegisterID regID) { return cpu.fprName(regID); }
    };

    // This function emits code to preserve the CPUState (e.g. registers),
    // call a user supplied probe function, and restore the CPUState before
    // continuing with other JIT generated code.
    //
    // The user supplied probe function will be called with a single pointer to
    // a ProbeContext struct (defined above) which contains, among other things,
    // the preserved CPUState. This allows the user probe function to inspect
    // the CPUState at that point in the JIT generated code.
    //
    // If the user probe function alters the register values in the ProbeContext,
    // the altered values will be loaded into the CPU registers when the probe
    // returns.
    //
    // The ProbeContext is stack allocated and is only valid for the duration
    // of the call to the user probe function.
    //
    // Note: probe() should be implemented by the target specific MacroAssembler.
    // This prototype is only provided here to document the interface.

    void probe(ProbeFunction, void* arg1, void* arg2);

#endif // ENABLE(MASM_PROBE)

    AssemblerType m_assembler;
    
    static void linkJump(void* code, Jump jump, CodeLocationLabel target)
    {
        AssemblerType::linkJump(code, jump.m_label, target.dataLocation());
    }

    static void linkPointer(void* code, AssemblerLabel label, void* value)
    {
        AssemblerType::linkPointer(code, label, value);
    }

    static void* getLinkerAddress(void* code, AssemblerLabel label)
    {
        return AssemblerType::getRelocatedAddress(code, label);
    }

    static unsigned getLinkerCallReturnOffset(Call call)
    {
        return AssemblerType::getCallReturnOffset(call.m_label);
    }

    static void repatchJump(CodeLocationJump jump, CodeLocationLabel destination)
    {
        AssemblerType::relinkJump(jump.dataLocation(), destination.dataLocation());
    }
    
    static void repatchJumpToNop(CodeLocationJump jump)
    {
        AssemblerType::relinkJumpToNop(jump.dataLocation());
    }

    static void repatchNearCall(CodeLocationNearCall nearCall, CodeLocationLabel destination)
    {
        switch (nearCall.callMode()) {
        case NearCallMode::Tail:
            AssemblerType::relinkJump(nearCall.dataLocation(), destination.dataLocation());
            return;
        case NearCallMode::Regular:
            AssemblerType::relinkCall(nearCall.dataLocation(), destination.executableAddress());
            return;
        }
        RELEASE_ASSERT_NOT_REACHED();
    }

    static void repatchCompact(CodeLocationDataLabelCompact dataLabelCompact, int32_t value)
    {
        AssemblerType::repatchCompact(dataLabelCompact.dataLocation(), value);
    }
    
    static void repatchInt32(CodeLocationDataLabel32 dataLabel32, int32_t value)
    {
        AssemblerType::repatchInt32(dataLabel32.dataLocation(), value);
    }

    static void repatchPointer(CodeLocationDataLabelPtr dataLabelPtr, void* value)
    {
        AssemblerType::repatchPointer(dataLabelPtr.dataLocation(), value);
    }
    
    static void* readPointer(CodeLocationDataLabelPtr dataLabelPtr)
    {
        return AssemblerType::readPointer(dataLabelPtr.dataLocation());
    }
    
    static void replaceWithLoad(CodeLocationConvertibleLoad label)
    {
        AssemblerType::replaceWithLoad(label.dataLocation());
    }
    
    static void replaceWithAddressComputation(CodeLocationConvertibleLoad label)
    {
        AssemblerType::replaceWithAddressComputation(label.dataLocation());
    }

    template<typename Functor>
    void addLinkTask(const Functor& functor)
    {
        m_linkTasks.append(createSharedTask<void(LinkBuffer&)>(functor));
    }

    void emitNops(size_t memoryToFillWithNopsInBytes)
    {
        AssemblerBuffer& buffer = m_assembler.buffer();
        size_t startCodeSize = buffer.codeSize();
        size_t targetCodeSize = startCodeSize + memoryToFillWithNopsInBytes;
        buffer.ensureSpace(memoryToFillWithNopsInBytes);
        bool isCopyingToExecutableMemory = false;
        AssemblerType::fillNops(static_cast<char*>(buffer.data()) + startCodeSize, memoryToFillWithNopsInBytes, isCopyingToExecutableMemory);
        buffer.setCodeSize(targetCodeSize);
    }

protected:
    AbstractMacroAssembler()
        : m_randomSource(0)
    {
        invalidateAllTempRegisters();
    }

    uint32_t random()
    {
        if (!m_randomSourceIsInitialized) {
            m_randomSourceIsInitialized = true;
            m_randomSource.setSeed(cryptographicallyRandomNumber());
        }
        return m_randomSource.getUint32();
    }

    bool m_randomSourceIsInitialized { false };
    WeakRandom m_randomSource;

#if ENABLE(DFG_REGISTER_ALLOCATION_VALIDATION)
    Vector<RegisterAllocationOffset, 10> m_registerAllocationForOffsets;
#endif

    static bool haveScratchRegisterForBlinding()
    {
        return false;
    }
    static RegisterID scratchRegisterForBlinding()
    {
        UNREACHABLE_FOR_PLATFORM();
        return firstRegister();
    }
    static bool canBlind() { return false; }
    static bool shouldBlindForSpecificArch(uint32_t) { return false; }
    static bool shouldBlindForSpecificArch(uint64_t) { return false; }

    class CachedTempRegister {
        friend class DataLabelPtr;
        friend class DataLabel32;
        friend class DataLabelCompact;
        friend class Jump;
        friend class Label;

    public:
        CachedTempRegister(AbstractMacroAssemblerType* masm, RegisterID registerID)
            : m_masm(masm)
            , m_registerID(registerID)
            , m_value(0)
            , m_validBit(1 << static_cast<unsigned>(registerID))
        {
            ASSERT(static_cast<unsigned>(registerID) < (sizeof(unsigned) * 8));
        }

        ALWAYS_INLINE RegisterID registerIDInvalidate() { invalidate(); return m_registerID; }

        ALWAYS_INLINE RegisterID registerIDNoInvalidate() { return m_registerID; }

        bool value(intptr_t& value)
        {
            value = m_value;
            return m_masm->isTempRegisterValid(m_validBit);
        }

        void setValue(intptr_t value)
        {
            m_value = value;
            m_masm->setTempRegisterValid(m_validBit);
        }

        ALWAYS_INLINE void invalidate() { m_masm->clearTempRegisterValid(m_validBit); }

    private:
        AbstractMacroAssemblerType* m_masm;
        RegisterID m_registerID;
        intptr_t m_value;
        unsigned m_validBit;
    };

    ALWAYS_INLINE void invalidateAllTempRegisters()
    {
        m_tempRegistersValidBits = 0;
    }

    ALWAYS_INLINE bool isTempRegisterValid(unsigned registerMask)
    {
        return (m_tempRegistersValidBits & registerMask);
    }

    ALWAYS_INLINE void clearTempRegisterValid(unsigned registerMask)
    {
        m_tempRegistersValidBits &=  ~registerMask;
    }

    ALWAYS_INLINE void setTempRegisterValid(unsigned registerMask)
    {
        m_tempRegistersValidBits |= registerMask;
    }

    friend class AllowMacroScratchRegisterUsage;
    friend class DisallowMacroScratchRegisterUsage;
    unsigned m_tempRegistersValidBits;
    bool m_allowScratchRegister { true };

    Vector<RefPtr<SharedTask<void(LinkBuffer&)>>> m_linkTasks;

    friend class LinkBuffer;
}; // class AbstractMacroAssembler

template <class AssemblerType, class MacroAssemblerType>
inline typename AbstractMacroAssembler<AssemblerType, MacroAssemblerType>::BaseIndex
AbstractMacroAssembler<AssemblerType, MacroAssemblerType>::Address::indexedBy(
    typename AbstractMacroAssembler<AssemblerType, MacroAssemblerType>::RegisterID index,
    typename AbstractMacroAssembler<AssemblerType, MacroAssemblerType>::Scale scale) const
{
    return BaseIndex(base, index, scale, offset);
}

#endif // ENABLE(ASSEMBLER)

} // namespace JSC

#if ENABLE(ASSEMBLER)

namespace WTF {

class PrintStream;

void printInternal(PrintStream& out, JSC::AbstractMacroAssemblerBase::StatusCondition);

} // namespace WTF

#endif // ENABLE(ASSEMBLER)

