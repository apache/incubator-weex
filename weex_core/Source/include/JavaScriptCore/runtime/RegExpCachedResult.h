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

#include "RegExpObject.h"

namespace JSC {

class JSArray;
class JSString;

// RegExpCachedResult is used to track the cached results of the last
// match, stores on the RegExp constructor (e.g. $&, $_, $1, $2 ...).
// These values will be lazily generated on demand, so the cached result
// may be in a lazy or reified state. A lazy state is indicated by a
// value of m_result indicating a successful match, and a reified state
// is indicated by setting m_result to MatchResult::failed().
// Following a successful match, m_result, m_lastInput and m_lastRegExp
// can be used to reify the results from the match, following reification
// m_reifiedResult and m_reifiedInput hold the cached results.
class RegExpCachedResult {
public:
    RegExpCachedResult(VM& vm, JSObject* owner, RegExp* emptyRegExp)
        : m_result(0, 0)
        , m_reified(false)
    {
        m_lastInput.set(vm, owner, jsEmptyString(&vm));
        m_lastRegExp.set(vm, owner, emptyRegExp);
    }

    ALWAYS_INLINE void record(VM& vm, JSObject* owner, RegExp* regExp, JSString* input, MatchResult result)
    {
        vm.heap.writeBarrier(owner);
        m_lastRegExp.setWithoutWriteBarrier(regExp);
        m_lastInput.setWithoutWriteBarrier(input);
        m_result = result;
        m_reified = false;
    }

    JSArray* lastResult(ExecState*, JSObject* owner);
    void setInput(ExecState*, JSObject* owner, JSString*);

    JSString* leftContext(ExecState*, JSObject* owner);
    JSString* rightContext(ExecState*, JSObject* owner);

    JSString* input()
    {
        return m_reified ? m_reifiedInput.get() : m_lastInput.get();
    }

    void visitChildren(SlotVisitor&);

    static ptrdiff_t offsetOfLastRegExp() { return OBJECT_OFFSETOF(RegExpCachedResult, m_lastRegExp); }
    static ptrdiff_t offsetOfLastInput() { return OBJECT_OFFSETOF(RegExpCachedResult, m_lastInput); }
    static ptrdiff_t offsetOfResult() { return OBJECT_OFFSETOF(RegExpCachedResult, m_result); }
    static ptrdiff_t offsetOfReified() { return OBJECT_OFFSETOF(RegExpCachedResult, m_reified); }

private:
    MatchResult m_result;
    bool m_reified;
    WriteBarrier<JSString> m_lastInput;
    WriteBarrier<RegExp> m_lastRegExp;
    WriteBarrier<JSArray> m_reifiedResult;
    WriteBarrier<JSString> m_reifiedInput;
    WriteBarrier<JSString> m_reifiedLeftContext;
    WriteBarrier<JSString> m_reifiedRightContext;
};

} // namespace JSC
