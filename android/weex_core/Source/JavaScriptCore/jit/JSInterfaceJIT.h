/*
 * Copyright (C) 2010, 2016 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "BytecodeConventions.h"
#include "CCallHelpers.h"
#include "FPRInfo.h"
#include "GPRInfo.h"
#include "JITCode.h"
#include "JITOperations.h"
#include "JSCJSValue.h"
#include "JSString.h"
#include "MacroAssembler.h"

#if ENABLE(JIT)

namespace JSC {
    class JSInterfaceJIT : public CCallHelpers, public GPRInfo, public FPRInfo {
    public:
        JSInterfaceJIT(VM* vm, CodeBlock* codeBlock = 0)
            : CCallHelpers(vm, codeBlock)
        {
        }

#if USE(JSVALUE32_64)
        static const unsigned Int32Tag = static_cast<unsigned>(JSValue::Int32Tag);
#else
        static const unsigned Int32Tag = static_cast<unsigned>(TagTypeNumber >> 32);
#endif
        inline Jump emitLoadJSCell(unsigned virtualRegisterIndex, RegisterID payload);
        inline Jump emitLoadInt32(unsigned virtualRegisterIndex, RegisterID dst);
        inline Jump emitLoadDouble(unsigned virtualRegisterIndex, FPRegisterID dst, RegisterID scratch);

#if USE(JSVALUE32_64)
        inline Jump emitJumpIfNotJSCell(unsigned virtualRegisterIndex);
        inline Address tagFor(int index, RegisterID base = callFrameRegister);
#endif

#if USE(JSVALUE64)
        Jump emitJumpIfNotJSCell(RegisterID);
        Jump emitJumpIfNumber(RegisterID);
        Jump emitJumpIfNotNumber(RegisterID);
        void emitTagInt(RegisterID src, RegisterID dest);
#endif

        Jump emitJumpIfNotType(RegisterID baseReg, JSType);

        void emitGetFromCallFrameHeaderPtr(int entry, RegisterID to, RegisterID from = callFrameRegister);
        void emitPutToCallFrameHeader(RegisterID from, int entry);
        void emitPutToCallFrameHeader(void* value, int entry);
        void emitPutCellToCallFrameHeader(RegisterID from, int entry);

        inline Address payloadFor(int index, RegisterID base = callFrameRegister);
        inline Address intPayloadFor(int index, RegisterID base = callFrameRegister);
        inline Address intTagFor(int index, RegisterID base = callFrameRegister);
        inline Address addressFor(int index, RegisterID base = callFrameRegister);
    };

    struct ThunkHelpers {
        static unsigned jsStringLengthOffset() { return OBJECT_OFFSETOF(JSString, m_length); }
        static unsigned jsStringValueOffset() { return OBJECT_OFFSETOF(JSString, m_value); }
    };

#if USE(JSVALUE32_64)
    inline JSInterfaceJIT::Jump JSInterfaceJIT::emitLoadJSCell(unsigned virtualRegisterIndex, RegisterID payload)
    {
        loadPtr(payloadFor(virtualRegisterIndex), payload);
        return emitJumpIfNotJSCell(virtualRegisterIndex);
    }

    inline JSInterfaceJIT::Jump JSInterfaceJIT::emitJumpIfNotJSCell(unsigned virtualRegisterIndex)
    {
        ASSERT(static_cast<int>(virtualRegisterIndex) < FirstConstantRegisterIndex);
        return branch32(NotEqual, tagFor(virtualRegisterIndex), TrustedImm32(JSValue::CellTag));
    }
    
    inline JSInterfaceJIT::Jump JSInterfaceJIT::emitLoadInt32(unsigned virtualRegisterIndex, RegisterID dst)
    {
        ASSERT(static_cast<int>(virtualRegisterIndex) < FirstConstantRegisterIndex);
        loadPtr(payloadFor(virtualRegisterIndex), dst);
        return branch32(NotEqual, tagFor(static_cast<int>(virtualRegisterIndex)), TrustedImm32(JSValue::Int32Tag));
    }
    
    inline JSInterfaceJIT::Address JSInterfaceJIT::tagFor(int virtualRegisterIndex, RegisterID base)
    {
        ASSERT(virtualRegisterIndex < FirstConstantRegisterIndex);
        return Address(base, (static_cast<unsigned>(virtualRegisterIndex) * sizeof(Register)) + OBJECT_OFFSETOF(JSValue, u.asBits.tag));
    }
    
    inline JSInterfaceJIT::Address JSInterfaceJIT::payloadFor(int virtualRegisterIndex, RegisterID base)
    {
        ASSERT(virtualRegisterIndex < FirstConstantRegisterIndex);
        return Address(base, (static_cast<unsigned>(virtualRegisterIndex) * sizeof(Register)) + OBJECT_OFFSETOF(JSValue, u.asBits.payload));
    }

    inline JSInterfaceJIT::Address JSInterfaceJIT::intPayloadFor(int virtualRegisterIndex, RegisterID base)
    {
        return payloadFor(virtualRegisterIndex, base);
    }

    inline JSInterfaceJIT::Address JSInterfaceJIT::intTagFor(int virtualRegisterIndex, RegisterID base)
    {
        return tagFor(virtualRegisterIndex, base);
    }

    inline JSInterfaceJIT::Jump JSInterfaceJIT::emitLoadDouble(unsigned virtualRegisterIndex, FPRegisterID dst, RegisterID scratch)
    {
        ASSERT(static_cast<int>(virtualRegisterIndex) < FirstConstantRegisterIndex);
        loadPtr(tagFor(virtualRegisterIndex), scratch);
        Jump isDouble = branch32(Below, scratch, TrustedImm32(JSValue::LowestTag));
        Jump notInt = branch32(NotEqual, scratch, TrustedImm32(JSValue::Int32Tag));
        loadPtr(payloadFor(virtualRegisterIndex), scratch);
        convertInt32ToDouble(scratch, dst);
        Jump done = jump();
        isDouble.link(this);
        loadDouble(addressFor(virtualRegisterIndex), dst);
        done.link(this);
        return notInt;
    }

#endif

#if USE(JSVALUE64)
    ALWAYS_INLINE JSInterfaceJIT::Jump JSInterfaceJIT::emitJumpIfNotJSCell(RegisterID reg)
    {
        return branchTest64(NonZero, reg, tagMaskRegister);
    }

    ALWAYS_INLINE JSInterfaceJIT::Jump JSInterfaceJIT::emitJumpIfNumber(RegisterID reg)
    {
        return branchTest64(NonZero, reg, tagTypeNumberRegister);
    }
    ALWAYS_INLINE JSInterfaceJIT::Jump JSInterfaceJIT::emitJumpIfNotNumber(RegisterID reg)
    {
        return branchTest64(Zero, reg, tagTypeNumberRegister);
    }
    inline JSInterfaceJIT::Jump JSInterfaceJIT::emitLoadJSCell(unsigned virtualRegisterIndex, RegisterID dst)
    {
        load64(addressFor(virtualRegisterIndex), dst);
        return branchTest64(NonZero, dst, tagMaskRegister);
    }
    
    inline JSInterfaceJIT::Jump JSInterfaceJIT::emitLoadInt32(unsigned virtualRegisterIndex, RegisterID dst)
    {
        load64(addressFor(virtualRegisterIndex), dst);
        Jump result = branch64(Below, dst, tagTypeNumberRegister);
        zeroExtend32ToPtr(dst, dst);
        return result;
    }

    inline JSInterfaceJIT::Jump JSInterfaceJIT::emitLoadDouble(unsigned virtualRegisterIndex, FPRegisterID dst, RegisterID scratch)
    {
        load64(addressFor(virtualRegisterIndex), scratch);
        Jump notNumber = emitJumpIfNotNumber(scratch);
        Jump notInt = branch64(Below, scratch, tagTypeNumberRegister);
        convertInt32ToDouble(scratch, dst);
        Jump done = jump();
        notInt.link(this);
        add64(tagTypeNumberRegister, scratch);
        move64ToDouble(scratch, dst);
        done.link(this);
        return notNumber;
    }

    // operand is int32_t, must have been zero-extended if register is 64-bit.
    ALWAYS_INLINE void JSInterfaceJIT::emitTagInt(RegisterID src, RegisterID dest)
    {
        if (src != dest)
            move(src, dest);
        or64(tagTypeNumberRegister, dest);
    }
#endif

#if USE(JSVALUE64)
    inline JSInterfaceJIT::Address JSInterfaceJIT::payloadFor(int virtualRegisterIndex, RegisterID base)
    {
        ASSERT(virtualRegisterIndex < FirstConstantRegisterIndex);
        return addressFor(virtualRegisterIndex, base);
    }

    inline JSInterfaceJIT::Address JSInterfaceJIT::intPayloadFor(int virtualRegisterIndex, RegisterID base)
    {
        ASSERT(virtualRegisterIndex < FirstConstantRegisterIndex);
        return Address(base, (static_cast<unsigned>(virtualRegisterIndex) * sizeof(Register)) + OBJECT_OFFSETOF(EncodedValueDescriptor, asBits.payload));
    }
    inline JSInterfaceJIT::Address JSInterfaceJIT::intTagFor(int virtualRegisterIndex, RegisterID base)
    {
        ASSERT(virtualRegisterIndex < FirstConstantRegisterIndex);
        return Address(base, (static_cast<unsigned>(virtualRegisterIndex) * sizeof(Register)) + OBJECT_OFFSETOF(EncodedValueDescriptor, asBits.tag));
    }
#endif

    ALWAYS_INLINE JSInterfaceJIT::Jump JSInterfaceJIT::emitJumpIfNotType(RegisterID baseReg, JSType type)
    {
        return branch8(NotEqual, Address(baseReg, JSCell::typeInfoTypeOffset()), TrustedImm32(type));
    }

    ALWAYS_INLINE void JSInterfaceJIT::emitGetFromCallFrameHeaderPtr(int entry, RegisterID to, RegisterID from)
    {
        loadPtr(Address(from, entry * sizeof(Register)), to);
    }

    ALWAYS_INLINE void JSInterfaceJIT::emitPutToCallFrameHeader(RegisterID from, int entry)
    {
#if USE(JSVALUE32_64)
        storePtr(from, payloadFor(entry, callFrameRegister));
#else
        store64(from, addressFor(entry, callFrameRegister));
#endif
    }

    ALWAYS_INLINE void JSInterfaceJIT::emitPutToCallFrameHeader(void* value, int entry)
    {
        storePtr(TrustedImmPtr(value), Address(callFrameRegister, entry * sizeof(Register)));
    }

    ALWAYS_INLINE void JSInterfaceJIT::emitPutCellToCallFrameHeader(RegisterID from, int entry)
    {
#if USE(JSVALUE32_64)
        store32(TrustedImm32(JSValue::CellTag), tagFor(entry, callFrameRegister));
        store32(from, payloadFor(entry, callFrameRegister));
#else
        store64(from, addressFor(entry, callFrameRegister));
#endif
    }

    inline JSInterfaceJIT::Address JSInterfaceJIT::addressFor(int virtualRegisterIndex, RegisterID base)
    {
        ASSERT(virtualRegisterIndex < FirstConstantRegisterIndex);
        return Address(base, (static_cast<unsigned>(virtualRegisterIndex) * sizeof(Register)));
    }

} // namespace JSC

#endif // ENABLE(JIT)
