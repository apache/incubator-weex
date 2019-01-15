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
