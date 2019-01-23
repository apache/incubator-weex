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

