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

#include "ConsoleTypes.h"
#include <wtf/Forward.h>

namespace Inspector {
class ScriptArguments;
}

namespace JSC {

class ExecState;

class ConsoleClient {
public:
    virtual ~ConsoleClient() { }

    JS_EXPORT_PRIVATE static void printConsoleMessage(MessageSource, MessageType, MessageLevel, const String& message, const String& url, unsigned lineNumber, unsigned columnNumber);
    JS_EXPORT_PRIVATE static void printConsoleMessageWithArguments(MessageSource, MessageType, MessageLevel, JSC::ExecState*, Ref<Inspector::ScriptArguments>&&);

    void logWithLevel(ExecState*, Ref<Inspector::ScriptArguments>&&, MessageLevel);
    void clear(ExecState*);
    void dir(ExecState*, Ref<Inspector::ScriptArguments>&&);
    void dirXML(ExecState*, Ref<Inspector::ScriptArguments>&&);
    void table(ExecState*, Ref<Inspector::ScriptArguments>&&);
    void trace(ExecState*, Ref<Inspector::ScriptArguments>&&);
    void assertion(ExecState*, Ref<Inspector::ScriptArguments>&&);
    void group(ExecState*, Ref<Inspector::ScriptArguments>&&);
    void groupCollapsed(ExecState*, Ref<Inspector::ScriptArguments>&&);
    void groupEnd(ExecState*, Ref<Inspector::ScriptArguments>&&);

    virtual void messageWithTypeAndLevel(MessageType, MessageLevel, JSC::ExecState*, Ref<Inspector::ScriptArguments>&&) = 0;
    virtual void count(ExecState*, Ref<Inspector::ScriptArguments>&&) = 0;
    virtual void profile(ExecState*, const String& title) = 0;
    virtual void profileEnd(ExecState*, const String& title) = 0;
    virtual void takeHeapSnapshot(ExecState*, const String& title) = 0;
    virtual void time(ExecState*, const String& title) = 0;
    virtual void timeEnd(ExecState*, const String& title) = 0;
    virtual void timeStamp(ExecState*, Ref<Inspector::ScriptArguments>&&) = 0;

private:
    enum ArgumentRequirement { ArgumentRequired, ArgumentNotRequired };
    void internalMessageWithTypeAndLevel(MessageType, MessageLevel, JSC::ExecState*, Ref<Inspector::ScriptArguments>&&, ArgumentRequirement);
};

} // namespace JSC
