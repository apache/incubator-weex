/*
 * Copyright (C) 2015-2017 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "JSCell.h"
#include "StackTrace.h"
#include "Structure.h"
#include <wtf/MonotonicTime.h>

namespace JSC {

struct CellProfile {
    enum Liveness {
        Unknown,
        Dead,
        Live
    };

    CellProfile(HeapCell* cell, HeapCell::Kind kind, Liveness liveness)
        : m_cell(cell)
        , m_kind(kind)
        , m_liveness(liveness)
        , m_timestamp(MonotonicTime::now())
    {
        if (m_kind == HeapCell::JSCell && m_liveness != Dead)
            m_className = jsCell()->structure()->classInfo()->className;
    }

    CellProfile(CellProfile&& other)
        : m_cell(other.m_cell)
        , m_kind(other.m_kind)
        , m_liveness(other.m_liveness)
        , m_timestamp(other.m_timestamp)
        , m_className(other.m_className)
        , m_stackTrace(WTFMove(other.m_stackTrace))
    { }

    HeapCell* cell() const { return m_cell; }
    JSCell* jsCell() const
    {
        ASSERT(isJSCell());
        return static_cast<JSCell*>(m_cell);
    }

    bool isJSCell() const { return m_kind == HeapCell::JSCell; }
    
    HeapCell::Kind kind() const { return m_kind; }

    bool isLive() const { return m_liveness == Live; }
    bool isDead() const { return m_liveness == Dead; }

    void setIsLive() { m_liveness = Live; }
    void setIsDead() { m_liveness = Dead; }

    MonotonicTime timestamp() const { return m_timestamp; }

    const char* className() const { return m_className; }

    StackTrace* stackTrace() const { return m_stackTrace.get(); }
    void setStackTrace(StackTrace* trace) { m_stackTrace = std::unique_ptr<StackTrace>(trace); }

private:
    HeapCell* m_cell;
    HeapCell::Kind m_kind;
    Liveness m_liveness { Unknown };
    MonotonicTime m_timestamp;
    const char* m_className { nullptr };
    std::unique_ptr<StackTrace> m_stackTrace;
};

} // namespace JSC
