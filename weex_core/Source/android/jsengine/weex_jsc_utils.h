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

#ifdef USE_JS_RUNTIME
#include "js_runtime/weex/utils/weex_jsc_utils.h"
#else

#ifndef WEEXV8_UTILS_H
#define WEEXV8_UTILS_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <dlfcn.h>
#include <cstring>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <type_traits>
#include <unistd.h>

#include "base/base64/base64.h"
#include "base/crash/crash_handler.h"
#include "base/utils/log_utils.h"
#include "config.h"
#include "include/WeexApiHeader.h"
#include "include/JavaScriptCore/runtime/JSExportMacros.h"
#include "include/JavaScriptCore/inspector/ScriptCallStackFactory.h"
#include "include/JavaScriptCore/inspector/ScriptArguments.h"
#include "include/JavaScriptCore/runtime/ConsoleClient.h"
#include "include/JavaScriptCore/heap/HeapTimer.h"
#include "include/JavaScriptCore/heap/Strong.h"
#include "include/JavaScriptCore/heap/StrongInlines.h"
#include "include/JavaScriptCore/runtime/Completion.h"
#include "include/JavaScriptCore/runtime/EnumerationMode.h"
#include "include/JavaScriptCore/runtime/Exception.h"
#include "include/JavaScriptCore/runtime/ExceptionHelpers.h"
#include "include/JavaScriptCore/runtime/JSFunction.h"
#include "include/JavaScriptCore/runtime/InitializeThreading.h"
#include "include/JavaScriptCore/runtime/JSString.h"
#include "include/JavaScriptCore/runtime/JSArrayBuffer.h"
#include "include/JavaScriptCore/runtime/Lookup.h"
#include "include/JavaScriptCore/runtime/ObjectConstructor.h"
#include "include/wtf/CurrentTime.h"
#include "include/wtf/Trace.h"
#include "include/wtf/ICUCompatible.h"
#include "include/wtf/text/Base64.h"
#include "include/wtf/text/StringBuilder.h"
#include "include/wtf/unicode/WTFUTF8.h"

#include "android/jsengine/object/weex_simple_object.h"
#include "android/jsengine/object/args.h"
#include "android/jsengine/wson/wson_jsc.h"

#include "third_party/IPC/Buffering/IPCBuffer.h"
#include "third_party/IPC/IPCArguments.h"
#include "third_party/IPC/IPCByteArray.h"
#include "third_party/IPC/IPCHandler.h"
#include "third_party/IPC/IPCListener.h"
#include "third_party/IPC/IPCMessageJS.h"
#include "third_party/IPC/IPCResult.h"
#include "third_party/IPC/IPCSender.h"
#include "third_party/IPC/IPCString.h"
#include "third_party/IPC/IPCType.h"
#include "third_party/IPC/IPCFutexPageQueue.h"
#include "third_party/IPC/IPCException.h"
#include "third_party/IPC/Serializing/IPCSerializer.h"
#include "include/JavaScriptCore/runtime/StructureInlines.h"
#include "include/JavaScriptCore/runtime/JSCJSValueInlines.h"

#if !defined(PATH_MAX)
#define PATH_MAX 4096
#endif

#define MICROSEC  ((uint64_t) 1e6)

#define TIMESPCE 1000

using namespace JSC;
using namespace WeexCore;

extern bool config_use_wson;

String jString2String(const uint16_t *str, size_t length);


String weexString2String(const WeexString *weexString);

String char2String(const char *string);

void addString(IPCSerializer *serializer, const String &s);

void getArgumentAsJString(IPCSerializer *serializer, ExecState *state, int argument);

WeexByteArray *getArgumentAsWeexByteArrayJSON(ExecState *state, int argument);
WeexString *genWeexStringSS(const uint16_t *str, size_t length);
WeexByteArray *genWeexByteArraySS(const char *str, size_t strLen);

JSValue jString2JSValue(ExecState *state, const uint16_t *str, size_t length);

JSValue String2JSValue(ExecState *state, String s);

JSValue parseToObject(ExecState *state, const String &data);

void getArgumentAsJByteArrayJSON(IPCSerializer *serializer, ExecState *state, int argument);

void getArgumentAsJByteArray(IPCSerializer *serializer, ExecState *state, int argument);
void freeInitFrameworkParams(std::vector<INIT_FRAMEWORK_PARAMS *> &params);
void freeParams(std::vector<VALUE_WITH_TYPE *> &params);
void FillVectorOfInitFramework(std::vector<INIT_FRAMEWORK_PARAMS *> &params,
                               IPCArguments *arguments, size_t start,
                               size_t end);
std::unique_ptr<char[]> getCharJSONStringFromState(ExecState *state, int argument);

std::unique_ptr<char[]> getCharOrJSONStringFromState(ExecState *state, int argument);

std::unique_ptr<char[]> getCharStringFromState(ExecState *state, int argument);

/** auto choose use wson or json */
void getWsonOrJsonArgsFromState(ExecState *state, int argument, Args& args);

/** force string */
void getStringArgsFromState(ExecState *state, int argument, Args& args);

/**
 * get wson args with wson parser
 */
void getWsonArgsFromState(ExecState *state, int argument, Args& args);

/**
 * get json args with JSONStringify
 */
void getJSONArgsFromState(ExecState *state, int argument, Args& args);

/**
 * add to ipc argument
 * 
 */
void addObjectArgsToIPC(IPCSerializer *serializer, Args& args);
void addStringArgsToIPC(IPCSerializer *serializer, Args& args);


std::unique_ptr<char[]> newCharString(const char *str, size_t length);
void printLogOnFileWithNameS(const char * name,const char *log);
void getArgumentAsCString(IPCSerializer *serializer, ExecState *state, int argument);

void initHeapTimer();

void deinitHeapTimer();

void markupStateInternally();

void doUpdateGlobalSwitchConfig(const char *config);

class WeexGlobalObject;

void setJSFVersion(WeexGlobalObject *globalObject);

String exceptionToString(JSGlobalObject *globalObject, JSValue exception);

void ReportException(JSGlobalObject *globalObject,
                     Exception *exception,
                     const char *jinstanceid,
                     const char *func);

bool ExecuteJavaScript(JSGlobalObject *globalObject,
                       const String &source,
                       const String &url,
                       bool report_exceptions,
                       const char *func,
                       const char *instanceId = "");

uint64_t microTime();
int __atomic_inc(volatile int *ptr);
int genTaskId() ;
WeexByteArray *IPCByteArrayToWeexByteArray(const IPCByteArray *byteArray);


namespace WEEXICU {

    class unique_fd {
    public:
        explicit unique_fd(int fd);

        ~unique_fd();

        int get() const;

    private:
        int m_fd;
    };

    static std::string __attribute__((noinline)) findPath();

    static void findPath(std::string &executablePath, std::string &icuDataPath) {
        unsigned long target = reinterpret_cast<unsigned long>(__builtin_return_address(0));
        FILE *f = fopen("/proc/self/maps", "r");
        if (!f) {
            return;
        }
        char buffer[256];
        char *line;
        while ((line = fgets(buffer, 256, f))) {
            if (icuDataPath.empty() && strstr(line, "icudt")) {
                icuDataPath.assign(strstr(line, "/"));
                icuDataPath = icuDataPath.substr(0, icuDataPath.length() - 1);
                continue;
            }
            char *end;
            unsigned long val;
            errno = 0;
            val = strtoul(line, &end, 16);
            if (errno)
                continue;
            if (val > target)
                continue;
            end += 1;
            errno = 0;
            val = strtoul(end, &end, 16);
            if (errno)
                continue;
            if (val > target) {
                char *s = strstr(end, "/");
                if (s != nullptr)
                    executablePath.assign(s);
                std::size_t found = executablePath.rfind('/');
                if (found != std::string::npos) {
                    executablePath = executablePath.substr(0, found);
                }
            }
            if (!executablePath.empty()
                && !icuDataPath.empty()) {
                break;
            }
        }
        fclose(f);
        return;
    }


#define FAIL_WITH_STRERROR(tag) \
    LOGE(" fails: %s.\n", strerror(errno)); \
    return false;

#define MAYBE_FAIL_WITH_ICU_ERROR(s) \
    if (status != U_ZERO_ERROR) {\
        LOGE("Couldn't initialize ICU (" "): %s (%s)" "\n", u_errorName(status), path.c_str()); \
        return false; \
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

    static bool initICUEnv(bool multiProcess) {
        static bool isInit = false;
        if (isInit)
            return true;

        char *path;
        if (!multiProcess) {
            std::string executablePath;
            std::string icuDataPath;
            findPath(executablePath, icuDataPath);
            path = new char[icuDataPath.length() + 1];
            std::strcpy(path, icuDataPath.c_str());
        } else {
            path = getenv("ICU_DATA_PATH");
        }
        LOGE("initICUEnv patch:%s", path);

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
        if (strlen(path) > 0) {
            isInit = true;
            return true;//mapIcuData(std::string(path));
        }
        return false;
    }

}

#endif //WEEXV8_UTILS_H
#endif //USE_JS_RUNTIME
