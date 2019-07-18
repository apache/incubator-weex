/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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

