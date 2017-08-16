/*
 * Copyright (C) 2007-2009, 2014-2017 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Cameron Zwarich (cwzwarich@uwaterloo.ca)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "JSGlobalObject.h"

#include "ArrayConstructor.h"
#include "ArrayIteratorAdaptiveWatchpoint.h"
#include "ArrayIteratorPrototype.h"
#include "ArrayPrototype.h"
#include "AtomicsObject.h"
#include "AsyncFunctionConstructor.h"
#include "AsyncFunctionPrototype.h"
#include "BooleanConstructor.h"
#include "BooleanPrototype.h"
#include "BuiltinNames.h"
#include "ClonedArguments.h"
#include "CodeBlock.h"
#include "CodeCache.h"
#include "ConsoleObject.h"
#include "DateConstructor.h"
#include "DatePrototype.h"
#include "Debugger.h"
#include "DebuggerScope.h"
#include "DirectArguments.h"
#include "DirectEvalExecutable.h"
#include "ECMAScriptSpecInternalFunctions.h"
#include "Error.h"
#include "ErrorConstructor.h"
#include "ErrorPrototype.h"
#include "FunctionConstructor.h"
#include "FunctionPrototype.h"
#include "GeneratorFunctionConstructor.h"
#include "GeneratorFunctionPrototype.h"
#include "GeneratorPrototype.h"
#include "GetterSetter.h"
#include "HeapIterationScope.h"
#include "IndirectEvalExecutable.h"
#include "InspectorInstrumentationObject.h"
#include "Interpreter.h"
#include "IteratorPrototype.h"
#include "JSAPIWrapperObject.h"
#include "JSArrayBuffer.h"
#include "JSArrayBufferConstructor.h"
#include "JSArrayBufferPrototype.h"
#include "JSAsyncFunction.h"
#include "JSBoundFunction.h"
#include "JSCInlines.h"
#include "JSCallbackConstructor.h"
#include "JSCallbackFunction.h"
#include "JSCallbackObject.h"
#include "JSCustomGetterSetterFunction.h"
#include "JSDataView.h"
#include "JSDataViewPrototype.h"
#include "JSDollarVM.h"
#include "JSDollarVMPrototype.h"
#include "JSFixedArray.h"
#include "JSFunction.h"
#include "JSGeneratorFunction.h"
#include "JSGenericTypedArrayViewConstructorInlines.h"
#include "JSGenericTypedArrayViewInlines.h"
#include "JSGenericTypedArrayViewPrototypeInlines.h"
#include "JSGlobalObjectFunctions.h"
#include "JSInternalPromise.h"
#include "JSInternalPromiseConstructor.h"
#include "JSInternalPromisePrototype.h"
#include "JSJob.h"
#include "JSLexicalEnvironment.h"
#include "JSLock.h"
#include "JSMap.h"
#include "JSMapIterator.h"
#include "JSModuleEnvironment.h"
#include "JSModuleLoader.h"
#include "JSModuleNamespaceObject.h"
#include "JSModuleRecord.h"
#include "JSNativeStdFunction.h"
#include "JSONObject.h"
#include "JSPromise.h"
#include "JSPromiseConstructor.h"
#include "JSPromisePrototype.h"
#include "JSPropertyNameIterator.h"
#include "JSSet.h"
#include "JSSetIterator.h"
#include "JSStringIterator.h"
#include "JSTemplateRegistryKey.h"
#include "JSTypedArrayConstructors.h"
#include "JSTypedArrayPrototypes.h"
#include "JSTypedArrayViewConstructor.h"
#include "JSTypedArrayViewPrototype.h"
#include "JSTypedArrays.h"
#include "JSWeakMap.h"
#include "JSWeakSet.h"
#include "JSWebAssembly.h"
#include "JSWebAssemblyCallee.h"
#include "JSWithScope.h"
#include "LazyClassStructureInlines.h"
#include "LazyPropertyInlines.h"
#include "Lookup.h"
#include "MapConstructor.h"
#include "MapIteratorPrototype.h"
#include "MapPrototype.h"
#include "MarkedSpaceInlines.h"
#include "MathObject.h"
#include "Microtask.h"
#include "ModuleLoaderPrototype.h"
#include "NativeErrorConstructor.h"
#include "NativeErrorPrototype.h"
#include "NullGetterFunction.h"
#include "NullSetterFunction.h"
#include "NumberConstructor.h"
#include "NumberPrototype.h"
#include "ObjCCallbackFunction.h"
#include "ObjectConstructor.h"
#include "ObjectPropertyConditionSet.h"
#include "ObjectPrototype.h"
#include "ParserError.h"
#include "ProxyConstructor.h"
#include "ProxyObject.h"
#include "ProxyRevoke.h"
#include "ReflectObject.h"
#include "RegExpConstructor.h"
#include "RegExpMatchesArray.h"
#include "RegExpObject.h"
#include "RegExpPrototype.h"
#include "ScopedArguments.h"
#include "SetConstructor.h"
#include "SetIteratorPrototype.h"
#include "SetPrototype.h"
#include "StrictEvalActivation.h"
#include "StringConstructor.h"
#include "StringIteratorPrototype.h"
#include "StringPrototype.h"
#include "Symbol.h"
#include "SymbolConstructor.h"
#include "SymbolPrototype.h"
#include "VariableWriteFireDetail.h"
#include "WeakGCMapInlines.h"
#include "WeakMapConstructor.h"
#include "WeakMapPrototype.h"
#include "WeakSetConstructor.h"
#include "WeakSetPrototype.h"
#include <wtf/RandomNumber.h>

#if ENABLE(INTL)
#include "IntlObject.h"
#include <unicode/ucol.h>
#include <unicode/udat.h>
#include <unicode/unum.h>
#endif // ENABLE(INTL)

#if ENABLE(REMOTE_INSPECTOR)
#include "JSGlobalObjectDebuggable.h"
#include "JSGlobalObjectInspectorController.h"
#endif

#if ENABLE(WEB_REPLAY)
#include "EmptyInputCursor.h"
#include "JSReplayInputs.h"
#endif

namespace JSC {

static JSValue createProxyProperty(VM& vm, JSObject* object)
{
    JSGlobalObject* global = jsCast<JSGlobalObject*>(object);
    return ProxyConstructor::create(vm, ProxyConstructor::createStructure(vm, global, global->functionPrototype()));
}

static JSValue createJSONProperty(VM& vm, JSObject* object)
{
    JSGlobalObject* global = jsCast<JSGlobalObject*>(object);
    return JSONObject::create(vm, JSONObject::createStructure(vm, global, global->objectPrototype()));
}

static JSValue createMathProperty(VM& vm, JSObject* object)
{
    JSGlobalObject* global = jsCast<JSGlobalObject*>(object);
    return MathObject::create(vm, global, MathObject::createStructure(vm, global, global->objectPrototype()));
}

static JSValue createConsoleProperty(VM& vm, JSObject* object)
{
    JSGlobalObject* global = jsCast<JSGlobalObject*>(object);
    return ConsoleObject::create(vm, global, ConsoleObject::createStructure(vm, global, constructEmptyObject(global->globalExec())));
}

static JSValue createAtomicsProperty(VM& vm, JSObject* object)
{
    JSGlobalObject* global = jsCast<JSGlobalObject*>(object);
    return AtomicsObject::create(vm, global, AtomicsObject::createStructure(vm, global, global->objectPrototype()));
}

static EncodedJSValue JSC_HOST_CALL makeBoundFunction(ExecState* exec)
{
    VM& vm = exec->vm();
    JSGlobalObject* globalObject = exec->lexicalGlobalObject();

    JSObject* target = asObject(exec->uncheckedArgument(0));
    JSValue boundThis = exec->uncheckedArgument(1);
    JSValue boundArgs = exec->uncheckedArgument(2);
    JSValue length = exec->uncheckedArgument(3);
    JSString* name = asString(exec->uncheckedArgument(4));

    return JSValue::encode(JSBoundFunction::create(
        vm, exec, globalObject, target, boundThis, boundArgs.isCell() ? jsCast<JSArray*>(boundArgs) : nullptr, length.asInt32(), name->value(exec)));
}

static EncodedJSValue JSC_HOST_CALL hasOwnLengthProperty(ExecState* exec)
{
    VM& vm = exec->vm();
    JSObject* target = asObject(exec->uncheckedArgument(0));
    return JSValue::encode(jsBoolean(target->hasOwnProperty(exec, vm.propertyNames->length)));
}

} // namespace JSC

#include "JSGlobalObject.lut.h"

namespace JSC {

const ClassInfo JSGlobalObject::s_info = { "GlobalObject", &Base::s_info, &globalObjectTable, CREATE_METHOD_TABLE(JSGlobalObject) };

const GlobalObjectMethodTable JSGlobalObject::s_globalObjectMethodTable = {
    &supportsRichSourceInfo,
    &shouldInterruptScript,
    &javaScriptRuntimeFlags,
    nullptr,
    &shouldInterruptScriptBeforeTimeout,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};

/* Source for JSGlobalObject.lut.h
@begin globalObjectTable
  parseFloat            globalFuncParseFloat                         DontEnum|Function 1
  isNaN                 JSBuiltin                                    DontEnum|Function 1
  isFinite              JSBuiltin                                    DontEnum|Function 1
  escape                globalFuncEscape                             DontEnum|Function 1
  unescape              globalFuncUnescape                           DontEnum|Function 1
  decodeURI             globalFuncDecodeURI                          DontEnum|Function 1
  decodeURIComponent    globalFuncDecodeURIComponent                 DontEnum|Function 1
  encodeURI             globalFuncEncodeURI                          DontEnum|Function 1
  encodeURIComponent    globalFuncEncodeURIComponent                 DontEnum|Function 1
  EvalError             JSGlobalObject::m_evalErrorConstructor       DontEnum|CellProperty
  ReferenceError        JSGlobalObject::m_referenceErrorConstructor  DontEnum|CellProperty
  SyntaxError           JSGlobalObject::m_syntaxErrorConstructor     DontEnum|CellProperty
  URIError              JSGlobalObject::m_URIErrorConstructor        DontEnum|CellProperty
  Proxy                 createProxyProperty                          DontEnum|PropertyCallback
  JSON                  createJSONProperty                           DontEnum|PropertyCallback
  Math                  createMathProperty                           DontEnum|PropertyCallback
  Atomics               createAtomicsProperty                        DontEnum|PropertyCallback
  console               createConsoleProperty                        DontEnum|PropertyCallback
  Int8Array             JSGlobalObject::m_typedArrayInt8             DontEnum|ClassStructure
  Int16Array            JSGlobalObject::m_typedArrayInt16            DontEnum|ClassStructure
  Int32Array            JSGlobalObject::m_typedArrayInt32            DontEnum|ClassStructure
  Uint8Array            JSGlobalObject::m_typedArrayUint8            DontEnum|ClassStructure
  Uint8ClampedArray     JSGlobalObject::m_typedArrayUint8Clamped     DontEnum|ClassStructure
  Uint16Array           JSGlobalObject::m_typedArrayUint16           DontEnum|ClassStructure
  Uint32Array           JSGlobalObject::m_typedArrayUint32           DontEnum|ClassStructure
  Float32Array          JSGlobalObject::m_typedArrayFloat32          DontEnum|ClassStructure
  Float64Array          JSGlobalObject::m_typedArrayFloat64          DontEnum|ClassStructure
  DataView              JSGlobalObject::m_typedArrayDataView         DontEnum|ClassStructure
  Set                   JSGlobalObject::m_setStructure               DontEnum|ClassStructure
  Date                  JSGlobalObject::m_dateStructure              DontEnum|ClassStructure
  Boolean               JSGlobalObject::m_booleanObjectStructure     DontEnum|ClassStructure
  Number                JSGlobalObject::m_numberObjectStructure      DontEnum|ClassStructure
  WeakMap               JSGlobalObject::m_weakMapStructure           DontEnum|ClassStructure
  WeakSet               JSGlobalObject::m_weakSetStructure           DontEnum|ClassStructure
@end
*/

static EncodedJSValue JSC_HOST_CALL getTemplateObject(ExecState* exec)
{
    JSValue thisValue = exec->thisValue();
    ASSERT(thisValue.inherits(exec->vm(), JSTemplateRegistryKey::info()));
    return JSValue::encode(exec->lexicalGlobalObject()->templateRegistry().getTemplateObject(exec, jsCast<JSTemplateRegistryKey*>(thisValue)));
}


static EncodedJSValue JSC_HOST_CALL enqueueJob(ExecState* exec)
{
    VM& vm = exec->vm();
    JSGlobalObject* globalObject = exec->lexicalGlobalObject();

    JSValue job = exec->argument(0);
    JSValue arguments = exec->argument(1);
    ASSERT(arguments.inherits(vm, JSArray::info()));

    globalObject->queueMicrotask(createJSJob(vm, job, jsCast<JSArray*>(arguments)));

    return JSValue::encode(jsUndefined());
}

JSGlobalObject::JSGlobalObject(VM& vm, Structure* structure, const GlobalObjectMethodTable* globalObjectMethodTable)
    : Base(vm, structure, 0)
    , m_vm(vm)
#if ENABLE(WEB_REPLAY)
    , m_inputCursor(EmptyInputCursor::create())
#endif
    , m_masqueradesAsUndefinedWatchpoint(adoptRef(new WatchpointSet(IsWatched)))
    , m_havingABadTimeWatchpoint(adoptRef(new WatchpointSet(IsWatched)))
    , m_varInjectionWatchpoint(adoptRef(new WatchpointSet(IsWatched)))
    , m_weakRandom(Options::forceWeakRandomSeed() ? Options::forcedWeakRandomSeed() : static_cast<unsigned>(randomNumber() * (std::numeric_limits<unsigned>::max() + 1.0)))
    , m_arrayIteratorProtocolWatchpoint(IsWatched)
    , m_arraySpeciesWatchpoint(ClearWatchpoint)
    , m_templateRegistry(vm)
    , m_evalEnabled(true)
    , m_runtimeFlags()
    , m_consoleClient(nullptr)
    , m_globalObjectMethodTable(globalObjectMethodTable ? globalObjectMethodTable : &s_globalObjectMethodTable)
{
}

JSGlobalObject::~JSGlobalObject()
{
#if ENABLE(REMOTE_INSPECTOR)
    m_inspectorController->globalObjectDestroyed();
#endif

    if (m_debugger)
        m_debugger->detach(this, Debugger::GlobalObjectIsDestructing);
}

void JSGlobalObject::destroy(JSCell* cell)
{
    static_cast<JSGlobalObject*>(cell)->JSGlobalObject::~JSGlobalObject();
}

void JSGlobalObject::setGlobalThis(VM& vm, JSObject* globalThis)
{
    m_globalThis.set(vm, this, globalThis);
}

static JSObject* getGetterById(ExecState* exec, JSObject* base, const Identifier& ident)
{
    JSValue baseValue = JSValue(base);
    PropertySlot slot(baseValue, PropertySlot::InternalMethodType::VMInquiry);
    baseValue.getPropertySlot(exec, ident, slot);
    return slot.getPureResult().toObject(exec);
}

void JSGlobalObject::init(VM& vm)
{
    ASSERT(vm.currentThreadIsHoldingAPILock());
    auto catchScope = DECLARE_CATCH_SCOPE(vm);

    Base::setStructure(vm, Structure::toCacheableDictionaryTransition(vm, structure()));

    m_debugger = 0;

#if ENABLE(REMOTE_INSPECTOR)
    m_inspectorController = std::make_unique<Inspector::JSGlobalObjectInspectorController>(*this);
    m_inspectorDebuggable = std::make_unique<JSGlobalObjectDebuggable>(*this);
    m_inspectorDebuggable->init();
    m_consoleClient = m_inspectorController->consoleClient();
#endif

    m_functionPrototype.set(vm, this, FunctionPrototype::create(vm, FunctionPrototype::createStructure(vm, this, jsNull()))); // The real prototype will be set once ObjectPrototype is created.
    m_calleeStructure.set(vm, this, JSCallee::createStructure(vm, this, jsNull()));

    m_globalLexicalEnvironment.set(vm, this, JSGlobalLexicalEnvironment::create(vm, JSGlobalLexicalEnvironment::createStructure(vm, this), this));
    // Need to create the callee structure (above) before creating the callee.
    JSCallee* globalCallee = JSCallee::create(vm, this, globalScope());
    m_globalCallee.set(vm, this, globalCallee);

    ExecState::initGlobalExec(JSGlobalObject::globalExec(), globalCallee);
    ExecState* exec = JSGlobalObject::globalExec();

    m_functionStructure.set(vm, this, JSFunction::createStructure(vm, this, m_functionPrototype.get()));
    m_customGetterSetterFunctionStructure.initLater(
        [] (const Initializer<Structure>& init) {
            init.set(JSCustomGetterSetterFunction::createStructure(init.vm, init.owner, init.owner->m_functionPrototype.get()));
        });
    m_boundFunctionStructure.initLater(
        [] (const Initializer<Structure>& init) {
            init.set(JSBoundFunction::createStructure(init.vm, init.owner, init.owner->m_functionPrototype.get()));
        });
    m_getterSetterStructure.set(vm, this, GetterSetter::createStructure(vm, this, jsNull()));
    m_nativeStdFunctionStructure.initLater(
        [] (const Initializer<Structure>& init) {
            init.set(JSNativeStdFunction::createStructure(init.vm, init.owner, init.owner->m_functionPrototype.get()));
        });
    m_namedFunctionStructure.initLater(
        [] (const Initializer<Structure>& init) {
            init.set(Structure::addPropertyTransition(init.vm, init.owner->m_functionStructure.get(), init.vm.propertyNames->name, DontDelete | ReadOnly | DontEnum, init.owner->m_functionNameOffset));
        });
    JSFunction* callFunction = 0;
    JSFunction* applyFunction = 0;
    JSFunction* hasInstanceSymbolFunction = 0;
    m_functionPrototype->addFunctionProperties(exec, this, &callFunction, &applyFunction, &hasInstanceSymbolFunction);
    m_callFunction.set(vm, this, callFunction);
    m_applyFunction.set(vm, this, applyFunction);
    m_arrayProtoToStringFunction.initLater(
        [] (const Initializer<JSFunction>& init) {
            init.set(JSFunction::create(init.vm, init.owner, 0, init.vm.propertyNames->toString.string(), arrayProtoFuncToString, NoIntrinsic));
        });
    m_arrayProtoValuesFunction.initLater(
        [] (const Initializer<JSFunction>& init) {
            init.set(JSFunction::createBuiltinFunction(init.vm, arrayPrototypeValuesCodeGenerator(init.vm), init.owner));
        });
    m_initializePromiseFunction.initLater(
        [] (const Initializer<JSFunction>& init) {
            init.set(JSFunction::createBuiltinFunction(init.vm, promiseOperationsInitializePromiseCodeGenerator(init.vm), init.owner));
        });

    m_iteratorProtocolFunction.initLater(
        [] (const Initializer<JSFunction>& init) {
            init.set(JSFunction::createBuiltinFunction(init.vm, iteratorHelpersPerformIterationCodeGenerator(init.vm), init.owner));
        });

    m_newPromiseCapabilityFunction.set(vm, this, JSFunction::createBuiltinFunction(vm, promiseOperationsNewPromiseCapabilityCodeGenerator(vm), this));
    m_functionProtoHasInstanceSymbolFunction.set(vm, this, hasInstanceSymbolFunction);
    m_throwTypeErrorGetterSetter.initLater(
        [] (const Initializer<GetterSetter>& init) {
            JSFunction* thrower = init.owner->throwTypeErrorFunction();
            GetterSetter* getterSetter = GetterSetter::create(init.vm, init.owner);
            getterSetter->setGetter(init.vm, init.owner, thrower);
            getterSetter->setSetter(init.vm, init.owner, thrower);
            init.set(getterSetter);
        });

    m_nullGetterFunction.set(vm, this, NullGetterFunction::create(vm, NullGetterFunction::createStructure(vm, this, m_functionPrototype.get())));
    m_nullSetterFunction.set(vm, this, NullSetterFunction::create(vm, NullSetterFunction::createStructure(vm, this, m_functionPrototype.get())));
    m_objectPrototype.set(vm, this, ObjectPrototype::create(vm, this, ObjectPrototype::createStructure(vm, this, jsNull())));
    GetterSetter* protoAccessor = GetterSetter::create(vm, this);
    protoAccessor->setGetter(vm, this, JSFunction::create(vm, this, 0, makeString("get ", vm.propertyNames->underscoreProto.string()), globalFuncProtoGetter));
    protoAccessor->setSetter(vm, this, JSFunction::create(vm, this, 0, makeString("set ", vm.propertyNames->underscoreProto.string()), globalFuncProtoSetter));
    m_objectPrototype->putDirectNonIndexAccessor(vm, vm.propertyNames->underscoreProto, protoAccessor, Accessor | DontEnum);
    m_functionPrototype->structure()->setPrototypeWithoutTransition(vm, m_objectPrototype.get());
    m_objectStructureForObjectConstructor.set(vm, this, vm.prototypeMap.emptyObjectStructureForPrototype(this, m_objectPrototype.get(), JSFinalObject::defaultInlineCapacity()));
    m_objectProtoValueOfFunction.set(vm, this, jsCast<JSFunction*>(objectPrototype()->getDirect(vm, vm.propertyNames->valueOf)));
    
    JSFunction* thrower = JSFunction::create(vm, this, 0, String(), globalFuncThrowTypeErrorArgumentsCalleeAndCaller);
    GetterSetter* getterSetter = GetterSetter::create(vm, this);
    getterSetter->setGetter(vm, this, thrower);
    getterSetter->setSetter(vm, this, thrower);
    m_throwTypeErrorArgumentsCalleeAndCallerGetterSetter.set(vm, this, getterSetter);
    
    m_functionPrototype->initRestrictedProperties(exec, this);

    m_speciesGetterSetter.set(vm, this, GetterSetter::create(vm, this));
    m_speciesGetterSetter->setGetter(vm, this, JSFunction::createBuiltinFunction(vm, globalOperationsSpeciesGetterCodeGenerator(vm), this, "get [Symbol.species]"));

    m_typedArrayProto.initLater(
        [] (const Initializer<JSTypedArrayViewPrototype>& init) {
            init.set(JSTypedArrayViewPrototype::create(init.vm, init.owner, JSTypedArrayViewPrototype::createStructure(init.vm, init.owner, init.owner->m_objectPrototype.get())));
            
            // Make sure that the constructor gets initialized, too.
            init.owner->m_typedArraySuperConstructor.get(init.owner);
        });
    m_typedArraySuperConstructor.initLater(
        [] (const Initializer<JSTypedArrayViewConstructor>& init) {
            JSTypedArrayViewPrototype* prototype = init.owner->m_typedArrayProto.get(init.owner);
            JSTypedArrayViewConstructor* constructor = JSTypedArrayViewConstructor::create(init.vm, init.owner, JSTypedArrayViewConstructor::createStructure(init.vm, init.owner, init.owner->m_functionPrototype.get()), prototype, init.owner->m_speciesGetterSetter.get());
            prototype->putDirectWithoutTransition(init.vm, init.vm.propertyNames->constructor, constructor, DontEnum);
            init.set(constructor);
        });
    
#define INIT_TYPED_ARRAY_LATER(type) \
    m_typedArray ## type.initLater( \
        [] (LazyClassStructure::Initializer& init) { \
            init.setPrototype(JS ## type ## ArrayPrototype::create(init.vm, init.global, JS ## type ## ArrayPrototype::createStructure(init.vm, init.global, init.global->m_typedArrayProto.get(init.global)))); \
            init.setStructure(JS ## type ## Array::createStructure(init.vm, init.global, init.prototype)); \
            init.setConstructor(JS ## type ## ArrayConstructor::create(init.vm, init.global, JS ## type ## ArrayConstructor::createStructure(init.vm, init.global, init.global->m_typedArraySuperConstructor.get(init.global)), init.prototype, ASCIILiteral(#type "Array"), typedArrayConstructorAllocate ## type ## ArrayCodeGenerator(init.vm))); \
            init.global->putDirectWithoutTransition(init.vm, init.vm.propertyNames->builtinNames().type ## ArrayPrivateName(), init.constructor, DontEnum); \
        });
    FOR_EACH_TYPED_ARRAY_TYPE_EXCLUDING_DATA_VIEW(INIT_TYPED_ARRAY_LATER)
#undef INIT_TYPED_ARRAY_LATER
    
    m_typedArrayDataView.initLater(
        [] (LazyClassStructure::Initializer& init) {
            init.setPrototype(JSDataViewPrototype::create(init.vm, JSDataViewPrototype::createStructure(init.vm, init.global, init.global->m_objectPrototype.get())));
            init.setStructure(JSDataView::createStructure(init.vm, init.global, init.prototype));
            init.setConstructor(JSDataViewConstructor::create(init.vm, init.global, JSDataViewConstructor::createStructure(init.vm, init.global, init.global->m_functionPrototype.get()), init.prototype, ASCIILiteral("DataView"), nullptr));
        });
    
    m_lexicalEnvironmentStructure.set(vm, this, JSLexicalEnvironment::createStructure(vm, this));
    m_moduleEnvironmentStructure.initLater(
        [] (const Initializer<Structure>& init) {
            init.set(JSModuleEnvironment::createStructure(init.vm, init.owner));
        });
    m_strictEvalActivationStructure.set(vm, this, StrictEvalActivation::createStructure(vm, this, jsNull()));
    m_debuggerScopeStructure.initLater(
        [] (const Initializer<Structure>& init) {
            init.set(DebuggerScope::createStructure(init.vm, init.owner));
        });
    m_withScopeStructure.initLater(
        [] (const Initializer<Structure>& init) {
            init.set(JSWithScope::createStructure(init.vm, init.owner, jsNull()));
        });
    
    m_nullPrototypeObjectStructure.initLater(
        [] (const Initializer<Structure>& init) {
            init.set(JSFinalObject::createStructure(init.vm, init.owner, jsNull(), JSFinalObject::defaultInlineCapacity()));
        });
    
    m_callbackFunctionStructure.initLater(
        [] (const Initializer<Structure>& init) {
            init.set(JSCallbackFunction::createStructure(init.vm, init.owner, init.owner->m_functionPrototype.get()));
        });
    m_directArgumentsStructure.set(vm, this, DirectArguments::createStructure(vm, this, m_objectPrototype.get()));
    m_scopedArgumentsStructure.set(vm, this, ScopedArguments::createStructure(vm, this, m_objectPrototype.get()));
    m_clonedArgumentsStructure.set(vm, this, ClonedArguments::createStructure(vm, this, m_objectPrototype.get()));
    m_callbackConstructorStructure.initLater(
        [] (const Initializer<Structure>& init) {
            init.set(JSCallbackConstructor::createStructure(init.vm, init.owner, init.owner->m_objectPrototype.get()));
        });
    m_callbackObjectStructure.initLater(
        [] (const Initializer<Structure>& init) {
            init.set(JSCallbackObject<JSDestructibleObject>::createStructure(init.vm, init.owner, init.owner->m_objectPrototype.get()));
        });

#if JSC_OBJC_API_ENABLED
    m_objcCallbackFunctionStructure.initLater(
        [] (const Initializer<Structure>& init) {
            init.set(ObjCCallbackFunction::createStructure(init.vm, init.owner, init.owner->m_functionPrototype.get()));
        });
    m_objcWrapperObjectStructure.initLater(
        [] (const Initializer<Structure>& init) {
            init.set(JSCallbackObject<JSAPIWrapperObject>::createStructure(init.vm, init.owner, init.owner->m_objectPrototype.get()));
        });
#endif
    
    m_arrayPrototype.set(vm, this, ArrayPrototype::create(vm, this, ArrayPrototype::createStructure(vm, this, m_objectPrototype.get())));
    
    m_originalArrayStructureForIndexingShape[UndecidedShape >> IndexingShapeShift].set(vm, this, JSArray::createStructure(vm, this, m_arrayPrototype.get(), ArrayWithUndecided));
    m_originalArrayStructureForIndexingShape[Int32Shape >> IndexingShapeShift].set(vm, this, JSArray::createStructure(vm, this, m_arrayPrototype.get(), ArrayWithInt32));
    m_originalArrayStructureForIndexingShape[DoubleShape >> IndexingShapeShift].set(vm, this, JSArray::createStructure(vm, this, m_arrayPrototype.get(), ArrayWithDouble));
    m_originalArrayStructureForIndexingShape[ContiguousShape >> IndexingShapeShift].set(vm, this, JSArray::createStructure(vm, this, m_arrayPrototype.get(), ArrayWithContiguous));
    m_originalArrayStructureForIndexingShape[ArrayStorageShape >> IndexingShapeShift].set(vm, this, JSArray::createStructure(vm, this, m_arrayPrototype.get(), ArrayWithArrayStorage));
    m_originalArrayStructureForIndexingShape[SlowPutArrayStorageShape >> IndexingShapeShift].set(vm, this, JSArray::createStructure(vm, this, m_arrayPrototype.get(), ArrayWithSlowPutArrayStorage));
    for (unsigned i = 0; i < NumberOfIndexingShapes; ++i)
        m_arrayStructureForIndexingShapeDuringAllocation[i] = m_originalArrayStructureForIndexingShape[i];

    m_regExpPrototype.set(vm, this, RegExpPrototype::create(vm, this, RegExpPrototype::createStructure(vm, this, m_objectPrototype.get())));
    m_regExpStructure.set(vm, this, RegExpObject::createStructure(vm, this, m_regExpPrototype.get()));
    m_regExpMatchesArrayStructure.set(vm, this, createRegExpMatchesArrayStructure(vm, this));

    m_moduleRecordStructure.set(vm, this, JSModuleRecord::createStructure(vm, this, m_objectPrototype.get()));
    m_moduleNamespaceObjectStructure.set(vm, this, JSModuleNamespaceObject::createStructure(vm, this, jsNull()));
    {
        bool isCallable = false;
        m_proxyObjectStructure.set(vm, this, ProxyObject::createStructure(vm, this, jsNull(), isCallable));
        isCallable = true;
        m_callableProxyObjectStructure.set(vm, this, ProxyObject::createStructure(vm, this, jsNull(), isCallable));
    }
    m_proxyRevokeStructure.set(vm, this, ProxyRevoke::createStructure(vm, this, m_functionPrototype.get()));

    m_parseIntFunction.set(vm, this, JSFunction::create(vm, this, 2, vm.propertyNames->parseInt.string(), globalFuncParseInt, ParseIntIntrinsic));
    putDirectWithoutTransition(vm, vm.propertyNames->parseInt, m_parseIntFunction.get(), DontEnum);
    
    m_arrayBufferPrototype.set(vm, this, JSArrayBufferPrototype::create(vm, this, JSArrayBufferPrototype::createStructure(vm, this, m_objectPrototype.get()), ArrayBufferSharingMode::Default));
    m_arrayBufferStructure.set(vm, this, JSArrayBuffer::createStructure(vm, this, m_arrayBufferPrototype.get()));
    m_sharedArrayBufferPrototype.set(vm, this, JSArrayBufferPrototype::create(vm, this, JSArrayBufferPrototype::createStructure(vm, this, m_objectPrototype.get()), ArrayBufferSharingMode::Shared));
    m_sharedArrayBufferStructure.set(vm, this, JSArrayBuffer::createStructure(vm, this, m_sharedArrayBufferPrototype.get()));

#define CREATE_PROTOTYPE_FOR_SIMPLE_TYPE(capitalName, lowerName, properName, instanceType, jsName, prototypeBase) \
m_ ## lowerName ## Prototype.set(vm, this, capitalName##Prototype::create(vm, this, capitalName##Prototype::createStructure(vm, this, m_ ## prototypeBase ## Prototype.get()))); \
m_ ## properName ## Structure.set(vm, this, instanceType::createStructure(vm, this, m_ ## lowerName ## Prototype.get()));
    
    FOR_EACH_SIMPLE_BUILTIN_TYPE(CREATE_PROTOTYPE_FOR_SIMPLE_TYPE)
    
#undef CREATE_PROTOTYPE_FOR_SIMPLE_TYPE

#define CREATE_PROTOTYPE_FOR_LAZY_TYPE(capitalName, lowerName, properName, instanceType, jsName, prototypeBase) \
    m_ ## properName ## Structure.initLater(\
        [] (LazyClassStructure::Initializer& init) { \
            init.setPrototype(capitalName##Prototype::create(init.vm, init.global, capitalName##Prototype::createStructure(init.vm, init.global, init.global->m_ ## prototypeBase ## Prototype.get()))); \
            init.setStructure(instanceType::createStructure(init.vm, init.global, init.prototype)); \
            init.setConstructor(capitalName ## Constructor::create(init.vm, capitalName ## Constructor::createStructure(init.vm, init.global, init.global->m_functionPrototype.get()), jsCast<capitalName ## Prototype*>(init.prototype), init.global->m_speciesGetterSetter.get())); \
        });
    
    FOR_EACH_LAZY_BUILTIN_TYPE(CREATE_PROTOTYPE_FOR_LAZY_TYPE)
    
#undef CREATE_PROTOTYPE_FOR_LAZY_TYPE
    
    m_iteratorPrototype.set(vm, this, IteratorPrototype::create(vm, this, IteratorPrototype::createStructure(vm, this, m_objectPrototype.get())));

#define CREATE_PROTOTYPE_FOR_DERIVED_ITERATOR_TYPE(capitalName, lowerName, properName, instanceType, jsName, prototypeBase) \
    m_ ## lowerName ## Structure.initLater( \
        [] (const Initializer<Structure>& init) { \
            JSObject* prototype = capitalName ## Prototype::create(init.vm, init.owner, capitalName ## Prototype::createStructure(init.vm, init.owner, init.owner->m_iteratorPrototype.get())); \
            init.set(instanceType::createStructure(init.vm, init.owner, prototype)); \
        });
    FOR_EACH_BUILTIN_DERIVED_ITERATOR_TYPE(CREATE_PROTOTYPE_FOR_DERIVED_ITERATOR_TYPE)
#undef CREATE_PROTOTYPE_FOR_DERIVED_ITERATOR_TYPE

    m_propertyNameIteratorStructure.set(vm, this, JSPropertyNameIterator::createStructure(vm, this, m_iteratorPrototype.get()));
    m_generatorPrototype.set(vm, this, GeneratorPrototype::create(vm, this, GeneratorPrototype::createStructure(vm, this, m_iteratorPrototype.get())));
    m_moduleLoaderPrototype.set(vm, this, ModuleLoaderPrototype::create(vm, this, ModuleLoaderPrototype::createStructure(vm, this, m_objectPrototype.get())));
    
    // Constructors

    ObjectConstructor* objectConstructor = ObjectConstructor::create(vm, this, ObjectConstructor::createStructure(vm, this, m_functionPrototype.get()), m_objectPrototype.get());
    m_objectConstructor.set(vm, this, objectConstructor);

    JSFunction* throwTypeErrorFunction = JSFunction::create(vm, this, 0, String(), globalFuncThrowTypeError);
    m_throwTypeErrorFunction.set(vm, this, throwTypeErrorFunction);

    JSCell* functionConstructor = FunctionConstructor::create(vm, FunctionConstructor::createStructure(vm, this, m_functionPrototype.get()), m_functionPrototype.get());

    ArrayConstructor* arrayConstructor = ArrayConstructor::create(vm, this, ArrayConstructor::createStructure(vm, this, m_functionPrototype.get()), m_arrayPrototype.get(), m_speciesGetterSetter.get());
    m_arrayConstructor.set(vm, this, arrayConstructor);
    
    m_regExpConstructor.set(vm, this, RegExpConstructor::create(vm, RegExpConstructor::createStructure(vm, this, m_functionPrototype.get()), m_regExpPrototype.get(), m_speciesGetterSetter.get()));
    
    JSArrayBufferConstructor* arrayBufferConstructor = JSArrayBufferConstructor::create(vm, JSArrayBufferConstructor::createStructure(vm, this, m_functionPrototype.get()), m_arrayBufferPrototype.get(), m_speciesGetterSetter.get(), ArrayBufferSharingMode::Default);
    m_arrayBufferPrototype->putDirectWithoutTransition(vm, vm.propertyNames->constructor, arrayBufferConstructor, DontEnum);
    JSArrayBufferConstructor* sharedArrayBufferConstructor = nullptr;
    sharedArrayBufferConstructor = JSArrayBufferConstructor::create(vm, JSArrayBufferConstructor::createStructure(vm, this, m_functionPrototype.get()), m_sharedArrayBufferPrototype.get(), m_speciesGetterSetter.get(), ArrayBufferSharingMode::Shared);
    m_sharedArrayBufferPrototype->putDirectWithoutTransition(vm, vm.propertyNames->constructor, sharedArrayBufferConstructor, DontEnum);
    
#define CREATE_CONSTRUCTOR_FOR_SIMPLE_TYPE(capitalName, lowerName, properName, instanceType, jsName, prototypeBase) \
capitalName ## Constructor* lowerName ## Constructor = capitalName ## Constructor::create(vm, capitalName ## Constructor::createStructure(vm, this, m_functionPrototype.get()), m_ ## lowerName ## Prototype.get(), m_speciesGetterSetter.get()); \
m_ ## lowerName ## Prototype->putDirectWithoutTransition(vm, vm.propertyNames->constructor, lowerName ## Constructor, DontEnum); \

    FOR_EACH_SIMPLE_BUILTIN_TYPE(CREATE_CONSTRUCTOR_FOR_SIMPLE_TYPE)
    
#undef CREATE_CONSTRUCTOR_FOR_SIMPLE_TYPE

    m_errorConstructor.set(vm, this, errorConstructor);
    m_promiseConstructor.set(vm, this, promiseConstructor);
    m_internalPromiseConstructor.set(vm, this, internalPromiseConstructor);
    
    m_nativeErrorPrototypeStructure.set(vm, this, NativeErrorPrototype::createStructure(vm, this, m_errorPrototype.get()));
    m_nativeErrorStructure.set(vm, this, NativeErrorConstructor::createStructure(vm, this, m_functionPrototype.get()));
    m_evalErrorConstructor.initLater(
        [] (const Initializer<NativeErrorConstructor>& init) {
            init.set(NativeErrorConstructor::create(init.vm, init.owner, init.owner->m_nativeErrorStructure.get(), init.owner->m_nativeErrorPrototypeStructure.get(), ASCIILiteral("EvalError")));
        });
    m_rangeErrorConstructor.set(vm, this, NativeErrorConstructor::create(vm, this, m_nativeErrorStructure.get(), m_nativeErrorPrototypeStructure.get(), ASCIILiteral("RangeError")));
    m_referenceErrorConstructor.initLater(
        [] (const Initializer<NativeErrorConstructor>& init) {
            init.set(NativeErrorConstructor::create(init.vm, init.owner, init.owner->m_nativeErrorStructure.get(), init.owner->m_nativeErrorPrototypeStructure.get(), ASCIILiteral("ReferenceError")));
        });
    m_syntaxErrorConstructor.initLater(
        [] (const Initializer<NativeErrorConstructor>& init) {
            init.set(NativeErrorConstructor::create(init.vm, init.owner, init.owner->m_nativeErrorStructure.get(), init.owner->m_nativeErrorPrototypeStructure.get(), ASCIILiteral("SyntaxError")));
        });
    m_typeErrorConstructor.set(vm, this, NativeErrorConstructor::create(vm, this, m_nativeErrorStructure.get(), m_nativeErrorPrototypeStructure.get(), ASCIILiteral("TypeError")));
    m_URIErrorConstructor.initLater(
        [] (const Initializer<NativeErrorConstructor>& init) {
            init.set(NativeErrorConstructor::create(init.vm, init.owner, init.owner->m_nativeErrorStructure.get(), init.owner->m_nativeErrorPrototypeStructure.get(), ASCIILiteral("URIError")));
        });

    m_generatorFunctionPrototype.set(vm, this, GeneratorFunctionPrototype::create(vm, GeneratorFunctionPrototype::createStructure(vm, this, m_functionPrototype.get())));
    GeneratorFunctionConstructor* generatorFunctionConstructor = GeneratorFunctionConstructor::create(vm, GeneratorFunctionConstructor::createStructure(vm, this, functionConstructor), m_generatorFunctionPrototype.get());
    m_generatorFunctionPrototype->putDirectWithoutTransition(vm, vm.propertyNames->constructor, generatorFunctionConstructor, DontEnum | ReadOnly);
    m_generatorFunctionStructure.set(vm, this, JSGeneratorFunction::createStructure(vm, this, m_generatorFunctionPrototype.get()));

    m_generatorPrototype->putDirectWithoutTransition(vm, vm.propertyNames->constructor, m_generatorFunctionPrototype.get(), DontEnum | ReadOnly);
    m_generatorFunctionPrototype->putDirectWithoutTransition(vm, vm.propertyNames->prototype, m_generatorPrototype.get(), DontEnum | ReadOnly);

    m_asyncFunctionPrototype.set(vm, this, AsyncFunctionPrototype::create(vm, AsyncFunctionPrototype::createStructure(vm, this, m_functionPrototype.get())));
    AsyncFunctionConstructor* asyncFunctionConstructor = AsyncFunctionConstructor::create(vm, AsyncFunctionConstructor::createStructure(vm, this, functionConstructor), m_asyncFunctionPrototype.get());
    m_asyncFunctionPrototype->putDirectWithoutTransition(vm, vm.propertyNames->constructor, asyncFunctionConstructor, DontEnum | ReadOnly);
    m_asyncFunctionStructure.set(vm, this, JSAsyncFunction::createStructure(vm, this, m_asyncFunctionPrototype.get()));

    m_objectPrototype->putDirectWithoutTransition(vm, vm.propertyNames->constructor, objectConstructor, DontEnum);
    m_functionPrototype->putDirectWithoutTransition(vm, vm.propertyNames->constructor, functionConstructor, DontEnum);
    m_arrayPrototype->putDirectWithoutTransition(vm, vm.propertyNames->constructor, arrayConstructor, DontEnum);
    m_regExpPrototype->putDirectWithoutTransition(vm, vm.propertyNames->constructor, m_regExpConstructor.get(), DontEnum);
    
    putDirectWithoutTransition(vm, vm.propertyNames->Object, objectConstructor, DontEnum);
    putDirectWithoutTransition(vm, vm.propertyNames->Function, functionConstructor, DontEnum);
    putDirectWithoutTransition(vm, vm.propertyNames->Array, arrayConstructor, DontEnum);
    putDirectWithoutTransition(vm, vm.propertyNames->RegExp, m_regExpConstructor.get(), DontEnum);
    putDirectWithoutTransition(vm, vm.propertyNames->RangeError, m_rangeErrorConstructor.get(), DontEnum);
    putDirectWithoutTransition(vm, vm.propertyNames->TypeError, m_typeErrorConstructor.get(), DontEnum);

    putDirectWithoutTransition(vm, vm.propertyNames->builtinNames().ObjectPrivateName(), objectConstructor, DontEnum | DontDelete | ReadOnly);
    putDirectWithoutTransition(vm, vm.propertyNames->builtinNames().ArrayPrivateName(), arrayConstructor, DontEnum | DontDelete | ReadOnly);

    putDirectWithoutTransition(vm, vm.propertyNames->ArrayBuffer, arrayBufferConstructor, DontEnum);
    putDirectWithoutTransition(vm, vm.propertyNames->SharedArrayBuffer, sharedArrayBufferConstructor, DontEnum);

#define PUT_CONSTRUCTOR_FOR_SIMPLE_TYPE(capitalName, lowerName, properName, instanceType, jsName, prototypeBase) \
putDirectWithoutTransition(vm, vm.propertyNames-> jsName, lowerName ## Constructor, DontEnum); \

    FOR_EACH_SIMPLE_BUILTIN_TYPE_WITH_CONSTRUCTOR(PUT_CONSTRUCTOR_FOR_SIMPLE_TYPE)

#undef PUT_CONSTRUCTOR_FOR_SIMPLE_TYPE
    m_iteratorResultObjectStructure.set(vm, this, createIteratorResultObjectStructure(vm, *this));
    
    m_evalFunction.set(vm, this, JSFunction::create(vm, this, 1, vm.propertyNames->eval.string(), globalFuncEval));
    putDirectWithoutTransition(vm, vm.propertyNames->eval, m_evalFunction.get(), DontEnum);
    
#if ENABLE(INTL)
    IntlObject* intl = IntlObject::create(vm, this, IntlObject::createStructure(vm, this, m_objectPrototype.get()));
    putDirectWithoutTransition(vm, vm.propertyNames->Intl, intl, DontEnum);
#endif // ENABLE(INTL)
    ReflectObject* reflectObject = ReflectObject::create(vm, this, ReflectObject::createStructure(vm, this, m_objectPrototype.get()));
    putDirectWithoutTransition(vm, vm.propertyNames->Reflect, reflectObject, DontEnum);

    m_moduleLoaderStructure.set(vm, this, JSModuleLoader::createStructure(vm, this, m_moduleLoaderPrototype.get()));
    m_moduleLoader.set(vm, this, JSModuleLoader::create(globalExec(), vm, this, m_moduleLoaderStructure.get()));
    if (Options::exposeInternalModuleLoader())
        putDirectWithoutTransition(vm, vm.propertyNames->Loader, m_moduleLoader.get(), DontEnum);

    JSFunction* builtinLog = JSFunction::create(vm, this, 1, vm.propertyNames->emptyIdentifier.string(), globalFuncBuiltinLog);

    JSFunction* privateFuncAbs = JSFunction::create(vm, this, 0, String(), mathProtoFuncAbs, AbsIntrinsic);
    JSFunction* privateFuncFloor = JSFunction::create(vm, this, 0, String(), mathProtoFuncFloor, FloorIntrinsic);
    JSFunction* privateFuncTrunc = JSFunction::create(vm, this, 0, String(), mathProtoFuncTrunc, TruncIntrinsic);

    JSFunction* privateFuncGetTemplateObject = JSFunction::create(vm, this, 0, String(), getTemplateObject);
    JSFunction* privateFuncImportModule = JSFunction::create(vm, this, 0, String(), globalFuncImportModule);
    JSFunction* privateFuncToObject = JSFunction::create(vm, this, 0, String(), privateToObject);
    JSFunction* privateFuncTypedArrayLength = JSFunction::create(vm, this, 0, String(), typedArrayViewPrivateFuncLength);
    JSFunction* privateFuncTypedArrayGetOriginalConstructor = JSFunction::create(vm, this, 0, String(), typedArrayViewPrivateFuncGetOriginalConstructor);
    JSFunction* privateFuncTypedArraySort = JSFunction::create(vm, this, 0, String(), typedArrayViewPrivateFuncSort);
    JSFunction* privateFuncIsTypedArrayView = JSFunction::create(vm, this, 0, String(), typedArrayViewPrivateFuncIsTypedArrayView, IsTypedArrayViewIntrinsic);
    JSFunction* privateFuncTypedArraySubarrayCreate = JSFunction::create(vm, this, 0, String(), typedArrayViewPrivateFuncSubarrayCreate);
    JSFunction* privateFuncIsBoundFunction = JSFunction::create(vm, this, 0, String(), isBoundFunction);
    JSFunction* privateFuncHasInstanceBoundFunction = JSFunction::create(vm, this, 0, String(), hasInstanceBoundFunction);
    JSFunction* privateFuncInstanceOf = JSFunction::create(vm, this, 0, String(), objectPrivateFuncInstanceOf);
    JSFunction* privateFuncThisTimeValue = JSFunction::create(vm, this, 0, String(), dateProtoFuncGetTime);
    JSFunction* privateFuncThisNumberValue = JSFunction::create(vm, this, 0, String(), numberProtoFuncValueOf);
    JSFunction* privateFuncIsArrayConstructor = JSFunction::create(vm, this, 0, String(), arrayConstructorPrivateFuncIsArrayConstructor);
    JSFunction* privateFuncIsArraySlow = JSFunction::create(vm, this, 0, String(), arrayConstructorPrivateFuncIsArraySlow);
    JSFunction* privateFuncConcatMemcpy = JSFunction::create(vm, this, 0, String(), arrayProtoPrivateFuncConcatMemcpy);
    JSFunction* privateFuncAppendMemcpy = JSFunction::create(vm, this, 0, String(), arrayProtoPrivateFuncAppendMemcpy);
    JSFunction* privateFuncConcatSlowPath = JSFunction::createBuiltinFunction(vm, arrayPrototypeConcatSlowPathCodeGenerator(vm), this);

    JSObject* regExpProtoFlagsGetterObject = getGetterById(exec, m_regExpPrototype.get(), vm.propertyNames->flags);
    ASSERT_UNUSED(catchScope, !catchScope.exception());
    JSObject* regExpProtoGlobalGetterObject = getGetterById(exec, m_regExpPrototype.get(), vm.propertyNames->global);
    ASSERT(!catchScope.exception());
    m_regExpProtoGlobalGetter.set(vm, this, regExpProtoGlobalGetterObject);
    JSObject* regExpProtoIgnoreCaseGetterObject = getGetterById(exec, m_regExpPrototype.get(), vm.propertyNames->ignoreCase);
    ASSERT(!catchScope.exception());
    JSObject* regExpProtoMultilineGetterObject = getGetterById(exec, m_regExpPrototype.get(), vm.propertyNames->multiline);
    ASSERT(!catchScope.exception());
    JSObject* regExpProtoSourceGetterObject = getGetterById(exec, m_regExpPrototype.get(), vm.propertyNames->source);
    ASSERT(!catchScope.exception());
    JSObject* regExpProtoStickyGetterObject = getGetterById(exec, m_regExpPrototype.get(), vm.propertyNames->sticky);
    ASSERT(!catchScope.exception());
    JSObject* regExpProtoUnicodeGetterObject = getGetterById(exec, m_regExpPrototype.get(), vm.propertyNames->unicode);
    ASSERT(!catchScope.exception());
    m_regExpProtoUnicodeGetter.set(vm, this, regExpProtoUnicodeGetterObject);
    JSObject* builtinRegExpExec = asObject(m_regExpPrototype->getDirect(vm, vm.propertyNames->exec).asCell());
    m_regExpProtoExec.set(vm, this, builtinRegExpExec);
    JSObject* regExpSymbolReplace = asObject(m_regExpPrototype->getDirect(vm, vm.propertyNames->replaceSymbol).asCell());
    m_regExpProtoSymbolReplace.set(vm, this, regExpSymbolReplace);

#define CREATE_PRIVATE_GLOBAL_FUNCTION(name, code) JSFunction* name ## PrivateFunction = JSFunction::createBuiltinFunction(vm, code ## CodeGenerator(vm), this);
    JSC_FOREACH_BUILTIN_FUNCTION_PRIVATE_GLOBAL_NAME(CREATE_PRIVATE_GLOBAL_FUNCTION)
#undef CREATE_PRIVATE_GLOBAL_FUNCTION

    JSObject* arrayIteratorPrototype = ArrayIteratorPrototype::create(vm, this, ArrayIteratorPrototype::createStructure(vm, this, m_iteratorPrototype.get()));
    createArrayIteratorPrivateFunction->putDirect(vm, vm.propertyNames->prototype, arrayIteratorPrototype);

    GlobalPropertyInfo staticGlobals[] = {
#define INIT_PRIVATE_GLOBAL(name, code) GlobalPropertyInfo(vm.propertyNames->builtinNames().name ## PrivateName(), name ## PrivateFunction, DontEnum | DontDelete | ReadOnly),
        JSC_FOREACH_BUILTIN_FUNCTION_PRIVATE_GLOBAL_NAME(INIT_PRIVATE_GLOBAL)
#undef INIT_PRIVATE_GLOBAL
        GlobalPropertyInfo(vm.propertyNames->NaN, jsNaN(), DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->Infinity, jsNumber(std::numeric_limits<double>::infinity()), DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->undefinedKeyword, jsUndefined(), DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().ownEnumerablePropertyKeysPrivateName(), JSFunction::create(vm, this, 0, String(), ownEnumerablePropertyKeys), DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().getTemplateObjectPrivateName(), privateFuncGetTemplateObject, DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().importModulePrivateName(), privateFuncImportModule, DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().toObjectPrivateName(), privateFuncToObject, DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().enqueueJobPrivateName(), JSFunction::create(vm, this, 0, String(), enqueueJob), DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().ErrorPrivateName(), m_errorConstructor.get(), DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().RangeErrorPrivateName(), m_rangeErrorConstructor.get(), DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().TypeErrorPrivateName(), m_typeErrorConstructor.get(), DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().typedArrayLengthPrivateName(), privateFuncTypedArrayLength, DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().typedArrayGetOriginalConstructorPrivateName(), privateFuncTypedArrayGetOriginalConstructor, DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().typedArraySortPrivateName(), privateFuncTypedArraySort, DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().isTypedArrayViewPrivateName(), privateFuncIsTypedArrayView, DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().typedArraySubarrayCreatePrivateName(), privateFuncTypedArraySubarrayCreate, DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().isBoundFunctionPrivateName(), privateFuncIsBoundFunction, DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().hasInstanceBoundFunctionPrivateName(), privateFuncHasInstanceBoundFunction, DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().instanceOfPrivateName(), privateFuncInstanceOf, DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().BuiltinLogPrivateName(), builtinLog, DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().NumberPrivateName(), numberConstructor, DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().RegExpPrivateName(), m_regExpConstructor.get(), DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().StringPrivateName(), stringConstructor, DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().absPrivateName(), privateFuncAbs, DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().floorPrivateName(), privateFuncFloor, DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().truncPrivateName(), privateFuncTrunc, DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().PromisePrivateName(), promiseConstructor, DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().ReflectPrivateName(), reflectObject, DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().InternalPromisePrivateName(), internalPromiseConstructor, DontEnum | DontDelete | ReadOnly),

        GlobalPropertyInfo(vm.propertyNames->builtinNames().repeatCharacterPrivateName(), JSFunction::create(vm, this, 2, String(), stringProtoFuncRepeatCharacter), DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().SetIteratorPrivateName(), JSFunction::create(vm, this, 1, String(), privateFuncSetIterator), DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().setIteratorNextPrivateName(), JSFunction::create(vm, this, 0, String(), privateFuncSetIteratorNext), DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().isArrayPrivateName(), arrayConstructor->getDirect(vm, vm.propertyNames->isArray), DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().isArraySlowPrivateName(), privateFuncIsArraySlow, DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().isArrayConstructorPrivateName(), privateFuncIsArrayConstructor, DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().concatMemcpyPrivateName(), privateFuncConcatMemcpy, DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().appendMemcpyPrivateName(), privateFuncAppendMemcpy, DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().concatSlowPathPrivateName(), privateFuncConcatSlowPath, DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().MapIteratorPrivateName(), JSFunction::create(vm, this, 1, String(), privateFuncMapIterator), DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().mapIteratorNextPrivateName(), JSFunction::create(vm, this, 0, String(), privateFuncMapIteratorNext), DontEnum | DontDelete | ReadOnly),

        GlobalPropertyInfo(vm.propertyNames->builtinNames().InspectorInstrumentationPrivateName(), InspectorInstrumentationObject::create(vm, this, InspectorInstrumentationObject::createStructure(vm, this, m_objectPrototype.get())), DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().MapPrivateName(), mapConstructor, DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().thisTimeValuePrivateName(), privateFuncThisTimeValue, DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().thisNumberValuePrivateName(), privateFuncThisNumberValue, DontEnum | DontDelete | ReadOnly),
#if ENABLE(INTL)
        GlobalPropertyInfo(vm.propertyNames->builtinNames().CollatorPrivateName(), intl->getDirect(vm, vm.propertyNames->Collator), DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().DateTimeFormatPrivateName(), intl->getDirect(vm, vm.propertyNames->DateTimeFormat), DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().NumberFormatPrivateName(), intl->getDirect(vm, vm.propertyNames->NumberFormat), DontEnum | DontDelete | ReadOnly),
#endif // ENABLE(INTL)

        GlobalPropertyInfo(vm.propertyNames->builtinNames().isConstructorPrivateName(), JSFunction::create(vm, this, 1, String(), esSpecIsConstructor, NoIntrinsic), DontEnum | DontDelete | ReadOnly),

        GlobalPropertyInfo(vm.propertyNames->builtinNames().regExpProtoFlagsGetterPrivateName(), regExpProtoFlagsGetterObject, DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().regExpProtoGlobalGetterPrivateName(), regExpProtoGlobalGetterObject, DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().regExpProtoIgnoreCaseGetterPrivateName(), regExpProtoIgnoreCaseGetterObject, DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().regExpProtoMultilineGetterPrivateName(), regExpProtoMultilineGetterObject, DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().regExpProtoSourceGetterPrivateName(), regExpProtoSourceGetterObject, DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().regExpProtoStickyGetterPrivateName(), regExpProtoStickyGetterObject, DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().regExpProtoUnicodeGetterPrivateName(), regExpProtoUnicodeGetterObject, DontEnum | DontDelete | ReadOnly),

        // RegExp.prototype helpers.
        GlobalPropertyInfo(vm.propertyNames->builtinNames().regExpBuiltinExecPrivateName(), builtinRegExpExec, DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().regExpCreatePrivateName(), JSFunction::create(vm, this, 2, String(), esSpecRegExpCreate, NoIntrinsic), DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().regExpMatchFastPrivateName(), JSFunction::create(vm, this, 1, String(), regExpProtoFuncMatchFast), DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().regExpSearchFastPrivateName(), JSFunction::create(vm, this, 1, String(), regExpProtoFuncSearchFast), DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().regExpSplitFastPrivateName(), JSFunction::create(vm, this, 2, String(), regExpProtoFuncSplitFast), DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().regExpPrototypeSymbolReplacePrivateName(), m_regExpPrototype->getDirect(vm, vm.propertyNames->replaceSymbol), DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().regExpTestFastPrivateName(), JSFunction::create(vm, this, 1, String(), regExpProtoFuncTestFast, RegExpTestFastIntrinsic), DontEnum | DontDelete | ReadOnly),

        // String.prototype helpers.
        GlobalPropertyInfo(vm.propertyNames->builtinNames().stringIncludesInternalPrivateName(), JSFunction::create(vm, this, 1, String(), builtinStringIncludesInternal), DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().stringSplitFastPrivateName(), JSFunction::create(vm, this, 2, String(), stringProtoFuncSplitFast), DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().stringSubstrInternalPrivateName(), JSFunction::create(vm, this, 2, String(), builtinStringSubstrInternal), DontEnum | DontDelete | ReadOnly),

        // Function prototype helpers.
        GlobalPropertyInfo(vm.propertyNames->builtinNames().makeBoundFunctionPrivateName(), JSFunction::create(vm, this, 5, String(), makeBoundFunction), DontEnum | DontDelete | ReadOnly),
        GlobalPropertyInfo(vm.propertyNames->builtinNames().hasOwnLengthPropertyPrivateName(), JSFunction::create(vm, this, 1, String(), hasOwnLengthProperty), DontEnum | DontDelete | ReadOnly),
    };
    addStaticGlobals(staticGlobals, WTF_ARRAY_LENGTH(staticGlobals));
    
    m_specialPointers[Special::CallFunction] = m_callFunction.get();
    m_specialPointers[Special::ApplyFunction] = m_applyFunction.get();
    m_specialPointers[Special::ObjectConstructor] = objectConstructor;
    m_specialPointers[Special::ArrayConstructor] = arrayConstructor;

    m_linkTimeConstants[static_cast<unsigned>(LinkTimeConstant::ThrowTypeErrorFunction)] = m_throwTypeErrorFunction.get();

    if (UNLIKELY(Options::useDollarVM())) {
        JSDollarVMPrototype* dollarVMPrototype = JSDollarVMPrototype::create(vm, this, JSDollarVMPrototype::createStructure(vm, this, m_objectPrototype.get()));
        m_dollarVMStructure.set(vm, this, JSDollarVM::createStructure(vm, this, dollarVMPrototype));
        JSDollarVM* dollarVM = JSDollarVM::create(vm, m_dollarVMStructure.get());

        GlobalPropertyInfo extraStaticGlobals[] = {
            GlobalPropertyInfo(vm.propertyNames->builtinNames().dollarVMPrivateName(), dollarVM, DontEnum | DontDelete | ReadOnly),
        };
        addStaticGlobals(extraStaticGlobals, WTF_ARRAY_LENGTH(extraStaticGlobals));

        putDirectWithoutTransition(vm, Identifier::fromString(exec, "$vm"), dollarVM, DontEnum);
    }

#if ENABLE(WEBASSEMBLY)
    if (Options::useWebAssembly()) {
        auto* webAssemblyPrototype = WebAssemblyPrototype::create(vm, this, WebAssemblyPrototype::createStructure(vm, this, m_objectPrototype.get()));
        m_webAssemblyStructure.set(vm, this, JSWebAssembly::createStructure(vm, this, webAssemblyPrototype));
        m_webAssemblyModuleRecordStructure.set(vm, this, WebAssemblyModuleRecord::createStructure(vm, this, m_objectPrototype.get()));
        m_webAssemblyFunctionStructure.set(vm, this, WebAssemblyFunction::createStructure(vm, this, m_functionPrototype.get()));
        m_webAssemblyWrapperFunctionStructure.set(vm, this, WebAssemblyWrapperFunction::createStructure(vm, this, m_functionPrototype.get()));
        auto* webAssembly = JSWebAssembly::create(vm, this, m_webAssemblyStructure.get());
        putDirectWithoutTransition(vm, Identifier::fromString(exec, "WebAssembly"), webAssembly, DontEnum);

#define CREATE_WEBASSEMBLY_CONSTRUCTOR(capitalName, lowerName, properName, instanceType, jsName, prototypeBase) do { \
        typedef capitalName ## Prototype Prototype; \
        typedef capitalName ## Constructor Constructor; \
        typedef JS ## capitalName JSObj; \
        auto* base = m_ ## prototypeBase ## Prototype.get(); \
        auto* prototype = Prototype::create(vm, this, Prototype::createStructure(vm, this, base)); \
        auto* structure = JSObj::createStructure(vm, this, prototype); \
        auto* constructor = Constructor::create(vm, Constructor::createStructure(vm, this, this->functionPrototype()), prototype); \
        prototype->putDirectWithoutTransition(vm, vm.propertyNames->constructor, constructor, DontEnum); \
        m_ ## lowerName ## Prototype.set(vm, this, prototype); \
        m_ ## properName ## Structure.set(vm, this, structure); \
        webAssembly->putDirectWithoutTransition(vm, Identifier::fromString(this->globalExec(), #jsName), constructor, DontEnum); \
    } while (0);

        FOR_EACH_WEBASSEMBLY_CONSTRUCTOR_TYPE(CREATE_WEBASSEMBLY_CONSTRUCTOR)

#undef CREATE_WEBASSEMBLY_CONSTRUCTOR
    }
#endif // ENABLE(WEBASSEMBLY)

    {
        ExecState* exec = globalExec();

        auto setupAdaptiveWatchpoint = [&] (JSObject* base, const Identifier& ident) -> ObjectPropertyCondition {
            // Performing these gets should not throw.
            PropertySlot slot(base, PropertySlot::InternalMethodType::Get);
            bool result = base->getOwnPropertySlot(base, exec, ident, slot);
            ASSERT_UNUSED(result, result);
            ASSERT(!catchScope.exception());
            RELEASE_ASSERT(slot.isCacheableValue());
            JSValue functionValue = slot.getValue(exec, ident);
            ASSERT(!catchScope.exception());
            ASSERT(jsDynamicCast<JSFunction*>(vm, functionValue));

            ObjectPropertyCondition condition = generateConditionForSelfEquivalence(m_vm, nullptr, base, ident.impl());
            RELEASE_ASSERT(condition.requiredValue() == functionValue);

            bool isWatchable = condition.isWatchable(PropertyCondition::EnsureWatchability);
            RELEASE_ASSERT(isWatchable); // We allow this to install the necessary watchpoints.

            return condition;
        };

        {
            ObjectPropertyCondition condition = setupAdaptiveWatchpoint(arrayIteratorPrototype, m_vm.propertyNames->next);
            m_arrayIteratorPrototypeNext = std::make_unique<ArrayIteratorAdaptiveWatchpoint>(condition, this);
            m_arrayIteratorPrototypeNext->install();
        }

        {
            ArrayPrototype* arrayPrototype = this->arrayPrototype();
            ObjectPropertyCondition condition = setupAdaptiveWatchpoint(arrayPrototype, m_vm.propertyNames->iteratorSymbol);
            m_arrayPrototypeSymbolIteratorWatchpoint = std::make_unique<ArrayIteratorAdaptiveWatchpoint>(condition, this);
            m_arrayPrototypeSymbolIteratorWatchpoint->install();
        }
    }

    resetPrototype(vm, getPrototypeDirect());
}

bool JSGlobalObject::put(JSCell* cell, ExecState* exec, PropertyName propertyName, JSValue value, PutPropertySlot& slot)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSGlobalObject* thisObject = jsCast<JSGlobalObject*>(cell);
    ASSERT(!Heap::heap(value) || Heap::heap(value) == Heap::heap(thisObject));

    if (UNLIKELY(isThisValueAltered(slot, thisObject))) {
        scope.release();
        return ordinarySetSlow(exec, thisObject, propertyName, value, slot.thisValue(), slot.isStrictMode());
    }

    bool shouldThrowReadOnlyError = slot.isStrictMode();
    bool ignoreReadOnlyErrors = false;
    bool putResult = false;
    bool done = symbolTablePutTouchWatchpointSet(thisObject, exec, propertyName, value, shouldThrowReadOnlyError, ignoreReadOnlyErrors, putResult);
    ASSERT((!!scope.exception() == (done && !putResult)) || !shouldThrowReadOnlyError);
    if (done)
        return putResult;
    scope.release();
    return Base::put(thisObject, exec, propertyName, value, slot);
}

bool JSGlobalObject::defineOwnProperty(JSObject* object, ExecState* exec, PropertyName propertyName, const PropertyDescriptor& descriptor, bool shouldThrow)
{
    JSGlobalObject* thisObject = jsCast<JSGlobalObject*>(object);
    PropertySlot slot(thisObject, PropertySlot::InternalMethodType::VMInquiry);
    // silently ignore attempts to add accessors aliasing vars.
    if (descriptor.isAccessorDescriptor() && symbolTableGet(thisObject, propertyName, slot))
        return false;
    return Base::defineOwnProperty(thisObject, exec, propertyName, descriptor, shouldThrow);
}

void JSGlobalObject::addGlobalVar(const Identifier& ident)
{
    ConcurrentJSLocker locker(symbolTable()->m_lock);
    SymbolTableEntry entry = symbolTable()->get(locker, ident.impl());
    if (!entry.isNull())
        return;
    
    ScopeOffset offset = symbolTable()->takeNextScopeOffset(locker);
    SymbolTableEntry newEntry(VarOffset(offset), 0);
    newEntry.prepareToWatch();
    symbolTable()->add(locker, ident.impl(), WTFMove(newEntry));
    
    ScopeOffset offsetForAssert = addVariables(1, jsUndefined());
    RELEASE_ASSERT(offsetForAssert == offset);
}

void JSGlobalObject::addFunction(ExecState* exec, const Identifier& propertyName)
{
    VM& vm = exec->vm();
    VM::DeletePropertyModeScope scope(vm, VM::DeletePropertyMode::IgnoreConfigurable);
    methodTable(vm)->deleteProperty(this, exec, propertyName);
    addGlobalVar(propertyName);
}

void JSGlobalObject::setGlobalScopeExtension(JSScope* scope)
{
    m_globalScopeExtension.set(vm(), this, scope);
}

void JSGlobalObject::clearGlobalScopeExtension()
{
    m_globalScopeExtension.clear();
}

static inline JSObject* lastInPrototypeChain(JSObject* object)
{
    JSObject* o = object;
    while (o->getPrototypeDirect().isObject())
        o = asObject(o->getPrototypeDirect());
    return o;
}

// Private namespace for helpers for JSGlobalObject::haveABadTime()
namespace {

class ObjectsWithBrokenIndexingFinder : public MarkedBlock::VoidFunctor {
public:
    ObjectsWithBrokenIndexingFinder(MarkedArgumentBuffer&, JSGlobalObject*);
    IterationStatus operator()(HeapCell*, HeapCell::Kind) const;

private:
    void visit(JSCell*);

    MarkedArgumentBuffer& m_foundObjects;
    JSGlobalObject* m_globalObject;
};

ObjectsWithBrokenIndexingFinder::ObjectsWithBrokenIndexingFinder(
    MarkedArgumentBuffer& foundObjects, JSGlobalObject* globalObject)
    : m_foundObjects(foundObjects)
    , m_globalObject(globalObject)
{
}

inline bool hasBrokenIndexing(JSObject* object)
{
    // This will change if we have more indexing types.
    IndexingType type = object->indexingType();
    // This could be made obviously more efficient, but isn't made so right now, because
    // we expect this to be an unlikely slow path anyway.
    return hasUndecided(type) || hasInt32(type) || hasDouble(type) || hasContiguous(type) || hasArrayStorage(type);
}

inline void ObjectsWithBrokenIndexingFinder::visit(JSCell* cell)
{
    if (!cell->isObject())
        return;
    
    JSObject* object = asObject(cell);

    // Run this filter first, since it's cheap, and ought to filter out a lot of objects.
    if (!hasBrokenIndexing(object))
        return;
    
    // We only want to have a bad time in the affected global object, not in the entire
    // VM. But we have to be careful, since there may be objects that claim to belong to
    // a different global object that have prototypes from our global object.
    bool foundGlobalObject = false;
    for (JSObject* current = object; ;) {
        if (current->globalObject() == m_globalObject) {
            foundGlobalObject = true;
            break;
        }
        
        JSValue prototypeValue = current->getPrototypeDirect();
        if (prototypeValue.isNull())
            break;
        current = asObject(prototypeValue);
    }
    if (!foundGlobalObject)
        return;
    
    m_foundObjects.append(object);
}

IterationStatus ObjectsWithBrokenIndexingFinder::operator()(HeapCell* cell, HeapCell::Kind kind) const
{
    if (kind == HeapCell::JSCell) {
        // FIXME: This const_cast exists because this isn't a C++ lambda.
        // https://bugs.webkit.org/show_bug.cgi?id=159644
        const_cast<ObjectsWithBrokenIndexingFinder*>(this)->visit(static_cast<JSCell*>(cell));
    }
    return IterationStatus::Continue;
}

} // end private namespace for helpers for JSGlobalObject::haveABadTime()

void JSGlobalObject::haveABadTime(VM& vm)
{
    ASSERT(&vm == &this->vm());
    
    if (isHavingABadTime())
        return;
    
    // Make sure that all allocations or indexed storage transitions that are inlining
    // the assumption that it's safe to transition to a non-SlowPut array storage don't
    // do so anymore.
    m_havingABadTimeWatchpoint->fireAll(vm, "Having a bad time");
    ASSERT(isHavingABadTime()); // The watchpoint is what tells us that we're having a bad time.
    
    // Make sure that all JSArray allocations that load the appropriate structure from
    // this object now load a structure that uses SlowPut.
    for (unsigned i = 0; i < NumberOfIndexingShapes; ++i)
        m_arrayStructureForIndexingShapeDuringAllocation[i].set(vm, this, originalArrayStructureForIndexingType(ArrayWithSlowPutArrayStorage));

    // Same for any special array structures.
    Structure* slowPutStructure;
    slowPutStructure = createRegExpMatchesArraySlowPutStructure(vm, this);
    m_regExpMatchesArrayStructure.set(vm, this, slowPutStructure);
    slowPutStructure = ClonedArguments::createSlowPutStructure(vm, this, m_objectPrototype.get());
    m_clonedArgumentsStructure.set(vm, this, slowPutStructure);

    // Make sure that all objects that have indexed storage switch to the slow kind of
    // indexed storage.
    MarkedArgumentBuffer foundObjects; // Use MarkedArgumentBuffer because switchToSlowPutArrayStorage() may GC.
    ObjectsWithBrokenIndexingFinder finder(foundObjects, this);
    {
        HeapIterationScope iterationScope(vm.heap);
        vm.heap.objectSpace().forEachLiveCell(iterationScope, finder);
    }
    while (!foundObjects.isEmpty()) {
        JSObject* object = asObject(foundObjects.last());
        foundObjects.removeLast();
        ASSERT(hasBrokenIndexing(object));
        object->switchToSlowPutArrayStorage(vm);
    }
}

// Set prototype, and also insert the object prototype at the end of the chain.
void JSGlobalObject::resetPrototype(VM& vm, JSValue prototype)
{
    setPrototypeDirect(vm, prototype);

    JSObject* oldLastInPrototypeChain = lastInPrototypeChain(this);
    JSObject* objectPrototype = m_objectPrototype.get();
    if (oldLastInPrototypeChain != objectPrototype)
        oldLastInPrototypeChain->setPrototypeDirect(vm, objectPrototype);

    // Whenever we change the prototype of the global object, we need to create a new JSProxy with the correct prototype.
    setGlobalThis(vm, JSProxy::create(vm, JSProxy::createStructure(vm, this, prototype, PureForwardingProxyType), this));
}

void JSGlobalObject::visitChildren(JSCell* cell, SlotVisitor& visitor)
{ 
    JSGlobalObject* thisObject = jsCast<JSGlobalObject*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());
    Base::visitChildren(thisObject, visitor);

    visitor.append(thisObject->m_globalThis);

    visitor.append(thisObject->m_globalLexicalEnvironment);
    visitor.append(thisObject->m_globalScopeExtension);
    visitor.append(thisObject->m_globalCallee);
    visitor.append(thisObject->m_regExpConstructor);
    visitor.append(thisObject->m_errorConstructor);
    visitor.append(thisObject->m_nativeErrorPrototypeStructure);
    visitor.append(thisObject->m_nativeErrorStructure);
    thisObject->m_evalErrorConstructor.visit(visitor);
    visitor.append(thisObject->m_rangeErrorConstructor);
    thisObject->m_referenceErrorConstructor.visit(visitor);
    thisObject->m_syntaxErrorConstructor.visit(visitor);
    visitor.append(thisObject->m_typeErrorConstructor);
    thisObject->m_URIErrorConstructor.visit(visitor);
    visitor.append(thisObject->m_objectConstructor);
    visitor.append(thisObject->m_promiseConstructor);

    visitor.append(thisObject->m_nullGetterFunction);
    visitor.append(thisObject->m_nullSetterFunction);

    visitor.append(thisObject->m_parseIntFunction);
    visitor.append(thisObject->m_evalFunction);
    visitor.append(thisObject->m_callFunction);
    visitor.append(thisObject->m_applyFunction);
    visitor.append(thisObject->m_throwTypeErrorFunction);
    thisObject->m_arrayProtoToStringFunction.visit(visitor);
    thisObject->m_arrayProtoValuesFunction.visit(visitor);
    thisObject->m_initializePromiseFunction.visit(visitor);
    thisObject->m_iteratorProtocolFunction.visit(visitor);
    visitor.append(thisObject->m_objectProtoValueOfFunction);
    visitor.append(thisObject->m_newPromiseCapabilityFunction);
    visitor.append(thisObject->m_functionProtoHasInstanceSymbolFunction);
    thisObject->m_throwTypeErrorGetterSetter.visit(visitor);
    visitor.append(thisObject->m_throwTypeErrorArgumentsCalleeAndCallerGetterSetter);
    visitor.append(thisObject->m_moduleLoader);

    visitor.append(thisObject->m_objectPrototype);
    visitor.append(thisObject->m_functionPrototype);
    visitor.append(thisObject->m_arrayPrototype);
    visitor.append(thisObject->m_errorPrototype);
    visitor.append(thisObject->m_iteratorPrototype);
    visitor.append(thisObject->m_generatorFunctionPrototype);
    visitor.append(thisObject->m_generatorPrototype);
    visitor.append(thisObject->m_asyncFunctionPrototype);
    visitor.append(thisObject->m_moduleLoaderPrototype);

    thisObject->m_debuggerScopeStructure.visit(visitor);
    thisObject->m_withScopeStructure.visit(visitor);
    visitor.append(thisObject->m_strictEvalActivationStructure);
    visitor.append(thisObject->m_lexicalEnvironmentStructure);
    thisObject->m_moduleEnvironmentStructure.visit(visitor);
    visitor.append(thisObject->m_directArgumentsStructure);
    visitor.append(thisObject->m_scopedArgumentsStructure);
    visitor.append(thisObject->m_clonedArgumentsStructure);
    visitor.append(thisObject->m_objectStructureForObjectConstructor);
    for (unsigned i = 0; i < NumberOfIndexingShapes; ++i)
        visitor.append(thisObject->m_originalArrayStructureForIndexingShape[i]);
    for (unsigned i = 0; i < NumberOfIndexingShapes; ++i)
        visitor.append(thisObject->m_arrayStructureForIndexingShapeDuringAllocation[i]);
    thisObject->m_callbackConstructorStructure.visit(visitor);
    thisObject->m_callbackFunctionStructure.visit(visitor);
    thisObject->m_callbackObjectStructure.visit(visitor);
    visitor.append(thisObject->m_propertyNameIteratorStructure);
#if JSC_OBJC_API_ENABLED
    thisObject->m_objcCallbackFunctionStructure.visit(visitor);
    thisObject->m_objcWrapperObjectStructure.visit(visitor);
#endif
    thisObject->m_nullPrototypeObjectStructure.visit(visitor);
    visitor.append(thisObject->m_errorStructure);
    visitor.append(thisObject->m_calleeStructure);
    visitor.append(thisObject->m_functionStructure);
    thisObject->m_customGetterSetterFunctionStructure.visit(visitor);
    thisObject->m_boundFunctionStructure.visit(visitor);
    visitor.append(thisObject->m_getterSetterStructure);
    thisObject->m_nativeStdFunctionStructure.visit(visitor);
    thisObject->m_namedFunctionStructure.visit(visitor);
    visitor.append(thisObject->m_symbolObjectStructure);
    visitor.append(thisObject->m_regExpStructure);
    visitor.append(thisObject->m_generatorFunctionStructure);
    visitor.append(thisObject->m_asyncFunctionStructure);
    visitor.append(thisObject->m_iteratorResultObjectStructure);
    visitor.append(thisObject->m_regExpMatchesArrayStructure);
    visitor.append(thisObject->m_moduleRecordStructure);
    visitor.append(thisObject->m_moduleNamespaceObjectStructure);
    visitor.append(thisObject->m_dollarVMStructure);
    visitor.append(thisObject->m_proxyObjectStructure);
    visitor.append(thisObject->m_callableProxyObjectStructure);
    visitor.append(thisObject->m_proxyRevokeStructure);
    visitor.append(thisObject->m_moduleLoaderStructure);
    
    visitor.append(thisObject->m_arrayBufferPrototype);
    visitor.append(thisObject->m_arrayBufferStructure);
    visitor.append(thisObject->m_sharedArrayBufferPrototype);
    visitor.append(thisObject->m_sharedArrayBufferStructure);

#define VISIT_SIMPLE_TYPE(CapitalName, lowerName, properName, instanceType, jsName, prototypeBase) \
    visitor.append(thisObject->m_ ## lowerName ## Prototype); \
    visitor.append(thisObject->m_ ## properName ## Structure); \

    FOR_EACH_SIMPLE_BUILTIN_TYPE(VISIT_SIMPLE_TYPE)
    
#if ENABLE(WEBASSEMBLY)
    visitor.append(thisObject->m_webAssemblyStructure);
    visitor.append(thisObject->m_webAssemblyModuleRecordStructure);
    visitor.append(thisObject->m_webAssemblyFunctionStructure);
    visitor.append(thisObject->m_webAssemblyWrapperFunctionStructure);
    FOR_EACH_WEBASSEMBLY_CONSTRUCTOR_TYPE(VISIT_SIMPLE_TYPE)
#endif // ENABLE(WEBASSEMBLY)

#undef VISIT_SIMPLE_TYPE

#define VISIT_LAZY_TYPE(CapitalName, lowerName, properName, instanceType, jsName, prototypeBase) \
    thisObject->m_ ## properName ## Structure.visit(visitor);
    
    FOR_EACH_LAZY_BUILTIN_TYPE(VISIT_LAZY_TYPE)
    FOR_EACH_BUILTIN_DERIVED_ITERATOR_TYPE(VISIT_LAZY_TYPE)

#undef VISIT_LAZY_TYPE

    for (unsigned i = NUMBER_OF_TYPED_ARRAY_TYPES; i--;)
        thisObject->lazyTypedArrayStructure(indexToTypedArrayType(i)).visit(visitor);
    
    visitor.append(thisObject->m_speciesGetterSetter);
    thisObject->m_typedArrayProto.visit(visitor);
    thisObject->m_typedArraySuperConstructor.visit(visitor);
}

JSValue JSGlobalObject::toThis(JSCell*, ExecState* exec, ECMAMode ecmaMode)
{
    if (ecmaMode == StrictMode)
        return jsUndefined();
    return exec->globalThisValue();
}

ExecState* JSGlobalObject::globalExec()
{
    return CallFrame::create(m_globalCallFrame);
}

void JSGlobalObject::addStaticGlobals(GlobalPropertyInfo* globals, int count)
{
    ScopeOffset startOffset = addVariables(count, jsUndefined());

    for (int i = 0; i < count; ++i) {
        GlobalPropertyInfo& global = globals[i];
        ASSERT(global.attributes & DontDelete);
        
        WatchpointSet* watchpointSet = nullptr;
        WriteBarrierBase<Unknown>* variable = nullptr;
        {
            ConcurrentJSLocker locker(symbolTable()->m_lock);
            ScopeOffset offset = symbolTable()->takeNextScopeOffset(locker);
            RELEASE_ASSERT(offset == startOffset + i);
            SymbolTableEntry newEntry(VarOffset(offset), global.attributes);
            newEntry.prepareToWatch();
            watchpointSet = newEntry.watchpointSet();
            symbolTable()->add(locker, global.identifier.impl(), WTFMove(newEntry));
            variable = &variableAt(offset);
        }
        symbolTablePutTouchWatchpointSet(vm(), this, global.identifier, global.value, variable, watchpointSet);
    }
}

bool JSGlobalObject::getOwnPropertySlot(JSObject* object, ExecState* exec, PropertyName propertyName, PropertySlot& slot)
{
    if (Base::getOwnPropertySlot(object, exec, propertyName, slot))
        return true;
    return symbolTableGet(jsCast<JSGlobalObject*>(object), propertyName, slot);
}

void JSGlobalObject::clearRareData(JSCell* cell)
{
    jsCast<JSGlobalObject*>(cell)->m_rareData = nullptr;
}

void slowValidateCell(JSGlobalObject* globalObject)
{
    RELEASE_ASSERT(globalObject->isGlobalObject());
    ASSERT_GC_OBJECT_INHERITS(globalObject, JSGlobalObject::info());
}

void JSGlobalObject::setRemoteDebuggingEnabled(bool enabled)
{
#if ENABLE(REMOTE_INSPECTOR)
    m_inspectorDebuggable->setRemoteDebuggingAllowed(enabled);
#else
    UNUSED_PARAM(enabled);
#endif
}

bool JSGlobalObject::remoteDebuggingEnabled() const
{
#if ENABLE(REMOTE_INSPECTOR)
    return m_inspectorDebuggable->remoteDebuggingAllowed();
#else
    return false;
#endif
}

#if ENABLE(WEB_REPLAY)
void JSGlobalObject::setInputCursor(Ref<InputCursor>&& cursor)
{
    m_inputCursor = WTFMove(cursor);
    // Save or set the random seed. This performed here rather than the constructor
    // to avoid threading the input cursor through all the abstraction layers.
    if (m_inputCursor->isCapturing())
        m_inputCursor->appendInput<SetRandomSeed>(m_weakRandom.seed());
    else if (m_inputCursor->isReplaying()) {
        if (SetRandomSeed* input = m_inputCursor->fetchInput<SetRandomSeed>())
            m_weakRandom.setSeed(static_cast<unsigned>(input->randomSeed()));
    }
}
#endif

void JSGlobalObject::setName(const String& name)
{
    m_name = name;

#if ENABLE(REMOTE_INSPECTOR)
    m_inspectorDebuggable->update();
#endif
}

# if ENABLE(INTL)
const HashSet<String>& JSGlobalObject::intlCollatorAvailableLocales()
{
    if (m_intlCollatorAvailableLocales.isEmpty()) {
        int32_t count = ucol_countAvailable();
        for (int32_t i = 0; i < count; ++i) {
            String locale(ucol_getAvailable(i));
            convertICULocaleToBCP47LanguageTag(locale);
            m_intlCollatorAvailableLocales.add(locale);
        }
    }
    return m_intlCollatorAvailableLocales;
}

const HashSet<String>& JSGlobalObject::intlDateTimeFormatAvailableLocales()
{
    if (m_intlDateTimeFormatAvailableLocales.isEmpty()) {
        int32_t count = udat_countAvailable();
        for (int32_t i = 0; i < count; ++i) {
            String locale(udat_getAvailable(i));
            convertICULocaleToBCP47LanguageTag(locale);
            m_intlDateTimeFormatAvailableLocales.add(locale);
        }
    }
    return m_intlDateTimeFormatAvailableLocales;
}

const HashSet<String>& JSGlobalObject::intlNumberFormatAvailableLocales()
{
    if (m_intlNumberFormatAvailableLocales.isEmpty()) {
        int32_t count = unum_countAvailable();
        for (int32_t i = 0; i < count; ++i) {
            String locale(unum_getAvailable(i));
            convertICULocaleToBCP47LanguageTag(locale);
            m_intlNumberFormatAvailableLocales.add(locale);
        }
    }
    return m_intlNumberFormatAvailableLocales;
}
#endif // ENABLE(INTL)

void JSGlobalObject::queueMicrotask(Ref<Microtask>&& task)
{
    if (globalObjectMethodTable()->queueTaskToEventLoop) {
        globalObjectMethodTable()->queueTaskToEventLoop(this, WTFMove(task));
        return;
    }

    vm().queueMicrotask(this, WTFMove(task));
}

bool JSGlobalObject::hasDebugger() const
{ 
    return m_debugger;
}

bool JSGlobalObject::hasInteractiveDebugger() const 
{ 
    return m_debugger && m_debugger->isInteractivelyDebugging();
}

JSGlobalObject* JSGlobalObject::create(VM& vm, Structure* structure)
{
    JSGlobalObject* globalObject = new (NotNull, allocateCell<JSGlobalObject>(vm.heap)) JSGlobalObject(vm, structure);
    globalObject->finishCreation(vm);
    return globalObject;
}

void JSGlobalObject::finishCreation(VM& vm)
{
    Base::finishCreation(vm);
    structure()->setGlobalObject(vm, this);
    m_runtimeFlags = m_globalObjectMethodTable->javaScriptRuntimeFlags(this);
    init(vm);
    setGlobalThis(vm, JSProxy::create(vm, JSProxy::createStructure(vm, this, getPrototypeDirect(), PureForwardingProxyType), this));
}

void JSGlobalObject::finishCreation(VM& vm, JSObject* thisValue)
{
    Base::finishCreation(vm);
    structure()->setGlobalObject(vm, this);
    m_runtimeFlags = m_globalObjectMethodTable->javaScriptRuntimeFlags(this);
    init(vm);
    setGlobalThis(vm, thisValue);
}

} // namespace JSC
