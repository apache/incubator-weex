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

#include "ExecutableAllocator.h"
#include <wtf/DataLog.h>
#include <wtf/PrintStream.h>
#include <wtf/RefPtr.h>
#include <wtf/text/CString.h>

// ASSERT_VALID_CODE_POINTER checks that ptr is a non-null pointer, and that it is a valid
// instruction address on the platform (for example, check any alignment requirements).
#if CPU(ARM_THUMB2) && ENABLE(JIT)
// ARM instructions must be 16-bit aligned. Thumb2 code pointers to be loaded into
// into the processor are decorated with the bottom bit set, while traditional ARM has
// the lower bit clear. Since we don't know what kind of pointer, we check for both
// decorated and undecorated null.
#define ASSERT_VALID_CODE_POINTER(ptr) \
    ASSERT(reinterpret_cast<intptr_t>(ptr) & ~1)
#define ASSERT_VALID_CODE_OFFSET(offset) \
    ASSERT(!(offset & 1)) // Must be multiple of 2.
#else
#define ASSERT_VALID_CODE_POINTER(ptr) \
    ASSERT(ptr)
#define ASSERT_VALID_CODE_OFFSET(offset) // Anything goes!
#endif

namespace JSC {

enum OpcodeID : unsigned;

// FunctionPtr:
//
// FunctionPtr should be used to wrap pointers to C/C++ functions in JSC
// (particularly, the stub functions).
class FunctionPtr {
public:
    FunctionPtr()
        : m_value(0)
    {
    }

    template<typename returnType>
    FunctionPtr(returnType(*value)())
        : m_value((void*)value)
    {
        ASSERT_VALID_CODE_POINTER(m_value);
    }

    template<typename returnType, typename argType1>
    FunctionPtr(returnType(*value)(argType1))
        : m_value((void*)value)
    {
        ASSERT_VALID_CODE_POINTER(m_value);
    }

    template<typename returnType, typename argType1, typename argType2>
    FunctionPtr(returnType(*value)(argType1, argType2))
        : m_value((void*)value)
    {
        ASSERT_VALID_CODE_POINTER(m_value);
    }

    template<typename returnType, typename argType1, typename argType2, typename argType3>
    FunctionPtr(returnType(*value)(argType1, argType2, argType3))
        : m_value((void*)value)
    {
        ASSERT_VALID_CODE_POINTER(m_value);
    }

    template<typename returnType, typename argType1, typename argType2, typename argType3, typename argType4>
    FunctionPtr(returnType(*value)(argType1, argType2, argType3, argType4))
        : m_value((void*)value)
    {
        ASSERT_VALID_CODE_POINTER(m_value);
    }

    template<typename returnType, typename argType1, typename argType2, typename argType3, typename argType4, typename argType5>
    FunctionPtr(returnType(*value)(argType1, argType2, argType3, argType4, argType5))
        : m_value((void*)value)
    {
        ASSERT_VALID_CODE_POINTER(m_value);
    }

    template<typename returnType, typename argType1, typename argType2, typename argType3, typename argType4, typename argType5, typename argType6>
    FunctionPtr(returnType(*value)(argType1, argType2, argType3, argType4, argType5, argType6))
        : m_value((void*)value)
    {
        ASSERT_VALID_CODE_POINTER(m_value);
    }
// MSVC doesn't seem to treat functions with different calling conventions as
// different types; these methods already defined for fastcall, below.
#if CALLING_CONVENTION_IS_STDCALL && !OS(WINDOWS)

    template<typename returnType>
    FunctionPtr(returnType (CDECL *value)())
        : m_value((void*)value)
    {
        ASSERT_VALID_CODE_POINTER(m_value);
    }

    template<typename returnType, typename argType1>
    FunctionPtr(returnType (CDECL *value)(argType1))
        : m_value((void*)value)
    {
        ASSERT_VALID_CODE_POINTER(m_value);
    }

    template<typename returnType, typename argType1, typename argType2>
    FunctionPtr(returnType (CDECL *value)(argType1, argType2))
        : m_value((void*)value)
    {
        ASSERT_VALID_CODE_POINTER(m_value);
    }

    template<typename returnType, typename argType1, typename argType2, typename argType3>
    FunctionPtr(returnType (CDECL *value)(argType1, argType2, argType3))
        : m_value((void*)value)
    {
        ASSERT_VALID_CODE_POINTER(m_value);
    }

    template<typename returnType, typename argType1, typename argType2, typename argType3, typename argType4>
    FunctionPtr(returnType (CDECL *value)(argType1, argType2, argType3, argType4))
        : m_value((void*)value)
    {
        ASSERT_VALID_CODE_POINTER(m_value);
    }
#endif

#if COMPILER_SUPPORTS(FASTCALL_CALLING_CONVENTION)

    template<typename returnType>
    FunctionPtr(returnType (FASTCALL *value)())
        : m_value((void*)value)
    {
        ASSERT_VALID_CODE_POINTER(m_value);
    }

    template<typename returnType, typename argType1>
    FunctionPtr(returnType (FASTCALL *value)(argType1))
        : m_value((void*)value)
    {
        ASSERT_VALID_CODE_POINTER(m_value);
    }

    template<typename returnType, typename argType1, typename argType2>
    FunctionPtr(returnType (FASTCALL *value)(argType1, argType2))
        : m_value((void*)value)
    {
        ASSERT_VALID_CODE_POINTER(m_value);
    }

    template<typename returnType, typename argType1, typename argType2, typename argType3>
    FunctionPtr(returnType (FASTCALL *value)(argType1, argType2, argType3))
        : m_value((void*)value)
    {
        ASSERT_VALID_CODE_POINTER(m_value);
    }

    template<typename returnType, typename argType1, typename argType2, typename argType3, typename argType4>
    FunctionPtr(returnType (FASTCALL *value)(argType1, argType2, argType3, argType4))
        : m_value((void*)value)
    {
        ASSERT_VALID_CODE_POINTER(m_value);
    }
#endif

    template<typename FunctionType>
    explicit FunctionPtr(FunctionType* value)
        // Using a C-ctyle cast here to avoid compiler error on RVTC:
        // Error:  #694: reinterpret_cast cannot cast away const or other type qualifiers
        // (I guess on RVTC function pointers have a different constness to GCC/MSVC?)
        : m_value((void*)value)
    {
        ASSERT_VALID_CODE_POINTER(m_value);
    }

    void* value() const { return m_value; }
    void* executableAddress() const { return m_value; }


private:
    void* m_value;
};

// ReturnAddressPtr:
//
// ReturnAddressPtr should be used to wrap return addresses generated by processor
// 'call' instructions exectued in JIT code.  We use return addresses to look up
// exception and optimization information, and to repatch the call instruction
// that is the source of the return address.
class ReturnAddressPtr {
public:
    ReturnAddressPtr()
        : m_value(0)
    {
    }

    explicit ReturnAddressPtr(void* value)
        : m_value(value)
    {
        ASSERT_VALID_CODE_POINTER(m_value);
    }

    explicit ReturnAddressPtr(FunctionPtr function)
        : m_value(function.value())
    {
        ASSERT_VALID_CODE_POINTER(m_value);
    }

    void* value() const { return m_value; }
    
    void dump(PrintStream& out) const
    {
        out.print(RawPointer(m_value));
    }

private:
    void* m_value;
};

// MacroAssemblerCodePtr:
//
// MacroAssemblerCodePtr should be used to wrap pointers to JIT generated code.
class MacroAssemblerCodePtr {
public:
    MacroAssemblerCodePtr()
        : m_value(0)
    {
    }

    explicit MacroAssemblerCodePtr(void* value)
#if CPU(ARM_THUMB2)
        // Decorate the pointer as a thumb code pointer.
        : m_value(reinterpret_cast<char*>(value) + 1)
#else
        : m_value(value)
#endif
    {
        ASSERT_VALID_CODE_POINTER(m_value);
    }
    
    static MacroAssemblerCodePtr createFromExecutableAddress(void* value)
    {
        ASSERT_VALID_CODE_POINTER(value);
        MacroAssemblerCodePtr result;
        result.m_value = value;
        return result;
    }

    static MacroAssemblerCodePtr createLLIntCodePtr(OpcodeID codeId);

    explicit MacroAssemblerCodePtr(ReturnAddressPtr ra)
        : m_value(ra.value())
    {
        ASSERT_VALID_CODE_POINTER(m_value);
    }

    void* executableAddress() const { return m_value; }
#if CPU(ARM_THUMB2)
    // To use this pointer as a data address remove the decoration.
    void* dataLocation() const { ASSERT_VALID_CODE_POINTER(m_value); return reinterpret_cast<char*>(m_value) - 1; }
#else
    void* dataLocation() const { ASSERT_VALID_CODE_POINTER(m_value); return m_value; }
#endif

    explicit operator bool() const { return m_value; }
    
    bool operator==(const MacroAssemblerCodePtr& other) const
    {
        return m_value == other.m_value;
    }

    void dumpWithName(const char* name, PrintStream& out) const;
    
    void dump(PrintStream& out) const;
    
    enum EmptyValueTag { EmptyValue };
    enum DeletedValueTag { DeletedValue };
    
    MacroAssemblerCodePtr(EmptyValueTag)
        : m_value(emptyValue())
    {
    }
    
    MacroAssemblerCodePtr(DeletedValueTag)
        : m_value(deletedValue())
    {
    }
    
    bool isEmptyValue() const { return m_value == emptyValue(); }
    bool isDeletedValue() const { return m_value == deletedValue(); }
    
    unsigned hash() const { return PtrHash<void*>::hash(m_value); }

private:
    static void* emptyValue() { return bitwise_cast<void*>(static_cast<intptr_t>(1)); }
    static void* deletedValue() { return bitwise_cast<void*>(static_cast<intptr_t>(2)); }
    
    void* m_value;
};

struct MacroAssemblerCodePtrHash {
    static unsigned hash(const MacroAssemblerCodePtr& ptr) { return ptr.hash(); }
    static bool equal(const MacroAssemblerCodePtr& a, const MacroAssemblerCodePtr& b)
    {
        return a == b;
    }
    static const bool safeToCompareToEmptyOrDeleted = true;
};

// MacroAssemblerCodeRef:
//
// A reference to a section of JIT generated code.  A CodeRef consists of a
// pointer to the code, and a ref pointer to the pool from within which it
// was allocated.
class MacroAssemblerCodeRef {
private:
    // This is private because it's dangerous enough that we want uses of it
    // to be easy to find - hence the static create method below.
    explicit MacroAssemblerCodeRef(MacroAssemblerCodePtr codePtr)
        : m_codePtr(codePtr)
    {
        ASSERT(m_codePtr);
    }

public:
    MacroAssemblerCodeRef()
    {
    }

    MacroAssemblerCodeRef(Ref<ExecutableMemoryHandle>&& executableMemory)
        : m_codePtr(executableMemory->start())
        , m_executableMemory(WTFMove(executableMemory))
    {
        ASSERT(m_executableMemory->isManaged());
        ASSERT(m_executableMemory->start());
        ASSERT(m_codePtr);
    }
    
    // Use this only when you know that the codePtr refers to code that is
    // already being kept alive through some other means. Typically this means
    // that codePtr is immortal.
    static MacroAssemblerCodeRef createSelfManagedCodeRef(MacroAssemblerCodePtr codePtr)
    {
        return MacroAssemblerCodeRef(codePtr);
    }
    
    // Helper for creating self-managed code refs from LLInt.
    static MacroAssemblerCodeRef createLLIntCodeRef(OpcodeID codeId);

    ExecutableMemoryHandle* executableMemory() const
    {
        return m_executableMemory.get();
    }
    
    MacroAssemblerCodePtr code() const
    {
        return m_codePtr;
    }
    
    size_t size() const
    {
        if (!m_executableMemory)
            return 0;
        return m_executableMemory->sizeInBytes();
    }

    bool tryToDisassemble(PrintStream& out, const char* prefix = "") const;
    
    bool tryToDisassemble(const char* prefix = "") const;
    
    JS_EXPORT_PRIVATE CString disassembly() const;
    
    explicit operator bool() const { return !!m_codePtr; }
    
    void dump(PrintStream& out) const;

private:
    MacroAssemblerCodePtr m_codePtr;
    RefPtr<ExecutableMemoryHandle> m_executableMemory;
};

} // namespace JSC

namespace WTF {

template<typename T> struct DefaultHash;
template<> struct DefaultHash<JSC::MacroAssemblerCodePtr> {
    typedef JSC::MacroAssemblerCodePtrHash Hash;
};

template<typename T> struct HashTraits;
template<> struct HashTraits<JSC::MacroAssemblerCodePtr> : public CustomHashTraits<JSC::MacroAssemblerCodePtr> { };

} // namespace WTF
