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

#include <wtf/Forward.h>

namespace JSC {
class Exception;
class ExecState;
class JSValue;
}

namespace Inspector {

class ScriptArguments;
class ScriptCallStack;

// FIXME: The subtle differences between these should be eliminated.
JS_EXPORT_PRIVATE Ref<ScriptCallStack> createScriptCallStack(JSC::ExecState*, size_t maxStackSize);
JS_EXPORT_PRIVATE Ref<ScriptCallStack> createScriptCallStackForConsole(JSC::ExecState*, size_t maxStackSize);
JS_EXPORT_PRIVATE Ref<ScriptCallStack> createScriptCallStackFromException(JSC::ExecState*, JSC::Exception*, size_t maxStackSize);
JS_EXPORT_PRIVATE Ref<ScriptArguments> createScriptArguments(JSC::ExecState*, unsigned skipArgumentCount);

} // namespace Inspector
