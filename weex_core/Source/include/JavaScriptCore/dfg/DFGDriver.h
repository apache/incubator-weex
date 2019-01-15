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

#include "CallFrame.h"
#include "DFGCompilationMode.h"
#include "DFGPlan.h"

namespace JSC {

class CodeBlock;
class JITCode;
class MacroAssemblerCodePtr;
class VM;

namespace DFG {

JS_EXPORT_PRIVATE unsigned getNumCompilations();

// If the worklist is non-null, we do a concurrent compile. Otherwise we do a synchronous
// compile. Even if we do a synchronous compile, we call the callback with the result.
CompilationResult compile(
    VM&, CodeBlock*, CodeBlock* profiledDFGCodeBlock, CompilationMode,
    unsigned osrEntryBytecodeIndex, const Operands<JSValue>& mustHandleValues,
    Ref<DeferredCompilationCallback>&&);

} } // namespace JSC::DFG
