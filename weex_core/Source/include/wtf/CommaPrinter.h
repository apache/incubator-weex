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
#ifndef CommaPrinter_h
#define CommaPrinter_h

#include "PrintStream.h"

namespace WTF {

class CommaPrinter {
public:
    CommaPrinter(const char* comma = ", ", const char* start = "")
        : m_comma(comma)
        , m_start(start)
        , m_didPrint(false)
    {
    }
    
    void dump(PrintStream& out) const
    {
        if (!m_didPrint) {
            out.print(m_start);
            m_didPrint = true;
            return;
        }
        
        out.print(m_comma);
    }
    
    bool didPrint() const { return m_didPrint; }
    
private:
    const char* m_comma;
    const char* m_start;
    mutable bool m_didPrint;
};

} // namespace WTF

using WTF::CommaPrinter;

#endif // CommaPrinter_h

