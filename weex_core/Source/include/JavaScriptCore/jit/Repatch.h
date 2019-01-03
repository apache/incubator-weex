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

#if ENABLE(JIT)

#include "CCallHelpers.h"
#include "CallVariant.h"
#include "JITOperations.h"
#include "PutKind.h"

namespace JSC {

enum class GetByIDKind {
    Normal,
    Try,
    WithThis
};

void repatchGetByID(ExecState*, JSValue, const Identifier&, const PropertySlot&, StructureStubInfo&, GetByIDKind);
void buildGetByIDList(ExecState*, JSValue, const Identifier&, const PropertySlot&, StructureStubInfo&);
void buildGetByIDProtoList(ExecState*, JSValue, const Identifier&, const PropertySlot&, StructureStubInfo&);
void repatchPutByID(ExecState*, JSValue, Structure*, const Identifier&, const PutPropertySlot&, StructureStubInfo&, PutKind);
void buildPutByIdList(ExecState*, JSValue, Structure*, const Identifier&, const PutPropertySlot&, StructureStubInfo&, PutKind);
void repatchIn(ExecState*, JSCell*, const Identifier&, bool wasFound, const PropertySlot&, StructureStubInfo&);
void linkFor(ExecState*, CallLinkInfo&, CodeBlock*, JSFunction* callee, MacroAssemblerCodePtr);
void linkDirectFor(ExecState*, CallLinkInfo&, CodeBlock*, MacroAssemblerCodePtr);
void linkSlowFor(ExecState*, CallLinkInfo&);
void unlinkFor(VM&, CallLinkInfo&);
void linkVirtualFor(ExecState*, CallLinkInfo&);
void linkPolymorphicCall(ExecState*, CallLinkInfo&, CallVariant);
void resetGetByID(CodeBlock*, StructureStubInfo&, GetByIDKind);
void resetPutByID(CodeBlock*, StructureStubInfo&);
void resetIn(CodeBlock*, StructureStubInfo&);
void ftlThunkAwareRepatchCall(CodeBlock*, CodeLocationCall, FunctionPtr newCalleeFunction);

} // namespace JSC

#endif // ENABLE(JIT)
