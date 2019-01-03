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

#include "Options.h"

namespace JSC {

class ExecState;
class ThrowScope;

// Call this only if you know that exception fuzzing is enabled.
void doExceptionFuzzing(ExecState*, ThrowScope&, const char* where, void* returnPC);

// This is what you should call if you don't know if fuzzing is enabled.
ALWAYS_INLINE void doExceptionFuzzingIfEnabled(ExecState* exec, ThrowScope& scope, const char* where, void* returnPC)
{
    if (LIKELY(!Options::useExceptionFuzz()))
        return;
    doExceptionFuzzing(exec, scope, where, returnPC);
}

} // namespace JSC
