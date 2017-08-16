/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
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

#include "SourceCode.h"
#include "TieredMMapArray.h"
#include <wtf/text/CString.h>

namespace JSC {

class CodeProfile {
    WTF_MAKE_FAST_ALLOCATED;
public:
    CodeProfile(const SourceCode& source, CodeProfile* parent)
        : m_file(source.provider()->url().utf8())
        , m_lineNumber(source.firstLine().oneBasedInt())
        , m_parent(parent)
    {
        if (parent)
            parent->addChild(std::unique_ptr<CodeProfile>(this));
    }

    void sample(void* pc, void** framePointer);
    void report();

    CodeProfile* parent()
    {
        return m_parent;
    }
    
    void addChild(std::unique_ptr<CodeProfile> child)
    {
        m_children.append(WTFMove(child));
    }

private:
    enum CodeType {
        EngineCode,
        GlobalThunk,
        RegExpCode,
        DFGJIT,
        BaselineOnly,
        BaselineProfile,
        BaselineOSR,
        EngineFrame,
        NumberOfCodeTypes
    };
    struct CodeRecord {
        CodeRecord(void* pc, CodeType type)
            : pc(pc)
            , type(type)
        {
        }
        void* pc;
        CodeType type;
    };

    CString m_file;
    unsigned m_lineNumber;
    CodeProfile* m_parent;
    Vector<std::unique_ptr<CodeProfile>> m_children;
    TieredMMapArray<CodeRecord> m_samples;

    static const char* s_codeTypeNames[NumberOfCodeTypes];
};

} // namespace JSC
