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

#include "CodeBlock.h"

namespace JSC {

// Program code is not marked by any function, so we make the global object
// responsible for marking it.

class GlobalCodeBlock : public CodeBlock {
    typedef CodeBlock Base;

protected:
    GlobalCodeBlock(VM* vm, Structure* structure, CopyParsedBlockTag, GlobalCodeBlock& other)
        : CodeBlock(vm, structure, CopyParsedBlock, other)
    {
    }

    GlobalCodeBlock(VM* vm, Structure* structure, ScriptExecutable* ownerExecutable, UnlinkedCodeBlock* unlinkedCodeBlock, JSScope* scope, RefPtr<SourceProvider>&& sourceProvider, unsigned sourceOffset, unsigned firstLineColumnOffset)
        : CodeBlock(vm, structure, ownerExecutable, unlinkedCodeBlock, scope, WTFMove(sourceProvider), sourceOffset, firstLineColumnOffset)
    {
    }
};

} // namespace JSC
