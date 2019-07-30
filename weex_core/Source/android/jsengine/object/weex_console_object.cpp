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
//
// Created by Darin on 03/08/2018.
//
#include "android/jsengine/object/weex_console_object.h"

#include "android/jsengine/weex_jsc_utils.h"
#include "base/utils/log_utils.h"

//#include "inspector/ScriptCallStackFactory.h"
//#include "inspector/ScriptArguments.h"
//#include "ConsoleClient.h"
//#include "JSObject.h"

STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(WeexConsoleObject);
const ClassInfo WeexConsoleObject::s_info = {"Console", &Base::s_info, 0, CREATE_METHOD_TABLE(WeexConsoleObject)};

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncDebug(ExecState *);

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncError(ExecState *);

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncLog(ExecState *);

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncInfo(ExecState *);

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncWarn(ExecState *);

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncClear(ExecState *);

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncDir(ExecState *);

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncDirXML(ExecState *);

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncTable(ExecState *);

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncTrace(ExecState *);

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncAssert(ExecState *);

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncCount(ExecState *);

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncProfile(ExecState *);

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncProfileEnd(ExecState *);

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncTakeHeapSnapshot(ExecState *);

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncTime(ExecState *);

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncTimeEnd(ExecState *);

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncTimeStamp(ExecState *);

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncGroup(ExecState *);

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncGroupCollapsed(ExecState *);

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncGroupEnd(ExecState *);


void addFunction(VM &vm, JSGlobalObject *object, const char *name, NativeFunction function, unsigned arguments) {
    Identifier identifier = Identifier::fromString(&vm, name);
    object->putDirect(vm, identifier, JSFunction::create(vm, object, arguments, identifier.string(), function));
}

WeexConsoleObject::WeexConsoleObject(VM &vm, Structure *structure) : JSNonFinalObject(vm, structure) {

}

void WeexConsoleObject::finishCreation(VM &vm, JSGlobalObject *globalObject) {
    Base::finishCreation(vm);

    ASSERT(inherits(vm, info()));

    // For legacy reasons, console properties are enumerable, writable, deleteable,
    // and all have a length of 0. This may change if Console is standardized.

    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("debug", consoleProtoFuncDebug, None, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("error", consoleProtoFuncError, None, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("log", consoleProtoFuncLog, None, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("info", consoleProtoFuncInfo, None, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("warn", consoleProtoFuncWarn, None, 0);

    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->clear, consoleProtoFuncClear, None, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("dir", consoleProtoFuncDir, None, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("dirxml", consoleProtoFuncDirXML, None, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("table", consoleProtoFuncTable, None, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("trace", consoleProtoFuncTrace, None, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("assert", consoleProtoFuncAssert, None, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->count, consoleProtoFuncCount, None, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("profile", consoleProtoFuncProfile, None, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("profileEnd", consoleProtoFuncProfileEnd, None, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("time", consoleProtoFuncTime, None, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("timeEnd", consoleProtoFuncTimeEnd, None, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("timeStamp", consoleProtoFuncTimeStamp, None, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("takeHeapSnapshot", consoleProtoFuncTakeHeapSnapshot, None, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("group", consoleProtoFuncGroup, None, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("groupCollapsed", consoleProtoFuncGroupCollapsed, None, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("groupEnd", consoleProtoFuncGroupEnd, None, 0);
}

static String valueToStringWithUndefinedOrNullCheck(ExecState *exec, JSValue value) {
    if (value.isUndefinedOrNull())
        return String();
    return value.toWTFString(exec);
}

static void printConsoleLog(ExecState *exec, MessageLevel level) {
    JSValue val = exec->argument(0);
    if (val.isString()) {
        String s = val.toWTFString(exec);
        Weex::LogUtil::ConsoleLogPrint((int) level, "jsLog timeline", s.utf8().data());
        return;
    } else if (val.isObject()) {
        VM &vm_catch = exec->vm();
        auto scope = DECLARE_CATCH_SCOPE(vm_catch);
        String str = JSONStringify(exec, val, 0);
        JSC::VM &vm = exec->vm();
        if (UNLIKELY(scope.exception())) {
            scope.clearException();
            return;
        }
        Weex::LogUtil::ConsoleLogPrint((int)level, "jsLog", str.utf8().data());
    }
    Weex::LogUtil::ConsoleLogPrint((int) level, "jsLog", "[object]");
}

static EncodedJSValue consoleLogWithLevel(ExecState *exec, MessageLevel level) {
    printConsoleLog(exec, level);
//    ConsoleClient *client = exec->lexicalGlobalObject()->consoleClient();
//    if (!client)
//        return JSValue::encode(jsUndefined());
//
//    client->logWithLevel(exec, Inspector::createScriptArguments(exec, 0), level);
    JSValue::equalSlowCaseInline(exec,JSC::jsUndefined(),JSC::jsUndefined());
    return JSValue::encode(jsUndefined());
}

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncDebug(ExecState *exec) {
    return consoleLogWithLevel(exec, MessageLevel::Debug);
}

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncError(ExecState *exec) {
    return consoleLogWithLevel(exec, MessageLevel::Error);
}

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncLog(ExecState *exec) {
    return consoleLogWithLevel(exec, MessageLevel::Log);
}

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncInfo(ExecState *exec) {
    return consoleLogWithLevel(exec, MessageLevel::Info);
}

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncWarn(ExecState *exec) {
    return consoleLogWithLevel(exec, MessageLevel::Warning);
}

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncClear(ExecState *exec) {
    ConsoleClient *client = exec->lexicalGlobalObject()->consoleClient();
    if (!client)
        return JSValue::encode(jsUndefined());

    client->clear(exec);
    return JSValue::encode(jsUndefined());
}

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncDir(ExecState *exec) {
    ConsoleClient *client = exec->lexicalGlobalObject()->consoleClient();
    if (!client)
        return JSValue::encode(jsUndefined());

    client->dir(exec, Inspector::createScriptArguments(exec, 0));
    return JSValue::encode(jsUndefined());
}

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncDirXML(ExecState *exec) {
    ConsoleClient *client = exec->lexicalGlobalObject()->consoleClient();
    if (!client)
        return JSValue::encode(jsUndefined());

    client->dirXML(exec, Inspector::createScriptArguments(exec, 0));
    return JSValue::encode(jsUndefined());
}

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncTable(ExecState *exec) {
    ConsoleClient *client = exec->lexicalGlobalObject()->consoleClient();
    if (!client)
        return JSValue::encode(jsUndefined());

    client->table(exec, Inspector::createScriptArguments(exec, 0));
    return JSValue::encode(jsUndefined());
}

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncTrace(ExecState *exec) {
    ConsoleClient *client = exec->lexicalGlobalObject()->consoleClient();
    if (!client)
        return JSValue::encode(jsUndefined());

    client->trace(exec, Inspector::createScriptArguments(exec, 0));
    return JSValue::encode(jsUndefined());
}

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncAssert(ExecState *exec) {
    VM &vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    ConsoleClient *client = exec->lexicalGlobalObject()->consoleClient();
    if (!client)
        return JSValue::encode(jsUndefined());

    bool condition = exec->argument(0).toBoolean(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    if (condition)
        return JSValue::encode(jsUndefined());

    client->assertion(exec, Inspector::createScriptArguments(exec, 1));
    return JSValue::encode(jsUndefined());
}

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncCount(ExecState *exec) {
    ConsoleClient *client = exec->lexicalGlobalObject()->consoleClient();
    if (!client)
        return JSValue::encode(jsUndefined());

    client->count(exec, Inspector::createScriptArguments(exec, 0));
    return JSValue::encode(jsUndefined());
}

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncProfile(ExecState *exec) {
    VM &vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    ConsoleClient *client = exec->lexicalGlobalObject()->consoleClient();
    if (!client)
        return JSValue::encode(jsUndefined());

    size_t argsCount = exec->argumentCount();
    if (!argsCount) {
        client->profile(exec, String());
        return JSValue::encode(jsUndefined());
    }

    const String &title(valueToStringWithUndefinedOrNullCheck(exec, exec->argument(0)));
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    client->profile(exec, title);
    return JSValue::encode(jsUndefined());
}

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncProfileEnd(ExecState *exec) {
    VM &vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    ConsoleClient *client = exec->lexicalGlobalObject()->consoleClient();
    if (!client)
        return JSValue::encode(jsUndefined());

    size_t argsCount = exec->argumentCount();
    if (!argsCount) {
        client->profileEnd(exec, String());
        return JSValue::encode(jsUndefined());
    }

    const String &title(valueToStringWithUndefinedOrNullCheck(exec, exec->argument(0)));
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    client->profileEnd(exec, title);
    return JSValue::encode(jsUndefined());
}

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncTakeHeapSnapshot(ExecState *exec) {
    VM &vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    ConsoleClient *client = exec->lexicalGlobalObject()->consoleClient();
    if (!client)
        return JSValue::encode(jsUndefined());

    size_t argsCount = exec->argumentCount();
    if (!argsCount) {
        client->takeHeapSnapshot(exec, String());
        return JSValue::encode(jsUndefined());
    }

    const String &title(valueToStringWithUndefinedOrNullCheck(exec, exec->argument(0)));
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    client->takeHeapSnapshot(exec, title);
    return JSValue::encode(jsUndefined());
}

static String valueOrDefaultLabelString(ExecState *exec, JSValue value) {
    if (value.isUndefined())
        return ASCIILiteral("default");
    return value.toWTFString(exec);
}

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncTime(ExecState *exec) {
    VM &vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    ConsoleClient *client = exec->lexicalGlobalObject()->consoleClient();
    if (!client)
        return JSValue::encode(jsUndefined());

    String title;
    if (exec->argumentCount() < 1)
        title = ASCIILiteral("default");
    else {
        title = valueOrDefaultLabelString(exec, exec->argument(0));
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
    }

    client->time(exec, title);
    return JSValue::encode(jsUndefined());
}

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncTimeEnd(ExecState *exec) {
    VM &vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    ConsoleClient *client = exec->lexicalGlobalObject()->consoleClient();
    if (!client)
        return JSValue::encode(jsUndefined());

    String title;
    if (exec->argumentCount() < 1)
        title = ASCIILiteral("default");
    else {
        title = valueOrDefaultLabelString(exec, exec->argument(0));
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
    }

    client->timeEnd(exec, title);
    return JSValue::encode(jsUndefined());
}

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncTimeStamp(ExecState *exec) {
    ConsoleClient *client = exec->lexicalGlobalObject()->consoleClient();
    if (!client)
        return JSValue::encode(jsUndefined());

    client->timeStamp(exec, Inspector::createScriptArguments(exec, 0));
    return JSValue::encode(jsUndefined());
}

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncGroup(ExecState *exec) {
    ConsoleClient *client = exec->lexicalGlobalObject()->consoleClient();
    if (!client)
        return JSValue::encode(jsUndefined());

    client->group(exec, Inspector::createScriptArguments(exec, 0));
    return JSValue::encode(jsUndefined());
}

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncGroupCollapsed(ExecState *exec) {
    ConsoleClient *client = exec->lexicalGlobalObject()->consoleClient();
    if (!client)
        return JSValue::encode(jsUndefined());

    client->groupCollapsed(exec, Inspector::createScriptArguments(exec, 0));
    return JSValue::encode(jsUndefined());
}

static EncodedJSValue JSC_HOST_CALL consoleProtoFuncGroupEnd(ExecState *exec) {
    ConsoleClient *client = exec->lexicalGlobalObject()->consoleClient();
    if (!client)
        return JSValue::encode(jsUndefined());

    client->groupEnd(exec, Inspector::createScriptArguments(exec, 0));
    return JSValue::encode(jsUndefined());
}
