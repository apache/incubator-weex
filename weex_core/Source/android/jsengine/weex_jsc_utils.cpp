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
// Created by Darin on 11/02/2018.
//

#include "android/jsengine/weex_jsc_utils.h"

#include <iostream>
#include <fstream>

#include "android/jsengine/object/weex_global_object.h"
#include "android/jsengine/object/weex_env.h"
#include "core/bridge/script_bridge.h"

//extern WEEX_CORE_JS_API_FUNCTIONS *weex_core_js_api_functions;

using namespace JSC;
 void printLogOnFileWithNameS(const char * name, const char *log) {
    std::string string("/data/data/com.alibaba.weex/");
    string.append(name);
    std::ofstream mcfile;
    mcfile.open(string.c_str(), std::ios::app);
    mcfile << log << std::endl;
    mcfile.close();
}

String jString2String(const uint16_t *str, size_t length) {
    UChar *dst;
    String s = String::createUninitialized(length, dst);
    memcpy(dst, str, length * sizeof(UChar));
    return s;
}


String weexString2String(const WeexString *weexString) {
    if (weexString != nullptr) {
        return jString2String(weexString->content, weexString->length);
    }

    return String("");
}

String char2String(const char *string) {
    if (string == nullptr)
        return String::fromUTF8("");

    return String::fromUTF8(string);
}

std::unique_ptr<char[]> newCharString(const char *str, size_t length) {
    auto count = length + 1;

    std::unique_ptr<char[]> buffer(new char[count]);

    if (buffer == nullptr)
        return nullptr;

    char *result = reinterpret_cast<char *>(buffer.get());

    if (result == nullptr)
        return nullptr;

    memset(result, 0, count);
    memcpy(result, str, length);
    result[length] = '\0';
    return buffer;
}

std::unique_ptr<char[]> getCharJSONStringFromState(ExecState *state, int argument) {
    if (argument >= state->argumentCount()) {
        return nullptr;
    }
    JSValue val = state->argument(argument);
    VM &vm = state->vm();
    if (val.isObject()) {
        VM &vm = state->vm();
        auto scope = DECLARE_CATCH_SCOPE(vm);
        String str = JSONStringify(state, val, 0);
        if (UNLIKELY(scope.exception())) {
            scope.clearException();
            return nullptr;
        }
        CString data = str.utf8();
        return newCharString(data.data(), data.length());
    } else {
        return nullptr;
    }
}

std::unique_ptr<char[]> getCharOrJSONStringFromState(ExecState *state, int argument) {
    if (argument >= state->argumentCount()) {
        return nullptr;
    }
    JSValue val = state->argument(argument);
    if (val.isString() || val.isNumber()) {
        String str(val.toWTFString(state));
        CString data = str.utf8();
        return newCharString(data.data(), data.length());
    } else if (val.isObject()) {
        VM &vm = state->vm();
        auto scope = DECLARE_CATCH_SCOPE(vm);
        String str = JSONStringify(state, val, 0);
        if (UNLIKELY(scope.exception())) {
            scope.clearException();
            return nullptr;
        }
        CString data = str.utf8();
        return newCharString(data.data(), data.length());
    } else {
        return nullptr;
    }
}

void getWsonOrJsonArgsFromState(ExecState *state, int argument, Args& args){

     bool useWson = WeexEnv::getEnv()->useWson();

    if(useWson){
        if (argument >= state->argumentCount()) {
             args.setWson((wson_buffer*)nullptr);
             return;
        }
        JSValue val = state->argument(argument);
        wson_buffer* buffer = wson::toWson(state, val);
        args.setWson(buffer);
        return;
    }else{
        if (argument >= state->argumentCount()) {
            String str("");
            args.setString(str);
            return;
        }
        JSValue val = state->argument(argument);
        if(val.isString()){
             String str = val.toWTFString(state);
             args.setString(str);
             return;
        }else if(val.isObject()){
            VM &vm = state->vm();
            auto scope = DECLARE_CATCH_SCOPE(vm);
            String str = JSONStringify(state, val, 0);
            args.setString(str);
            return;
        }
    }
    String str("");
    args.setString(str);
    return;
}

void getStringArgsFromState(ExecState *state, int argument,Args& args){
    if (argument >= state->argumentCount()) {
        String str("");
       args.setString(str);
       return;
    }
    JSValue val = state->argument(argument);
    String str = val.toWTFString(state);
    args.setString(str);
    return;
}

void getWsonArgsFromState(ExecState *state, int argument, Args& args){
     if (argument >= state->argumentCount()) {
         args.setWson((wson_buffer*)nullptr);
         return;
     }
     JSValue val = state->argument(argument);
     wson_buffer* buffer = wson::toWson(state, val);
     args.setWson(buffer);
     return;
}

void getJSONArgsFromState(ExecState *state, int argument, Args& args){
    JSValue val = state->argument(argument);
    if(val.isString()){
            String str = val.toWTFString(state);
            args.setString(str);
            return;
    }else if(val.isObject()){
        VM &vm = state->vm();
        auto scope = DECLARE_CATCH_SCOPE(vm);
        String str = JSONStringify(state, val, 0);
         args.setString(str);
        return;
    }
   String str("");
   args.setString(str);
   return;
}



void addObjectArgsToIPC(IPCSerializer *serializer, Args& args){
    if(args.getType() == ARGS_TYPE_WSON){
        wson_buffer* buffer = args.wson;
        if(buffer != nullptr && (((char*)buffer->data)[0] != WSON_NULL_TYPE) ){
           serializer->add((const char*)buffer->data,  buffer->position);
        }else{
           serializer->addJSUndefined();
        }
    }else {
        String str = args.json;
        CString data = str.utf8();
        serializer->add(data.data(), data.length());
    }
}

void addStringArgsToIPC(IPCSerializer *serializer, Args& args){
    if(args.json.length() > 0){
        String str = args.json;
        CString data = str.utf8();
        serializer->add(data.data(), data.length());
    }else{
      serializer->addJSUndefined();
    }
}

std::unique_ptr<char[]>  getCharStringFromState(ExecState *state, int argument) {
    JSValue val = state->argument(argument);
    String s = val.toWTFString(state);
    return newCharString(s.utf8().data(), s.length());
}

void getArgumentAsCString(IPCSerializer *serializer, ExecState *state, int argument) {
    JSValue val = state->argument(argument);
    String s = val.toWTFString(state);
    serializer->add(s.utf8().data(), s.length());
}


void addString(IPCSerializer *serializer, const String &s) {
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


void getArgumentAsJString(IPCSerializer *serializer, ExecState *state, int argument) {
    JSValue val = state->argument(argument);
    String s = val.toWTFString(state);
    addString(serializer, s);
}


void getArgumentAsJByteArrayJSON(IPCSerializer *serializer, ExecState *state, int argument) {
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


WeexByteArray *getArgumentAsWeexByteArrayJSON(ExecState *state, int argument) {
    if (argument >= state->argumentCount()) {
        return nullptr;
    }
    JSValue val = state->argument(argument);
    VM &vm = state->vm();
    if (val.isObject()) {
        auto scope = DECLARE_CATCH_SCOPE(vm);
        String str = JSONStringify(state, val, 0);
        JSC::VM &vm = state->vm();
        if (UNLIKELY(scope.exception())) {
            scope.clearException();
            return nullptr;
        }
        CString data = str.utf8();

        //serializer->add(data.data(), data.length());

        return genWeexByteArraySS(data.data(), data.length());
    }

    return nullptr;
}

WeexString *genWeexStringSS(const uint16_t *str, size_t length) {
    size_t byteSize = length * sizeof(uint16_t);
    auto *string = (WeexString *) malloc(byteSize + sizeof(WeexString));
    if (string == nullptr)
        return nullptr;

    memset(string, 0, byteSize + sizeof(WeexString));
    string->length = length;
    memcpy(string->content, str, byteSize);
    return string;
}
WeexByteArray *genWeexByteArraySS(const char *str, size_t strLen) {
    auto *ret = (WeexByteArray *) malloc(strLen + sizeof(WeexByteArray));

    if (ret == nullptr)
        return nullptr;

    memset(ret, 0, strLen + sizeof(WeexByteArray));

    ret->length = strLen;
    memcpy(ret->content, str, strLen);

    ret->content[strLen] = '\0';

    return const_cast<WeexByteArray *> (ret);
}

JSValue jString2JSValue(ExecState *state, const uint16_t *str, size_t length) {
    String s = jString2String(str, length);
    if (s.isEmpty()) {
        return jsEmptyString(&state->vm());
    } else if (s.length() == 1) {
        return jsSingleCharacterString(&state->vm(), s[0]);
    }
    return jsNontrivialString(&state->vm(), WTFMove(s));
}

JSValue String2JSValue(ExecState *state, String s) {
    if (s.isEmpty()) {
        return jsEmptyString(&state->vm());
    } else if (s.length() == 1) {
        return jsSingleCharacterString(&state->vm(), s[0]);
    }
    return jsNontrivialString(&state->vm(), WTFMove(s));
}

JSValue parseToObject(ExecState *state, const String &data) {
    VM &vm = state->vm();
    auto scope = DECLARE_CATCH_SCOPE(vm);
    JSValue ret = JSONParse(state, data);
    if (UNLIKELY(scope.exception())) {
        scope.clearException();
        return jsUndefined();
    }
    if (!ret)
        return jsUndefined();
    return ret;
}

void getArgumentAsJByteArray(IPCSerializer *serializer, ExecState *state, int argument) {
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
void freeInitFrameworkParams(std::vector<INIT_FRAMEWORK_PARAMS *> &params) {
    for (auto &param : params) {
        free(param->type);
        free(param->value);
        free(param);
    }
}

void freeParams(std::vector<VALUE_WITH_TYPE *> &params) {
    for (auto &param : params) {
        if (param->type == ParamsType::STRING ||
            param->type == ParamsType::JSONSTRING) {
            free(param->value.string);
        }
        if (param->type == ParamsType::BYTEARRAY) {
            free(param->value.byteArray);
        }
        free(param);
    }
}

void FillVectorOfInitFramework(std::vector<INIT_FRAMEWORK_PARAMS *> &params,
                                     IPCArguments *arguments, size_t start,
                                     size_t end) {
    for (size_t i = start; i < end; i += 2) {
        if (arguments->getType(i) != IPCType::BYTEARRAY) {
            continue;
        }
        if (arguments->getType(1 + i) != IPCType::BYTEARRAY) {
            continue;
        }
        const IPCByteArray *ba = arguments->getByteArray(1 + i);
        const IPCByteArray *ba_type = arguments->getByteArray(i);
        auto init_framework_params =
                (INIT_FRAMEWORK_PARAMS *) malloc(sizeof(INIT_FRAMEWORK_PARAMS));
        if (init_framework_params == nullptr) {
            break;
        }
        memset(init_framework_params, 0, sizeof(INIT_FRAMEWORK_PARAMS));
        init_framework_params->type = IPCByteArrayToWeexByteArray(ba_type);
        init_framework_params->value = IPCByteArrayToWeexByteArray(ba);
        params.push_back(init_framework_params);
    }
}

void initHeapTimer() {
    HeapTimer::startTimerThread();
}

void deinitHeapTimer() {
    HeapTimer::stopTimerThread();
}

void markupStateInternally() {
}


void doUpdateGlobalSwitchConfig(const char *config) {
    if (!config) {
        return;
    }
    LOGE("doUpdateGlobalSwitchConfig %s", config);
    if (strstr(config, "wson_off") != NULL) {
        WeexEnv::getEnv()->setUseWson(false);
    } else {
        WeexEnv::getEnv()->setUseWson(true);
    }
}


String exceptionToString(JSGlobalObject *globalObject, JSValue exception) {
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

void setJSFVersion(WeexGlobalObject *globalObject) {
    VM &vm = globalObject->vm();
    PropertyName getJSFMVersionProperty(Identifier::fromString(&vm, "getJSFMVersion"));
    ExecState *state = globalObject->globalExec();
    JSValue getJSFMVersionFunction = globalObject->get(state, getJSFMVersionProperty);
    MarkedArgumentBuffer args;
    CallData callData;
    CallType callType = getCallData(getJSFMVersionFunction, callData);
    NakedPtr<Exception> returnedException;
    JSValue version = call(globalObject->globalExec(), getJSFMVersionFunction, callType, callData, globalObject, args,
                           returnedException);
    if (returnedException) {
        ReportException(globalObject, returnedException.get(), "", "");
    }
    String str = version.toWTFString(state);
    CString data = str.utf8();

    globalObject->js_bridge()->core_side()->SetJSVersion(data.data());
}


void ReportException(JSGlobalObject *_globalObject, Exception *exception, const char *instanceid,
                     const char *func) {
    String exceptionInfo = exceptionToString(_globalObject, exception->value());
    CString data = exceptionInfo.utf8();

    auto *globalObject = static_cast<WeexGlobalObject *>(_globalObject);
    const char *exception_char = data.data();
    globalObject->js_bridge()->core_side()->ReportException(instanceid, func, exception_char == nullptr? "" : exception_char);
}


uint64_t microTime() {
    struct timeval tv;

    gettimeofday(&tv, nullptr);

    return (((uint64_t) tv.tv_sec) * MICROSEC + tv.tv_usec);
}


int __atomic_inc(volatile int *ptr) {
    return __sync_fetch_and_add(ptr,1);
}
static int taskIdGenerator = 0;
int genTaskId(){
    return __atomic_inc(&taskIdGenerator);
}

/**
 * this function is to execute a section of JavaScript content.
 */
bool ExecuteJavaScript(JSGlobalObject *globalObject,
                       const String &source,
                       const String &url,
                       bool report_exceptions,
                       const char *func,
                       const char *instanceId) {
    SourceOrigin sourceOrigin(String::fromUTF8("(weex)"));
    NakedPtr<Exception> evaluationException;
    JSValue returnValue = evaluate(globalObject->globalExec(), makeSource(source, sourceOrigin, url), JSValue(),
                                   evaluationException);

    if (report_exceptions && evaluationException) {
        ReportException(globalObject, evaluationException.get(), instanceId, func);
        return false;
    }

    if (evaluationException)
        return false;

    globalObject->vm().drainMicrotasks();

    return true;
}

char *getCharFromUniquePtr(std::unique_ptr<char[]> str) {
    return str == nullptr ? nullptr : str.get();
}

WeexByteArray *IPCByteArrayToWeexByteArray(const IPCByteArray *byteArray) {
    return genWeexByteArraySS(byteArray->content, byteArray->length);
}


namespace WEEXICU {
    unique_fd::unique_fd(int fd)
            : m_fd(fd) {
    }

    unique_fd::~unique_fd() {
        close(m_fd);
    }

    int unique_fd::get() const {
        return m_fd;
    }
}
