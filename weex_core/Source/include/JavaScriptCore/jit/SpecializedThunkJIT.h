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

#if ENABLE(JIT)

#include "JIT.h"
#include "JITInlines.h"
#include "JSInterfaceJIT.h"
#include "LinkBuffer.h"

namespace JSC {

    class SpecializedThunkJIT : public JSInterfaceJIT {
    public:
        static const int ThisArgument = -1;
        SpecializedThunkJIT(VM* vm, int expectedArgCount)
            : JSInterfaceJIT(vm)
        {
            emitFunctionPrologue();
            emitSaveThenMaterializeTagRegisters();
            // Check that we have the expected number of arguments
            m_failures.append(branch32(NotEqual, payloadFor(CallFrameSlot::argumentCount), TrustedImm32(expectedArgCount + 1)));
        }
        
        explicit SpecializedThunkJIT(VM* vm)
            : JSInterfaceJIT(vm)
        {
            emitFunctionPrologue();
            emitSaveThenMaterializeTagRegisters();
        }
        
        void loadDoubleArgument(int argument, FPRegisterID dst, RegisterID scratch)
        {
            unsigned src = CallFrame::argumentOffset(argument);
            m_failures.append(emitLoadDouble(src, dst, scratch));
        }
        
        void loadCellArgument(int argument, RegisterID dst)
        {
            unsigned src = CallFrame::argumentOffset(argument);
            m_failures.append(emitLoadJSCell(src, dst));
        }
        
        void loadJSStringArgument(VM& vm, int argument, RegisterID dst)
        {
            loadCellArgument(argument, dst);
            m_failures.append(branchStructure(NotEqual, 
                Address(dst, JSCell::structureIDOffset()), 
                vm.stringStructure.get()));
        }
        
        void loadArgumentWithSpecificClass(const ClassInfo* classInfo, int argument, RegisterID dst, RegisterID scratch)
        {
            loadCellArgument(argument, dst);
            emitLoadStructure(dst, scratch, dst);
            appendFailure(branchPtr(NotEqual, Address(scratch, Structure::classInfoOffset()), TrustedImmPtr(classInfo)));
            // We have to reload the argument since emitLoadStructure clobbered it.
            loadCellArgument(argument, dst);
        }
        
        void loadInt32Argument(int argument, RegisterID dst, Jump& failTarget)
        {
            unsigned src = CallFrame::argumentOffset(argument);
            failTarget = emitLoadInt32(src, dst);
        }
        
        void loadInt32Argument(int argument, RegisterID dst)
        {
            Jump conversionFailed;
            loadInt32Argument(argument, dst, conversionFailed);
            m_failures.append(conversionFailed);
        }
        
        void appendFailure(const Jump& failure)
        {
            m_failures.append(failure);
        }
#if USE(JSVALUE64)
        void returnJSValue(RegisterID src)
        {
            if (src != regT0)
                move(src, regT0);
            
            emitRestoreSavedTagRegisters();
            emitFunctionEpilogue();
            ret();
        }
#else
        void returnJSValue(RegisterID payload, RegisterID tag)
        {
            ASSERT_UNUSED(payload, payload == regT0);
            ASSERT_UNUSED(tag, tag == regT1);
            emitRestoreSavedTagRegisters();
            emitFunctionEpilogue();
            ret();
        }
#endif
        
        void returnDouble(FPRegisterID src)
        {
#if USE(JSVALUE64)
            moveDoubleTo64(src, regT0);
            Jump zero = branchTest64(Zero, regT0);
            sub64(tagTypeNumberRegister, regT0);
            Jump done = jump();
            zero.link(this);
            move(tagTypeNumberRegister, regT0);
            done.link(this);
#else
            moveDoubleToInts(src, regT0, regT1);
            Jump lowNonZero = branchTestPtr(NonZero, regT1);
            Jump highNonZero = branchTestPtr(NonZero, regT0);
            move(TrustedImm32(0), regT0);
            move(TrustedImm32(Int32Tag), regT1);
            lowNonZero.link(this);
            highNonZero.link(this);
#endif
            emitRestoreSavedTagRegisters();
            emitFunctionEpilogue();
            ret();
        }

        void returnInt32(RegisterID src)
        {
            if (src != regT0)
                move(src, regT0);
            tagReturnAsInt32();
            emitRestoreSavedTagRegisters();
            emitFunctionEpilogue();
            ret();
        }

        void returnJSCell(RegisterID src)
        {
            if (src != regT0)
                move(src, regT0);
            tagReturnAsJSCell();
            emitRestoreSavedTagRegisters();
            emitFunctionEpilogue();
            ret();
        }
        
        MacroAssemblerCodeRef finalize(MacroAssemblerCodePtr fallback, const char* thunkKind)
        {
            LinkBuffer patchBuffer(*m_vm, *this, GLOBAL_THUNK_ID);
            patchBuffer.link(m_failures, CodeLocationLabel(fallback));
            for (unsigned i = 0; i < m_calls.size(); i++)
                patchBuffer.link(m_calls[i].first, m_calls[i].second);
            return FINALIZE_CODE(patchBuffer, ("Specialized thunk for %s", thunkKind));
        }

        // Assumes that the target function uses fpRegister0 as the first argument
        // and return value. Like any sensible architecture would.
        void callDoubleToDouble(FunctionPtr function)
        {
            m_calls.append(std::make_pair(call(), function));
        }
        
        void callDoubleToDoublePreservingReturn(FunctionPtr function)
        {
            if (!isX86())
                preserveReturnAddressAfterCall(regT3);
            callDoubleToDouble(function);
            if (!isX86())
                restoreReturnAddressBeforeReturn(regT3);
        }

    private:
        void tagReturnAsInt32()
        {
#if USE(JSVALUE64)
            or64(tagTypeNumberRegister, regT0);
#else
            move(TrustedImm32(JSValue::Int32Tag), regT1);
#endif
        }

        void tagReturnAsJSCell()
        {
#if USE(JSVALUE32_64)
            move(TrustedImm32(JSValue::CellTag), regT1);
#endif
        }
        
        MacroAssembler::JumpList m_failures;
        Vector<std::pair<Call, FunctionPtr>> m_calls;
    };

}

#endif // ENABLE(JIT)
