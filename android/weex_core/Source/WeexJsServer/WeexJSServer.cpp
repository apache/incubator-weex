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
#include "ICUCompatible.h"
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
#include "StrongInlines.h"
#include "StructureInlines.h"
#include "StructureRareDataInlines.h"
#include "SuperSampler.h"
#include "TestRunnerUtils.h"
#include "TypeProfilerLog.h"
#include "WasmFaultSignalHandler.h"
#include "WasmMemory.h"
#include "WasmPlan.h"
#include <locale.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <type_traits>
#include <wtf/CommaPrinter.h>
#include <wtf/CurrentTime.h>
#include <wtf/MainThread.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/StringPrintStream.h>
#include <wtf/text/StringBuilder.h>
#include <wtf/unicode/WTFUTF8.h>

#include "../base/base64/base64.h"

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
#include <arm/arch.h>
#include <fenv.h>
#endif

#if !defined(PATH_MAX)
#define PATH_MAX 4096
#endif

using namespace JSC;
using namespace WTF;

#include "./base/LogUtils.h"
#include "Buffering/IPCBuffer.h"
#include "CrashHandler.h"
#include "IPCArguments.h"
#include "IPCByteArray.h"
#include "IPCException.h"
#include "IPCFutexPageQueue.h"
#include "IPCHandler.h"
#include "IPCListener.h"
#include "IPCMessageJS.h"
#include "IPCResult.h"
#include "IPCSender.h"
#include "IPCString.h"
#include "IPCType.h"
#include "Serializing/IPCSerializer.h"
#include "Trace.h"
#include "WeexJSServer.h"

#include <dlfcn.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace {

static EncodedJSValue JSC_HOST_CALL
functionGCAndSweep(ExecState
*);
static EncodedJSValue JSC_HOST_CALL
functionCallNative(ExecState
*);
static EncodedJSValue JSC_HOST_CALL
functionCallNativeModule(ExecState
*);
static EncodedJSValue JSC_HOST_CALL
functionCallNativeComponent(ExecState
*);
static EncodedJSValue JSC_HOST_CALL
functionCallAddElement(ExecState
*);
static EncodedJSValue JSC_HOST_CALL
functionSetTimeoutNative(ExecState
*);
static EncodedJSValue JSC_HOST_CALL
functionNativeLog(ExecState
*);
static EncodedJSValue JSC_HOST_CALL
functionNotifyTrimMemory(ExecState
*);
static EncodedJSValue JSC_HOST_CALL
functionMarkupState(ExecState
*);
static EncodedJSValue JSC_HOST_CALL
functionAtob(ExecState
*);
static EncodedJSValue JSC_HOST_CALL
functionBtoa(ExecState
*);
static EncodedJSValue JSC_HOST_CALL
functionCallCreateBody(ExecState
*);
static EncodedJSValue JSC_HOST_CALL
functionCallUpdateFinish(ExecState
*);
static EncodedJSValue JSC_HOST_CALL
functionCallCreateFinish(ExecState
*);
static EncodedJSValue JSC_HOST_CALL
functionCallRefreshFinish(ExecState
*);
static EncodedJSValue JSC_HOST_CALL
functionCallUpdateAttrs(ExecState
*);
static EncodedJSValue JSC_HOST_CALL
functionCallUpdateStyle(ExecState
*);
static EncodedJSValue JSC_HOST_CALL
functionCallRemoveElement(ExecState
*);
static EncodedJSValue JSC_HOST_CALL
functionCallMoveElement(ExecState
*);
static EncodedJSValue JSC_HOST_CALL
functionCallAddEvent(ExecState
*);
static EncodedJSValue JSC_HOST_CALL
functionCallRemoveEvent(ExecState
*);


static bool ExecuteJavaScript(JSGlobalObject *globalObject,
                              const String &source,
                              const String &url,
                              bool report_exceptions);

static void ReportException(JSGlobalObject *globalObject,
                            Exception *exception,
                            const char *jinstanceid,
                            const char *func);

class GlobalObject;

static void setJSFVersion(GlobalObject *globalObject);

static String exceptionToString(JSGlobalObject *globalObject, JSValue exception);

static void markupStateInternally();

static void initHeapTimer() {
  HeapTimer::startTimerThread();
}

static void deinitHeapTimer() {
  HeapTimer::stopTimerThread();
}

class GlobalObject : public JSGlobalObject {
private:
    GlobalObject(VM &, Structure *);

public:
    typedef JSGlobalObject Base;

    friend class WeexJSServer;

    WeexJSServer *m_server{nullptr};

    static GlobalObject *create(VM &vm, Structure *structure) {
      GlobalObject *object = new(NotNull, allocateCell<GlobalObject>(vm.heap)) GlobalObject(vm, structure);
      object->finishCreation(vm);
      return object;
    }

    static const bool needsDestruction = false;

    DECLARE_INFO;
    static const GlobalObjectMethodTable s_globalObjectMethodTable;

    static Structure *createStructure(VM &vm, JSValue prototype) {
      return Structure::create(vm, 0, prototype, TypeInfo(GlobalObjectType, StructureFlags), info());
    }

    static RuntimeFlags javaScriptRuntimeFlags(const JSGlobalObject *) { return RuntimeFlags::createAllEnabled(); }

    void initWXEnvironment(IPCArguments *arguments);

    void initFunction();

protected:
    void finishCreation(VM &vm) {
      Base::finishCreation(vm);
      addFunction(vm, "gc", functionGCAndSweep, 0);
    }

    void addFunction(VM &vm, JSObject *object, const char *name, NativeFunction function, unsigned arguments) {
      Identifier identifier = Identifier::fromString(&vm, name);
      object->putDirect(vm, identifier, JSFunction::create(vm, this, arguments, identifier.string(), function));
    }

    void addFunction(VM &vm, const char *name, NativeFunction function, unsigned arguments) {
      addFunction(vm, this, name, function, arguments);
    }

    void addConstructableFunction(VM &vm, const char *name, NativeFunction function, unsigned arguments) {
      Identifier identifier = Identifier::fromString(&vm, name);
      putDirect(vm, identifier,
                JSFunction::create(vm, this, arguments, identifier.string(), function, NoIntrinsic, function));
    }

    void addString(VM &vm, JSObject *object, const char *name, String &&value) {
      Identifier identifier = Identifier::fromString(&vm, name);
      JSString *jsString = jsNontrivialString(&vm, WTFMove(value));
      object->putDirect(vm, identifier, jsString);
    }

    void addValue(VM &vm, JSObject *object, const char *name, JSValue value) {
      Identifier identifier = Identifier::fromString(&vm, name);
      object->putDirect(vm, identifier, value);
    }

    void addValue(VM &vm, const char *name, JSValue value) {
      addValue(vm, this, name, value);
    }
};

class SimpleObject : public JSNonFinalObject {
public:
    SimpleObject(VM &vm, Structure *structure)
            : Base(vm, structure) {
    }

    typedef JSNonFinalObject Base;
    static const bool needsDestruction = false;

    static SimpleObject *create(VM &vm, JSGlobalObject *globalObject) {
      Structure *structure = createStructure(vm, globalObject, jsNull());
      SimpleObject *simpleObject = new(NotNull,
                                       allocateCell<SimpleObject>(vm.heap, sizeof(SimpleObject))) SimpleObject(vm,
                                                                                                               structure);
      simpleObject->finishCreation(vm);
      return simpleObject;
    }

    static void visitChildren(JSCell *cell, SlotVisitor &visitor) {
      SimpleObject *thisObject = jsCast<SimpleObject *>(cell);
      ASSERT_GC_OBJECT_INHERITS(thisObject, info());
      Base::visitChildren(thisObject, visitor);
    }

    static Structure *createStructure(VM &vm, JSGlobalObject *globalObject, JSValue prototype) {
      return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
    }

    DECLARE_INFO;
};

const ClassInfo SimpleObject::s_info = {"Object", &Base::s_info, nullptr, CREATE_METHOD_TABLE(SimpleObject)};

const ClassInfo GlobalObject::s_info = {"global", &JSGlobalObject::s_info, nullptr,
                                        CREATE_METHOD_TABLE(GlobalObject)};
const GlobalObjectMethodTable GlobalObject::s_globalObjectMethodTable = {
        &supportsRichSourceInfo,
        &shouldInterruptScript,
        &javaScriptRuntimeFlags,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr
};

GlobalObject::GlobalObject(VM &vm, Structure *structure)
        : JSGlobalObject(vm, structure, &s_globalObjectMethodTable) {
}

void GlobalObject::initWXEnvironment(IPCArguments *arguments) {
  size_t count = arguments->getCount();
  VM &vm = this->vm();
  JSNonFinalObject *WXEnvironment = SimpleObject::create(vm, this);

  for (size_t i = 1; i < count; i += 2) {
    if (arguments->getType(i) != IPCType::BYTEARRAY) {
      continue;
    }
    if (arguments->getType(1 + i) != IPCType::BYTEARRAY) {
      continue;
    }
    const IPCByteArray *ba = arguments->getByteArray(1 + i);
    String &&value = String::fromUTF8(ba->content);
    addString(vm, WXEnvironment, arguments->getByteArray(i)->content, WTFMove(value));
  }
  addValue(vm, "WXEnvironment", WXEnvironment);
}

void GlobalObject::initFunction() {
  VM &vm = this->vm();
  const HashTableValue JSEventTargetPrototypeTableValues[] = {
          {"callNative",          JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionCallNative), (intptr_t)(3)}},
          {"callNativeModule",    JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionCallNativeModule), (intptr_t)(5)}},
          {"callNativeComponent", JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionCallNativeComponent), (intptr_t)(5)}},
          {"callAddElement",      JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionCallAddElement), (intptr_t)(5)}},
          {"setTimeoutNative",    JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionSetTimeoutNative), (intptr_t)(2)}},
          {"nativeLog",           JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionNativeLog), (intptr_t)(5)}},
          {"notifyTrimMemory",    JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionNotifyTrimMemory), (intptr_t)(0)}},
          {"markupState",         JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionMarkupState), (intptr_t)(0)}},

          {"atob",                JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionAtob), (intptr_t)(1)}},
          {"btoa",                JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionBtoa), (intptr_t)(1)}},
          {"callCreateBody",      JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionCallCreateBody), (intptr_t)(3)}},
          {"callUpdateFinish",    JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionCallUpdateFinish), (intptr_t)(3)}},
          {"callCreateFinish",    JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionCallCreateFinish), (intptr_t)(3)}},
          {"callRefreshFinish",   JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionCallRefreshFinish), (intptr_t)(3)}},
          {"callUpdateAttrs",     JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionCallUpdateAttrs), (intptr_t)(4)}},
          {"callUpdateStyle",     JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionCallUpdateStyle), (intptr_t)(4)}},
          {"callRemoveElement",   JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionCallRemoveElement), (intptr_t)(3)}},
          {"callMoveElement",     JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionCallMoveElement), (intptr_t)(5)}},
          {"callAddEvent",        JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionCallAddEvent), (intptr_t)(4)}},
          {"callRemoveEvent",     JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionCallRemoveEvent), (intptr_t)(4)}},
  };
  reifyStaticProperties(vm, JSEventTargetPrototypeTableValues, *this);
}

static void addString(IPCSerializer *serializer, const String &s) {
  if (s.is8Bit()) {
    CString data = s.utf8();
    const char *cstr = data.data();
    size_t length = data.length();
    Vector<UChar, 1024> buffer(length);
    UChar *p = buffer.data();
    bool sourceIsAllASCII;

    if (WTF::Unicode::conversionOK ==
        WTF::Unicode::convertUTF8ToUTF16(&cstr, cstr + length, &p, p + length, &sourceIsAllASCII)) {
      serializer->add(buffer.data(), std::distance(buffer.data(), p));
    } else {
      uint16_t tmp = 0;
      serializer->add(&tmp, 0);
    }
  } else {
    serializer->add(s.characters16(), s.length());
  }
}

static void getArgumentAsJString(IPCSerializer *serializer, ExecState *state, int argument) {
  JSValue val = state->argument(argument);
  String s = val.toWTFString(state);
  addString(serializer, s);
}

static void getArgumentAsJByteArrayJSON(IPCSerializer *serializer, ExecState *state, int argument) {
  jbyteArray ba = nullptr;
  if (argument >= state->argumentCount()) {
    serializer->addJSUndefined();
    return;
  }
  JSValue val = state->argument(argument);
  VM &vm = state->vm();
  if (val.isObject()) {
    auto scope = DECLARE_CATCH_SCOPE(vm);
    String str = JSONStringify(state, val, 0);
    JSC::VM &vm = state->vm();
    if (UNLIKELY(scope.exception())) {
      scope.clearException();
      serializer->addJSUndefined();
      return;
    }
    CString data = str.utf8();
    serializer->add(data.data(), data.length());
  }
}

String jString2String(const uint16_t *str, size_t length) {
  UChar *dst;
  String s = String::createUninitialized(length, dst);
  memcpy(dst, str, length * sizeof(UChar));
  return s;
}

static JSValue jString2JSValue(ExecState * state, const
uint16_t *str, size_t
length) {
String s = jString2String(str, length);
if (s.

isEmpty()

) {
return
jsEmptyString(&state
->

vm()

);
}
else if (s.

length()

== 1) {
return
jsSingleCharacterString(&state
->

vm(), s[0]

);
}
return
jsNontrivialString(&state
->

vm(), WTFMove(s)

);
}

static JSValue parseToObject(ExecState * state, const
String &data
)
{
VM &vm = state->vm();
auto scope = DECLARE_CATCH_SCOPE(vm);
JSValue ret = JSONParse(state, data);
if (
UNLIKELY(scope
.

exception()

)) {
scope.

clearException();

return

jsUndefined();

}
if (!ret)
return

jsUndefined();

return
ret;
}

static void getArgumentAsJByteArray(IPCSerializer *serializer, ExecState *state, int argument) {
  jbyteArray ba = nullptr;
  if (argument >= state->argumentCount()) {
    serializer->addJSUndefined();
    return;
  }
  JSValue val = state->argument(argument);
  if (val.isString()) {
    String str(val.toWTFString(state));
    CString data = str.utf8();
    serializer->add(data.data(), data.length());
  } else {
    getArgumentAsJByteArrayJSON(serializer, state, argument);
  }
}

EncodedJSValue JSC_HOST_CALL
functionGCAndSweep(ExecState
* exec)
{
JSLockHolder lock(exec);
// exec->heap()->collectAllGarbage();
return
JSValue::encode(jsNumber(exec->heap()->sizeAfterLastFullCollection()));
}

EncodedJSValue JSC_HOST_CALL
functionCallNative(ExecState
* state)
{
base::debug::TraceScope traceScope("weex", "callNative");

GlobalObject *globalObject = static_cast<GlobalObject *>(state->lexicalGlobalObject());
WeexJSServer *server = globalObject->m_server;
IPCSender *sender = server->getSender();
IPCSerializer *serializer = server->getSerializer();
serializer->setMsg(static_cast
<uint32_t>(IPCProxyMsg::CALLNATIVE)
);
//instacneID args[0]
getArgumentAsJString(serializer, state,
0);
//task args[1]
getArgumentAsJByteArray(serializer, state,
1);
//callback args[2]
getArgumentAsJString(serializer, state,
2);
std::unique_ptr <IPCBuffer> buffer = serializer->finish();
std::unique_ptr <IPCResult> result = sender->send(buffer.get());
if (result->

getType()

!= IPCType::INT32) {
LOGE("functionCallNative: unexpected result: %d", result->

getType()

);
return
JSValue::encode(jsNumber(0));
}

return
JSValue::encode(jsNumber(result->get<int32_t>()));
}

EncodedJSValue JSC_HOST_CALL
functionCallNativeModule(ExecState
* state)
{
base::debug::TraceScope traceScope("weex", "callNativeModule");
VM &vm = state->vm();

GlobalObject *globalObject = static_cast<GlobalObject *>(state->lexicalGlobalObject());
WeexJSServer *server = globalObject->m_server;
IPCSender *sender = server->getSender();
IPCSerializer *serializer = server->getSerializer();
serializer->setMsg(static_cast
<uint32_t>(IPCProxyMsg::CALLNATIVEMODULE)
);
//instacneID args[0]
getArgumentAsJString(serializer, state,
0);

//module args[1]
getArgumentAsJString(serializer, state,
1);

//method args[2]
getArgumentAsJString(serializer, state,
2);

// arguments args[3]
getArgumentAsJByteArrayJSON(serializer, state,
3);
//arguments args[4]
getArgumentAsJByteArrayJSON(serializer, state,
4);
std::unique_ptr <IPCBuffer> buffer = serializer->finish();
std::unique_ptr <IPCResult> result = sender->send(buffer.get());
JSValue ret;
switch (result->

getType()

) {
case
IPCType::DOUBLE:
        ret = jsNumber(result->get<double>());
break;
case
IPCType::STRING:
        ret = jString2JSValue(state, result->getStringContent(), result->getStringLength());
break;
case

IPCType::JSONSTRING: {
  String val = jString2String(result->getStringContent(), result->getStringLength());
  ret = parseToObject(state, val);
}

break;
default:
ret = jsUndefined();
}
return
JSValue::encode(ret);
}

EncodedJSValue JSC_HOST_CALL
functionCallNativeComponent(ExecState
* state)
{
base::debug::TraceScope traceScope("weex", "callNativeComponent");

GlobalObject *globalObject = static_cast<GlobalObject *>(state->lexicalGlobalObject());
WeexJSServer *server = globalObject->m_server;
IPCSender *sender = server->getSender();
IPCSerializer *serializer = server->getSerializer();
serializer->setMsg(static_cast
<uint32_t>(IPCProxyMsg::CALLNATIVECOMPONENT)
);

//instacneID args[0]
getArgumentAsJString(serializer, state,
0);

//module args[1]
getArgumentAsJString(serializer, state,
1);

//method args[2]
getArgumentAsJString(serializer, state,
2);

// arguments args[3]
getArgumentAsJByteArrayJSON(serializer, state,
3);

//arguments args[4]
getArgumentAsJByteArrayJSON(serializer, state,
4);
std::unique_ptr <IPCBuffer> buffer = serializer->finish();
std::unique_ptr <IPCResult> result = sender->send(buffer.get());
if (result->

getType()

!= IPCType::INT32) {
LOGE("functionCallNativeComponent: unexpected result: %d", result->

getType()

);
return
JSValue::encode(jsNumber(0));
}

return
JSValue::encode(jsNumber(result->get<int32_t>()));
}

EncodedJSValue JSC_HOST_CALL
functionCallAddElement(ExecState
* state)
{
base::debug::TraceScope traceScope("weex", "callAddElement");
GlobalObject *globalObject = static_cast<GlobalObject *>(state->lexicalGlobalObject());
WeexJSServer *server = globalObject->m_server;
IPCSender *sender = server->getSender();
IPCSerializer *serializer = server->getSerializer();
serializer->setMsg(static_cast
<uint32_t>(IPCProxyMsg::CALLADDELEMENT)
| MSG_FLAG_ASYNC);
//instacneID args[0]
getArgumentAsJString(serializer, state,
0);
//instacneID args[1]
getArgumentAsJString(serializer, state,
1);
//dom node args[2]
getArgumentAsJByteArray(serializer, state,
2);
//index  args[3]
getArgumentAsJString(serializer, state,
3);
//callback  args[4]
getArgumentAsJString(serializer, state,
4);
std::unique_ptr <IPCBuffer> buffer = serializer->finish();
std::unique_ptr <IPCResult> result = sender->send(buffer.get());

return
JSValue::encode(jsNumber(0));
}

EncodedJSValue JSC_HOST_CALL
functionCallCreateBody(ExecState
* state)
{
base::debug::TraceScope traceScope("weex", "callCreateBody");
GlobalObject *globalObject = static_cast<GlobalObject *>(state->lexicalGlobalObject());
WeexJSServer *server = globalObject->m_server;
IPCSender *sender = server->getSender();
IPCSerializer *serializer = server->getSerializer();
serializer->setMsg(static_cast
<uint32_t>(IPCProxyMsg::CALLCREATEBODY)
);
//instacneID args[0]
getArgumentAsJString(serializer, state,
0);
//task args[1]
getArgumentAsJByteArray(serializer, state,
1);
//callback args[2]
getArgumentAsJString(serializer, state,
2);
std::unique_ptr <IPCBuffer> buffer = serializer->finish();
std::unique_ptr <IPCResult> result = sender->send(buffer.get());
if (result->

getType()

!= IPCType::INT32) {
LOGE("functionCallNative: unexpected result: %d", result->

getType()

);
return
JSValue::encode(jsNumber(0));
}
return
JSValue::encode(jsNumber(result->get<int32_t>()));
}

EncodedJSValue JSC_HOST_CALL
functionCallUpdateFinish(ExecState
* state)
{
base::debug::TraceScope traceScope("weex", "functionCallUpdateFinish");
GlobalObject *globalObject = static_cast<GlobalObject *>(state->lexicalGlobalObject());
WeexJSServer *server = globalObject->m_server;
IPCSender *sender = server->getSender();
IPCSerializer *serializer = server->getSerializer();
serializer->setMsg(static_cast
<uint32_t>(IPCProxyMsg::CALLUPDATEFINISH)
);
//instacneID args[0]
getArgumentAsJString(serializer, state,
0);
//task args[1]
getArgumentAsJByteArray(serializer, state,
1);
//callback args[2]
getArgumentAsJString(serializer, state,
2);
std::unique_ptr <IPCBuffer> buffer = serializer->finish();
std::unique_ptr <IPCResult> result = sender->send(buffer.get());
if (result->

getType()

!= IPCType::INT32) {
LOGE("functionCallUpdateFinish: unexpected result: %d", result->

getType()

);
return
JSValue::encode(jsNumber(0));
}
return
JSValue::encode(jsNumber(result->get<int32_t>()));
}

EncodedJSValue JSC_HOST_CALL
functionCallCreateFinish(ExecState
* state)
{
base::debug::TraceScope traceScope("weex", "functionCallCreateFinish");
GlobalObject *globalObject = static_cast<GlobalObject *>(state->lexicalGlobalObject());
WeexJSServer *server = globalObject->m_server;
IPCSender *sender = server->getSender();
IPCSerializer *serializer = server->getSerializer();
serializer->setMsg(static_cast
<uint32_t>(IPCProxyMsg::CALLCREATEFINISH)
);
//instacneID args[0]
getArgumentAsJString(serializer, state,
0);
//task args[1]
getArgumentAsJByteArray(serializer, state,
1);
//callback args[2]
getArgumentAsJString(serializer, state,
2);
std::unique_ptr <IPCBuffer> buffer = serializer->finish();
std::unique_ptr <IPCResult> result = sender->send(buffer.get());
if (result->

getType()

!= IPCType::INT32) {
LOGE("functionCallCreateFinish: unexpected result: %d", result->

getType()

);
return
JSValue::encode(jsNumber(0));
}
return
JSValue::encode(jsNumber(result->get<int32_t>()));
}

EncodedJSValue JSC_HOST_CALL
functionCallRefreshFinish(ExecState
* state)
{
base::debug::TraceScope traceScope("weex", "functionCallRefreshFinish");
GlobalObject *globalObject = static_cast<GlobalObject *>(state->lexicalGlobalObject());
WeexJSServer *server = globalObject->m_server;
IPCSender *sender = server->getSender();
IPCSerializer *serializer = server->getSerializer();
serializer->setMsg(static_cast
<uint32_t>(IPCProxyMsg::CALLREFRESHFINISH)
);
//instacneID args[0]
getArgumentAsJString(serializer, state,
0);
//task args[1]
getArgumentAsJByteArray(serializer, state,
1);
//callback args[2]
getArgumentAsJString(serializer, state,
2);
std::unique_ptr <IPCBuffer> buffer = serializer->finish();
std::unique_ptr <IPCResult> result = sender->send(buffer.get());
if (result->

getType()

!= IPCType::INT32) {
LOGE("functionCallRefreshFinish: unexpected result: %d", result->

getType()

);
return
JSValue::encode(jsNumber(0));
}
return
JSValue::encode(jsNumber(result->get<int32_t>()));
}


EncodedJSValue JSC_HOST_CALL
functionCallUpdateAttrs(ExecState
* state)
{
base::debug::TraceScope traceScope("weex", "functionCallUpdateAttrs");
GlobalObject *globalObject = static_cast<GlobalObject *>(state->lexicalGlobalObject());
WeexJSServer *server = globalObject->m_server;
IPCSender *sender = server->getSender();
IPCSerializer *serializer = server->getSerializer();
serializer->setMsg(static_cast
<uint32_t>(IPCProxyMsg::CALLUPDATEATTRS)
);
//instacneID args[0]
getArgumentAsJString(serializer, state,
0);
//instacneID args[1]
getArgumentAsJString(serializer, state,
1);
//task args[2]
getArgumentAsJByteArray(serializer, state,
2);
//callback args[3]
getArgumentAsJString(serializer, state,
3);
std::unique_ptr <IPCBuffer> buffer = serializer->finish();
std::unique_ptr <IPCResult> result = sender->send(buffer.get());
if (result->

getType()

!= IPCType::INT32) {
LOGE("functionCallUpdateAttrs: unexpected result: %d", result->

getType()

);
return
JSValue::encode(jsNumber(0));
}
return
JSValue::encode(jsNumber(result->get<int32_t>()));
}

EncodedJSValue JSC_HOST_CALL
functionCallUpdateStyle(ExecState
* state)
{
base::debug::TraceScope traceScope("weex", "functionCallUpdateStyle");
GlobalObject *globalObject = static_cast<GlobalObject *>(state->lexicalGlobalObject());
WeexJSServer *server = globalObject->m_server;
IPCSender *sender = server->getSender();
IPCSerializer *serializer = server->getSerializer();
serializer->setMsg(static_cast
<uint32_t>(IPCProxyMsg::CALLUPDATESTYLE)
);
//instacneID args[0]
getArgumentAsJString(serializer, state,
0);
//instacneID args[1]
getArgumentAsJString(serializer, state,
1);
//task args[2]
getArgumentAsJByteArray(serializer, state,
2);
//callback args[3]
getArgumentAsJString(serializer, state,
3);
std::unique_ptr <IPCBuffer> buffer = serializer->finish();
std::unique_ptr <IPCResult> result = sender->send(buffer.get());
if (result->

getType()

!= IPCType::INT32) {
LOGE("functionCallUpdateStyle: unexpected result: %d", result->

getType()

);
return
JSValue::encode(jsNumber(0));
}
return
JSValue::encode(jsNumber(result->get<int32_t>()));
}

EncodedJSValue JSC_HOST_CALL
functionCallRemoveElement(ExecState
* state)
{
base::debug::TraceScope traceScope("weex", "functionCallRemoveElement");
GlobalObject *globalObject = static_cast<GlobalObject *>(state->lexicalGlobalObject());
WeexJSServer *server = globalObject->m_server;
IPCSender *sender = server->getSender();
IPCSerializer *serializer = server->getSerializer();
serializer->setMsg(static_cast
<uint32_t>(IPCProxyMsg::CALLREMOVEELEMENT)
);
//instacneID args[0]
getArgumentAsJString(serializer, state,
0);
//instacneID args[1]
getArgumentAsJString(serializer, state,
1);
//callback args[2]
getArgumentAsJString(serializer, state,
2);
std::unique_ptr <IPCBuffer> buffer = serializer->finish();
std::unique_ptr <IPCResult> result = sender->send(buffer.get());
if (result->

getType()

!= IPCType::INT32) {
LOGE("functionCallRemoveElement: unexpected result: %d", result->

getType()

);
return
JSValue::encode(jsNumber(0));
}
return
JSValue::encode(jsNumber(result->get<int32_t>()));
}

EncodedJSValue JSC_HOST_CALL
functionCallMoveElement(ExecState
* state)
{
base::debug::TraceScope traceScope("weex", "functionCallMoveElement");
GlobalObject *globalObject = static_cast<GlobalObject *>(state->lexicalGlobalObject());
WeexJSServer *server = globalObject->m_server;
IPCSender *sender = server->getSender();
IPCSerializer *serializer = server->getSerializer();
serializer->setMsg(static_cast
<uint32_t>(IPCProxyMsg::CALLMOVEELEMENT)
);
//instacneID args[0]
getArgumentAsJString(serializer, state,
0);
//instacneID args[1]
getArgumentAsJString(serializer, state,
1);
//callback args[2]
getArgumentAsJString(serializer, state,
2);

//callback args[3]
getArgumentAsJString(serializer, state,
3);

//callback args[4]
getArgumentAsJString(serializer, state,
4);

std::unique_ptr <IPCBuffer> buffer = serializer->finish();
std::unique_ptr <IPCResult> result = sender->send(buffer.get());
if (result->

getType()

!= IPCType::INT32) {
LOGE("functionCallMoveElement: unexpected result: %d", result->

getType()

);
return
JSValue::encode(jsNumber(0));
}
return
JSValue::encode(jsNumber(result->get<int32_t>()));
}

EncodedJSValue JSC_HOST_CALL
functionCallAddEvent(ExecState
* state)
{
base::debug::TraceScope traceScope("weex", "functionCallAddEvent");
GlobalObject *globalObject = static_cast<GlobalObject *>(state->lexicalGlobalObject());
WeexJSServer *server = globalObject->m_server;
IPCSender *sender = server->getSender();
IPCSerializer *serializer = server->getSerializer();
serializer->setMsg(static_cast
<uint32_t>(IPCProxyMsg::CALLADDEVENT)
);
//instacneID args[0]
getArgumentAsJString(serializer, state,
0);
//instacneID args[1]
getArgumentAsJString(serializer, state,
1);
//callback args[2]
getArgumentAsJString(serializer, state,
2);
//callback args[3]
getArgumentAsJString(serializer, state,
3);

std::unique_ptr <IPCBuffer> buffer = serializer->finish();
std::unique_ptr <IPCResult> result = sender->send(buffer.get());
if (result->

getType()

!= IPCType::INT32) {
LOGE("functionCallAddEvent: unexpected result: %d", result->

getType()

);
return
JSValue::encode(jsNumber(0));
}
return
JSValue::encode(jsNumber(result->get<int32_t>()));
}

EncodedJSValue JSC_HOST_CALL
functionCallRemoveEvent(ExecState
* state)
{
base::debug::TraceScope traceScope("weex", "functionCallRemoveEvent");
GlobalObject *globalObject = static_cast<GlobalObject *>(state->lexicalGlobalObject());
WeexJSServer *server = globalObject->m_server;
IPCSender *sender = server->getSender();
IPCSerializer *serializer = server->getSerializer();
serializer->setMsg(static_cast
<uint32_t>(IPCProxyMsg::CALLREMOVEEVENT)
);
//instacneID args[0]
getArgumentAsJString(serializer, state,
0);
//instacneID args[1]
getArgumentAsJString(serializer, state,
1);
//callback args[2]
getArgumentAsJString(serializer, state,
2);
//callback args[3]
getArgumentAsJString(serializer, state,
3);

std::unique_ptr <IPCBuffer> buffer = serializer->finish();
std::unique_ptr <IPCResult> result = sender->send(buffer.get());
if (result->

getType()

!= IPCType::INT32) {
LOGE("functionCallRemoveEvent: unexpected result: %d", result->

getType()

);
return
JSValue::encode(jsNumber(0));
}
return
JSValue::encode(jsNumber(result->get<int32_t>()));
}

EncodedJSValue JSC_HOST_CALL
functionSetTimeoutNative(ExecState
* state)
{
base::debug::TraceScope traceScope("weex", "setTimeoutNative");
GlobalObject *globalObject = static_cast<GlobalObject *>(state->lexicalGlobalObject());
WeexJSServer *server = globalObject->m_server;
IPCSender *sender = server->getSender();
IPCSerializer *serializer = server->getSerializer();
serializer->setMsg(static_cast
<uint32_t>(IPCProxyMsg::SETTIMEOUT)
);
//callbackId
getArgumentAsJString(serializer, state,
0);

//time
getArgumentAsJString(serializer, state,
1);

std::unique_ptr <IPCBuffer> buffer = serializer->finish();
std::unique_ptr <IPCResult> result = sender->send(buffer.get());
if (result->

getType()

!= IPCType::INT32) {
LOGE("functionCallNativeComponent: unexpected result: %d", result->

getType()

);
return
JSValue::encode(jsNumber(0));
}

return
JSValue::encode(jsNumber(result->get<int32_t>()));
}

EncodedJSValue JSC_HOST_CALL
functionNativeLog(ExecState
* state)
{
bool result = false;

StringBuilder sb;
for (
int i = 0;
i<state->

argumentCount();

i++) {
sb.
append(state
->
argument(i)
.
toWTFString(state)
);
}

if (!sb.

isEmpty()

) {
GlobalObject *globalObject = static_cast<GlobalObject *>(state->lexicalGlobalObject());
WeexJSServer *server = globalObject->m_server;
IPCSender *sender = server->getSender();
IPCSerializer *serializer = server->getSerializer();

serializer->setMsg(static_cast
<uint32_t>(IPCProxyMsg::NATIVELOG)
| MSG_FLAG_ASYNC);
String s = sb.toString();
addString(serializer, s
);
std::unique_ptr <IPCBuffer> buffer = serializer->finish();
sender->
send(buffer
.

get()

);
}
return
JSValue::encode(jsBoolean(true));
}

EncodedJSValue JSC_HOST_CALL
functionNotifyTrimMemory(ExecState
* state)
{
return
functionGCAndSweep(state);
}

EncodedJSValue JSC_HOST_CALL
functionMarkupState(ExecState
* state)
{
markupStateInternally();

return

JSValue::encode (jsUndefined());

}

EncodedJSValue JSC_HOST_CALL
functionAtob(ExecState
*state)
{
LOGD("[ExtendJSApi] functionAtob");
base::debug::TraceScope traceScope("weex", "atob");

JSValue ret = jsUndefined();
JSValue val = state->argument(0);

if (!val.

isUndefined()

)
{
String original = val.toWTFString(state);

std::string input_str(original.utf8().data());

//LOGD("btoa input: %s", input_str.c_str());

std::string output_str;

Base64Decode(input_str, &output_str
);
//LOGD("btoa output: %s", output_str.c_str());

WTF::String s(output_str.c_str());
ret = jsNontrivialString(&state->vm(), WTFMove(s));
}
else
{
//ret = "";
}
LOGD("[ExtendJSApi] functionAtob complete");

return
JSValue::encode(ret);
}

EncodedJSValue JSC_HOST_CALL
functionBtoa(ExecState
*state)
{
LOGD("[ExtendJSApi] functionBtoa");
base::debug::TraceScope traceScope("weex", "btoa");

JSValue ret = jsUndefined();
JSValue val = state->argument(0);

//if (!val.isUndefined()) {
String original = val.toWTFString(state);

std::string input_str(original.utf8().data());

LOGD("btoa input: %s", input_str.

c_str()

);

std::string output_str;

Base64Encode(input_str, &output_str
);
LOGD("btoa output: %s", output_str.

c_str()

);

//JSString* asString(JSValue value)

WTF::String s(output_str.c_str());
ret = jsNontrivialString(&state->vm(), WTFMove(s));
//}else{
//ret = "";
//}

LOGD("[ExtendJSApi] functionBtoa complete");

return
JSValue::encode(ret);
}

/**
 * this function is to execute a section of JavaScript content.
 */
bool ExecuteJavaScript(JSGlobalObject *globalObject,
                       const String &source,
                       const String &url,
                       bool report_exceptions) {
  SourceOrigin sourceOrigin(String::fromUTF8("(weex)"));
  NakedPtr <Exception> evaluationException;
  JSValue returnValue = evaluate(globalObject->globalExec(), makeSource(source, sourceOrigin, url), JSValue(),
                                 evaluationException);
  if (report_exceptions && evaluationException) {
    ReportException(globalObject, evaluationException.get(), "", "");
  }
  if (evaluationException)
    return false;
  globalObject->vm().drainMicrotasks();
  return true;
}

void ReportException(JSGlobalObject *_globalObject, Exception *exception, const char *instanceid,
                     const char *func) {
  String exceptionInfo = exceptionToString(_globalObject, exception->value());
  CString data = exceptionInfo.utf8();
  GlobalObject *globalObject = static_cast<GlobalObject *>(_globalObject);
  WeexJSServer *server = globalObject->m_server;
  IPCSender *sender = server->getSender();
  IPCSerializer *serializer = server->getSerializer();
  serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::REPORTEXCEPTION));
  serializer->add(instanceid, strlen(instanceid));
  serializer->add(func, strlen(func));
  serializer->add(data.data(), data.length());

  std::unique_ptr <IPCBuffer> buffer = serializer->finish();
  std::unique_ptr <IPCResult> result = sender->send(buffer.get());
  if (result->getType() != IPCType::VOID) {
    LOGE("REPORTEXCEPTION: unexpected result: %d", result->getType());
  }
}

void setJSFVersion(GlobalObject *globalObject) {
  VM &vm = globalObject->vm();
  PropertyName
  getJSFMVersionProperty(Identifier::fromString(&vm, "getJSFMVersion"));
  ExecState * state = globalObject->globalExec();
  JSValue getJSFMVersionFunction = globalObject->get(state, getJSFMVersionProperty);
  MarkedArgumentBuffer args;
  CallData callData;
  CallType callType = getCallData(getJSFMVersionFunction, callData);
  NakedPtr <Exception> returnedException;
  JSValue version = call(globalObject->globalExec(), getJSFMVersionFunction, callType, callData, globalObject, args,
                         returnedException);
  if (returnedException) {
    ReportException(globalObject, returnedException.get(), "", "");
  }
  String str = version.toWTFString(state);
  CString data = str.utf8();

  WeexJSServer *server = globalObject->m_server;
  IPCSender *sender = server->getSender();
  IPCSerializer *serializer = server->getSerializer();
  serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::SETJSFVERSION));
  serializer->add(data.data(), data.length());
  std::unique_ptr <IPCBuffer> buffer = serializer->finish();
  std::unique_ptr <IPCResult> result = sender->send(buffer.get());
  if (result->getType() != IPCType::VOID) {
    LOGE("setJSFVersion: unexpected result: %d", result->getType());
  }
}

static String exceptionToString(JSGlobalObject *globalObject, JSValue exception) {
  StringBuilder sb;
  VM &vm = globalObject->vm();
  auto scope = DECLARE_CATCH_SCOPE(vm);

#define CHECK_EXCEPTION()           \
    do {                            \
        if (scope.exception()) {    \
            scope.clearException(); \
            return String();        \
        }                           \
    } while (false)

  sb.append(String::format("Exception: %s\n", exception.toWTFString(globalObject->globalExec()).utf8().data()));

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
    sb.append(String::format(
            "at %s:%s\n",
            fileNameValue.toWTFString(globalObject->globalExec()).utf8().data(),
            lineNumberValue.toWTFString(globalObject->globalExec()).utf8().data()));
  }

  if (!stackValue.isUndefinedOrNull())
    sb.append(String::format("%s\n", stackValue.toWTFString(globalObject->globalExec()).utf8().data()));

#undef CHECK_EXCEPTION
  return sb.toString();
}

static void markupStateInternally() {
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
        : m_fd(fd) {
}

unique_fd::~unique_fd() {
  close(m_fd);
}

int unique_fd::get() const {
  return m_fd;
}

#define FAIL_WITH_STRERROR(tag)             \
    LOGE(" fails: %s.\n", strerror(errno)); \
    return false;

#define MAYBE_FAIL_WITH_ICU_ERROR(s)            \
    if (status != U_ZERO_ERROR) {               \
        LOGE("Couldn't initialize ICU ("        \
             "): %s (%s)"                       \
             "\n",                              \
            u_errorName(status), path.c_str()); \
        return false;                           \
    }

extern "C" {
void udata_setCommonData(const void *data, UErrorCode *pErrorCode);
}

static bool mapIcuData(const std::string &path) {
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
  void *data = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd.get(), 0);
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

static bool initICUEnv() {
  const char *path = getenv("ICU_DATA_PATH");
  if (!path) {
    return false;
  }
  if (!dlopen("libicuuc.so", RTLD_NOW)) {
    LOGE("load icuuc so");
    return false;
  }
  if (!dlopen("libicui18n.so", RTLD_NOW)) {
    LOGE("load icui18n so");
    return false;
  }
  if (!initICU()) {
    LOGE("initICU failed");
    return false;
  }
  return mapIcuData(std::string(path));
}

static void initCrashHandler() {
  const char *path = getenv("CRASH_FILE_PATH");
  if (path) {
    crash_handler::initializeCrashHandler(path);
  }
}

}

struct WeexJSServer::WeexJSServerImpl {
    WeexJSServerImpl(int fd, bool enableTrace);

    bool enableTrace;
    RefPtr <VM> globalVM;
    Strong <JSGlobalObject> globalObject;
    std::unique_ptr <IPCFutexPageQueue> futexPageQueue;
    std::unique_ptr <IPCSender> sender;
    std::unique_ptr <IPCHandler> handler;
    std::unique_ptr <IPCListener> listener;
    std::unique_ptr <IPCSerializer> serializer;
};

WeexJSServer::WeexJSServerImpl::WeexJSServerImpl(int _fd, bool _enableTrace)
        : enableTrace(_enableTrace) {
  void *base = mmap(nullptr, IPCFutexPageQueue::ipc_size, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, 0);
  if (base == MAP_FAILED) {
    int _errno = errno;
    close(_fd);
    throw IPCException("failed to map ashmem region: %s", strerror(_errno));
  }
  close(_fd);
  futexPageQueue.reset(new IPCFutexPageQueue(base, IPCFutexPageQueue::ipc_size, 1));
  handler = std::move(createIPCHandler());
  sender = std::move(createIPCSender(futexPageQueue.get(), handler.get()));
  listener = std::move(createIPCListener(futexPageQueue.get(), handler.get()));
  serializer = std::move(createIPCSerializer());
}

WeexJSServer::WeexJSServer(int fd, bool enableTrace)
        : m_impl(new WeexJSServerImpl(fd, enableTrace)) {
  IPCHandler *handler = m_impl->handler.get();
  handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::INITFRAMEWORK), [this](IPCArguments *arguments) {

      base::debug::TraceEvent::StartATrace(nullptr);

      base::debug::TraceScope traceScope("weex", "initFramework");
      if (!initICUEnv()) {
        LOGE("failed to init ICUEnv");
        return createInt32Result(static_cast<int32_t>(false));
      }
      initCrashHandler();
      Options::enableRestrictedOptions(true);

      // Initialize JSC before getting VM.
      WTF::initializeMainThread();
      initHeapTimer();
      JSC::initializeThreading();
#if ENABLE(WEBASSEMBLY)
      JSC::Wasm::enableFastMemory();
#endif

      m_impl->globalVM = std::move(VM::create(LargeHeap));
      VM &vm = *m_impl->globalVM.get();
      JSLockHolder locker(&vm);

      int result;

      GlobalObject *globalObject = GlobalObject::create(vm, GlobalObject::createStructure(vm, jsNull()));
      globalObject->m_server = this;
      m_impl->globalObject.set(vm, globalObject);
      globalObject->initWXEnvironment(arguments);
      globalObject->initFunction();
      const IPCString *ipcSource = arguments->getString(0);
      String source = jString2String(ipcSource->content, ipcSource->length);
      if (!ExecuteJavaScript(globalObject, source, "(weex framework)", true)) {
        return createInt32Result(static_cast<int32_t>(false));
      }

      setJSFVersion(globalObject);
      return createInt32Result(static_cast<int32_t>(true));
  });

  handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::EXECJSSERVICE), [this](IPCArguments *arguments) {
      base::debug::TraceScope traceScope("weex", "exeJSService");
      JSGlobalObject *globalObject = m_impl->globalObject.get();
      const IPCString *ipcSource = arguments->getString(0);
      String source = jString2String(ipcSource->content, ipcSource->length);
      VM &vm = globalObject->vm();
      JSLockHolder locker(&vm);
      if (!ExecuteJavaScript(globalObject, source, ("weex service"), true)) {
        LOGE("jsLog JNI_Error >>> scriptStr :%s", source.utf8().data());
        return createInt32Result(static_cast<int32_t>(false));
      }
      return createInt32Result(static_cast<int32_t>(true));

  });

  handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::TAKEHEAPSNAPSHOT), [this](IPCArguments *arguments) {
      return createVoidResult();

  });

  handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::EXECJS), [this](IPCArguments *arguments) {
      JSGlobalObject *globalObject = m_impl->globalObject.get();
      VM &vm = globalObject->vm();
      JSLockHolder locker(&vm);
      const IPCString *ipcInstanceId = arguments->getString(0);
      const IPCString *ipcNamespaceStr = arguments->getString(1);
      const IPCString *ipcFunc = arguments->getString(2);
      String instanceId = jString2String(ipcInstanceId->content, ipcInstanceId->length);
      String namespaceStr = jString2String(ipcNamespaceStr->content, ipcNamespaceStr->length);
      String func = jString2String(ipcFunc->content, ipcFunc->length);
      MarkedArgumentBuffer obj;
      base::debug::TraceScope traceScope("weex", "exeJS", "function", func.utf8().data());
      ExecState * state = globalObject->globalExec();
      size_t count = arguments->getCount();
      for (size_t i = 3; i < count; ++i) {
        switch (arguments->getType(i)) {
          case IPCType::DOUBLE:
            obj.append(jsNumber(arguments->get<double>(i)));
            break;
          case IPCType::STRING: {
            const IPCString *ipcstr = arguments->getString(i);
            obj.append(jString2JSValue(state, ipcstr->content, ipcstr->length));
          }
            break;
          case IPCType::JSONSTRING: {
            const IPCString *ipcstr = arguments->getString(i);

            String str = jString2String(ipcstr->content, ipcstr->length);

            JSValue o = parseToObject(state, str);
            obj.append(o);
          }
            break;
          default:
            obj.append(jsUndefined());
            break;
        }
      }

      Identifier funcIdentifier = Identifier::fromString(&vm, func);

      JSValue function;
      JSValue result;
      if (namespaceStr.isEmpty()) {
        function = globalObject->get(state, funcIdentifier);
      } else {
        Identifier namespaceIdentifier = Identifier::fromString(&vm, namespaceStr);
        JSValue master = globalObject->get(state, namespaceIdentifier);
        if (!master.isObject()) {
          return createInt32Result(static_cast<int32_t>(false));
        }
        function = master.toObject(state)->get(state, funcIdentifier);
      }
      CallData callData;
      CallType callType = getCallData(function, callData);
      NakedPtr <Exception> returnedException;
      JSValue ret = call(state, function, callType, callData, globalObject, obj, returnedException);

      globalObject->vm().drainMicrotasks();

      if (returnedException) {
        ReportException(globalObject, returnedException.get(), instanceId.utf8().data(), func.utf8().data());
        return createInt32Result(static_cast<int32_t>(false));
      }
      return createInt32Result(static_cast<int32_t>(true));

  });
}

WeexJSServer::~WeexJSServer() {
}

void WeexJSServer::loop() {
  m_impl->listener->listen();
}

IPCSender *WeexJSServer::getSender() {
  return m_impl->sender.get();
}

IPCSerializer *WeexJSServer::getSerializer() {
  return m_impl->serializer.get();
}
