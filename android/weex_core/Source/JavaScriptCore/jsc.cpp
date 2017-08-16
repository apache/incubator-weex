/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2004-2017 Apple Inc. All rights reserved.
 *  Copyright (C) 2006 Bjoern Graf (bjoern.graf@gmail.com)
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

#include "config.h"

#include "ArrayBuffer.h"
#include "ArrayPrototype.h"
#include "BuiltinExecutableCreator.h"
#include "BuiltinNames.h"
#include "ButterflyInlines.h"
#include "CodeBlock.h"
#include "Completion.h"
#include "ConfigFile.h"
#include "DOMJITGetterSetter.h"
#include "DOMJITPatchpoint.h"
#include "DOMJITPatchpointParams.h"
#include "Disassembler.h"
#include "Exception.h"
#include "ExceptionHelpers.h"
#include "GetterSetter.h"
#include "HeapProfiler.h"
#include "HeapSnapshotBuilder.h"
#include "HeapTimer.h"
#include "InitializeThreading.h"
#include "Interpreter.h"
#include "JIT.h"
#include "JSArray.h"
#include "JSArrayBuffer.h"
#include "JSCInlines.h"
#include "JSFunction.h"
#include "JSInternalPromise.h"
#include "JSInternalPromiseDeferred.h"
#include "JSLock.h"
#include "JSModuleLoader.h"
#include "JSNativeStdFunction.h"
#include "JSONObject.h"
#include "JSProxy.h"
#include "JSSourceCode.h"
#include "JSString.h"
#include "JSTypedArrays.h"
#include "JSWebAssemblyCallee.h"
#include "LLIntData.h"
#include "LLIntThunks.h"
#include "ObjectConstructor.h"
#include "ParserError.h"
#include "ProfilerDatabase.h"
#include "ProtoCallFrame.h"
#include "ReleaseHeapAccessScope.h"
#include "SamplingProfiler.h"
#include "ShadowChicken.h"
#include "StackVisitor.h"
#include "StructureInlines.h"
#include "StructureRareDataInlines.h"
#include "SuperSampler.h"
#include "TestRunnerUtils.h"
#include "TypeProfilerLog.h"
#include "WasmFaultSignalHandler.h"
#include "WasmPlan.h"
#include "WasmMemory.h"
#include "ICUCompatible.h"
#include <locale.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <thread>
#include <type_traits>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <wtf/CommaPrinter.h>
#include <wtf/CurrentTime.h>
#include <wtf/MainThread.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/StringPrintStream.h>
#include <wtf/text/StringBuilder.h>

#if OS(WINDOWS)
#include <direct.h>
#else
#include <unistd.h>
#endif

#if HAVE(READLINE)
// readline/history.h has a Function typedef which conflicts with the WTF::Function template from WTF/Forward.h
// We #define it to something else to avoid this conflict.
#define Function ReadlineFunction
#include <readline/history.h>
#include <readline/readline.h>
#undef Function
#endif

#if HAVE(SYS_TIME_H)
#include <sys/time.h>
#endif

#if HAVE(SIGNAL_H)
#include <signal.h>
#endif

#if COMPILER(MSVC)
#include <crtdbg.h>
#include <mmsystem.h>
#include <windows.h>
#endif

#if PLATFORM(IOS) && CPU(ARM_THUMB2)
#include <fenv.h>
#include <arm/arch.h>
#endif

#if !defined(PATH_MAX)
#define PATH_MAX 4096
#endif

using namespace JSC;
using namespace WTF;

namespace {

NO_RETURN_WITH_VALUE static void jscExit(int status)
{
    waitForAsynchronousDisassembly();
    
#if ENABLE(DFG_JIT)
    if (DFG::isCrashing()) {
        for (;;) {
#if OS(WINDOWS)
            Sleep(1000);
#else
            pause();
#endif
        }
    }
#endif // ENABLE(DFG_JIT)
    exit(status);
}

class Element;
class ElementHandleOwner;
class Masuqerader;
class Root;
class RuntimeArray;

class Element : public JSNonFinalObject {
public:
    Element(VM& vm, Structure* structure)
        : Base(vm, structure)
    {
    }

    typedef JSNonFinalObject Base;

    Root* root() const { return m_root.get(); }
    void setRoot(VM& vm, Root* root) { m_root.set(vm, this, root); }

    static Element* create(VM& vm, JSGlobalObject* globalObject, Root* root)
    {
        Structure* structure = createStructure(vm, globalObject, jsNull());
        Element* element = new (NotNull, allocateCell<Element>(vm.heap, sizeof(Element))) Element(vm, structure);
        element->finishCreation(vm, root);
        return element;
    }

    void finishCreation(VM&, Root*);

    static void visitChildren(JSCell* cell, SlotVisitor& visitor)
    {
        Element* thisObject = jsCast<Element*>(cell);
        ASSERT_GC_OBJECT_INHERITS(thisObject, info());
        Base::visitChildren(thisObject, visitor);
        visitor.append(thisObject->m_root);
    }

    static ElementHandleOwner* handleOwner();

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
    }

    DECLARE_INFO;

private:
    WriteBarrier<Root> m_root;
};

class ElementHandleOwner : public WeakHandleOwner {
public:
    bool isReachableFromOpaqueRoots(Handle<JSC::Unknown> handle, void*, SlotVisitor& visitor) override
    {
        Element* element = jsCast<Element*>(handle.slot()->asCell());
        return visitor.containsOpaqueRoot(element->root());
    }
};

class Masquerader : public JSNonFinalObject {
public:
    Masquerader(VM& vm, Structure* structure)
        : Base(vm, structure)
    {
    }

    typedef JSNonFinalObject Base;
    static const unsigned StructureFlags = Base::StructureFlags | JSC::MasqueradesAsUndefined;

    static Masquerader* create(VM& vm, JSGlobalObject* globalObject)
    {
        globalObject->masqueradesAsUndefinedWatchpoint()->fireAll(vm, "Masquerading object allocated");
        Structure* structure = createStructure(vm, globalObject, jsNull());
        Masquerader* result = new (NotNull, allocateCell<Masquerader>(vm.heap, sizeof(Masquerader))) Masquerader(vm, structure);
        result->finishCreation(vm);
        return result;
    }

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
    }

    DECLARE_INFO;
};

class Root : public JSDestructibleObject {
public:
    Root(VM& vm, Structure* structure)
        : Base(vm, structure)
    {
    }

    Element* element()
    {
        return m_element.get();
    }

    void setElement(Element* element)
    {
        Weak<Element> newElement(element, Element::handleOwner());
        m_element.swap(newElement);
    }

    static Root* create(VM& vm, JSGlobalObject* globalObject)
    {
        Structure* structure = createStructure(vm, globalObject, jsNull());
        Root* root = new (NotNull, allocateCell<Root>(vm.heap, sizeof(Root))) Root(vm, structure);
        root->finishCreation(vm);
        return root;
    }

    typedef JSDestructibleObject Base;

    DECLARE_INFO;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
    }

    static void visitChildren(JSCell* thisObject, SlotVisitor& visitor)
    {
        Base::visitChildren(thisObject, visitor);
        visitor.addOpaqueRoot(thisObject);
    }

private:
    Weak<Element> m_element;
};

class ImpureGetter : public JSNonFinalObject {
public:
    ImpureGetter(VM& vm, Structure* structure)
        : Base(vm, structure)
    {
    }

    DECLARE_INFO;
    typedef JSNonFinalObject Base;
    static const unsigned StructureFlags = Base::StructureFlags | JSC::GetOwnPropertySlotIsImpure | JSC::OverridesGetOwnPropertySlot;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
    }

    static ImpureGetter* create(VM& vm, Structure* structure, JSObject* delegate)
    {
        ImpureGetter* getter = new (NotNull, allocateCell<ImpureGetter>(vm.heap, sizeof(ImpureGetter))) ImpureGetter(vm, structure);
        getter->finishCreation(vm, delegate);
        return getter;
    }

    void finishCreation(VM& vm, JSObject* delegate)
    {
        Base::finishCreation(vm);
        if (delegate)
            m_delegate.set(vm, this, delegate);
    }

    static bool getOwnPropertySlot(JSObject* object, ExecState* exec, PropertyName name, PropertySlot& slot)
    {
        VM& vm = exec->vm();
        auto scope = DECLARE_THROW_SCOPE(vm);
        ImpureGetter* thisObject = jsCast<ImpureGetter*>(object);
        
        if (thisObject->m_delegate) {
            if (thisObject->m_delegate->getPropertySlot(exec, name, slot))
                return true;
            RETURN_IF_EXCEPTION(scope, false);
        }

        return Base::getOwnPropertySlot(object, exec, name, slot);
    }

    static void visitChildren(JSCell* cell, SlotVisitor& visitor)
    {
        Base::visitChildren(cell, visitor);
        ImpureGetter* thisObject = jsCast<ImpureGetter*>(cell);
        visitor.append(thisObject->m_delegate);
    }

    void setDelegate(VM& vm, JSObject* delegate)
    {
        m_delegate.set(vm, this, delegate);
    }

private:
    WriteBarrier<JSObject> m_delegate;
};

class CustomGetter : public JSNonFinalObject {
public:
    CustomGetter(VM& vm, Structure* structure)
        : Base(vm, structure)
    {
    }

    DECLARE_INFO;
    typedef JSNonFinalObject Base;
    static const unsigned StructureFlags = Base::StructureFlags | JSC::OverridesGetOwnPropertySlot;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
    }

    static CustomGetter* create(VM& vm, Structure* structure)
    {
        CustomGetter* getter = new (NotNull, allocateCell<CustomGetter>(vm.heap, sizeof(CustomGetter))) CustomGetter(vm, structure);
        getter->finishCreation(vm);
        return getter;
    }

    static bool getOwnPropertySlot(JSObject* object, ExecState* exec, PropertyName propertyName, PropertySlot& slot)
    {
        CustomGetter* thisObject = jsCast<CustomGetter*>(object);
        if (propertyName == PropertyName(Identifier::fromString(exec, "customGetter"))) {
            slot.setCacheableCustom(thisObject, DontDelete | ReadOnly | DontEnum, thisObject->customGetter);
            return true;
        }
        
        if (propertyName == PropertyName(Identifier::fromString(exec, "customGetterAccessor"))) {
            slot.setCacheableCustom(thisObject, DontDelete | ReadOnly | DontEnum | CustomAccessor, thisObject->customGetterAcessor);
            return true;
        }
        
        return JSObject::getOwnPropertySlot(thisObject, exec, propertyName, slot);
    }

private:
    static EncodedJSValue customGetter(ExecState* exec, EncodedJSValue thisValue, PropertyName)
    {
        VM& vm = exec->vm();
        auto scope = DECLARE_THROW_SCOPE(vm);

        CustomGetter* thisObject = jsDynamicCast<CustomGetter*>(vm, JSValue::decode(thisValue));
        if (!thisObject)
            return throwVMTypeError(exec, scope);
        bool shouldThrow = thisObject->get(exec, PropertyName(Identifier::fromString(exec, "shouldThrow"))).toBoolean(exec);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
        if (shouldThrow)
            return throwVMTypeError(exec, scope);
        return JSValue::encode(jsNumber(100));
    }
    
    static EncodedJSValue customGetterAcessor(ExecState* exec, EncodedJSValue thisValue, PropertyName)
    {
        VM& vm = exec->vm();
        auto scope = DECLARE_THROW_SCOPE(vm);
        
        JSObject* thisObject = jsDynamicCast<JSObject*>(vm, JSValue::decode(thisValue));
        if (!thisObject)
            return throwVMTypeError(exec, scope);
        bool shouldThrow = thisObject->get(exec, PropertyName(Identifier::fromString(exec, "shouldThrow"))).toBoolean(exec);
        if (shouldThrow)
            return throwVMTypeError(exec, scope);
        return JSValue::encode(jsNumber(100));
    }
};

class RuntimeArray : public JSArray {
public:
    typedef JSArray Base;
    static const unsigned StructureFlags = Base::StructureFlags | OverridesGetOwnPropertySlot | InterceptsGetOwnPropertySlotByIndexEvenWhenLengthIsNotZero | OverridesGetPropertyNames;

    static RuntimeArray* create(ExecState* exec)
    {
        VM& vm = exec->vm();
        JSGlobalObject* globalObject = exec->lexicalGlobalObject();
        Structure* structure = createStructure(vm, globalObject, createPrototype(vm, globalObject));
        RuntimeArray* runtimeArray = new (NotNull, allocateCell<RuntimeArray>(*exec->heap())) RuntimeArray(exec, structure);
        runtimeArray->finishCreation(exec);
        vm.heap.addFinalizer(runtimeArray, destroy);
        return runtimeArray;
    }

    ~RuntimeArray() { }

    static void destroy(JSCell* cell)
    {
        static_cast<RuntimeArray*>(cell)->RuntimeArray::~RuntimeArray();
    }

    static const bool needsDestruction = false;

    static bool getOwnPropertySlot(JSObject* object, ExecState* exec, PropertyName propertyName, PropertySlot& slot)
    {
        RuntimeArray* thisObject = jsCast<RuntimeArray*>(object);
        if (propertyName == exec->propertyNames().length) {
            slot.setCacheableCustom(thisObject, DontDelete | ReadOnly | DontEnum, thisObject->lengthGetter);
            return true;
        }

        std::optional<uint32_t> index = parseIndex(propertyName);
        if (index && index.value() < thisObject->getLength()) {
            slot.setValue(thisObject, DontDelete | DontEnum, jsNumber(thisObject->m_vector[index.value()]));
            return true;
        }

        return JSObject::getOwnPropertySlot(thisObject, exec, propertyName, slot);
    }

    static bool getOwnPropertySlotByIndex(JSObject* object, ExecState* exec, unsigned index, PropertySlot& slot)
    {
        RuntimeArray* thisObject = jsCast<RuntimeArray*>(object);
        if (index < thisObject->getLength()) {
            slot.setValue(thisObject, DontDelete | DontEnum, jsNumber(thisObject->m_vector[index]));
            return true;
        }

        return JSObject::getOwnPropertySlotByIndex(thisObject, exec, index, slot);
    }

    static NO_RETURN_DUE_TO_CRASH bool put(JSCell*, ExecState*, PropertyName, JSValue, PutPropertySlot&)
    {
        RELEASE_ASSERT_NOT_REACHED();
    }

    static NO_RETURN_DUE_TO_CRASH bool deleteProperty(JSCell*, ExecState*, PropertyName)
    {
        RELEASE_ASSERT_NOT_REACHED();
    }

    unsigned getLength() const { return m_vector.size(); }

    DECLARE_INFO;

    static ArrayPrototype* createPrototype(VM&, JSGlobalObject* globalObject)
    {
        return globalObject->arrayPrototype();
    }

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(DerivedArrayType, StructureFlags), info(), ArrayClass);
    }

protected:
    void finishCreation(ExecState* exec)
    {
        VM& vm = exec->vm();
        Base::finishCreation(vm);
        ASSERT(inherits(vm, info()));

        for (size_t i = 0; i < exec->argumentCount(); i++)
            m_vector.append(exec->argument(i).toInt32(exec));
    }

private:
    RuntimeArray(ExecState* exec, Structure* structure)
        : JSArray(exec->vm(), structure, 0)
    {
    }

    static EncodedJSValue lengthGetter(ExecState* exec, EncodedJSValue thisValue, PropertyName)
    {
        VM& vm = exec->vm();
        auto scope = DECLARE_THROW_SCOPE(vm);

        RuntimeArray* thisObject = jsDynamicCast<RuntimeArray*>(vm, JSValue::decode(thisValue));
        if (!thisObject)
            return throwVMTypeError(exec, scope);
        return JSValue::encode(jsNumber(thisObject->getLength()));
    }

    Vector<int> m_vector;
};

class SimpleObject : public JSNonFinalObject {
public:
    SimpleObject(VM& vm, Structure* structure)
        : Base(vm, structure)
    {
    }

    typedef JSNonFinalObject Base;
    static const bool needsDestruction = false;

    static SimpleObject* create(VM& vm, JSGlobalObject* globalObject)
    {
        Structure* structure = createStructure(vm, globalObject, jsNull());
        SimpleObject* simpleObject = new (NotNull, allocateCell<SimpleObject>(vm.heap, sizeof(SimpleObject))) SimpleObject(vm, structure);
        simpleObject->finishCreation(vm);
        return simpleObject;
    }

    static void visitChildren(JSCell* cell, SlotVisitor& visitor)
    {
        SimpleObject* thisObject = jsCast<SimpleObject*>(cell);
        ASSERT_GC_OBJECT_INHERITS(thisObject, info());
        Base::visitChildren(thisObject, visitor);
        visitor.append(thisObject->m_hiddenValue);
    }

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
    }

    JSValue hiddenValue()
    {
        return m_hiddenValue.get();
    }

    void setHiddenValue(VM& vm, JSValue value)
    {
        ASSERT(value.isCell());
        m_hiddenValue.set(vm, this, value);
    }

    DECLARE_INFO;

private:
    WriteBarrier<JSC::Unknown> m_hiddenValue;
};

class DOMJITNode : public JSNonFinalObject {
public:
    DOMJITNode(VM& vm, Structure* structure)
        : Base(vm, structure)
    {
    }

    DECLARE_INFO;
    typedef JSNonFinalObject Base;
    static const unsigned StructureFlags = Base::StructureFlags;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(JSC::JSType(LastJSCObjectType + 1), StructureFlags), info());
    }

#if ENABLE(JIT)
    static Ref<DOMJIT::Patchpoint> checkDOMJITNode()
    {
        Ref<DOMJIT::Patchpoint> patchpoint = DOMJIT::Patchpoint::create();
        patchpoint->setGenerator([=](CCallHelpers& jit, DOMJIT::PatchpointParams& params) {
            CCallHelpers::JumpList failureCases;
            failureCases.append(jit.branch8(
                CCallHelpers::NotEqual,
                CCallHelpers::Address(params[0].gpr(), JSCell::typeInfoTypeOffset()),
                CCallHelpers::TrustedImm32(JSC::JSType(LastJSCObjectType + 1))));
            return failureCases;
        });
        return patchpoint;
    }
#endif

    static DOMJITNode* create(VM& vm, Structure* structure)
    {
        DOMJITNode* getter = new (NotNull, allocateCell<DOMJITNode>(vm.heap, sizeof(DOMJITNode))) DOMJITNode(vm, structure);
        getter->finishCreation(vm);
        return getter;
    }

    int32_t value() const
    {
        return m_value;
    }

    static ptrdiff_t offsetOfValue() { return OBJECT_OFFSETOF(DOMJITNode, m_value); }

private:
    int32_t m_value { 42 };
};

class DOMJITGetter : public DOMJITNode {
public:
    DOMJITGetter(VM& vm, Structure* structure)
        : Base(vm, structure)
    {
    }

    DECLARE_INFO;
    typedef DOMJITNode Base;
    static const unsigned StructureFlags = Base::StructureFlags;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(JSC::JSType(LastJSCObjectType + 1), StructureFlags), info());
    }

    static DOMJITGetter* create(VM& vm, Structure* structure)
    {
        DOMJITGetter* getter = new (NotNull, allocateCell<DOMJITGetter>(vm.heap, sizeof(DOMJITGetter))) DOMJITGetter(vm, structure);
        getter->finishCreation(vm);
        return getter;
    }

    class DOMJITNodeDOMJIT : public DOMJIT::GetterSetter {
    public:
        DOMJITNodeDOMJIT()
            : DOMJIT::GetterSetter(DOMJITGetter::customGetter, nullptr, DOMJITNode::info(), SpecInt32Only)
        {
        }

#if ENABLE(JIT)
        Ref<DOMJIT::Patchpoint> checkDOM() override
        {
            return DOMJITNode::checkDOMJITNode();
        }

        static EncodedJSValue JIT_OPERATION slowCall(ExecState* exec, void* pointer)
        {
            NativeCallFrameTracer tracer(&exec->vm(), exec);
            return JSValue::encode(jsNumber(static_cast<DOMJITGetter*>(pointer)->value()));
        }

        Ref<DOMJIT::CallDOMGetterPatchpoint> callDOMGetter() override
        {
            Ref<DOMJIT::CallDOMGetterPatchpoint> patchpoint = DOMJIT::CallDOMGetterPatchpoint::create();
            patchpoint->requireGlobalObject = false;
            patchpoint->setGenerator([=](CCallHelpers& jit, DOMJIT::PatchpointParams& params) {
                JSValueRegs results = params[0].jsValueRegs();
                GPRReg dom = params[1].gpr();
                params.addSlowPathCall(jit.jump(), jit, slowCall, results, dom);
                return CCallHelpers::JumpList();

            });
            return patchpoint;
        }
#endif
    };

    static DOMJIT::GetterSetter* domJITNodeGetterSetter()
    {
        static NeverDestroyed<DOMJITNodeDOMJIT> graph;
        return &graph.get();
    }

private:
    void finishCreation(VM& vm)
    {
        Base::finishCreation(vm);
        DOMJIT::GetterSetter* domJIT = domJITNodeGetterSetter();
        CustomGetterSetter* customGetterSetter = CustomGetterSetter::create(vm, domJIT->getter(), domJIT->setter(), domJIT);
        putDirectCustomAccessor(vm, Identifier::fromString(&vm, "customGetter"), customGetterSetter, ReadOnly | CustomAccessor);
    }

    static EncodedJSValue customGetter(ExecState* exec, EncodedJSValue thisValue, PropertyName)
    {
        VM& vm = exec->vm();
        auto scope = DECLARE_THROW_SCOPE(vm);

        DOMJITNode* thisObject = jsDynamicCast<DOMJITNode*>(vm, JSValue::decode(thisValue));
        if (!thisObject)
            return throwVMTypeError(exec, scope);
        return JSValue::encode(jsNumber(thisObject->value()));
    }
};

class DOMJITGetterComplex : public DOMJITNode {
public:
    DOMJITGetterComplex(VM& vm, Structure* structure)
        : Base(vm, structure)
    {
    }

    DECLARE_INFO;
    typedef DOMJITNode Base;
    static const unsigned StructureFlags = Base::StructureFlags;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(JSC::JSType(LastJSCObjectType + 1), StructureFlags), info());
    }

    static DOMJITGetterComplex* create(VM& vm, JSGlobalObject* globalObject, Structure* structure)
    {
        DOMJITGetterComplex* getter = new (NotNull, allocateCell<DOMJITGetterComplex>(vm.heap, sizeof(DOMJITGetterComplex))) DOMJITGetterComplex(vm, structure);
        getter->finishCreation(vm, globalObject);
        return getter;
    }

    class DOMJITNodeDOMJIT : public DOMJIT::GetterSetter {
    public:
        DOMJITNodeDOMJIT()
            : DOMJIT::GetterSetter(DOMJITGetterComplex::customGetter, nullptr, DOMJITNode::info(), SpecInt32Only)
        {
        }

#if ENABLE(JIT)
        Ref<DOMJIT::Patchpoint> checkDOM() override
        {
            return DOMJITNode::checkDOMJITNode();
        }

        static EncodedJSValue JIT_OPERATION slowCall(ExecState* exec, void* pointer)
        {
            VM& vm = exec->vm();
            NativeCallFrameTracer tracer(&vm, exec);
            auto scope = DECLARE_THROW_SCOPE(vm);
            auto* object = static_cast<DOMJITNode*>(pointer);
            auto* domjitGetterComplex = jsDynamicCast<DOMJITGetterComplex*>(vm, object);
            if (domjitGetterComplex) {
                if (domjitGetterComplex->m_enableException)
                    return JSValue::encode(throwException(exec, scope, createError(exec, ASCIILiteral("DOMJITGetterComplex slow call exception"))));
            }
            return JSValue::encode(jsNumber(object->value()));
        }

        Ref<DOMJIT::CallDOMGetterPatchpoint> callDOMGetter() override
        {
            RefPtr<DOMJIT::CallDOMGetterPatchpoint> patchpoint = DOMJIT::CallDOMGetterPatchpoint::create();
            static_assert(GPRInfo::numberOfRegisters >= 4, "Number of registers should be larger or equal to 4.");
            patchpoint->numGPScratchRegisters = GPRInfo::numberOfRegisters - 4;
            patchpoint->numFPScratchRegisters = 3;
            patchpoint->setGenerator([=](CCallHelpers& jit, DOMJIT::PatchpointParams& params) {
                JSValueRegs results = params[0].jsValueRegs();
                GPRReg domGPR = params[1].gpr();
                for (unsigned i = 0; i < patchpoint->numGPScratchRegisters; ++i)
                    jit.move(CCallHelpers::TrustedImm32(42), params.gpScratch(i));

                params.addSlowPathCall(jit.jump(), jit, slowCall, results, domGPR);
                return CCallHelpers::JumpList();

            });
            return *patchpoint.get();
        }
#endif
    };

    static DOMJIT::GetterSetter* domJITNodeGetterSetter()
    {
        static NeverDestroyed<DOMJITNodeDOMJIT> graph;
        return &graph.get();
    }

private:
    void finishCreation(VM& vm, JSGlobalObject* globalObject)
    {
        Base::finishCreation(vm);
        DOMJIT::GetterSetter* domJIT = domJITNodeGetterSetter();
        CustomGetterSetter* customGetterSetter = CustomGetterSetter::create(vm, domJIT->getter(), domJIT->setter(), domJIT);
        putDirectCustomAccessor(vm, Identifier::fromString(&vm, "customGetter"), customGetterSetter, ReadOnly | CustomAccessor);
        putDirectNativeFunction(vm, globalObject, Identifier::fromString(&vm, "enableException"), 0, functionEnableException, NoIntrinsic, 0);
    }

    static EncodedJSValue JSC_HOST_CALL functionEnableException(ExecState* exec)
    {
        VM& vm = exec->vm();
        auto* object = jsDynamicCast<DOMJITGetterComplex*>(vm, exec->thisValue());
        if (object)
            object->m_enableException = true;
        return JSValue::encode(jsUndefined());
    }

    static EncodedJSValue customGetter(ExecState* exec, EncodedJSValue thisValue, PropertyName)
    {
        VM& vm = exec->vm();
        auto scope = DECLARE_THROW_SCOPE(vm);

        auto* thisObject = jsDynamicCast<DOMJITNode*>(vm, JSValue::decode(thisValue));
        if (!thisObject)
            return throwVMTypeError(exec, scope);
        if (auto* domjitGetterComplex = jsDynamicCast<DOMJITGetterComplex*>(vm, JSValue::decode(thisValue))) {
            if (domjitGetterComplex->m_enableException)
                return JSValue::encode(throwException(exec, scope, createError(exec, ASCIILiteral("DOMJITGetterComplex slow call exception"))));
        }
        return JSValue::encode(jsNumber(thisObject->value()));
    }

    bool m_enableException { false };
};

class DOMJITFunctionObject : public DOMJITNode {
public:
    DOMJITFunctionObject(VM& vm, Structure* structure)
        : Base(vm, structure)
    {
    }

    DECLARE_INFO;
    typedef DOMJITNode Base;
    static const unsigned StructureFlags = Base::StructureFlags;


    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(JSC::JSType(LastJSCObjectType + 1), StructureFlags), info());
    }

    static DOMJITFunctionObject* create(VM& vm, JSGlobalObject* globalObject, Structure* structure)
    {
        DOMJITFunctionObject* object = new (NotNull, allocateCell<DOMJITFunctionObject>(vm.heap, sizeof(DOMJITFunctionObject))) DOMJITFunctionObject(vm, structure);
        object->finishCreation(vm, globalObject);
        return object;
    }

    static EncodedJSValue JSC_HOST_CALL safeFunction(ExecState* exec)
    {
        VM& vm = exec->vm();
        auto scope = DECLARE_THROW_SCOPE(vm);

        DOMJITNode* thisObject = jsDynamicCast<DOMJITNode*>(vm, exec->thisValue());
        if (!thisObject)
            return throwVMTypeError(exec, scope);
        return JSValue::encode(jsNumber(thisObject->value()));
    }

#if ENABLE(JIT)
    static EncodedJSValue JIT_OPERATION unsafeFunction(ExecState* exec, DOMJITNode* node)
    {
        NativeCallFrameTracer tracer(&exec->vm(), exec);
        return JSValue::encode(jsNumber(node->value()));
    }

    static Ref<DOMJIT::Patchpoint> checkDOMJITNode()
    {
        static const double value = 42.0;
        Ref<DOMJIT::Patchpoint> patchpoint = DOMJIT::Patchpoint::create();
        patchpoint->numFPScratchRegisters = 1;
        patchpoint->setGenerator([=](CCallHelpers& jit, DOMJIT::PatchpointParams& params) {
            CCallHelpers::JumpList failureCases;
            // May use scratch registers.
            jit.loadDouble(CCallHelpers::TrustedImmPtr(&value), params.fpScratch(0));
            failureCases.append(jit.branch8(
                CCallHelpers::NotEqual,
                CCallHelpers::Address(params[0].gpr(), JSCell::typeInfoTypeOffset()),
                CCallHelpers::TrustedImm32(JSC::JSType(LastJSCObjectType + 1))));
            return failureCases;
        });
        return patchpoint;
    }
#endif

private:
    void finishCreation(VM&, JSGlobalObject*);
};

#if ENABLE(JIT)
static const DOMJIT::Signature DOMJITFunctionObjectSignature((uintptr_t)DOMJITFunctionObject::unsafeFunction, DOMJITFunctionObject::checkDOMJITNode, DOMJITFunctionObject::info(), DOMJIT::Effect::forRead(DOMJIT::HeapRange::top()), SpecInt32Only);
#endif

void DOMJITFunctionObject::finishCreation(VM& vm, JSGlobalObject* globalObject)
{
    Base::finishCreation(vm);
#if ENABLE(JIT)
    putDirectNativeFunction(vm, globalObject, Identifier::fromString(&vm, "func"), 0, safeFunction, NoIntrinsic, &DOMJITFunctionObjectSignature, ReadOnly);
#else
    putDirectNativeFunction(vm, globalObject, Identifier::fromString(&vm, "func"), 0, safeFunction, NoIntrinsic, nullptr, ReadOnly);
#endif
}


const ClassInfo Element::s_info = { "Element", &Base::s_info, nullptr, CREATE_METHOD_TABLE(Element) };
const ClassInfo Masquerader::s_info = { "Masquerader", &Base::s_info, nullptr, CREATE_METHOD_TABLE(Masquerader) };
const ClassInfo Root::s_info = { "Root", &Base::s_info, nullptr, CREATE_METHOD_TABLE(Root) };
const ClassInfo ImpureGetter::s_info = { "ImpureGetter", &Base::s_info, nullptr, CREATE_METHOD_TABLE(ImpureGetter) };
const ClassInfo CustomGetter::s_info = { "CustomGetter", &Base::s_info, nullptr, CREATE_METHOD_TABLE(CustomGetter) };
const ClassInfo DOMJITNode::s_info = { "DOMJITNode", &Base::s_info, nullptr, CREATE_METHOD_TABLE(DOMJITNode) };
const ClassInfo DOMJITGetter::s_info = { "DOMJITGetter", &Base::s_info, nullptr, CREATE_METHOD_TABLE(DOMJITGetter) };
const ClassInfo DOMJITGetterComplex::s_info = { "DOMJITGetterComplex", &Base::s_info, nullptr, CREATE_METHOD_TABLE(DOMJITGetterComplex) };
const ClassInfo DOMJITFunctionObject::s_info = { "DOMJITFunctionObject", &Base::s_info, nullptr, CREATE_METHOD_TABLE(DOMJITFunctionObject) };
const ClassInfo RuntimeArray::s_info = { "RuntimeArray", &Base::s_info, nullptr, CREATE_METHOD_TABLE(RuntimeArray) };
const ClassInfo SimpleObject::s_info = { "SimpleObject", &Base::s_info, nullptr, CREATE_METHOD_TABLE(SimpleObject) };
static bool test262AsyncPassed { false };
static bool test262AsyncTest { false };

ElementHandleOwner* Element::handleOwner()
{
    static ElementHandleOwner* owner = 0;
    if (!owner)
        owner = new ElementHandleOwner();
    return owner;
}

void Element::finishCreation(VM& vm, Root* root)
{
    Base::finishCreation(vm);
    setRoot(vm, root);
    m_root->setElement(this);
}

}

static bool fillBufferWithContentsOfFile(const String& fileName, Vector<char>& buffer);

class CommandLine;
class GlobalObject;
class Workers;

template<typename Func>
int runJSC(CommandLine, const Func&);
static void checkException(GlobalObject*, bool isLastFile, bool hasException, JSValue, const String& uncaughtExceptionName, bool alwaysDumpUncaughtException, bool dump, bool& success);

class Message : public ThreadSafeRefCounted<Message> {
public:
    Message(ArrayBufferContents&&, int32_t);
    ~Message();
    
    ArrayBufferContents&& releaseContents() { return WTFMove(m_contents); }
    int32_t index() const { return m_index; }

private:
    ArrayBufferContents m_contents;
    int32_t m_index { 0 };
};

class Worker : public BasicRawSentinelNode<Worker> {
public:
    Worker(Workers&);
    ~Worker();
    
    void enqueue(const AbstractLocker&, RefPtr<Message>);
    RefPtr<Message> dequeue();
    
    static Worker& current();

private:
    static ThreadSpecific<Worker*>& currentWorker();

    Workers& m_workers;
    Deque<RefPtr<Message>> m_messages;
};

class Workers {
public:
    Workers();
    ~Workers();
    
    template<typename Func>
    void broadcast(const Func&);
    
    void report(String);
    String tryGetReport();
    String getReport();
    
    static Workers& singleton();
    
private:
    friend class Worker;
    
    Lock m_lock;
    Condition m_condition;
    SentinelLinkedList<Worker, BasicRawSentinelNode<Worker>> m_workers;
    Deque<String> m_reports;
};

static EncodedJSValue JSC_HOST_CALL functionCreateProxy(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionCreateRuntimeArray(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionCreateImpureGetter(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionCreateCustomGetterObject(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionCreateDOMJITNodeObject(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionCreateDOMJITGetterObject(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionCreateDOMJITGetterComplexObject(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionCreateDOMJITFunctionObject(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionCreateBuiltin(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionCreateGlobalObject(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionSetImpureGetterDelegate(ExecState*);

static EncodedJSValue JSC_HOST_CALL functionSetElementRoot(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionCreateRoot(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionCreateElement(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionGetElement(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionCreateSimpleObject(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionGetHiddenValue(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionSetHiddenValue(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionPrintStdOut(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionPrintStdErr(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionDebug(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionDescribe(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionDescribeArray(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionSleepSeconds(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionJSCStack(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionGCAndSweep(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionFullGC(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionEdenGC(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionForceGCSlowPaths(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionHeapSize(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionAddressOf(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionGetGetterSetter(ExecState*);
#ifndef NDEBUG
static EncodedJSValue JSC_HOST_CALL functionDumpCallFrame(ExecState*);
#endif
static EncodedJSValue JSC_HOST_CALL functionVersion(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionRun(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionRunString(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionLoad(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionLoadString(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionReadFile(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionCheckSyntax(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionReadline(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionPreciseTime(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionNeverInlineFunction(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionNoDFG(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionNoFTL(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionNoOSRExitFuzzing(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionOptimizeNextInvocation(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionNumberOfDFGCompiles(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionJSCOptions(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionReoptimizationRetryCount(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionTransferArrayBuffer(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionFailNextNewCodeBlock(ExecState*);
static NO_RETURN_WITH_VALUE EncodedJSValue JSC_HOST_CALL functionQuit(ExecState*);
static NO_RETURN_DUE_TO_CRASH EncodedJSValue JSC_HOST_CALL functionAbort(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionFalse1(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionFalse2(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionUndefined1(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionUndefined2(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionIsInt32(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionEffectful42(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionIdentity(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionMakeMasquerader(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionHasCustomProperties(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionDumpTypesForAllVariables(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionFindTypeForExpression(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionReturnTypeFor(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionDumpBasicBlockExecutionRanges(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionHasBasicBlockExecuted(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionBasicBlockExecutionCount(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionEnableExceptionFuzz(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionDrainMicrotasks(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionIs32BitPlatform(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionLoadModule(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionCheckModuleSyntax(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionPlatformSupportsSamplingProfiler(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionGenerateHeapSnapshot(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionResetSuperSamplerState(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionEnsureArrayStorage(ExecState*);
#if ENABLE(SAMPLING_PROFILER)
static EncodedJSValue JSC_HOST_CALL functionStartSamplingProfiler(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionSamplingProfilerStackTraces(ExecState*);
#endif

static EncodedJSValue JSC_HOST_CALL functionMaxArguments(ExecState*);

#if ENABLE(WEBASSEMBLY)
static EncodedJSValue JSC_HOST_CALL functionTestWasmModuleFunctions(ExecState*);
#endif

#if ENABLE(SAMPLING_FLAGS)
static EncodedJSValue JSC_HOST_CALL functionSetSamplingFlags(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionClearSamplingFlags(ExecState*);
#endif

static EncodedJSValue JSC_HOST_CALL functionShadowChickenFunctionsOnStack(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionSetGlobalConstRedeclarationShouldNotThrow(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionGetRandomSeed(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionSetRandomSeed(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionIsRope(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionCallerSourceOrigin(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionGlobalObjectForObject(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionDollarCreateRealm(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionDollarDetachArrayBuffer(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionDollarEvalScript(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionDollarAgentStart(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionDollarAgentReceiveBroadcast(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionDollarAgentReport(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionDollarAgentSleep(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionDollarAgentBroadcast(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionDollarAgentGetReport(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionDollarAgentLeaving(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionWaitForReport(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionHeapCapacity(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionFlashHeapAccess(ExecState*);

struct Script {
    enum class StrictMode {
        Strict,
        Sloppy
    };

    enum class ScriptType {
        Script,
        Module
    };

    enum class CodeSource {
        File,
        CommandLine
    };

    StrictMode strictMode;
    CodeSource codeSource;
    ScriptType scriptType;
    char* argument;

    Script(StrictMode strictMode, CodeSource codeSource, ScriptType scriptType, char *argument)
        : strictMode(strictMode)
        , codeSource(codeSource)
        , scriptType(scriptType)
        , argument(argument)
    {
        if (strictMode == StrictMode::Strict)
            ASSERT(codeSource == CodeSource::File);
    }
};

class CommandLine {
public:
    CommandLine(int argc, char** argv)
    {
        parseArguments(argc, argv);
    }

    bool m_interactive { false };
    bool m_dump { false };
    bool m_module { false };
    bool m_exitCode { false };
    Vector<Script> m_scripts;
    Vector<String> m_arguments;
    bool m_profile { false };
    String m_profilerOutput;
    String m_uncaughtExceptionName;
    bool m_alwaysDumpUncaughtException { false };
    bool m_dumpSamplingProfilerData { false };
    bool m_enableRemoteDebugging { false };

    void parseArguments(int, char**);
};

static const char interactivePrompt[] = ">>> ";

class StopWatch {
public:
    void start();
    void stop();
    long getElapsedMS(); // call stop() first

private:
    double m_startTime;
    double m_stopTime;
};

void StopWatch::start()
{
    m_startTime = monotonicallyIncreasingTime();
}

void StopWatch::stop()
{
    m_stopTime = monotonicallyIncreasingTime();
}

long StopWatch::getElapsedMS()
{
    return static_cast<long>((m_stopTime - m_startTime) * 1000);
}

template<typename Vector>
static inline String stringFromUTF(const Vector& utf8)
{
    return String::fromUTF8WithLatin1Fallback(utf8.data(), utf8.size());
}

template<typename Vector>
static inline SourceCode jscSource(const Vector& utf8, const SourceOrigin& sourceOrigin, const String& filename)
{
    String str = stringFromUTF(utf8);
    return makeSource(str, sourceOrigin, filename);
}

class GlobalObject : public JSGlobalObject {
private:
    GlobalObject(VM&, Structure*);

public:
    typedef JSGlobalObject Base;

    static GlobalObject* create(VM& vm, Structure* structure, const Vector<String>& arguments)
    {
        GlobalObject* object = new (NotNull, allocateCell<GlobalObject>(vm.heap)) GlobalObject(vm, structure);
        object->finishCreation(vm, arguments);
        return object;
    }

    static const bool needsDestruction = false;

    DECLARE_INFO;
    static const GlobalObjectMethodTable s_globalObjectMethodTable;

    static Structure* createStructure(VM& vm, JSValue prototype)
    {
        return Structure::create(vm, 0, prototype, TypeInfo(GlobalObjectType, StructureFlags), info());
    }

    static RuntimeFlags javaScriptRuntimeFlags(const JSGlobalObject*) { return RuntimeFlags::createAllEnabled(); }

protected:
    void finishCreation(VM& vm, const Vector<String>& arguments)
    {
        Base::finishCreation(vm);
        
        addFunction(vm, "debug", functionDebug, 1);
        addFunction(vm, "describe", functionDescribe, 1);
        addFunction(vm, "describeArray", functionDescribeArray, 1);
        addFunction(vm, "print", functionPrintStdOut, 1);
        addFunction(vm, "printErr", functionPrintStdErr, 1);
        addFunction(vm, "quit", functionQuit, 0);
        addFunction(vm, "abort", functionAbort, 0);
        addFunction(vm, "gc", functionGCAndSweep, 0);
        addFunction(vm, "fullGC", functionFullGC, 0);
        addFunction(vm, "edenGC", functionEdenGC, 0);
        addFunction(vm, "forceGCSlowPaths", functionForceGCSlowPaths, 0);
        addFunction(vm, "gcHeapSize", functionHeapSize, 0);
        addFunction(vm, "addressOf", functionAddressOf, 1);
        addFunction(vm, "getGetterSetter", functionGetGetterSetter, 2);
#ifndef NDEBUG
        addFunction(vm, "dumpCallFrame", functionDumpCallFrame, 0);
#endif
        addFunction(vm, "version", functionVersion, 1);
        addFunction(vm, "run", functionRun, 1);
        addFunction(vm, "runString", functionRunString, 1);
        addFunction(vm, "load", functionLoad, 1);
        addFunction(vm, "loadString", functionLoadString, 1);
        addFunction(vm, "readFile", functionReadFile, 2);
        addFunction(vm, "read", functionReadFile, 2);
        addFunction(vm, "checkSyntax", functionCheckSyntax, 1);
        addFunction(vm, "sleepSeconds", functionSleepSeconds, 1);
        addFunction(vm, "jscStack", functionJSCStack, 1);
        addFunction(vm, "readline", functionReadline, 0);
        addFunction(vm, "preciseTime", functionPreciseTime, 0);
        addFunction(vm, "neverInlineFunction", functionNeverInlineFunction, 1);
        addFunction(vm, "noInline", functionNeverInlineFunction, 1);
        addFunction(vm, "noDFG", functionNoDFG, 1);
        addFunction(vm, "noFTL", functionNoFTL, 1);
        addFunction(vm, "noOSRExitFuzzing", functionNoOSRExitFuzzing, 1);
        addFunction(vm, "numberOfDFGCompiles", functionNumberOfDFGCompiles, 1);
        addFunction(vm, "jscOptions", functionJSCOptions, 0);
        addFunction(vm, "optimizeNextInvocation", functionOptimizeNextInvocation, 1);
        addFunction(vm, "reoptimizationRetryCount", functionReoptimizationRetryCount, 1);
        addFunction(vm, "transferArrayBuffer", functionTransferArrayBuffer, 1);
        addFunction(vm, "failNextNewCodeBlock", functionFailNextNewCodeBlock, 1);
#if ENABLE(SAMPLING_FLAGS)
        addFunction(vm, "setSamplingFlags", functionSetSamplingFlags, 1);
        addFunction(vm, "clearSamplingFlags", functionClearSamplingFlags, 1);
#endif
        addFunction(vm, "shadowChickenFunctionsOnStack", functionShadowChickenFunctionsOnStack, 0);
        addFunction(vm, "setGlobalConstRedeclarationShouldNotThrow", functionSetGlobalConstRedeclarationShouldNotThrow, 0);
        addConstructableFunction(vm, "Root", functionCreateRoot, 0);
        addConstructableFunction(vm, "Element", functionCreateElement, 1);
        addFunction(vm, "getElement", functionGetElement, 1);
        addFunction(vm, "setElementRoot", functionSetElementRoot, 2);
        
        addConstructableFunction(vm, "SimpleObject", functionCreateSimpleObject, 0);
        addFunction(vm, "getHiddenValue", functionGetHiddenValue, 1);
        addFunction(vm, "setHiddenValue", functionSetHiddenValue, 2);
        
        putDirectNativeFunction(vm, this, Identifier::fromString(&vm, "DFGTrue"), 0, functionFalse1, DFGTrueIntrinsic, DontEnum);
        putDirectNativeFunction(vm, this, Identifier::fromString(&vm, "OSRExit"), 0, functionUndefined1, OSRExitIntrinsic, DontEnum);
        putDirectNativeFunction(vm, this, Identifier::fromString(&vm, "isFinalTier"), 0, functionFalse2, IsFinalTierIntrinsic, DontEnum);
        putDirectNativeFunction(vm, this, Identifier::fromString(&vm, "predictInt32"), 0, functionUndefined2, SetInt32HeapPredictionIntrinsic, DontEnum);
        putDirectNativeFunction(vm, this, Identifier::fromString(&vm, "isInt32"), 0, functionIsInt32, CheckInt32Intrinsic, DontEnum);
        putDirectNativeFunction(vm, this, Identifier::fromString(&vm, "fiatInt52"), 0, functionIdentity, FiatInt52Intrinsic, DontEnum);
        
        addFunction(vm, "effectful42", functionEffectful42, 0);
        addFunction(vm, "makeMasquerader", functionMakeMasquerader, 0);
        addFunction(vm, "hasCustomProperties", functionHasCustomProperties, 0);

        addFunction(vm, "createProxy", functionCreateProxy, 1);
        addFunction(vm, "createRuntimeArray", functionCreateRuntimeArray, 0);

        addFunction(vm, "createImpureGetter", functionCreateImpureGetter, 1);
        addFunction(vm, "createCustomGetterObject", functionCreateCustomGetterObject, 0);
        addFunction(vm, "createDOMJITNodeObject", functionCreateDOMJITNodeObject, 0);
        addFunction(vm, "createDOMJITGetterObject", functionCreateDOMJITGetterObject, 0);
        addFunction(vm, "createDOMJITGetterComplexObject", functionCreateDOMJITGetterComplexObject, 0);
        addFunction(vm, "createDOMJITFunctionObject", functionCreateDOMJITFunctionObject, 0);
        addFunction(vm, "createBuiltin", functionCreateBuiltin, 2);
        addFunction(vm, "createGlobalObject", functionCreateGlobalObject, 0);
        addFunction(vm, "setImpureGetterDelegate", functionSetImpureGetterDelegate, 2);

        addFunction(vm, "dumpTypesForAllVariables", functionDumpTypesForAllVariables , 0);
        addFunction(vm, "findTypeForExpression", functionFindTypeForExpression, 2);
        addFunction(vm, "returnTypeFor", functionReturnTypeFor, 1);

        addFunction(vm, "dumpBasicBlockExecutionRanges", functionDumpBasicBlockExecutionRanges , 0);
        addFunction(vm, "hasBasicBlockExecuted", functionHasBasicBlockExecuted, 2);
        addFunction(vm, "basicBlockExecutionCount", functionBasicBlockExecutionCount, 2);

        addFunction(vm, "enableExceptionFuzz", functionEnableExceptionFuzz, 0);

        addFunction(vm, "drainMicrotasks", functionDrainMicrotasks, 0);

        addFunction(vm, "getRandomSeed", functionGetRandomSeed, 0);
        addFunction(vm, "setRandomSeed", functionSetRandomSeed, 1);
        addFunction(vm, "isRope", functionIsRope, 1);
        addFunction(vm, "callerSourceOrigin", functionCallerSourceOrigin, 0);

        addFunction(vm, "globalObjectForObject", functionGlobalObjectForObject, 1);

        addFunction(vm, "is32BitPlatform", functionIs32BitPlatform, 0);

        addFunction(vm, "loadModule", functionLoadModule, 1);
        addFunction(vm, "checkModuleSyntax", functionCheckModuleSyntax, 1);

        addFunction(vm, "platformSupportsSamplingProfiler", functionPlatformSupportsSamplingProfiler, 0);
        addFunction(vm, "generateHeapSnapshot", functionGenerateHeapSnapshot, 0);
        addFunction(vm, "resetSuperSamplerState", functionResetSuperSamplerState, 0);
        addFunction(vm, "ensureArrayStorage", functionEnsureArrayStorage, 0);
#if ENABLE(SAMPLING_PROFILER)
        addFunction(vm, "startSamplingProfiler", functionStartSamplingProfiler, 0);
        addFunction(vm, "samplingProfilerStackTraces", functionSamplingProfilerStackTraces, 0);
#endif

        addFunction(vm, "maxArguments", functionMaxArguments, 0);

#if ENABLE(WEBASSEMBLY)
        addFunction(vm, "testWasmModuleFunctions", functionTestWasmModuleFunctions, 0);
#endif

        if (!arguments.isEmpty()) {
            JSArray* array = constructEmptyArray(globalExec(), 0);
            for (size_t i = 0; i < arguments.size(); ++i)
                array->putDirectIndex(globalExec(), i, jsString(globalExec(), arguments[i]));
            putDirect(vm, Identifier::fromString(globalExec(), "arguments"), array);
        }

        putDirect(vm, Identifier::fromString(globalExec(), "console"), jsUndefined());
        
        Structure* plainObjectStructure = JSFinalObject::createStructure(vm, this, objectPrototype(), 0);
        
        JSObject* dollar = JSFinalObject::create(vm, plainObjectStructure);
        putDirect(vm, Identifier::fromString(globalExec(), "$"), dollar);
        
        addFunction(vm, dollar, "createRealm", functionDollarCreateRealm, 0);
        addFunction(vm, dollar, "detachArrayBuffer", functionDollarDetachArrayBuffer, 1);
        addFunction(vm, dollar, "evalScript", functionDollarEvalScript, 1);
        
        dollar->putDirect(vm, Identifier::fromString(globalExec(), "global"), this);
        
        JSObject* agent = JSFinalObject::create(vm, plainObjectStructure);
        dollar->putDirect(vm, Identifier::fromString(globalExec(), "agent"), agent);
        
        // The test262 INTERPRETING.md document says that some of these functions are just in the main
        // thread and some are in the other threads. We just put them in all threads.
        addFunction(vm, agent, "start", functionDollarAgentStart, 1);
        addFunction(vm, agent, "receiveBroadcast", functionDollarAgentReceiveBroadcast, 1);
        addFunction(vm, agent, "report", functionDollarAgentReport, 1);
        addFunction(vm, agent, "sleep", functionDollarAgentSleep, 1);
        addFunction(vm, agent, "broadcast", functionDollarAgentBroadcast, 1);
        addFunction(vm, agent, "getReport", functionDollarAgentGetReport, 0);
        addFunction(vm, agent, "leaving", functionDollarAgentLeaving, 0);
        
        addFunction(vm, "waitForReport", functionWaitForReport, 0);

        addFunction(vm, "heapCapacity", functionHeapCapacity, 0);
        addFunction(vm, "flashHeapAccess", functionFlashHeapAccess, 0);
    }
    
    void addFunction(VM& vm, JSObject* object, const char* name, NativeFunction function, unsigned arguments)
    {
        Identifier identifier = Identifier::fromString(&vm, name);
        object->putDirect(vm, identifier, JSFunction::create(vm, this, arguments, identifier.string(), function));
    }

    void addFunction(VM& vm, const char* name, NativeFunction function, unsigned arguments)
    {
        addFunction(vm, this, name, function, arguments);
    }
    
    void addConstructableFunction(VM& vm, const char* name, NativeFunction function, unsigned arguments)
    {
        Identifier identifier = Identifier::fromString(&vm, name);
        putDirect(vm, identifier, JSFunction::create(vm, this, arguments, identifier.string(), function, NoIntrinsic, function));
    }

    static JSInternalPromise* moduleLoaderImportModule(JSGlobalObject*, ExecState*, JSModuleLoader*, JSString*, const SourceOrigin&);
    static JSInternalPromise* moduleLoaderResolve(JSGlobalObject*, ExecState*, JSModuleLoader*, JSValue, JSValue, JSValue);
    static JSInternalPromise* moduleLoaderFetch(JSGlobalObject*, ExecState*, JSModuleLoader*, JSValue, JSValue);
};

const ClassInfo GlobalObject::s_info = { "global", &JSGlobalObject::s_info, nullptr, CREATE_METHOD_TABLE(GlobalObject) };
const GlobalObjectMethodTable GlobalObject::s_globalObjectMethodTable = {
    &supportsRichSourceInfo,
    &shouldInterruptScript,
    &javaScriptRuntimeFlags,
    nullptr,
    &shouldInterruptScriptBeforeTimeout,
    &moduleLoaderImportModule,
    &moduleLoaderResolve,
    &moduleLoaderFetch,
    nullptr,
    nullptr,
    nullptr
};

GlobalObject::GlobalObject(VM& vm, Structure* structure)
    : JSGlobalObject(vm, structure, &s_globalObjectMethodTable)
{
}

static UChar pathSeparator()
{
#if OS(WINDOWS)
    return '\\';
#else
    return '/';
#endif
}

struct DirectoryName {
    // In unix, it is "/". In Windows, it becomes a drive letter like "C:\"
    String rootName;

    // If the directory name is "/home/WebKit", this becomes "home/WebKit". If the directory name is "/", this becomes "".
    String queryName;
};

struct ModuleName {
    ModuleName(const String& moduleName);

    bool startsWithRoot() const
    {
        return !queries.isEmpty() && queries[0].isEmpty();
    }

    Vector<String> queries;
};

ModuleName::ModuleName(const String& moduleName)
{
    // A module name given from code is represented as the UNIX style path. Like, `./A/B.js`.
    moduleName.split('/', true, queries);
}

static std::optional<DirectoryName> extractDirectoryName(const String& absolutePathToFile)
{
    size_t firstSeparatorPosition = absolutePathToFile.find(pathSeparator());
    if (firstSeparatorPosition == notFound)
        return std::nullopt;
    DirectoryName directoryName;
    directoryName.rootName = absolutePathToFile.substring(0, firstSeparatorPosition + 1); // Include the separator.
    size_t lastSeparatorPosition = absolutePathToFile.reverseFind(pathSeparator());
    ASSERT_WITH_MESSAGE(lastSeparatorPosition != notFound, "If the separator is not found, this function already returns when performing the forward search.");
    if (firstSeparatorPosition == lastSeparatorPosition)
        directoryName.queryName = StringImpl::empty();
    else {
        size_t queryStartPosition = firstSeparatorPosition + 1;
        size_t queryLength = lastSeparatorPosition - queryStartPosition; // Not include the last separator.
        directoryName.queryName = absolutePathToFile.substring(queryStartPosition, queryLength);
    }
    return directoryName;
}

static std::optional<DirectoryName> currentWorkingDirectory()
{
#if OS(WINDOWS)
    // https://msdn.microsoft.com/en-us/library/windows/desktop/aa364934.aspx
    // https://msdn.microsoft.com/en-us/library/windows/desktop/aa365247.aspx#maxpath
    // The _MAX_PATH in Windows is 260. If the path of the current working directory is longer than that, _getcwd truncates the result.
    // And other I/O functions taking a path name also truncate it. To avoid this situation,
    //
    // (1). When opening the file in Windows for modules, we always use the abosolute path and add "\\?\" prefix to the path name.
    // (2). When retrieving the current working directory, use GetCurrentDirectory instead of _getcwd.
    //
    // In the path utility functions inside the JSC shell, we does not handle the UNC and UNCW including the network host name.
    DWORD bufferLength = ::GetCurrentDirectoryW(0, nullptr);
    if (!bufferLength)
        return std::nullopt;
    // In Windows, wchar_t is the UTF-16LE.
    // https://msdn.microsoft.com/en-us/library/dd374081.aspx
    // https://msdn.microsoft.com/en-us/library/windows/desktop/ff381407.aspx
    auto buffer = std::make_unique<wchar_t[]>(bufferLength);
    DWORD lengthNotIncludingNull = ::GetCurrentDirectoryW(bufferLength, buffer.get());
    static_assert(sizeof(wchar_t) == sizeof(UChar), "In Windows, both are UTF-16LE");
    String directoryString = String(reinterpret_cast<UChar*>(buffer.get()));
    // We don't support network path like \\host\share\<path name>.
    if (directoryString.startsWith("\\\\"))
        return std::nullopt;
#else
    auto buffer = std::make_unique<char[]>(PATH_MAX);
    if (!getcwd(buffer.get(), PATH_MAX))
        return std::nullopt;
    String directoryString = String::fromUTF8(buffer.get());
#endif
    if (directoryString.isEmpty())
        return std::nullopt;

    if (directoryString[directoryString.length() - 1] == pathSeparator())
        return extractDirectoryName(directoryString);
    // Append the seperator to represents the file name. extractDirectoryName only accepts the absolute file name.
    return extractDirectoryName(makeString(directoryString, pathSeparator()));
}

static String resolvePath(const DirectoryName& directoryName, const ModuleName& moduleName)
{
    Vector<String> directoryPieces;
    directoryName.queryName.split(pathSeparator(), false, directoryPieces);

    // Only first '/' is recognized as the path from the root.
    if (moduleName.startsWithRoot())
        directoryPieces.clear();

    for (const auto& query : moduleName.queries) {
        if (query == String(ASCIILiteral(".."))) {
            if (!directoryPieces.isEmpty())
                directoryPieces.removeLast();
        } else if (!query.isEmpty() && query != String(ASCIILiteral(".")))
            directoryPieces.append(query);
    }

    StringBuilder builder;
    builder.append(directoryName.rootName);
    for (size_t i = 0; i < directoryPieces.size(); ++i) {
        builder.append(directoryPieces[i]);
        if (i + 1 != directoryPieces.size())
            builder.append(pathSeparator());
    }
    return builder.toString();
}

static String absolutePath(const String& fileName)
{
    auto directoryName = currentWorkingDirectory();
    if (!directoryName)
        return fileName;
    return resolvePath(directoryName.value(), ModuleName(fileName.impl()));
}

JSInternalPromise* GlobalObject::moduleLoaderImportModule(JSGlobalObject* globalObject, ExecState* exec, JSModuleLoader*, JSString* moduleNameValue, const SourceOrigin& sourceOrigin)
{
    VM& vm = globalObject->vm();
    auto scope = DECLARE_CATCH_SCOPE(vm);

    auto rejectPromise = [&] (JSValue error) {
        return JSInternalPromiseDeferred::create(exec, globalObject)->reject(exec, error);
    };

    if (sourceOrigin.isNull())
        return rejectPromise(createError(exec, ASCIILiteral("Could not resolve the module specifier.")));

    auto referrer = sourceOrigin.string();
    auto moduleName = moduleNameValue->value(exec);
    if (UNLIKELY(scope.exception())) {
        JSValue exception = scope.exception();
        scope.clearException();
        return rejectPromise(exception);
    }

    auto directoryName = extractDirectoryName(referrer.impl());
    if (!directoryName)
        return rejectPromise(createError(exec, makeString("Could not resolve the referrer name '", String(referrer.impl()), "'.")));

    return JSC::importModule(exec, Identifier::fromString(&vm, resolvePath(directoryName.value(), ModuleName(moduleName))), jsUndefined());
}

JSInternalPromise* GlobalObject::moduleLoaderResolve(JSGlobalObject* globalObject, ExecState* exec, JSModuleLoader*, JSValue keyValue, JSValue referrerValue, JSValue)
{
    VM& vm = globalObject->vm();
    auto scope = DECLARE_CATCH_SCOPE(vm);

    JSInternalPromiseDeferred* deferred = JSInternalPromiseDeferred::create(exec, globalObject);
    RELEASE_ASSERT(!scope.exception());
    const Identifier key = keyValue.toPropertyKey(exec);
    if (UNLIKELY(scope.exception())) {
        JSValue exception = scope.exception();
        scope.clearException();
        return deferred->reject(exec, exception);
    }

    if (key.isSymbol())
        return deferred->resolve(exec, keyValue);

    if (referrerValue.isUndefined()) {
        auto directoryName = currentWorkingDirectory();
        if (!directoryName)
            return deferred->reject(exec, createError(exec, ASCIILiteral("Could not resolve the current working directory.")));
        return deferred->resolve(exec, jsString(exec, resolvePath(directoryName.value(), ModuleName(key.impl()))));
    }

    const Identifier referrer = referrerValue.toPropertyKey(exec);
    if (UNLIKELY(scope.exception())) {
        JSValue exception = scope.exception();
        scope.clearException();
        return deferred->reject(exec, exception);
    }

    if (referrer.isSymbol()) {
        auto directoryName = currentWorkingDirectory();
        if (!directoryName)
            return deferred->reject(exec, createError(exec, ASCIILiteral("Could not resolve the current working directory.")));
        return deferred->resolve(exec, jsString(exec, resolvePath(directoryName.value(), ModuleName(key.impl()))));
    }

    // If the referrer exists, we assume that the referrer is the correct absolute path.
    auto directoryName = extractDirectoryName(referrer.impl());
    if (!directoryName)
        return deferred->reject(exec, createError(exec, makeString("Could not resolve the referrer name '", String(referrer.impl()), "'.")));
    auto result = deferred->resolve(exec, jsString(exec, resolvePath(directoryName.value(), ModuleName(key.impl()))));
    RELEASE_ASSERT(!scope.exception());
    return result;
}

static void convertShebangToJSComment(Vector<char>& buffer)
{
    if (buffer.size() >= 2) {
        if (buffer[0] == '#' && buffer[1] == '!')
            buffer[0] = buffer[1] = '/';
    }
}

static bool fillBufferWithContentsOfFile(FILE* file, Vector<char>& buffer)
{
    // We might have injected "use strict"; at the top.
    size_t initialSize = buffer.size();
    fseek(file, 0, SEEK_END);
    size_t bufferCapacity = ftell(file);
    fseek(file, 0, SEEK_SET);
    buffer.resize(bufferCapacity + initialSize);
    size_t readSize = fread(buffer.data() + initialSize, 1, buffer.size(), file);
    return readSize == buffer.size() - initialSize;
}

static bool fillBufferWithContentsOfFile(const String& fileName, Vector<char>& buffer)
{
    FILE* f = fopen(fileName.utf8().data(), "rb");
    if (!f) {
        fprintf(stderr, "Could not open file: %s\n", fileName.utf8().data());
        return false;
    }

    bool result = fillBufferWithContentsOfFile(f, buffer);
    fclose(f);

    return result;
}

static bool fetchScriptFromLocalFileSystem(const String& fileName, Vector<char>& buffer)
{
    if (!fillBufferWithContentsOfFile(fileName, buffer))
        return false;
    convertShebangToJSComment(buffer);
    return true;
}

static bool fetchModuleFromLocalFileSystem(const String& fileName, Vector<char>& buffer)
{
    // We assume that fileName is always an absolute path.
#if OS(WINDOWS)
    // https://msdn.microsoft.com/en-us/library/windows/desktop/aa365247.aspx#maxpath
    // Use long UNC to pass the long path name to the Windows APIs.
    String longUNCPathName = WTF::makeString("\\\\?\\", fileName);
    static_assert(sizeof(wchar_t) == sizeof(UChar), "In Windows, both are UTF-16LE");
    auto utf16Vector = longUNCPathName.charactersWithNullTermination();
    FILE* f = _wfopen(reinterpret_cast<wchar_t*>(utf16Vector.data()), L"rb");
#else
    FILE* f = fopen(fileName.utf8().data(), "r");
#endif
    if (!f) {
        fprintf(stderr, "Could not open file: %s\n", fileName.utf8().data());
        return false;
    }

    bool result = fillBufferWithContentsOfFile(f, buffer);
    if (result)
        convertShebangToJSComment(buffer);
    fclose(f);

    return result;
}

JSInternalPromise* GlobalObject::moduleLoaderFetch(JSGlobalObject* globalObject, ExecState* exec, JSModuleLoader*, JSValue key, JSValue)
{
    VM& vm = globalObject->vm();
    auto scope = DECLARE_CATCH_SCOPE(vm);
    JSInternalPromiseDeferred* deferred = JSInternalPromiseDeferred::create(exec, globalObject);
    String moduleKey = key.toWTFString(exec);
    if (UNLIKELY(scope.exception())) {
        JSValue exception = scope.exception();
        scope.clearException();
        return deferred->reject(exec, exception);
    }

    // Here, now we consider moduleKey as the fileName.
    Vector<char> utf8;
    if (!fetchModuleFromLocalFileSystem(moduleKey, utf8))
        return deferred->reject(exec, createError(exec, makeString("Could not open file '", moduleKey, "'.")));

    auto result = deferred->resolve(exec, JSSourceCode::create(exec->vm(), makeSource(stringFromUTF(utf8), SourceOrigin { moduleKey }, moduleKey, TextPosition(), SourceProviderSourceType::Module)));
    RELEASE_ASSERT(!scope.exception());
    return result;
}


static EncodedJSValue printInternal(ExecState* exec, FILE* out)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (test262AsyncTest) {
        JSValue value = exec->argument(0);
        if (value.isString() && WTF::equal(asString(value)->value(exec).impl(), "Test262:AsyncTestComplete"))
            test262AsyncPassed = true;
        return JSValue::encode(jsUndefined());
    }

    for (unsigned i = 0; i < exec->argumentCount(); ++i) {
        if (i)
            if (EOF == fputc(' ', out))
                goto fail;

        auto viewWithString = exec->uncheckedArgument(i).toString(exec)->viewWithUnderlyingString(*exec);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
        if (fprintf(out, "%s", viewWithString.view.utf8().data()) < 0)
            goto fail;
    }

    fputc('\n', out);
fail:
    fflush(out);
    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL functionPrintStdOut(ExecState* exec) { return printInternal(exec, stdout); }
EncodedJSValue JSC_HOST_CALL functionPrintStdErr(ExecState* exec) { return printInternal(exec, stderr); }

#ifndef NDEBUG
EncodedJSValue JSC_HOST_CALL functionDumpCallFrame(ExecState* exec)
{
    VMEntryFrame* topVMEntryFrame = exec->vm().topVMEntryFrame;
    ExecState* callerFrame = exec->callerFrame(topVMEntryFrame);
    if (callerFrame)
        exec->vm().interpreter->dumpCallFrame(callerFrame);
    return JSValue::encode(jsUndefined());
}
#endif

EncodedJSValue JSC_HOST_CALL functionDebug(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    auto viewWithString = exec->argument(0).toString(exec)->viewWithUnderlyingString(*exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    fprintf(stderr, "--> %s\n", viewWithString.view.utf8().data());
    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL functionDescribe(ExecState* exec)
{
    if (exec->argumentCount() < 1)
        return JSValue::encode(jsUndefined());
    return JSValue::encode(jsString(exec, toString(exec->argument(0))));
}

EncodedJSValue JSC_HOST_CALL functionDescribeArray(ExecState* exec)
{
    if (exec->argumentCount() < 1)
        return JSValue::encode(jsUndefined());
    VM& vm = exec->vm();
    JSObject* object = jsDynamicCast<JSObject*>(vm, exec->argument(0));
    if (!object)
        return JSValue::encode(jsNontrivialString(exec, ASCIILiteral("<not object>")));
    return JSValue::encode(jsNontrivialString(exec, toString("<Butterfly: ", RawPointer(object->butterfly()), "; public length: ", object->getArrayLength(), "; vector length: ", object->getVectorLength(), ">")));
}

EncodedJSValue JSC_HOST_CALL functionSleepSeconds(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (exec->argumentCount() >= 1) {
        Seconds seconds = Seconds(exec->argument(0).toNumber(exec));
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
        sleep(seconds);
    }
    
    return JSValue::encode(jsUndefined());
}

class FunctionJSCStackFunctor {
public:
    FunctionJSCStackFunctor(StringBuilder& trace)
        : m_trace(trace)
    {
    }

    StackVisitor::Status operator()(StackVisitor& visitor) const
    {
        m_trace.append(String::format("    %zu   %s\n", visitor->index(), visitor->toString().utf8().data()));
        return StackVisitor::Continue;
    }

private:
    StringBuilder& m_trace;
};

EncodedJSValue JSC_HOST_CALL functionJSCStack(ExecState* exec)
{
    StringBuilder trace;
    trace.appendLiteral("--> Stack trace:\n");

    FunctionJSCStackFunctor functor(trace);
    exec->iterate(functor);
    fprintf(stderr, "%s", trace.toString().utf8().data());
    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL functionCreateRoot(ExecState* exec)
{
    JSLockHolder lock(exec);
    return JSValue::encode(Root::create(exec->vm(), exec->lexicalGlobalObject()));
}

EncodedJSValue JSC_HOST_CALL functionCreateElement(ExecState* exec)
{
    VM& vm = exec->vm();
    JSLockHolder lock(vm);
    auto scope = DECLARE_THROW_SCOPE(vm);

    Root* root = jsDynamicCast<Root*>(vm, exec->argument(0));
    if (!root)
        return JSValue::encode(throwException(exec, scope, createError(exec, ASCIILiteral("Cannot create Element without a Root."))));
    return JSValue::encode(Element::create(vm, exec->lexicalGlobalObject(), root));
}

EncodedJSValue JSC_HOST_CALL functionGetElement(ExecState* exec)
{
    JSLockHolder lock(exec);
    VM& vm = exec->vm();
    Root* root = jsDynamicCast<Root*>(vm, exec->argument(0));
    if (!root)
        return JSValue::encode(jsUndefined());
    Element* result = root->element();
    return JSValue::encode(result ? result : jsUndefined());
}

EncodedJSValue JSC_HOST_CALL functionSetElementRoot(ExecState* exec)
{
    JSLockHolder lock(exec);
    VM& vm = exec->vm();
    Element* element = jsDynamicCast<Element*>(vm, exec->argument(0));
    Root* root = jsDynamicCast<Root*>(vm, exec->argument(1));
    if (element && root)
        element->setRoot(exec->vm(), root);
    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL functionCreateSimpleObject(ExecState* exec)
{
    JSLockHolder lock(exec);
    return JSValue::encode(SimpleObject::create(exec->vm(), exec->lexicalGlobalObject()));
}

EncodedJSValue JSC_HOST_CALL functionGetHiddenValue(ExecState* exec)
{
    VM& vm = exec->vm();
    JSLockHolder lock(vm);
    auto scope = DECLARE_THROW_SCOPE(vm);

    SimpleObject* simpleObject = jsDynamicCast<SimpleObject*>(vm, exec->argument(0));
    if (UNLIKELY(!simpleObject)) {
        throwTypeError(exec, scope, ASCIILiteral("Invalid use of getHiddenValue test function"));
        return encodedJSValue();
    }
    return JSValue::encode(simpleObject->hiddenValue());
}

EncodedJSValue JSC_HOST_CALL functionSetHiddenValue(ExecState* exec)
{
    VM& vm = exec->vm();
    JSLockHolder lock(vm);
    auto scope = DECLARE_THROW_SCOPE(vm);

    SimpleObject* simpleObject = jsDynamicCast<SimpleObject*>(vm, exec->argument(0));
    if (UNLIKELY(!simpleObject)) {
        throwTypeError(exec, scope, ASCIILiteral("Invalid use of setHiddenValue test function"));
        return encodedJSValue();
    }
    JSValue value = exec->argument(1);
    simpleObject->setHiddenValue(exec->vm(), value);
    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL functionCreateProxy(ExecState* exec)
{
    JSLockHolder lock(exec);
    JSValue target = exec->argument(0);
    if (!target.isObject())
        return JSValue::encode(jsUndefined());
    JSObject* jsTarget = asObject(target.asCell());
    Structure* structure = JSProxy::createStructure(exec->vm(), exec->lexicalGlobalObject(), jsTarget->getPrototypeDirect(), ImpureProxyType);
    JSProxy* proxy = JSProxy::create(exec->vm(), structure, jsTarget);
    return JSValue::encode(proxy);
}

EncodedJSValue JSC_HOST_CALL functionCreateRuntimeArray(ExecState* exec)
{
    JSLockHolder lock(exec);
    RuntimeArray* array = RuntimeArray::create(exec);
    return JSValue::encode(array);
}

EncodedJSValue JSC_HOST_CALL functionCreateImpureGetter(ExecState* exec)
{
    JSLockHolder lock(exec);
    JSValue target = exec->argument(0);
    JSObject* delegate = nullptr;
    if (target.isObject())
        delegate = asObject(target.asCell());
    Structure* structure = ImpureGetter::createStructure(exec->vm(), exec->lexicalGlobalObject(), jsNull());
    ImpureGetter* result = ImpureGetter::create(exec->vm(), structure, delegate);
    return JSValue::encode(result);
}

EncodedJSValue JSC_HOST_CALL functionCreateCustomGetterObject(ExecState* exec)
{
    JSLockHolder lock(exec);
    Structure* structure = CustomGetter::createStructure(exec->vm(), exec->lexicalGlobalObject(), jsNull());
    CustomGetter* result = CustomGetter::create(exec->vm(), structure);
    return JSValue::encode(result);
}

EncodedJSValue JSC_HOST_CALL functionCreateDOMJITNodeObject(ExecState* exec)
{
    JSLockHolder lock(exec);
    Structure* structure = DOMJITNode::createStructure(exec->vm(), exec->lexicalGlobalObject(), DOMJITGetter::create(exec->vm(), DOMJITGetter::createStructure(exec->vm(), exec->lexicalGlobalObject(), jsNull())));
    DOMJITNode* result = DOMJITNode::create(exec->vm(), structure);
    return JSValue::encode(result);
}

EncodedJSValue JSC_HOST_CALL functionCreateDOMJITGetterObject(ExecState* exec)
{
    JSLockHolder lock(exec);
    Structure* structure = DOMJITGetter::createStructure(exec->vm(), exec->lexicalGlobalObject(), jsNull());
    DOMJITGetter* result = DOMJITGetter::create(exec->vm(), structure);
    return JSValue::encode(result);
}

EncodedJSValue JSC_HOST_CALL functionCreateDOMJITGetterComplexObject(ExecState* exec)
{
    JSLockHolder lock(exec);
    Structure* structure = DOMJITGetterComplex::createStructure(exec->vm(), exec->lexicalGlobalObject(), jsNull());
    DOMJITGetterComplex* result = DOMJITGetterComplex::create(exec->vm(), exec->lexicalGlobalObject(), structure);
    return JSValue::encode(result);
}

EncodedJSValue JSC_HOST_CALL functionCreateDOMJITFunctionObject(ExecState* exec)
{
    JSLockHolder lock(exec);
    Structure* structure = DOMJITFunctionObject::createStructure(exec->vm(), exec->lexicalGlobalObject(), jsNull());
    DOMJITFunctionObject* result = DOMJITFunctionObject::create(exec->vm(), exec->lexicalGlobalObject(), structure);
    return JSValue::encode(result);
}

EncodedJSValue JSC_HOST_CALL functionSetImpureGetterDelegate(ExecState* exec)
{
    VM& vm = exec->vm();
    JSLockHolder lock(vm);
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue base = exec->argument(0);
    if (!base.isObject())
        return JSValue::encode(jsUndefined());
    JSValue delegate = exec->argument(1);
    if (!delegate.isObject())
        return JSValue::encode(jsUndefined());
    ImpureGetter* impureGetter = jsDynamicCast<ImpureGetter*>(vm, asObject(base.asCell()));
    if (UNLIKELY(!impureGetter)) {
        throwTypeError(exec, scope, ASCIILiteral("argument is not an ImpureGetter"));
        return encodedJSValue();
    }
    impureGetter->setDelegate(vm, asObject(delegate.asCell()));
    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL functionGCAndSweep(ExecState* exec)
{
    JSLockHolder lock(exec);
    exec->heap()->collectAllGarbage();
    return JSValue::encode(jsNumber(exec->heap()->sizeAfterLastFullCollection()));
}

EncodedJSValue JSC_HOST_CALL functionFullGC(ExecState* exec)
{
    JSLockHolder lock(exec);
    exec->heap()->collectSync(CollectionScope::Full);
    return JSValue::encode(jsNumber(exec->heap()->sizeAfterLastFullCollection()));
}

EncodedJSValue JSC_HOST_CALL functionEdenGC(ExecState* exec)
{
    JSLockHolder lock(exec);
    exec->heap()->collectSync(CollectionScope::Eden);
    return JSValue::encode(jsNumber(exec->heap()->sizeAfterLastEdenCollection()));
}

EncodedJSValue JSC_HOST_CALL functionForceGCSlowPaths(ExecState*)
{
    // It's best for this to be the first thing called in the 
    // JS program so the option is set to true before we JIT.
    Options::forceGCSlowPaths() = true;
    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL functionHeapSize(ExecState* exec)
{
    JSLockHolder lock(exec);
    return JSValue::encode(jsNumber(exec->heap()->size()));
}

// This function is not generally very helpful in 64-bit code as the tag and payload
// share a register. But in 32-bit JITed code the tag may not be checked if an
// optimization removes type checking requirements, such as in ===.
EncodedJSValue JSC_HOST_CALL functionAddressOf(ExecState* exec)
{
    JSValue value = exec->argument(0);
    if (!value.isCell())
        return JSValue::encode(jsUndefined());
    // Need to cast to uint64_t so bitwise_cast will play along.
    uint64_t asNumber = reinterpret_cast<uint64_t>(value.asCell());
    EncodedJSValue returnValue = JSValue::encode(jsNumber(bitwise_cast<double>(asNumber)));
    return returnValue;
}

static EncodedJSValue JSC_HOST_CALL functionGetGetterSetter(ExecState* exec)
{
    JSValue value = exec->argument(0);
    if (!value.isObject())
        return JSValue::encode(jsUndefined());

    JSValue property = exec->argument(1);
    if (!property.isString())
        return JSValue::encode(jsUndefined());

    PropertySlot slot(value, PropertySlot::InternalMethodType::VMInquiry);
    value.getPropertySlot(exec, asString(property)->toIdentifier(exec), slot);

    JSValue result;
    if (slot.isCacheableGetter())
        result = slot.getterSetter();
    else
        result = jsNull();

    return JSValue::encode(result);
}

EncodedJSValue JSC_HOST_CALL functionVersion(ExecState*)
{
    // We need this function for compatibility with the Mozilla JS tests but for now
    // we don't actually do any version-specific handling
    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL functionRun(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    String fileName = exec->argument(0).toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    Vector<char> script;
    if (!fetchScriptFromLocalFileSystem(fileName, script))
        return JSValue::encode(throwException(exec, scope, createError(exec, ASCIILiteral("Could not open file."))));

    GlobalObject* globalObject = GlobalObject::create(vm, GlobalObject::createStructure(vm, jsNull()), Vector<String>());

    JSArray* array = constructEmptyArray(globalObject->globalExec(), 0);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    for (unsigned i = 1; i < exec->argumentCount(); ++i) {
        array->putDirectIndex(globalObject->globalExec(), i - 1, exec->uncheckedArgument(i));
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
    }
    globalObject->putDirect(
        vm, Identifier::fromString(globalObject->globalExec(), "arguments"), array);

    NakedPtr<Exception> exception;
    StopWatch stopWatch;
    stopWatch.start();
    evaluate(globalObject->globalExec(), jscSource(script, SourceOrigin { absolutePath(fileName) }, fileName), JSValue(), exception);
    stopWatch.stop();

    if (exception) {
        throwException(globalObject->globalExec(), scope, exception);
        return JSValue::encode(jsUndefined());
    }
    
    return JSValue::encode(jsNumber(stopWatch.getElapsedMS()));
}

EncodedJSValue JSC_HOST_CALL functionRunString(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    String source = exec->argument(0).toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    GlobalObject* globalObject = GlobalObject::create(vm, GlobalObject::createStructure(vm, jsNull()), Vector<String>());

    JSArray* array = constructEmptyArray(globalObject->globalExec(), 0);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    for (unsigned i = 1; i < exec->argumentCount(); ++i) {
        array->putDirectIndex(globalObject->globalExec(), i - 1, exec->uncheckedArgument(i));
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
    }
    globalObject->putDirect(
        vm, Identifier::fromString(globalObject->globalExec(), "arguments"), array);

    NakedPtr<Exception> exception;
    evaluate(globalObject->globalExec(), makeSource(source, exec->callerSourceOrigin()), JSValue(), exception);

    if (exception) {
        scope.throwException(globalObject->globalExec(), exception);
        return JSValue::encode(jsUndefined());
    }
    
    return JSValue::encode(globalObject);
}

EncodedJSValue JSC_HOST_CALL functionLoad(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    String fileName = exec->argument(0).toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    Vector<char> script;
    if (!fetchScriptFromLocalFileSystem(fileName, script))
        return JSValue::encode(throwException(exec, scope, createError(exec, ASCIILiteral("Could not open file."))));

    JSGlobalObject* globalObject = exec->lexicalGlobalObject();
    
    NakedPtr<Exception> evaluationException;
    JSValue result = evaluate(globalObject->globalExec(), jscSource(script, SourceOrigin { absolutePath(fileName) }, fileName), JSValue(), evaluationException);
    if (evaluationException)
        throwException(exec, scope, evaluationException);
    return JSValue::encode(result);
}

EncodedJSValue JSC_HOST_CALL functionLoadString(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    String sourceCode = exec->argument(0).toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    JSGlobalObject* globalObject = exec->lexicalGlobalObject();

    NakedPtr<Exception> evaluationException;
    JSValue result = evaluate(globalObject->globalExec(), makeSource(sourceCode, exec->callerSourceOrigin()), JSValue(), evaluationException);
    if (evaluationException)
        throwException(exec, scope, evaluationException);
    return JSValue::encode(result);
}

EncodedJSValue JSC_HOST_CALL functionReadFile(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    String fileName = exec->argument(0).toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    bool isBinary = false;
    if (exec->argumentCount() > 1) {
        String type = exec->argument(1).toWTFString(exec);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
        if (type != "binary")
            return throwVMError(exec, scope, "Expected 'binary' as second argument.");
        isBinary = true;
    }

    Vector<char> content;
    if (!fillBufferWithContentsOfFile(fileName, content))
        return throwVMError(exec, scope, "Could not open file.");

    if (!isBinary)
        return JSValue::encode(jsString(exec, stringFromUTF(content)));

    Structure* structure = exec->lexicalGlobalObject()->typedArrayStructure(TypeUint8);
    auto length = content.size();
    JSObject* result = createUint8TypedArray(exec, structure, ArrayBuffer::createFromBytes(content.releaseBuffer().leakPtr(), length, [] (void* p) { fastFree(p); }), 0, length);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    return JSValue::encode(result);
}

EncodedJSValue JSC_HOST_CALL functionCheckSyntax(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    String fileName = exec->argument(0).toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    Vector<char> script;
    if (!fetchScriptFromLocalFileSystem(fileName, script))
        return JSValue::encode(throwException(exec, scope, createError(exec, ASCIILiteral("Could not open file."))));

    JSGlobalObject* globalObject = exec->lexicalGlobalObject();

    StopWatch stopWatch;
    stopWatch.start();

    JSValue syntaxException;
    bool validSyntax = checkSyntax(globalObject->globalExec(), jscSource(script, SourceOrigin { absolutePath(fileName) }, fileName), &syntaxException);
    stopWatch.stop();

    if (!validSyntax)
        throwException(exec, scope, syntaxException);
    return JSValue::encode(jsNumber(stopWatch.getElapsedMS()));
}

#if ENABLE(SAMPLING_FLAGS)
EncodedJSValue JSC_HOST_CALL functionSetSamplingFlags(ExecState* exec)
{
    for (unsigned i = 0; i < exec->argumentCount(); ++i) {
        unsigned flag = static_cast<unsigned>(exec->uncheckedArgument(i).toNumber(exec));
        if ((flag >= 1) && (flag <= 32))
            SamplingFlags::setFlag(flag);
    }
    return JSValue::encode(jsNull());
}

EncodedJSValue JSC_HOST_CALL functionClearSamplingFlags(ExecState* exec)
{
    for (unsigned i = 0; i < exec->argumentCount(); ++i) {
        unsigned flag = static_cast<unsigned>(exec->uncheckedArgument(i).toNumber(exec));
        if ((flag >= 1) && (flag <= 32))
            SamplingFlags::clearFlag(flag);
    }
    return JSValue::encode(jsNull());
}
#endif

EncodedJSValue JSC_HOST_CALL functionShadowChickenFunctionsOnStack(ExecState* exec)
{
    return JSValue::encode(exec->vm().shadowChicken().functionsOnStack(exec));
}

EncodedJSValue JSC_HOST_CALL functionSetGlobalConstRedeclarationShouldNotThrow(ExecState* exec)
{
    exec->vm().setGlobalConstRedeclarationShouldThrow(false);
    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL functionGetRandomSeed(ExecState* exec)
{
    return JSValue::encode(jsNumber(exec->lexicalGlobalObject()->weakRandom().seed()));
}

EncodedJSValue JSC_HOST_CALL functionSetRandomSeed(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    unsigned seed = exec->argument(0).toUInt32(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    exec->lexicalGlobalObject()->weakRandom().setSeed(seed);
    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL functionIsRope(ExecState* exec)
{
    JSValue argument = exec->argument(0);
    if (!argument.isString())
        return JSValue::encode(jsBoolean(false));
    const StringImpl* impl = asString(argument)->tryGetValueImpl();
    return JSValue::encode(jsBoolean(!impl));
}

EncodedJSValue JSC_HOST_CALL functionCallerSourceOrigin(ExecState* state)
{
    SourceOrigin sourceOrigin = state->callerSourceOrigin();
    if (sourceOrigin.isNull())
        return JSValue::encode(jsNull());
    return JSValue::encode(jsString(state, sourceOrigin.string()));
}

EncodedJSValue JSC_HOST_CALL functionGlobalObjectForObject(ExecState* exec)
{
    JSValue value = exec->argument(0);
    RELEASE_ASSERT(value.isObject());
    JSGlobalObject* globalObject = jsCast<JSObject*>(value)->globalObject();
    RELEASE_ASSERT(globalObject);
    return JSValue::encode(globalObject);
}

EncodedJSValue JSC_HOST_CALL functionReadline(ExecState* exec)
{
    Vector<char, 256> line;
    int c;
    while ((c = getchar()) != EOF) {
        // FIXME: Should we also break on \r? 
        if (c == '\n')
            break;
        line.append(c);
    }
    line.append('\0');
    return JSValue::encode(jsString(exec, line.data()));
}

EncodedJSValue JSC_HOST_CALL functionPreciseTime(ExecState*)
{
    return JSValue::encode(jsNumber(currentTime()));
}

EncodedJSValue JSC_HOST_CALL functionNeverInlineFunction(ExecState* exec)
{
    return JSValue::encode(setNeverInline(exec));
}

EncodedJSValue JSC_HOST_CALL functionNoDFG(ExecState* exec)
{
    return JSValue::encode(setNeverOptimize(exec));
}

EncodedJSValue JSC_HOST_CALL functionNoFTL(ExecState* exec)
{
    VM& vm = exec->vm();
    if (JSFunction* function = jsDynamicCast<JSFunction*>(vm, exec->argument(0))) {
        FunctionExecutable* executable = function->jsExecutable();
        executable->setNeverFTLOptimize(true);
    }

    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL functionNoOSRExitFuzzing(ExecState* exec)
{
    return JSValue::encode(setCannotUseOSRExitFuzzing(exec));
}

EncodedJSValue JSC_HOST_CALL functionOptimizeNextInvocation(ExecState* exec)
{
    return JSValue::encode(optimizeNextInvocation(exec));
}

EncodedJSValue JSC_HOST_CALL functionNumberOfDFGCompiles(ExecState* exec)
{
    return JSValue::encode(numberOfDFGCompiles(exec));
}

Message::Message(ArrayBufferContents&& contents, int32_t index)
    : m_contents(WTFMove(contents))
    , m_index(index)
{
}

Message::~Message()
{
}

Worker::Worker(Workers& workers)
    : m_workers(workers)
{
    auto locker = holdLock(m_workers.m_lock);
    m_workers.m_workers.append(this);
    
    *currentWorker() = this;
}

Worker::~Worker()
{
    auto locker = holdLock(m_workers.m_lock);
    RELEASE_ASSERT(isOnList());
    remove();
}

void Worker::enqueue(const AbstractLocker&, RefPtr<Message> message)
{
    m_messages.append(message);
}

RefPtr<Message> Worker::dequeue()
{
    auto locker = holdLock(m_workers.m_lock);
    while (m_messages.isEmpty())
        m_workers.m_condition.wait(m_workers.m_lock);
    return m_messages.takeFirst();
}

Worker& Worker::current()
{
    return **currentWorker();
}

ThreadSpecific<Worker*>& Worker::currentWorker()
{
    static ThreadSpecific<Worker*>* result;
    static std::once_flag flag;
    std::call_once(
        flag,
        [] () {
            result = new ThreadSpecific<Worker*>();
        });
    return *result;
}

Workers::Workers()
{
}

Workers::~Workers()
{
    UNREACHABLE_FOR_PLATFORM();
}

template<typename Func>
void Workers::broadcast(const Func& func)
{
    auto locker = holdLock(m_lock);
    for (Worker* worker = m_workers.begin(); worker != m_workers.end(); worker = worker->next()) {
        if (worker != &Worker::current())
            func(locker, *worker);
    }
    m_condition.notifyAll();
}

void Workers::report(String string)
{
    auto locker = holdLock(m_lock);
    m_reports.append(string.isolatedCopy());
    m_condition.notifyAll();
}

String Workers::tryGetReport()
{
    auto locker = holdLock(m_lock);
    if (m_reports.isEmpty())
        return String();
    return m_reports.takeFirst();
}

String Workers::getReport()
{
    auto locker = holdLock(m_lock);
    while (m_reports.isEmpty())
        m_condition.wait(m_lock);
    return m_reports.takeFirst();
}

Workers& Workers::singleton()
{
    static Workers* result;
    static std::once_flag flag;
    std::call_once(
        flag,
        [] {
            result = new Workers();
        });
    return *result;
}

EncodedJSValue JSC_HOST_CALL functionDollarCreateRealm(ExecState* exec)
{
    VM& vm = exec->vm();
    GlobalObject* result = GlobalObject::create(vm, GlobalObject::createStructure(vm, jsNull()), Vector<String>());
    return JSValue::encode(result->getDirect(vm, Identifier::fromString(exec, "$")));
}

EncodedJSValue JSC_HOST_CALL functionDollarDetachArrayBuffer(ExecState* exec)
{
    return functionTransferArrayBuffer(exec);
}

EncodedJSValue JSC_HOST_CALL functionDollarEvalScript(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    String sourceCode = exec->argument(0).toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    
    GlobalObject* globalObject = jsDynamicCast<GlobalObject*>(vm,
        exec->thisValue().get(exec, Identifier::fromString(exec, "global")));
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    if (!globalObject)
        return JSValue::encode(throwException(exec, scope, createError(exec, ASCIILiteral("Expected global to point to a global object"))));
    
    NakedPtr<Exception> evaluationException;
    JSValue result = evaluate(globalObject->globalExec(), makeSource(sourceCode, exec->callerSourceOrigin()), JSValue(), evaluationException);
    if (evaluationException)
        throwException(exec, scope, evaluationException);
    return JSValue::encode(result);
}

EncodedJSValue JSC_HOST_CALL functionDollarAgentStart(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    String sourceCode = exec->argument(0).toWTFString(exec).isolatedCopy();
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    
    Lock didStartLock;
    Condition didStartCondition;
    bool didStart = false;
    
    ThreadIdentifier thread = createThread(
        "JSC Agent",
        [sourceCode, &didStartLock, &didStartCondition, &didStart] () {
            CommandLine commandLine(0, nullptr);
            commandLine.m_interactive = false;
            runJSC(
                commandLine,
                [&] (VM&, GlobalObject* globalObject) {
                    // Notify the thread that started us that we have registered a worker.
                    {
                        auto locker = holdLock(didStartLock);
                        didStart = true;
                        didStartCondition.notifyOne();
                    }
                    
                    NakedPtr<Exception> evaluationException;
                    bool success = true;
                    JSValue result;
                    result = evaluate(globalObject->globalExec(), makeSource(sourceCode, SourceOrigin(ASCIILiteral("worker"))), JSValue(), evaluationException);
                    if (evaluationException)
                        result = evaluationException->value();
                    checkException(globalObject, true, evaluationException, result, String(), false, false, success);
                    if (!success)
                        exit(1);
                    return success;
                });
        });
    detachThread(thread);
    
    {
        auto locker = holdLock(didStartLock);
        while (!didStart)
            didStartCondition.wait(didStartLock);
    }
    
    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL functionDollarAgentReceiveBroadcast(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue callback = exec->argument(0);
    CallData callData;
    CallType callType = getCallData(callback, callData);
    if (callType == CallType::None)
        return JSValue::encode(throwException(exec, scope, createError(exec, ASCIILiteral("Expected callback"))));
    
    RefPtr<Message> message;
    {
        ReleaseHeapAccessScope releaseAccess(vm.heap);
        message = Worker::current().dequeue();
    }
    
    RefPtr<ArrayBuffer> nativeBuffer = ArrayBuffer::create(message->releaseContents());
    ArrayBufferSharingMode sharingMode = nativeBuffer->sharingMode();
    JSArrayBuffer* jsBuffer = JSArrayBuffer::create(vm, exec->lexicalGlobalObject()->arrayBufferStructure(sharingMode), WTFMove(nativeBuffer));
    
    MarkedArgumentBuffer args;
    args.append(jsBuffer);
    args.append(jsNumber(message->index()));
    return JSValue::encode(call(exec, callback, callType, callData, jsNull(), args));
}

EncodedJSValue JSC_HOST_CALL functionDollarAgentReport(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    String report = exec->argument(0).toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    
    Workers::singleton().report(report);
    
    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL functionDollarAgentSleep(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (exec->argumentCount() >= 1) {
        Seconds seconds = Seconds::fromMilliseconds(exec->argument(0).toNumber(exec));
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
        sleep(seconds);
    }
    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL functionDollarAgentBroadcast(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSArrayBuffer* jsBuffer = jsDynamicCast<JSArrayBuffer*>(vm, exec->argument(0));
    if (!jsBuffer || !jsBuffer->isShared())
        return JSValue::encode(throwException(exec, scope, createError(exec, ASCIILiteral("Expected SharedArrayBuffer"))));
    
    int32_t index = exec->argument(1).toInt32(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    
    Workers::singleton().broadcast(
        [&] (const AbstractLocker& locker, Worker& worker) {
            ArrayBuffer* nativeBuffer = jsBuffer->impl();
            ArrayBufferContents contents;
            nativeBuffer->transferTo(vm, contents); // "transferTo" means "share" if the buffer is shared.
            RefPtr<Message> message = adoptRef(new Message(WTFMove(contents), index));
            worker.enqueue(locker, message);
        });
    
    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL functionDollarAgentGetReport(ExecState* exec)
{
    VM& vm = exec->vm();

    String string = Workers::singleton().tryGetReport();
    if (!string)
        return JSValue::encode(jsNull());
    
    return JSValue::encode(jsString(&vm, string));
}

EncodedJSValue JSC_HOST_CALL functionDollarAgentLeaving(ExecState*)
{
    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL functionWaitForReport(ExecState* exec)
{
    VM& vm = exec->vm();

    String string;
    {
        ReleaseHeapAccessScope releaseAccess(vm.heap);
        string = Workers::singleton().getReport();
    }
    if (!string)
        return JSValue::encode(jsNull());
    
    return JSValue::encode(jsString(&vm, string));
}

EncodedJSValue JSC_HOST_CALL functionHeapCapacity(ExecState* exec)
{
    VM& vm = exec->vm();
    return JSValue::encode(jsNumber(vm.heap.capacity()));
}

EncodedJSValue JSC_HOST_CALL functionFlashHeapAccess(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    
    vm.heap.releaseAccess();
    if (exec->argumentCount() >= 1) {
        double ms = exec->argument(0).toNumber(exec);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
        sleep(Seconds::fromMilliseconds(ms));
    }
    vm.heap.acquireAccess();
    return JSValue::encode(jsUndefined());
}

template<typename ValueType>
typename std::enable_if<!std::is_fundamental<ValueType>::value>::type addOption(VM&, JSObject*, Identifier, ValueType) { }

template<typename ValueType>
typename std::enable_if<std::is_fundamental<ValueType>::value>::type addOption(VM& vm, JSObject* optionsObject, Identifier identifier, ValueType value)
{
    optionsObject->putDirect(vm, identifier, JSValue(value));
}

EncodedJSValue JSC_HOST_CALL functionJSCOptions(ExecState* exec)
{
    JSObject* optionsObject = constructEmptyObject(exec);
#define FOR_EACH_OPTION(type_, name_, defaultValue_, availability_, description_) \
    addOption(exec->vm(), optionsObject, Identifier::fromString(exec, #name_), Options::name_());
    JSC_OPTIONS(FOR_EACH_OPTION)
#undef FOR_EACH_OPTION
    return JSValue::encode(optionsObject);
}

EncodedJSValue JSC_HOST_CALL functionReoptimizationRetryCount(ExecState* exec)
{
    if (exec->argumentCount() < 1)
        return JSValue::encode(jsUndefined());
    
    CodeBlock* block = getSomeBaselineCodeBlockForFunction(exec->argument(0));
    if (!block)
        return JSValue::encode(jsNumber(0));
    
    return JSValue::encode(jsNumber(block->reoptimizationRetryCounter()));
}

EncodedJSValue JSC_HOST_CALL functionTransferArrayBuffer(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (exec->argumentCount() < 1)
        return JSValue::encode(throwException(exec, scope, createError(exec, ASCIILiteral("Not enough arguments"))));
    
    JSArrayBuffer* buffer = jsDynamicCast<JSArrayBuffer*>(vm, exec->argument(0));
    if (!buffer)
        return JSValue::encode(throwException(exec, scope, createError(exec, ASCIILiteral("Expected an array buffer"))));
    
    ArrayBufferContents dummyContents;
    buffer->impl()->transferTo(vm, dummyContents);
    
    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL functionFailNextNewCodeBlock(ExecState* exec)
{
    exec->vm().setFailNextNewCodeBlock();
    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL functionQuit(ExecState*)
{
    jscExit(EXIT_SUCCESS);

#if COMPILER(MSVC)
    // Without this, Visual Studio will complain that this method does not return a value.
    return JSValue::encode(jsUndefined());
#endif
}

EncodedJSValue JSC_HOST_CALL functionAbort(ExecState*)
{
    CRASH();
}

EncodedJSValue JSC_HOST_CALL functionFalse1(ExecState*) { return JSValue::encode(jsBoolean(false)); }
EncodedJSValue JSC_HOST_CALL functionFalse2(ExecState*) { return JSValue::encode(jsBoolean(false)); }

EncodedJSValue JSC_HOST_CALL functionUndefined1(ExecState*) { return JSValue::encode(jsUndefined()); }
EncodedJSValue JSC_HOST_CALL functionUndefined2(ExecState*) { return JSValue::encode(jsUndefined()); }
EncodedJSValue JSC_HOST_CALL functionIsInt32(ExecState* exec)
{
    for (size_t i = 0; i < exec->argumentCount(); ++i) {
        if (!exec->argument(i).isInt32())
            return JSValue::encode(jsBoolean(false));
    }
    return JSValue::encode(jsBoolean(true));
}

EncodedJSValue JSC_HOST_CALL functionIdentity(ExecState* exec) { return JSValue::encode(exec->argument(0)); }

EncodedJSValue JSC_HOST_CALL functionEffectful42(ExecState*)
{
    return JSValue::encode(jsNumber(42));
}

EncodedJSValue JSC_HOST_CALL functionMakeMasquerader(ExecState* exec)
{
    return JSValue::encode(Masquerader::create(exec->vm(), exec->lexicalGlobalObject()));
}

EncodedJSValue JSC_HOST_CALL functionHasCustomProperties(ExecState* exec)
{
    JSValue value = exec->argument(0);
    if (value.isObject())
        return JSValue::encode(jsBoolean(asObject(value)->hasCustomProperties()));
    return JSValue::encode(jsBoolean(false));
}

EncodedJSValue JSC_HOST_CALL functionDumpTypesForAllVariables(ExecState* exec)
{
    exec->vm().dumpTypeProfilerData();
    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL functionFindTypeForExpression(ExecState* exec)
{
    VM& vm = exec->vm();
    RELEASE_ASSERT(exec->vm().typeProfiler());
    vm.typeProfilerLog()->processLogEntries(ASCIILiteral("jsc Testing API: functionFindTypeForExpression"));

    JSValue functionValue = exec->argument(0);
    RELEASE_ASSERT(functionValue.isFunction());
    FunctionExecutable* executable = (jsDynamicCast<JSFunction*>(vm, functionValue.asCell()->getObject()))->jsExecutable();

    RELEASE_ASSERT(exec->argument(1).isString());
    String substring = asString(exec->argument(1))->value(exec);
    String sourceCodeText = executable->source().view().toString();
    unsigned offset = static_cast<unsigned>(sourceCodeText.find(substring) + executable->source().startOffset());
    
    String jsonString = exec->vm().typeProfiler()->typeInformationForExpressionAtOffset(TypeProfilerSearchDescriptorNormal, offset, executable->sourceID(), exec->vm());
    return JSValue::encode(JSONParse(exec, jsonString));
}

EncodedJSValue JSC_HOST_CALL functionReturnTypeFor(ExecState* exec)
{
    VM& vm = exec->vm();
    RELEASE_ASSERT(exec->vm().typeProfiler());
    vm.typeProfilerLog()->processLogEntries(ASCIILiteral("jsc Testing API: functionReturnTypeFor"));

    JSValue functionValue = exec->argument(0);
    RELEASE_ASSERT(functionValue.isFunction());
    FunctionExecutable* executable = (jsDynamicCast<JSFunction*>(vm, functionValue.asCell()->getObject()))->jsExecutable();

    unsigned offset = executable->typeProfilingStartOffset();
    String jsonString = exec->vm().typeProfiler()->typeInformationForExpressionAtOffset(TypeProfilerSearchDescriptorFunctionReturn, offset, executable->sourceID(), exec->vm());
    return JSValue::encode(JSONParse(exec, jsonString));
}

EncodedJSValue JSC_HOST_CALL functionDumpBasicBlockExecutionRanges(ExecState* exec)
{
    RELEASE_ASSERT(exec->vm().controlFlowProfiler());
    exec->vm().controlFlowProfiler()->dumpData();
    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL functionHasBasicBlockExecuted(ExecState* exec)
{
    VM& vm = exec->vm();
    RELEASE_ASSERT(vm.controlFlowProfiler());

    JSValue functionValue = exec->argument(0);
    RELEASE_ASSERT(functionValue.isFunction());
    FunctionExecutable* executable = (jsDynamicCast<JSFunction*>(vm, functionValue.asCell()->getObject()))->jsExecutable();

    RELEASE_ASSERT(exec->argument(1).isString());
    String substring = asString(exec->argument(1))->value(exec);
    String sourceCodeText = executable->source().view().toString();
    RELEASE_ASSERT(sourceCodeText.contains(substring));
    int offset = sourceCodeText.find(substring) + executable->source().startOffset();
    
    bool hasExecuted = vm.controlFlowProfiler()->hasBasicBlockAtTextOffsetBeenExecuted(offset, executable->sourceID(), vm);
    return JSValue::encode(jsBoolean(hasExecuted));
}

EncodedJSValue JSC_HOST_CALL functionBasicBlockExecutionCount(ExecState* exec)
{
    VM& vm = exec->vm();
    RELEASE_ASSERT(vm.controlFlowProfiler());

    JSValue functionValue = exec->argument(0);
    RELEASE_ASSERT(functionValue.isFunction());
    FunctionExecutable* executable = (jsDynamicCast<JSFunction*>(vm, functionValue.asCell()->getObject()))->jsExecutable();

    RELEASE_ASSERT(exec->argument(1).isString());
    String substring = asString(exec->argument(1))->value(exec);
    String sourceCodeText = executable->source().view().toString();
    RELEASE_ASSERT(sourceCodeText.contains(substring));
    int offset = sourceCodeText.find(substring) + executable->source().startOffset();
    
    size_t executionCount = vm.controlFlowProfiler()->basicBlockExecutionCountAtTextOffset(offset, executable->sourceID(), exec->vm());
    return JSValue::encode(JSValue(executionCount));
}

EncodedJSValue JSC_HOST_CALL functionEnableExceptionFuzz(ExecState*)
{
    Options::useExceptionFuzz() = true;
    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL functionDrainMicrotasks(ExecState* exec)
{
    exec->vm().drainMicrotasks();
    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL functionIs32BitPlatform(ExecState*)
{
#if USE(JSVALUE64)
    return JSValue::encode(JSValue(JSC::JSValue::JSFalse));
#else
    return JSValue::encode(JSValue(JSC::JSValue::JSTrue));
#endif
}

EncodedJSValue JSC_HOST_CALL functionLoadModule(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    String fileName = exec->argument(0).toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    Vector<char> script;
    if (!fetchScriptFromLocalFileSystem(fileName, script))
        return JSValue::encode(throwException(exec, scope, createError(exec, ASCIILiteral("Could not open file."))));

    JSInternalPromise* promise = loadAndEvaluateModule(exec, fileName);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    JSValue error;
    JSFunction* errorHandler = JSNativeStdFunction::create(vm, exec->lexicalGlobalObject(), 1, String(), [&](ExecState* exec) {
        error = exec->argument(0);
        return JSValue::encode(jsUndefined());
    });

    promise->then(exec, nullptr, errorHandler);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    vm.drainMicrotasks();
    if (error)
        return JSValue::encode(throwException(exec, scope, error));
    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL functionCreateBuiltin(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (exec->argumentCount() < 1 || !exec->argument(0).isString())
        return JSValue::encode(jsUndefined());

    String functionText = asString(exec->argument(0))->value(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    const SourceCode& source = makeSource(functionText, { });
    JSFunction* func = JSFunction::createBuiltinFunction(vm, createBuiltinExecutable(vm, source, Identifier::fromString(&vm, "foo"), ConstructorKind::None, ConstructAbility::CannotConstruct)->link(vm, source), exec->lexicalGlobalObject());

    return JSValue::encode(func);
}

EncodedJSValue JSC_HOST_CALL functionCreateGlobalObject(ExecState* exec)
{
    VM& vm = exec->vm();
    return JSValue::encode(GlobalObject::create(vm, GlobalObject::createStructure(vm, jsNull()), Vector<String>()));
}

EncodedJSValue JSC_HOST_CALL functionCheckModuleSyntax(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    String source = exec->argument(0).toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    StopWatch stopWatch;
    stopWatch.start();

    ParserError error;
    bool validSyntax = checkModuleSyntax(exec, makeSource(source, { }, String(), TextPosition(), SourceProviderSourceType::Module), error);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    stopWatch.stop();

    if (!validSyntax)
        throwException(exec, scope, jsNontrivialString(exec, toString("SyntaxError: ", error.message(), ":", error.line())));
    return JSValue::encode(jsNumber(stopWatch.getElapsedMS()));
}

EncodedJSValue JSC_HOST_CALL functionPlatformSupportsSamplingProfiler(ExecState*)
{
#if ENABLE(SAMPLING_PROFILER)
    return JSValue::encode(JSValue(JSC::JSValue::JSTrue));
#else
    return JSValue::encode(JSValue(JSC::JSValue::JSFalse));
#endif
}

EncodedJSValue JSC_HOST_CALL functionGenerateHeapSnapshot(ExecState* exec)
{
    VM& vm = exec->vm();
    JSLockHolder lock(vm);
    auto scope = DECLARE_THROW_SCOPE(vm);

    HeapSnapshotBuilder snapshotBuilder(exec->vm().ensureHeapProfiler());
    snapshotBuilder.buildSnapshot();

    String jsonString = snapshotBuilder.json();
    EncodedJSValue result = JSValue::encode(JSONParse(exec, jsonString));
    RELEASE_ASSERT(!scope.exception());
    return result;
}

EncodedJSValue JSC_HOST_CALL functionResetSuperSamplerState(ExecState*)
{
    resetSuperSamplerState();
    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL functionEnsureArrayStorage(ExecState* exec)
{
    VM& vm = exec->vm();
    for (unsigned i = 0; i < exec->argumentCount(); ++i) {
        if (JSObject* object = jsDynamicCast<JSObject*>(vm, exec->argument(0)))
            object->ensureArrayStorage(exec->vm());
    }
    return JSValue::encode(jsUndefined());
}

#if ENABLE(SAMPLING_PROFILER)
EncodedJSValue JSC_HOST_CALL functionStartSamplingProfiler(ExecState* exec)
{
    SamplingProfiler& samplingProfiler = exec->vm().ensureSamplingProfiler(WTF::Stopwatch::create());
    samplingProfiler.noticeCurrentThreadAsJSCExecutionThread();
    samplingProfiler.start();
    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL functionSamplingProfilerStackTraces(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (!vm.samplingProfiler())
        return JSValue::encode(throwException(exec, scope, createError(exec, ASCIILiteral("Sampling profiler was never started"))));

    String jsonString = vm.samplingProfiler()->stackTracesAsJSON();
    EncodedJSValue result = JSValue::encode(JSONParse(exec, jsonString));
    RELEASE_ASSERT(!scope.exception());
    return result;
}
#endif // ENABLE(SAMPLING_PROFILER)

EncodedJSValue JSC_HOST_CALL functionMaxArguments(ExecState*)
{
    return JSValue::encode(jsNumber(JSC::maxArguments));
}

#if ENABLE(WEBASSEMBLY)

static CString valueWithTypeOfWasmValue(ExecState* exec, VM& vm, JSValue value, JSValue wasmValue)
{
    JSString* type = asString(wasmValue.get(exec, makeIdentifier(vm, "type")));

    const String& typeString = type->value(exec);
    if (typeString == "i64" || typeString == "i32")
        return toCString(typeString, " ", RawPointer(bitwise_cast<void*>(value)));
    if (typeString == "f32")
        return toCString(typeString, " hex: ", RawPointer(bitwise_cast<void*>(value)), ", float: ", bitwise_cast<float>(static_cast<uint32_t>(JSValue::encode(value))));
    return toCString(typeString, " hex: ", RawPointer(bitwise_cast<void*>(value)), ", double: ", bitwise_cast<double>(value));
}

static JSValue box(ExecState* exec, VM& vm, JSValue wasmValue)
{
    auto scope = DECLARE_CATCH_SCOPE(vm);

    JSString* type = asString(wasmValue.get(exec, makeIdentifier(vm, "type")));
    ASSERT_UNUSED(scope, !scope.exception());
    JSValue value = wasmValue.get(exec, makeIdentifier(vm, "value"));
    ASSERT(!scope.exception());

    auto unboxString = [&] (const char* hexFormat, const char* decFormat, auto& result) {
        if (!value.isString())
            return false;

        const char* str = toCString(asString(value)->value(exec)).data();
        int scanResult;
        int length = std::strlen(str);
        if ((length > 2 && (str[0] == '0' && str[1] == 'x'))
            || (length > 3 && (str[0] == '-' && str[1] == '0' && str[2] == 'x')))
#if COMPILER(CLANG)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#endif
            scanResult = sscanf(str, hexFormat, &result);
        else
            scanResult = sscanf(str, decFormat, &result);
#if COMPILER(CLANG)
#pragma clang diagnostic pop
#endif
        RELEASE_ASSERT(scanResult != EOF);
        return true;
    };

    const String& typeString = type->value(exec);
    if (typeString == "i64") {
        int64_t result;
        if (!unboxString("%llx", "%lld", result))
            CRASH();
        return JSValue::decode(result);
    }

    if (typeString == "i32") {
        int32_t result;
        if (!unboxString("%x", "%d", result))
            result = value.asInt32();
        return JSValue::decode(static_cast<uint32_t>(result));
    }

    if (typeString == "f32") {
        float result;
        if (!unboxString("%a", "%f", result))
            result = value.toFloat(exec);
        return JSValue::decode(bitwise_cast<uint32_t>(result));
    }

    RELEASE_ASSERT(typeString == "f64");
    double result;
    if (!unboxString("%la", "%lf", result))
        result = value.asNumber();
    return JSValue::decode(bitwise_cast<uint64_t>(result));
}

// FIXME: https://bugs.webkit.org/show_bug.cgi?id=168582.
static JSValue callWasmFunction(VM* vm, JSGlobalObject* globalObject, JSWebAssemblyCallee* wasmCallee, const ArgList& boxedArgs)
{
    JSValue firstArgument;
    int argCount = 1;
    JSValue* remainingArgs = nullptr;
    if (boxedArgs.size()) {
        remainingArgs = boxedArgs.data();
        firstArgument = *remainingArgs;
        remainingArgs++;
        argCount = boxedArgs.size();
    }

    ProtoCallFrame protoCallFrame;
    protoCallFrame.init(nullptr, globalObject->globalExec()->jsCallee(), firstArgument, argCount, remainingArgs);

    return JSValue::decode(vmEntryToWasm(wasmCallee->entrypoint(), vm, &protoCallFrame));
}

// testWasmModule(JSArrayBufferView source, number functionCount, ...[[WasmValue, [WasmValue]]]) where the ith copy of [[result, [args]]] is a list
// of arguments to be passed to the ith wasm function as well as the expected result. WasmValue is an object with "type" and "value" properties.
static EncodedJSValue JSC_HOST_CALL functionTestWasmModuleFunctions(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (!Options::useWebAssembly())
        return throwVMTypeError(exec, scope, ASCIILiteral("testWasmModule should only be called if the useWebAssembly option is set"));

    JSArrayBufferView* source = jsCast<JSArrayBufferView*>(exec->argument(0));
    uint32_t functionCount = exec->argument(1).toUInt32(exec);

    if (exec->argumentCount() != functionCount + 2)
        CRASH();

    Wasm::Plan plan(&vm, static_cast<uint8_t*>(source->vector()), source->length());
    plan.run();
    if (plan.failed()) {
        dataLogLn("failed to parse module: ", plan.errorMessage());
        CRASH();
    }

    if (plan.internalFunctionCount() != functionCount)
        CRASH();

    MarkedArgumentBuffer callees;
    MarkedArgumentBuffer keepAlive;
    {
        unsigned lastIndex = UINT_MAX;
        plan.initializeCallees(exec->lexicalGlobalObject(),
            [&] (unsigned calleeIndex, JSWebAssemblyCallee* jsEntrypointCallee, JSWebAssemblyCallee* wasmEntrypointCallee) {
                RELEASE_ASSERT(!calleeIndex || (calleeIndex - 1 == lastIndex));
                callees.append(jsEntrypointCallee);
                keepAlive.append(wasmEntrypointCallee);
                lastIndex = calleeIndex;
            });
    }
    std::unique_ptr<Wasm::ModuleInformation> moduleInformation = plan.takeModuleInformation();
    RELEASE_ASSERT(!moduleInformation->memory);

    for (uint32_t i = 0; i < functionCount; ++i) {
        JSArray* testCases = jsCast<JSArray*>(exec->argument(i + 2));
        for (unsigned testIndex = 0; testIndex < testCases->length(); ++testIndex) {
            JSArray* test = jsCast<JSArray*>(testCases->getIndexQuickly(testIndex));
            JSObject* result = jsCast<JSObject*>(test->getIndexQuickly(0));
            JSArray* arguments = jsCast<JSArray*>(test->getIndexQuickly(1));

            MarkedArgumentBuffer boxedArgs;
            for (unsigned argIndex = 0; argIndex < arguments->length(); ++argIndex)
                boxedArgs.append(box(exec, vm, arguments->getIndexQuickly(argIndex)));

            JSValue callResult;
            {
                auto scope = DECLARE_THROW_SCOPE(vm);
                callResult = callWasmFunction(&vm, exec->lexicalGlobalObject(), jsCast<JSWebAssemblyCallee*>(callees.at(i)), boxedArgs);
                RETURN_IF_EXCEPTION(scope, { });
            }
            JSValue expected = box(exec, vm, result);
            if (callResult != expected) {
                dataLog("Arguments: ");
                CommaPrinter comma(", ");
                for (unsigned argIndex = 0; argIndex < arguments->length(); ++argIndex)
                    dataLog(comma, valueWithTypeOfWasmValue(exec, vm, boxedArgs.at(argIndex), arguments->getIndexQuickly(argIndex)));
                dataLogLn();

                WTFReportAssertionFailure(__FILE__, __LINE__, WTF_PRETTY_FUNCTION, toCString(" (callResult == ", valueWithTypeOfWasmValue(exec, vm, callResult, result), ", expected == ", valueWithTypeOfWasmValue(exec, vm, expected, result), ")").data());
                CRASH();
            }
        }
    }

    return encodedJSUndefined();
}

#endif // ENABLE(WEBASSEBLY)

// Use SEH for Release builds only to get rid of the crash report dialog
// (luckily the same tests fail in Release and Debug builds so far). Need to
// be in a separate main function because the jscmain function requires object
// unwinding.

#if COMPILER(MSVC) && !defined(_DEBUG)
#define TRY       __try {
#define EXCEPT(x) } __except (EXCEPTION_EXECUTE_HANDLER) { x; }
#else
#define TRY
#define EXCEPT(x)
#endif

int jscmain(int argc, char** argv);

static double s_desiredTimeout;
static double s_timeoutMultiplier = 1.0;

static NO_RETURN_DUE_TO_CRASH void timeoutThreadMain(void*)
{
    Seconds timeoutDuration(s_desiredTimeout * s_timeoutMultiplier);
    sleep(timeoutDuration);
    dataLog("Timed out after ", timeoutDuration, " seconds!\n");
    CRASH();
}

static void startTimeoutThreadIfNeeded()
{
    if (char* timeoutString = getenv("JSCTEST_timeout")) {
        if (sscanf(timeoutString, "%lf", &s_desiredTimeout) != 1) {
            dataLog("WARNING: timeout string is malformed, got ", timeoutString,
                " but expected a number. Not using a timeout.\n");
        } else
            createThread(timeoutThreadMain, 0, "jsc Timeout Thread");
    }
}

extern "C" {
int superJSCMain(int argc, char** argv);
}

int superJSCMain(int argc, char** argv)
{
#if PLATFORM(IOS) && CPU(ARM_THUMB2)
    // Enabled IEEE754 denormal support.
    fenv_t env;
    fegetenv( &env );
    env.__fpscr &= ~0x01000000u;
    fesetenv( &env );
#endif

#if OS(WINDOWS)
    // Cygwin calls ::SetErrorMode(SEM_FAILCRITICALERRORS), which we will inherit. This is bad for
    // testing/debugging, as it causes the post-mortem debugger not to be invoked. We reset the
    // error mode here to work around Cygwin's behavior. See <http://webkit.org/b/55222>.
    ::SetErrorMode(0);

#if defined(_DEBUG)
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
#endif

    timeBeginPeriod(1);
#endif

#if PLATFORM(GTK)
    if (!setlocale(LC_ALL, ""))
        WTFLogAlways("Locale not supported by C library.\n\tUsing the fallback 'C' locale.");
#endif

    // Need to initialize WTF threading before we start any threads. Cannot initialize JSC
    // threading yet, since that would do somethings that we'd like to defer until after we
    // have a chance to parse options.
    WTF::initializeThreading();

#if PLATFORM(IOS)
    Options::crashIfCantAllocateJITMemory() = true;
#endif

    // We can't use destructors in the following code because it uses Windows
    // Structured Exception Handling
    int res = 0;
    TRY
        res = jscmain(argc, argv);
    EXCEPT(res = 3)
    finalizeStatsAtEndOfTesting();

    jscExit(res);
}

static void dumpException(GlobalObject* globalObject, JSValue exception)
{
    VM& vm = globalObject->vm();
    auto scope = DECLARE_CATCH_SCOPE(vm);

#define CHECK_EXCEPTION() do { \
        if (scope.exception()) { \
            scope.clearException(); \
            return; \
        } \
    } while (false)

    printf("Exception: %s\n", exception.toWTFString(globalObject->globalExec()).utf8().data());

    Identifier nameID = Identifier::fromString(globalObject->globalExec(), "name");
    CHECK_EXCEPTION();
    Identifier fileNameID = Identifier::fromString(globalObject->globalExec(), "sourceURL");
    CHECK_EXCEPTION();
    Identifier lineNumberID = Identifier::fromString(globalObject->globalExec(), "line");
    CHECK_EXCEPTION();
    Identifier stackID = Identifier::fromString(globalObject->globalExec(), "stack");
    CHECK_EXCEPTION();

    JSValue nameValue = exception.get(globalObject->globalExec(), nameID);
    CHECK_EXCEPTION();
    JSValue fileNameValue = exception.get(globalObject->globalExec(), fileNameID);
    CHECK_EXCEPTION();
    JSValue lineNumberValue = exception.get(globalObject->globalExec(), lineNumberID);
    CHECK_EXCEPTION();
    JSValue stackValue = exception.get(globalObject->globalExec(), stackID);
    CHECK_EXCEPTION();
    
    if (nameValue.toWTFString(globalObject->globalExec()) == "SyntaxError"
        && (!fileNameValue.isUndefinedOrNull() || !lineNumberValue.isUndefinedOrNull())) {
        printf(
            "at %s:%s\n",
            fileNameValue.toWTFString(globalObject->globalExec()).utf8().data(),
            lineNumberValue.toWTFString(globalObject->globalExec()).utf8().data());
    }
    
    if (!stackValue.isUndefinedOrNull())
        printf("%s\n", stackValue.toWTFString(globalObject->globalExec()).utf8().data());

#undef CHECK_EXCEPTION
}

static bool checkUncaughtException(VM& vm, GlobalObject* globalObject, JSValue exception, const String& expectedExceptionName, bool alwaysDumpException)
{
    auto scope = DECLARE_CATCH_SCOPE(vm);
    scope.clearException();
    if (!exception) {
        printf("Expected uncaught exception with name '%s' but none was thrown\n", expectedExceptionName.utf8().data());
        return false;
    }

    ExecState* exec = globalObject->globalExec();
    JSValue exceptionClass = globalObject->get(exec, Identifier::fromString(exec, expectedExceptionName));
    if (!exceptionClass.isObject() || scope.exception()) {
        printf("Expected uncaught exception with name '%s' but given exception class is not defined\n", expectedExceptionName.utf8().data());
        return false;
    }

    bool isInstanceOfExpectedException = jsCast<JSObject*>(exceptionClass)->hasInstance(exec, exception);
    if (scope.exception()) {
        printf("Expected uncaught exception with name '%s' but given exception class fails performing hasInstance\n", expectedExceptionName.utf8().data());
        return false;
    }
    if (isInstanceOfExpectedException) {
        if (alwaysDumpException)
            dumpException(globalObject, exception);
        return true;
    }

    printf("Expected uncaught exception with name '%s' but exception value is not instance of this exception class\n", expectedExceptionName.utf8().data());
    dumpException(globalObject, exception);
    return false;
}

static void checkException(GlobalObject* globalObject, bool isLastFile, bool hasException, JSValue value, const String& uncaughtExceptionName, bool alwaysDumpUncaughtException, bool dump, bool& success)
{
    VM& vm = globalObject->vm();
    if (!uncaughtExceptionName || !isLastFile) {
        success = success && !hasException;
        if (dump && !hasException)
            printf("End: %s\n", value.toWTFString(globalObject->globalExec()).utf8().data());
        if (hasException)
            dumpException(globalObject, value);
    } else
        success = success && checkUncaughtException(vm, globalObject, (hasException) ? value : JSValue(), uncaughtExceptionName, alwaysDumpUncaughtException);
}

static bool runWithScripts(GlobalObject* globalObject, const Vector<Script>& scripts, const String& uncaughtExceptionName, bool alwaysDumpUncaughtException, bool dump, bool module)
{
    String fileName;
    Vector<char> scriptBuffer;

    if (dump)
        JSC::Options::dumpGeneratedBytecodes() = true;

    VM& vm = globalObject->vm();
    auto scope = DECLARE_CATCH_SCOPE(vm);
    bool success = true;

#if ENABLE(SAMPLING_FLAGS)
    SamplingFlags::start();
#endif

    for (size_t i = 0; i < scripts.size(); i++) {
        JSInternalPromise* promise = nullptr;
        bool isModule = module || scripts[i].scriptType == Script::ScriptType::Module;
        if (scripts[i].codeSource == Script::CodeSource::File) {
            fileName = scripts[i].argument;
            if (scripts[i].strictMode == Script::StrictMode::Strict)
                scriptBuffer.append("\"use strict\";\n", strlen("\"use strict\";\n"));

            if (isModule) {
                promise = loadAndEvaluateModule(globalObject->globalExec(), fileName);
                RELEASE_ASSERT(!scope.exception());
            } else {
                if (!fetchScriptFromLocalFileSystem(fileName, scriptBuffer))
                    return false; // fail early so we can catch missing files
            }
        } else {
            size_t commandLineLength = strlen(scripts[i].argument);
            scriptBuffer.resize(commandLineLength);
            std::copy(scripts[i].argument, scripts[i].argument + commandLineLength, scriptBuffer.begin());
            fileName = ASCIILiteral("[Command Line]");
        }

        bool isLastFile = i == scripts.size() - 1;
        if (isModule) {
            if (!promise)
                promise = loadAndEvaluateModule(globalObject->globalExec(), makeSource(stringFromUTF(scriptBuffer), SourceOrigin { absolutePath(fileName) }, fileName, TextPosition(), SourceProviderSourceType::Module));
            scope.clearException();

            JSFunction* fulfillHandler = JSNativeStdFunction::create(vm, globalObject, 1, String(), [&, isLastFile](ExecState* exec) {
                checkException(globalObject, isLastFile, false, exec->argument(0), uncaughtExceptionName, alwaysDumpUncaughtException, dump, success);
                return JSValue::encode(jsUndefined());
            });

            JSFunction* rejectHandler = JSNativeStdFunction::create(vm, globalObject, 1, String(), [&, isLastFile](ExecState* exec) {
                checkException(globalObject, isLastFile, true, exec->argument(0), uncaughtExceptionName, alwaysDumpUncaughtException, dump, success);
                return JSValue::encode(jsUndefined());
            });

            promise->then(globalObject->globalExec(), fulfillHandler, rejectHandler);
            RELEASE_ASSERT(!scope.exception());
            vm.drainMicrotasks();
        } else {
            NakedPtr<Exception> evaluationException;
            JSValue returnValue = evaluate(globalObject->globalExec(), jscSource(scriptBuffer, SourceOrigin { absolutePath(fileName) }, fileName), JSValue(), evaluationException);
            ASSERT(!scope.exception());
            if (evaluationException)
                returnValue = evaluationException->value();
            checkException(globalObject, isLastFile, evaluationException, returnValue, uncaughtExceptionName, alwaysDumpUncaughtException, dump, success);
        }

        scriptBuffer.clear();
        scope.clearException();
    }

#if ENABLE(REGEXP_TRACING)
    vm.dumpRegExpTrace();
#endif
    return success;
}

#define RUNNING_FROM_XCODE 0

static void runInteractive(GlobalObject* globalObject)
{
    VM& vm = globalObject->vm();
    auto scope = DECLARE_CATCH_SCOPE(vm);

    std::optional<DirectoryName> directoryName = currentWorkingDirectory();
    if (!directoryName)
        return;
    SourceOrigin sourceOrigin(resolvePath(directoryName.value(), ModuleName("interpreter")));
    
    bool shouldQuit = false;
    while (!shouldQuit) {
#if HAVE(READLINE) && !RUNNING_FROM_XCODE
        ParserError error;
        String source;
        do {
            error = ParserError();
            char* line = readline(source.isEmpty() ? interactivePrompt : "... ");
            shouldQuit = !line;
            if (!line)
                break;
            source = source + line;
            source = source + '\n';
            checkSyntax(globalObject->vm(), makeSource(source, sourceOrigin), error);
            if (!line[0]) {
                free(line);
                break;
            }
            add_history(line);
            free(line);
        } while (error.syntaxErrorType() == ParserError::SyntaxErrorRecoverable);
        
        if (error.isValid()) {
            printf("%s:%d\n", error.message().utf8().data(), error.line());
            continue;
        }
        
        
        NakedPtr<Exception> evaluationException;
        JSValue returnValue = evaluate(globalObject->globalExec(), makeSource(source, sourceOrigin), JSValue(), evaluationException);
#else
        printf("%s", interactivePrompt);
        Vector<char, 256> line;
        int c;
        while ((c = getchar()) != EOF) {
            // FIXME: Should we also break on \r? 
            if (c == '\n')
                break;
            line.append(c);
        }
        if (line.isEmpty())
            break;

        NakedPtr<Exception> evaluationException;
        JSValue returnValue = evaluate(globalObject->globalExec(), jscSource(line, sourceOrigin, sourceOrigin.string()), JSValue(), evaluationException);
#endif
        if (evaluationException)
            printf("Exception: %s\n", evaluationException->value().toWTFString(globalObject->globalExec()).utf8().data());
        else
            printf("%s\n", returnValue.toWTFString(globalObject->globalExec()).utf8().data());

        scope.clearException();
        globalObject->vm().drainMicrotasks();
    }
    printf("\n");
}

static NO_RETURN void printUsageStatement(bool help = false)
{
    fprintf(stderr, "Usage: jsc [options] [files] [-- arguments]\n");
    fprintf(stderr, "  -d         Dumps bytecode (debug builds only)\n");
    fprintf(stderr, "  -e         Evaluate argument as script code\n");
    fprintf(stderr, "  -f         Specifies a source file (deprecated)\n");
    fprintf(stderr, "  -h|--help  Prints this help message\n");
    fprintf(stderr, "  -i         Enables interactive mode (default if no files are specified)\n");
    fprintf(stderr, "  -m         Execute as a module\n");
#if HAVE(SIGNAL_H)
    fprintf(stderr, "  -s         Installs signal handlers that exit on a crash (Unix platforms only)\n");
#endif
    fprintf(stderr, "  -p <file>  Outputs profiling data to a file\n");
    fprintf(stderr, "  -x         Output exit code before terminating\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  --sample                   Collects and outputs sampling profiler data\n");
    fprintf(stderr, "  --test262-async            Check that some script calls the print function with the string 'Test262:AsyncTestComplete'\n");
    fprintf(stderr, "  --strict-file=<file>       Parse the given file as if it were in strict mode (this option may be passed more than once)\n");
    fprintf(stderr, "  --module-file=<file>       Parse and evaluate the given file as module (this option may be passed more than once)\n");
    fprintf(stderr, "  --exception=<name>         Check the last script exits with an uncaught exception with the specified name\n");
    fprintf(stderr, "  --dumpException            Dump uncaught exception text\n");
    fprintf(stderr, "  --options                  Dumps all JSC VM options and exits\n");
    fprintf(stderr, "  --dumpOptions              Dumps all non-default JSC VM options before continuing\n");
    fprintf(stderr, "  --<jsc VM option>=<value>  Sets the specified JSC VM option\n");
    fprintf(stderr, "\n");

    jscExit(help ? EXIT_SUCCESS : EXIT_FAILURE);
}

void CommandLine::parseArguments(int argc, char** argv)
{
    Options::initialize();
    
    int i = 1;
    JSC::Options::DumpLevel dumpOptionsLevel = JSC::Options::DumpLevel::None;
    bool needToExit = false;

    bool hasBadJSCOptions = false;
    for (; i < argc; ++i) {
        const char* arg = argv[i];
        if (!strcmp(arg, "-f")) {
            if (++i == argc)
                printUsageStatement();
            m_scripts.append(Script(Script::StrictMode::Sloppy, Script::CodeSource::File, Script::ScriptType::Script, argv[i]));
            continue;
        }
        if (!strcmp(arg, "-e")) {
            if (++i == argc)
                printUsageStatement();
            m_scripts.append(Script(Script::StrictMode::Sloppy, Script::CodeSource::CommandLine, Script::ScriptType::Script, argv[i]));
            continue;
        }
        if (!strcmp(arg, "-i")) {
            m_interactive = true;
            continue;
        }
        if (!strcmp(arg, "-d")) {
            m_dump = true;
            continue;
        }
        if (!strcmp(arg, "-p")) {
            if (++i == argc)
                printUsageStatement();
            m_profile = true;
            m_profilerOutput = argv[i];
            continue;
        }
        if (!strcmp(arg, "-m")) {
            m_module = true;
            continue;
        }
        if (!strcmp(arg, "-s")) {
#if HAVE(SIGNAL_H)
            signal(SIGILL, _exit);
            signal(SIGFPE, _exit);
            signal(SIGBUS, _exit);
            signal(SIGSEGV, _exit);
#endif
            continue;
        }
        if (!strcmp(arg, "-x")) {
            m_exitCode = true;
            continue;
        }
        if (!strcmp(arg, "--")) {
            ++i;
            break;
        }
        if (!strcmp(arg, "-h") || !strcmp(arg, "--help"))
            printUsageStatement(true);

        if (!strcmp(arg, "--options")) {
            dumpOptionsLevel = JSC::Options::DumpLevel::Verbose;
            needToExit = true;
            continue;
        }
        if (!strcmp(arg, "--dumpOptions")) {
            dumpOptionsLevel = JSC::Options::DumpLevel::Overridden;
            continue;
        }
        if (!strcmp(arg, "--sample")) {
            JSC::Options::useSamplingProfiler() = true;
            JSC::Options::collectSamplingProfilerDataForJSCShell() = true;
            m_dumpSamplingProfilerData = true;
            continue;
        }

        static const char* timeoutMultiplierOptStr = "--timeoutMultiplier=";
        static const unsigned timeoutMultiplierOptStrLength = strlen(timeoutMultiplierOptStr);
        if (!strncmp(arg, timeoutMultiplierOptStr, timeoutMultiplierOptStrLength)) {
            const char* valueStr = &arg[timeoutMultiplierOptStrLength];
            if (sscanf(valueStr, "%lf", &s_timeoutMultiplier) != 1)
                dataLog("WARNING: --timeoutMultiplier=", valueStr, " is invalid. Expects a numeric ratio.\n");
            continue;
        }

        if (!strcmp(arg, "--test262-async")) {
            test262AsyncTest = true;
            continue;
        }

        if (!strcmp(arg, "--remote-debug")) {
            m_enableRemoteDebugging = true;
            continue;
        }

        static const unsigned strictFileStrLength = strlen("--strict-file=");
        if (!strncmp(arg, "--strict-file=", strictFileStrLength)) {
            m_scripts.append(Script(Script::StrictMode::Strict, Script::CodeSource::File, Script::ScriptType::Script, argv[i] + strictFileStrLength));
            continue;
        }

        static const unsigned moduleFileStrLength = strlen("--module-file=");
        if (!strncmp(arg, "--module-file=", moduleFileStrLength)) {
            m_scripts.append(Script(Script::StrictMode::Sloppy, Script::CodeSource::File, Script::ScriptType::Module, argv[i] + moduleFileStrLength));
            continue;
        }

        if (!strcmp(arg, "--dumpException")) {
            m_alwaysDumpUncaughtException = true;
            continue;
        }

        static const unsigned exceptionStrLength = strlen("--exception=");
        if (!strncmp(arg, "--exception=", exceptionStrLength)) {
            m_uncaughtExceptionName = String(arg + exceptionStrLength);
            continue;
        }

        // See if the -- option is a JSC VM option.
        if (strstr(arg, "--") == arg) {
            if (!JSC::Options::setOption(&arg[2])) {
                hasBadJSCOptions = true;
                dataLog("ERROR: invalid option: ", arg, "\n");
            }
            continue;
        }

        // This arg is not recognized by the VM nor by jsc. Pass it on to the
        // script.
        m_scripts.append(Script(Script::StrictMode::Sloppy, Script::CodeSource::File, Script::ScriptType::Script, argv[i]));
    }

    if (hasBadJSCOptions && JSC::Options::validateOptions())
        CRASH();

    if (m_scripts.isEmpty())
        m_interactive = true;

    for (; i < argc; ++i)
        m_arguments.append(argv[i]);

    if (dumpOptionsLevel != JSC::Options::DumpLevel::None) {
        const char* optionsTitle = (dumpOptionsLevel == JSC::Options::DumpLevel::Overridden)
            ? "Modified JSC runtime options:"
            : "All JSC runtime options:";
        JSC::Options::dumpAllOptions(stderr, dumpOptionsLevel, optionsTitle);
    }
    JSC::Options::ensureOptionsAreCoherent();
    if (needToExit)
        jscExit(EXIT_SUCCESS);
}

template<typename Func>
int runJSC(CommandLine options, const Func& func)
{
    Worker worker(Workers::singleton());
    
    VM& vm = VM::create(LargeHeap).leakRef();
    HeapTimer::startTimerThread();
    JSLockHolder locker(&vm);

    int result;
    if (options.m_profile && !vm.m_perBytecodeProfiler)
        vm.m_perBytecodeProfiler = std::make_unique<Profiler::Database>(vm);

    GlobalObject* globalObject = GlobalObject::create(vm, GlobalObject::createStructure(vm, jsNull()), options.m_arguments);
    globalObject->setRemoteDebuggingEnabled(options.m_enableRemoteDebugging);
    bool success = func(vm, globalObject);
    if (options.m_interactive && success)
        runInteractive(globalObject);

    vm.drainMicrotasks();
    result = success && (test262AsyncTest == test262AsyncPassed) ? 0 : 3;

    if (options.m_exitCode)
        printf("jsc exiting %d\n", result);

    if (options.m_profile) {
        if (!vm.m_perBytecodeProfiler->save(options.m_profilerOutput.utf8().data()))
            fprintf(stderr, "could not save profiler output.\n");
    }

#if ENABLE(JIT)
    if (Options::useExceptionFuzz())
        printf("JSC EXCEPTION FUZZ: encountered %u checks.\n", numberOfExceptionFuzzChecks());
    bool fireAtEnabled =
    Options::fireExecutableAllocationFuzzAt() || Options::fireExecutableAllocationFuzzAtOrAfter();
    if (Options::useExecutableAllocationFuzz() && (!fireAtEnabled || Options::verboseExecutableAllocationFuzz()))
        printf("JSC EXECUTABLE ALLOCATION FUZZ: encountered %u checks.\n", numberOfExecutableAllocationFuzzChecks());
    if (Options::useOSRExitFuzz()) {
        printf("JSC OSR EXIT FUZZ: encountered %u static checks.\n", numberOfStaticOSRExitFuzzChecks());
        printf("JSC OSR EXIT FUZZ: encountered %u dynamic checks.\n", numberOfOSRExitFuzzChecks());
    }

    auto compileTimeStats = JIT::compileTimeStats();
    Vector<CString> compileTimeKeys;
    for (auto& entry : compileTimeStats)
        compileTimeKeys.append(entry.key);
    std::sort(compileTimeKeys.begin(), compileTimeKeys.end());
    for (CString key : compileTimeKeys)
        printf("%40s: %.3lf ms\n", key.data(), compileTimeStats.get(key));
#endif

    if (Options::gcAtEnd()) {
        // We need to hold the API lock to do a GC.
        JSLockHolder locker(&vm);
        vm.heap.collectAllGarbage();
    }

    if (options.m_dumpSamplingProfilerData) {
#if ENABLE(SAMPLING_PROFILER)
        JSLockHolder locker(&vm);
        vm.samplingProfiler()->reportTopFunctions();
        vm.samplingProfiler()->reportTopBytecodes();
#else
        dataLog("Sampling profiler is not enabled on this platform\n");
#endif
    }

    return result;
}

class unique_fd {
public:
    explicit unique_fd(int fd);
    ~unique_fd();
    int get() const;

private:
    int m_fd;
};

unique_fd::unique_fd(int fd)
    : m_fd(fd)
{
}

unique_fd::~unique_fd()
{
    close(m_fd);
}

int unique_fd::get() const
{
    return m_fd;
}

#define FAIL_WITH_STRERROR(tag) \
    fprintf(stderr, tag " fails: %s.\n", strerror(errno)); \
    return false;

#define MAYBE_FAIL_WITH_ICU_ERROR(s) \
    if (status != U_ZERO_ERROR) {\
        fprintf(stderr, "Couldn't initialize ICU (" "): %s (%s)" "\n", u_errorName(status), path.c_str()); \
        return false; \
    }

extern "C" {
void udata_setCommonData(const void *data, UErrorCode *pErrorCode);
}

static bool mapIcuData(const std::string& path)
{
    // Open the file and get its length.
    unique_fd fd(open(path.c_str(), O_RDONLY));
    if (fd.get() == -1) {
        FAIL_WITH_STRERROR("open");
    }
    struct stat sb;
    if (fstat(fd.get(), &sb) == -1) {
        FAIL_WITH_STRERROR("stat");
    }

    // Map it.
    void* data = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd.get(), 0);
    if (data == MAP_FAILED) {
        FAIL_WITH_STRERROR("mmap");
    }

    // Tell the kernel that accesses are likely to be random rather than sequential.
    if (madvise(data, sb.st_size, MADV_RANDOM) == -1) {
        FAIL_WITH_STRERROR("madvise(MADV_RANDOM)");
    }

    UErrorCode status = U_ZERO_ERROR;

    // Tell ICU to use our memory-mapped data.
    udata_setCommonData(data, &status);
    MAYBE_FAIL_WITH_ICU_ERROR("udata_setCommonData");

    return true;
}

static bool initICUEnv()
{
    if (!dlopen("libicuuc.so", RTLD_NOW)) {
        fprintf(stderr, "Could not open file: %s\n", "libicuuc");
        return false;
    }
    if (!dlopen("libicui18n.so", RTLD_NOW)) {
        fprintf(stderr, "Could not open file: %s\n", "libicui18n");
        return false;
    }
    if (!initICU()) {
        fprintf(stderr, "Could not initICU.\n");
        return false;
    }
#undef U_ICUDATA_NAME
    static const char* U_ICUDATA_NAME = "icudt55l";
    const char* systemPathPrefix = getenv("ANDROID_ROOT");
    if (systemPathPrefix == NULL) {
        fprintf(stderr, "ANDROID_ROOT environment variable not set" "\n"); \
        abort();
    }
    std::string systemPath;
    systemPath = systemPathPrefix;
    systemPath += "/usr/icu/";
    systemPath += U_ICUDATA_NAME;
    systemPath += ".dat";
    if (!mapIcuData(systemPath))
        return false;
    return true;
}

int jscmain(int argc, char** argv)
{
    if (!initICUEnv())
        return 1;
    // Need to override and enable restricted options before we start parsing options below.
    Options::enableRestrictedOptions(true);

    // Note that the options parsing can affect VM creation, and thus
    // comes first.
    CommandLine options(argc, argv);

    processConfigFile(Options::configFile(), "jsc");

    // Initialize JSC before getting VM.
    WTF::initializeMainThread();
    JSC::initializeThreading();
    startTimeoutThreadIfNeeded();
#if ENABLE(WEBASSEMBLY)
    JSC::Wasm::enableFastMemory();
#endif

    int result;
    result = runJSC(
        options,
        [&] (VM&, GlobalObject* globalObject) {
            return runWithScripts(globalObject, options.m_scripts, options.m_uncaughtExceptionName, options.m_alwaysDumpUncaughtException, options.m_dump, options.m_module);
        });

    printSuperSamplerState();

    return result;
}

#if OS(WINDOWS)
extern "C" __declspec(dllexport) int WINAPI dllLauncherEntryPoint(int argc, const char* argv[])
{
    return main(argc, const_cast<char**>(argv));
}
#endif
