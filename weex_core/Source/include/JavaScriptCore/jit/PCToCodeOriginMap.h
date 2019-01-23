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

#include "CodeOrigin.h"
#include "DFGCommon.h"
#include "MacroAssembler.h"
#include "VM.h"
#include <wtf/Bag.h>
#include <wtf/Optional.h>
#include <wtf/RedBlackTree.h>
#include <wtf/Vector.h>

namespace JSC {

#if ENABLE(FTL_JIT)
namespace B3 {
class PCToOriginMap;
}
#endif

class LinkBuffer;
class PCToCodeOriginMapBuilder;

class PCToCodeOriginMapBuilder {
    WTF_MAKE_NONCOPYABLE(PCToCodeOriginMapBuilder);
    friend class PCToCodeOriginMap;

public:
    PCToCodeOriginMapBuilder(VM&);
    PCToCodeOriginMapBuilder(PCToCodeOriginMapBuilder&& other);

#if ENABLE(FTL_JIT)
    PCToCodeOriginMapBuilder(VM&, B3::PCToOriginMap&&);
#endif

    void appendItem(MacroAssembler::Label, const CodeOrigin&);
    static CodeOrigin defaultCodeOrigin() { return CodeOrigin(0, nullptr); }

    bool didBuildMapping() const { return m_shouldBuildMapping; }

private:

    struct CodeRange {
        MacroAssembler::Label start;
        MacroAssembler::Label end;
        CodeOrigin codeOrigin;
    };

    VM& m_vm;
    Vector<CodeRange> m_codeRanges;
    bool m_shouldBuildMapping;
};

class PCToCodeOriginMap {
    WTF_MAKE_NONCOPYABLE(PCToCodeOriginMap);
public:
    PCToCodeOriginMap(PCToCodeOriginMapBuilder&&, LinkBuffer&);
    ~PCToCodeOriginMap();

    std::optional<CodeOrigin> findPC(void* pc) const;

    double memorySize();

private:
    size_t m_compressedPCBufferSize;
    size_t m_compressedCodeOriginsSize;
    uint8_t* m_compressedPCs;
    uint8_t* m_compressedCodeOrigins;
    uintptr_t m_pcRangeStart;
    uintptr_t m_pcRangeEnd;
};

} // namespace JSC

#endif // ENABLE(JIT)
