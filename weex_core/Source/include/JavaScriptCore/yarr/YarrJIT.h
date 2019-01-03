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

#if ENABLE(YARR_JIT)

#include "VM.h"
#include "MacroAssemblerCodeRef.h"
#include "MatchResult.h"
#include "Yarr.h"
#include "YarrPattern.h"

#if CPU(X86) && !COMPILER(MSVC)
#define YARR_CALL __attribute__ ((regparm (3)))
#else
#define YARR_CALL
#endif

namespace JSC {

class VM;
class ExecutablePool;

namespace Yarr {

class YarrCodeBlock {
#if CPU(X86_64) || CPU(ARM64)
    typedef MatchResult (*YarrJITCode8)(const LChar* input, unsigned start, unsigned length, int* output) YARR_CALL;
    typedef MatchResult (*YarrJITCode16)(const UChar* input, unsigned start, unsigned length, int* output) YARR_CALL;
    typedef MatchResult (*YarrJITCodeMatchOnly8)(const LChar* input, unsigned start, unsigned length) YARR_CALL;
    typedef MatchResult (*YarrJITCodeMatchOnly16)(const UChar* input, unsigned start, unsigned length) YARR_CALL;
#else
    typedef EncodedMatchResult (*YarrJITCode8)(const LChar* input, unsigned start, unsigned length, int* output) YARR_CALL;
    typedef EncodedMatchResult (*YarrJITCode16)(const UChar* input, unsigned start, unsigned length, int* output) YARR_CALL;
    typedef EncodedMatchResult (*YarrJITCodeMatchOnly8)(const LChar* input, unsigned start, unsigned length) YARR_CALL;
    typedef EncodedMatchResult (*YarrJITCodeMatchOnly16)(const UChar* input, unsigned start, unsigned length) YARR_CALL;
#endif

public:
    YarrCodeBlock()
        : m_needFallBack(false)
    {
    }

    ~YarrCodeBlock()
    {
    }

    void setFallBack(bool fallback) { m_needFallBack = fallback; }
    bool isFallBack() { return m_needFallBack; }

    bool has8BitCode() { return m_ref8.size(); }
    bool has16BitCode() { return m_ref16.size(); }
    void set8BitCode(MacroAssemblerCodeRef ref) { m_ref8 = ref; }
    void set16BitCode(MacroAssemblerCodeRef ref) { m_ref16 = ref; }

    bool has8BitCodeMatchOnly() { return m_matchOnly8.size(); }
    bool has16BitCodeMatchOnly() { return m_matchOnly16.size(); }
    void set8BitCodeMatchOnly(MacroAssemblerCodeRef matchOnly) { m_matchOnly8 = matchOnly; }
    void set16BitCodeMatchOnly(MacroAssemblerCodeRef matchOnly) { m_matchOnly16 = matchOnly; }

    MatchResult execute(const LChar* input, unsigned start, unsigned length, int* output)
    {
        ASSERT(has8BitCode());
        return MatchResult(reinterpret_cast<YarrJITCode8>(m_ref8.code().executableAddress())(input, start, length, output));
    }

    MatchResult execute(const UChar* input, unsigned start, unsigned length, int* output)
    {
        ASSERT(has16BitCode());
        return MatchResult(reinterpret_cast<YarrJITCode16>(m_ref16.code().executableAddress())(input, start, length, output));
    }

    MatchResult execute(const LChar* input, unsigned start, unsigned length)
    {
        ASSERT(has8BitCodeMatchOnly());
        return MatchResult(reinterpret_cast<YarrJITCodeMatchOnly8>(m_matchOnly8.code().executableAddress())(input, start, length));
    }

    MatchResult execute(const UChar* input, unsigned start, unsigned length)
    {
        ASSERT(has16BitCodeMatchOnly());
        return MatchResult(reinterpret_cast<YarrJITCodeMatchOnly16>(m_matchOnly16.code().executableAddress())(input, start, length));
    }

#if ENABLE(REGEXP_TRACING)
    void *get8BitMatchOnlyAddr()
    {
        if (!has8BitCodeMatchOnly())
            return 0;

        return m_matchOnly8.code().executableAddress();
    }

    void *get16BitMatchOnlyAddr()
    {
        if (!has16BitCodeMatchOnly())
            return 0;

        return m_matchOnly16.code().executableAddress();
    }

    void *get8BitMatchAddr()
    {
        if (!has8BitCode())
            return 0;

        return m_ref8.code().executableAddress();
    }

    void *get16BitMatchAddr()
    {
        if (!has16BitCode())
            return 0;

        return m_ref16.code().executableAddress();
    }
#endif

    size_t size() const
    {
        return m_ref8.size() + m_ref16.size() + m_matchOnly8.size() + m_matchOnly16.size();
    }

    void clear()
    {
        m_ref8 = MacroAssemblerCodeRef();
        m_ref16 = MacroAssemblerCodeRef();
        m_matchOnly8 = MacroAssemblerCodeRef();
        m_matchOnly16 = MacroAssemblerCodeRef();
        m_needFallBack = false;
    }

private:
    MacroAssemblerCodeRef m_ref8;
    MacroAssemblerCodeRef m_ref16;
    MacroAssemblerCodeRef m_matchOnly8;
    MacroAssemblerCodeRef m_matchOnly16;
    bool m_needFallBack;
};

enum YarrJITCompileMode {
    MatchOnly,
    IncludeSubpatterns
};
void jitCompile(YarrPattern&, YarrCharSize, VM*, YarrCodeBlock& jitObject, YarrJITCompileMode = IncludeSubpatterns);

} } // namespace JSC::Yarr

#endif
