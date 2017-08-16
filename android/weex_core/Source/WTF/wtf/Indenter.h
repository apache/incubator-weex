/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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

#ifndef Indenter_h
#define Indenter_h

#include <wtf/FilePrintStream.h>
#include <wtf/text/WTFString.h>

namespace WTF {

class Indenter {
public:
    Indenter(unsigned count = 0, String string = ASCIILiteral("  "))
        : m_count(count)
        , m_string(string)
    { }

    Indenter(const Indenter& other)
        : m_count(other.m_count)
        , m_string(other.m_string)
    { }

    void dump(PrintStream& out) const
    {
        unsigned levels = m_count;
        while (levels--)
            out.print(m_string);
    }

    unsigned operator++() { return ++m_count; }
    unsigned operator++(int) { return m_count++; }
    unsigned operator--() { return --m_count; }
    unsigned operator--(int) { return m_count--; }

private:
    unsigned m_count;
    String m_string;
};

} // namespace WTF

using WTF::Indenter;

#endif // Indenter_h
