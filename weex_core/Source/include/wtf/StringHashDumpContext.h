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
#ifndef StringHashDumpContext_h
#define StringHashDumpContext_h

#include <wtf/HashMap.h>
#include <wtf/SixCharacterHash.h>
#include <wtf/StdLibExtras.h>
#include <wtf/StringPrintStream.h>
#include <wtf/text/CString.h>

namespace WTF {

template<typename T>
class StringHashDumpContext {
public:
    StringHashDumpContext() { }
    
    CString getID(const T* value)
    {
        typename HashMap<const T*, CString>::iterator iter = m_forwardMap.find(value);
        if (iter != m_forwardMap.end())
            return iter->value;
        
        for (unsigned hashValue = toCString(*value).hash(); ; hashValue++) {
            CString fullHash = integerToSixCharacterHashString(hashValue).data();
            
            for (unsigned length = 2; length < 6; ++length) {
                CString shortHash = CString(fullHash.data(), length);
                if (!m_backwardMap.contains(shortHash)) {
                    m_forwardMap.add(value, shortHash);
                    m_backwardMap.add(shortHash, value);
                    return shortHash;
                }
            }
        }
    }
    
    void dumpBrief(const T* value, PrintStream& out)
    {
        value->dumpBrief(out, getID(value));
    }
    
    CString brief(const T* value)
    {
        StringPrintStream out;
        dumpBrief(value, out);
        return out.toCString();
    }
    
    bool isEmpty() const { return m_forwardMap.isEmpty(); }
    
    void dump(PrintStream& out, const char* prefix = "") const
    {
        out.print(prefix);
        T::dumpContextHeader(out);
        out.print("\n");
        
        Vector<CString> keys;
        unsigned maxKeySize = 0;
        for (
            typename HashMap<CString, const T*>::const_iterator iter = m_backwardMap.begin();
            iter != m_backwardMap.end();
            ++iter) {
            keys.append(iter->key);
            maxKeySize = std::max(maxKeySize, static_cast<unsigned>(brief(iter->value, iter->key).length()));
        }
        
        std::sort(keys.begin(), keys.end());
        
        for (unsigned i = 0; i < keys.size(); ++i) {
            const T* value = m_backwardMap.get(keys[i]);
            out.print(prefix, "    ");
            CString briefString = brief(value, keys[i]);
            out.print(briefString);
            for (unsigned n = briefString.length(); n < maxKeySize; ++n)
                out.print(" ");
            out.print(" = ", *value, "\n");
        }
    }
    
public:
    static CString brief(const T* value, const CString& string)
    {
        StringPrintStream out;
        value->dumpBrief(out, string);
        return out.toCString();
    }
    
    HashMap<const T*, CString> m_forwardMap;
    HashMap<CString, const T*> m_backwardMap;
};

} // namespace WTF

using WTF::StringHashDumpContext;

#endif // StringHashDumpContext_h

