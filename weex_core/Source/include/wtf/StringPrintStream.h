/*
 * Copyright (C) 2012, 2016 Apple Inc. All rights reserved.
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

#ifndef StringPrintStream_h
#define StringPrintStream_h

#include <wtf/PrintStream.h>
#include <wtf/text/CString.h>
#include <wtf/text/WTFString.h>

namespace WTF {

class StringPrintStream : public PrintStream {
public:
    WTF_EXPORT_PRIVATE StringPrintStream();
    WTF_EXPORT_PRIVATE virtual ~StringPrintStream();
    
    WTF_EXPORT_PRIVATE void vprintf(const char* format, va_list) override WTF_ATTRIBUTE_PRINTF(2, 0);

    size_t length() const { return m_next; }
    
    WTF_EXPORT_PRIVATE CString toCString();
    WTF_EXPORT_PRIVATE String toString();
    WTF_EXPORT_PRIVATE String toStringWithLatin1Fallback();
    WTF_EXPORT_PRIVATE void reset();
    
private:
    void increaseSize(size_t);
    
    char* m_buffer;
    size_t m_next;
    size_t m_size;
    char m_inlineBuffer[128];
};

// Stringify any type T that has a WTF::printInternal(PrintStream&, const T&)

template<typename... Types>
CString toCString(const Types&... values)
{
    StringPrintStream stream;
    stream.print(values...);
    return stream.toCString();
}

template<typename... Types>
String toString(const Types&... values)
{
    StringPrintStream stream;
    stream.print(values...);
    return stream.toString();
}

} // namespace WTF

using WTF::StringPrintStream;
using WTF::toCString;
using WTF::toString;

#endif // StringPrintStream_h

