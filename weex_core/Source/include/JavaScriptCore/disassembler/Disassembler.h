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

#include "JSExportMacros.h"
#include <functional>
#include <wtf/PrintStream.h>
#include <wtf/text/CString.h>

namespace JSC {

class MacroAssemblerCodePtr;
class MacroAssemblerCodeRef;

#if ENABLE(DISASSEMBLER)
bool tryToDisassemble(const MacroAssemblerCodePtr&, size_t, const char* prefix, PrintStream&);
#else
inline bool tryToDisassemble(const MacroAssemblerCodePtr&, size_t, const char*, PrintStream&)
{
    return false;
}
#endif

// Prints either the disassembly, or a line of text indicating that disassembly failed and
// the range of machine code addresses.
void disassemble(const MacroAssemblerCodePtr&, size_t, const char* prefix, PrintStream& out);

// Asynchronous disassembly. This happens on another thread, and calls the provided
// callback when the disassembly is done.
void disassembleAsynchronously(
    const CString& header, const MacroAssemblerCodeRef&, size_t, const char* prefix);

JS_EXPORT_PRIVATE void waitForAsynchronousDisassembly();

} // namespace JSC
