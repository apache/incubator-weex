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

#include "ConcurrentJSLock.h"
#include "ExecutableAllocator.h"
#include "MatchResult.h"
#include "RegExpKey.h"
#include "Structure.h"
#include "yarr/Yarr.h"
#include <wtf/Forward.h>
#include <wtf/text/WTFString.h>

#if ENABLE(YARR_JIT)
#include "yarr/YarrJIT.h"
#endif

namespace JSC {

struct RegExpRepresentation;
class VM;

JS_EXPORT_PRIVATE RegExpFlags regExpFlags(const String&);

class RegExp final : public JSCell {
public:
    typedef JSCell Base;
    static const unsigned StructureFlags = Base::StructureFlags | StructureIsImmortal;

    JS_EXPORT_PRIVATE static RegExp* create(VM&, const String& pattern, RegExpFlags);
    static const bool needsDestruction = true;
    static void destroy(JSCell*);
    static size_t estimatedSize(JSCell*);

    bool global() const { return m_flags & FlagGlobal; }
    bool ignoreCase() const { return m_flags & FlagIgnoreCase; }
    bool multiline() const { return m_flags & FlagMultiline; }
    bool sticky() const { return m_flags & FlagSticky; }
    bool globalOrSticky() const { return global() || sticky(); }
    bool unicode() const { return m_flags & FlagUnicode; }

    const String& pattern() const { return m_patternString; }

    bool isValid() const { return !m_constructionError && m_flags != InvalidFlags; }
    const char* errorMessage() const { return m_constructionError; }

    JS_EXPORT_PRIVATE int match(VM&, const String&, unsigned startOffset, Vector<int>& ovector);

    // Returns false if we couldn't run the regular expression for any reason.
    bool matchConcurrently(VM&, const String&, unsigned startOffset, int& position, Vector<int>& ovector);
    
    JS_EXPORT_PRIVATE MatchResult match(VM&, const String&, unsigned startOffset);

    bool matchConcurrently(VM&, const String&, unsigned startOffset, MatchResult&);

    // Call these versions of the match functions if you're desperate for performance.
    template<typename VectorType>
    int matchInline(VM&, const String&, unsigned startOffset, VectorType& ovector);
    MatchResult matchInline(VM&, const String&, unsigned startOffset);
    
    unsigned numSubpatterns() const { return m_numSubpatterns; }

    bool hasCode()
    {
        return m_state != NotCompiled;
    }

    bool hasCodeFor(Yarr::YarrCharSize);
    bool hasMatchOnlyCodeFor(Yarr::YarrCharSize);

    void deleteCode();

#if ENABLE(REGEXP_TRACING)
    void printTraceData();
#endif

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(CellType, StructureFlags), info());
    }

    DECLARE_INFO;

    RegExpKey key() { return RegExpKey(m_flags, m_patternString); }

protected:
    void finishCreation(VM&);

private:
    friend class RegExpCache;
    RegExp(VM&, const String&, RegExpFlags);

    static RegExp* createWithoutCaching(VM&, const String&, RegExpFlags);

    enum RegExpState {
        ParseError,
        JITCode,
        ByteCode,
        NotCompiled
    };

    RegExpState m_state;

    void compile(VM*, Yarr::YarrCharSize);
    void compileIfNecessary(VM&, Yarr::YarrCharSize);

    void compileMatchOnly(VM*, Yarr::YarrCharSize);
    void compileIfNecessaryMatchOnly(VM&, Yarr::YarrCharSize);

#if ENABLE(YARR_JIT_DEBUG)
    void matchCompareWithInterpreter(const String&, int startOffset, int* offsetVector, int jitResult);
#endif

    String m_patternString;
    RegExpFlags m_flags;
    const char* m_constructionError;
    unsigned m_numSubpatterns;
#if ENABLE(REGEXP_TRACING)
    double m_rtMatchOnlyTotalSubjectStringLen;
    double m_rtMatchTotalSubjectStringLen;
    unsigned m_rtMatchOnlyCallCount;
    unsigned m_rtMatchOnlyFoundCount;
    unsigned m_rtMatchCallCount;
    unsigned m_rtMatchFoundCount;
#endif
    ConcurrentJSLock m_lock;

#if ENABLE(YARR_JIT)
    Yarr::YarrCodeBlock m_regExpJITCode;
#endif
    std::unique_ptr<Yarr::BytecodePattern> m_regExpBytecode;
};

} // namespace JSC
