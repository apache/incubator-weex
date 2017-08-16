/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2003, 2006-2009, 2015-2016 Apple Inc. All rights reserved.
 *  Copyright (C) 2007 Cameron Zwarich (cwzwarich@uwaterloo.ca)
 *  Copyright (C) 2007 Maks Orlovich
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#pragma once

#include "FunctionRareData.h"
#include "InternalFunction.h"
#include "JSCallee.h"
#include "JSScope.h"
#include "Watchpoint.h"

namespace JSC {

class ExecutableBase;
class FunctionExecutable;
class FunctionPrototype;
class JSLexicalEnvironment;
class JSGlobalObject;
class LLIntOffsetsExtractor;
class NativeExecutable;
class SourceCode;
class InternalFunction;
namespace DFG {
class SpeculativeJIT;
class JITCompiler;
}

namespace DOMJIT {
class Signature;
}


JS_EXPORT_PRIVATE EncodedJSValue JSC_HOST_CALL callHostFunctionAsConstructor(ExecState*);

JS_EXPORT_PRIVATE String getCalculatedDisplayName(VM&, JSObject*);

class JSFunction : public JSCallee {
    friend class JIT;
    friend class DFG::SpeculativeJIT;
    friend class DFG::JITCompiler;
    friend class VM;
    friend class InternalFunction;

public:
    typedef JSCallee Base;
    const static unsigned StructureFlags = Base::StructureFlags | OverridesGetOwnPropertySlot | OverridesGetPropertyNames;

    static size_t allocationSize(size_t inlineCapacity)
    {
        ASSERT_UNUSED(inlineCapacity, !inlineCapacity);
        return sizeof(JSFunction);
    }

    JS_EXPORT_PRIVATE static JSFunction* create(VM&, JSGlobalObject*, int length, const String& name, NativeFunction, Intrinsic = NoIntrinsic, NativeFunction nativeConstructor = callHostFunctionAsConstructor, const DOMJIT::Signature* = nullptr);
    
    static JSFunction* createWithInvalidatedReallocationWatchpoint(VM&, FunctionExecutable*, JSScope*);

    static JSFunction* create(VM&, FunctionExecutable*, JSScope*);
    static JSFunction* create(VM&, FunctionExecutable*, JSScope*, Structure*);

    JS_EXPORT_PRIVATE static JSFunction* createBuiltinFunction(VM&, FunctionExecutable*, JSGlobalObject*);
    static JSFunction* createBuiltinFunction(VM&, FunctionExecutable*, JSGlobalObject*, const String& name);

    JS_EXPORT_PRIVATE String name(VM&);
    JS_EXPORT_PRIVATE String displayName(VM&);
    const String calculatedDisplayName(VM&);

    ExecutableBase* executable() const { return m_executable.get(); }

    // To call any of these methods include JSFunctionInlines.h
    bool isHostFunction() const;
    FunctionExecutable* jsExecutable() const;
    Intrinsic intrinsic() const;

    JS_EXPORT_PRIVATE const SourceCode* sourceCode() const;

    DECLARE_EXPORT_INFO;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype) 
    {
        ASSERT(globalObject);
        return Structure::create(vm, globalObject, prototype, TypeInfo(JSFunctionType, StructureFlags), info()); 
    }

    NativeFunction nativeFunction();
    NativeFunction nativeConstructor();

    static ConstructType getConstructData(JSCell*, ConstructData&);
    static CallType getCallData(JSCell*, CallData&);

    static inline ptrdiff_t offsetOfExecutable()
    {
        return OBJECT_OFFSETOF(JSFunction, m_executable);
    }

    static inline ptrdiff_t offsetOfRareData()
    {
        return OBJECT_OFFSETOF(JSFunction, m_rareData);
    }

    FunctionRareData* rareData(VM& vm)
    {
        if (UNLIKELY(!m_rareData))
            return allocateRareData(vm);
        return m_rareData.get();
    }

    FunctionRareData* rareData(ExecState* exec, unsigned inlineCapacity)
    {
        if (UNLIKELY(!m_rareData))
            return allocateAndInitializeRareData(exec, inlineCapacity);
        if (UNLIKELY(!m_rareData->isObjectAllocationProfileInitialized()))
            return initializeRareData(exec, inlineCapacity);
        return m_rareData.get();
    }

    FunctionRareData* rareData()
    {
        FunctionRareData* rareData = m_rareData.get();

        // The JS thread may be concurrently creating the rare data
        // If we see it, we want to ensure it has been properly created
        WTF::loadLoadFence();

        return rareData;
    }

    bool isHostOrBuiltinFunction() const;
    bool isBuiltinFunction() const;
    JS_EXPORT_PRIVATE bool isHostFunctionNonInline() const;
    bool isClassConstructorFunction() const;

    void setFunctionName(ExecState*, JSValue name);

protected:
    JS_EXPORT_PRIVATE JSFunction(VM&, JSGlobalObject*, Structure*);
    JSFunction(VM&, FunctionExecutable*, JSScope*, Structure*);

    void finishCreation(VM&, NativeExecutable*, int length, const String& name);
    using Base::finishCreation;

    FunctionRareData* allocateRareData(VM&);
    FunctionRareData* allocateAndInitializeRareData(ExecState*, size_t inlineCapacity);
    FunctionRareData* initializeRareData(ExecState*, size_t inlineCapacity);

    static bool getOwnPropertySlot(JSObject*, ExecState*, PropertyName, PropertySlot&);
    static void getOwnNonIndexPropertyNames(JSObject*, ExecState*, PropertyNameArray&, EnumerationMode = EnumerationMode());
    static bool defineOwnProperty(JSObject*, ExecState*, PropertyName, const PropertyDescriptor&, bool shouldThrow);

    static bool put(JSCell*, ExecState*, PropertyName, JSValue, PutPropertySlot&);

    static bool deleteProperty(JSCell*, ExecState*, PropertyName);

    static void visitChildren(JSCell*, SlotVisitor&);


private:
    static JSFunction* createImpl(VM& vm, FunctionExecutable* executable, JSScope* scope, Structure* structure)
    {
        JSFunction* function = new (NotNull, allocateCell<JSFunction>(vm.heap)) JSFunction(vm, executable, scope, structure);
        ASSERT(function->structure()->globalObject());
        function->finishCreation(vm);
        return function;
    }

    bool hasReifiedLength() const;
    bool hasReifiedName() const;
    void reifyLength(VM&);
    void reifyName(VM&, ExecState*);
    void reifyName(VM&, ExecState*, String name);

    enum class LazyPropertyType { NotLazyProperty, IsLazyProperty };
    LazyPropertyType reifyLazyPropertyIfNeeded(VM&, ExecState*, PropertyName);
    LazyPropertyType reifyBoundNameIfNeeded(VM&, ExecState*, PropertyName);

    friend class LLIntOffsetsExtractor;

    static EncodedJSValue argumentsGetter(ExecState*, EncodedJSValue, PropertyName);
    static EncodedJSValue callerGetter(ExecState*, EncodedJSValue, PropertyName);
    static EncodedJSValue lengthGetter(ExecState*, EncodedJSValue, PropertyName);
    static EncodedJSValue nameGetter(ExecState*, EncodedJSValue, PropertyName);

    WriteBarrier<ExecutableBase> m_executable;
    WriteBarrier<FunctionRareData> m_rareData;
};

} // namespace JSC
