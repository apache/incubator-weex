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

#include <wtf/DataLog.h>
#include <wtf/HashSet.h>
#include <wtf/LoggingHashID.h>
#include <wtf/LoggingHashTraits.h>

namespace WTF {

template<
    const char* typeArguments,
    typename ValueArg, typename HashArg = typename DefaultHash<ValueArg>::Hash,
    typename TraitsArg = HashTraits<ValueArg>,
    typename LoggingTraits = LoggingHashKeyTraits<ValueArg>>
class LoggingHashSet final {
    WTF_MAKE_FAST_ALLOCATED;

    typedef TraitsArg ValueTraits;
    typedef typename ValueTraits::TakeType TakeType;
    
public:
    typedef WTF::HashSet<ValueArg, HashArg, TraitsArg> HashSet;
    
    typedef typename HashSet::ValueType ValueType;
    typedef typename HashSet::iterator iterator;
    typedef typename HashSet::const_iterator const_iterator;
    typedef typename HashSet::AddResult AddResult;
    
    LoggingHashSet()
    {
        dataLog("auto* ", m_id, " = new HashSet<", typeArguments, ">();\n");
    }
    
    ~LoggingHashSet()
    {
        dataLog("delete ", m_id, ";\n");
    }
    
    LoggingHashSet(const LoggingHashSet& other)
        : m_set(other.m_set)
    {
        dataLog("auto* ", m_id, " = new HashSet<", typeArguments, ">(*", other.m_id, ");\n");
    }
    
    LoggingHashSet(LoggingHashSet&& other)
        : m_set(other.m_set)
    {
        dataLog("auto* ", m_id, " = new HashSet<", typeArguments, ">(WTFMove(*", other.m_id, "));\n");
    }
    
    LoggingHashSet& operator=(const LoggingHashSet& other)
    {
        dataLog("*", m_id, " = *", other.m_id, ";\n");
        m_set = other.m_set;
        return *this;
    }
    
    LoggingHashSet& operator=(LoggingHashSet&& other)
    {
        dataLog("*", m_id, " = WTFMove(*", other.m_id, ");\n");
        m_set = WTFMove(other.m_set);
        return *this;
    }
    
    void swap(LoggingHashSet& other)
    {
        dataLog(m_id, "->swap(*", other.m_id, ");\n");
        m_set.swap(other.m_set);
    }
    
    unsigned size() const { return m_set.size(); }
    unsigned capacity() const { return m_set.capacity(); }
    bool isEmpty() const { return m_set.isEmpty(); }
    
    iterator begin() const { return m_set.begin(); }
    iterator end() const { return m_set.end(); }
    
    iterator find(const ValueType& value) const
    {
        StringPrintStream string;
        string.print("{\n");
        string.print("    auto iter = ", m_id, "->find(");
        LoggingTraits::print(string, value);
        string.print(");\n");
        iterator result = m_set.find(value);
        if (result == m_set.end())
            string.print("    RELEASE_ASSERT(iter == ", m_id, "->end());\n");
        else
            string.print("    RELEASE_ASSERT(iter != ", m_id, "->end());\n");
        string.print("}\n");
        dataLog(string.toCString());
        return result;
    }
    
    bool contains(const ValueType& value) const
    {
        return find(value) != end();
    }
    
    // FIXME: Implement the translator versions of find() and friends.
    
    AddResult add(const ValueType& value)
    {
        StringPrintStream string;
        string.print(m_id, "->add(");
        LoggingTraits::print(string, value);
        string.print(");\n");
        dataLog(string.toCString());
        return m_set.add(value);
    }

    AddResult add(ValueType&& value)
    {
        StringPrintStream string;
        string.print(m_id, "->add(");
        LoggingTraits::print(string, value);
        string.print(");\n");
        dataLog(string.toCString());
        return m_set.add(WTFMove(value));
    }
    
    void addVoid(const ValueType& value)
    {
        StringPrintStream string;
        string.print(m_id, "->addVoid(");
        LoggingTraits::print(string, value);
        string.print(");\n");
        dataLog(string.toCString());
        m_set.addVoid(value);
    }

    void addVoid(ValueType&& value)
    {
        StringPrintStream string;
        string.print(m_id, "->addVoid(");
        LoggingTraits::print(string, value);
        string.print(");\n");
        dataLog(string.toCString());
        m_set.addVoid(WTFMove(value));
    }
    
    template<typename IteratorType>
    bool add(IteratorType begin, IteratorType end)
    {
        bool changed = false;
        for (IteratorType iter = begin; iter != end; ++iter)
            changed |= add(*iter).isNewEntry;
        return changed;
    }
    
    bool remove(iterator iter)
    {
        // FIXME: We should do much better than this!
        if (iter == end())
            return false;
        return remove(*iter);
    }
    
    bool remove(const ValueType& value)
    {
        StringPrintStream string;
        string.print(m_id, "->remove(");
        LoggingTraits::print(string, value);
        string.print(");\n");
        dataLog(string.toCString());
        return m_set.remove(value);
    }
    
    // FIXME: Implement removeIf
    
    void clear()
    {
        dataLog(m_id, "->clear();\n");
        m_set.clear();
    }
    
    TakeType take(const ValueType& value)
    {
        StringPrintStream string;
        string.print(m_id, "->remove(");
        LoggingTraits::print(string, value);
        string.print(");\n");
        dataLog(string.toCString());
        return m_set.take(value);
    }
    
    TakeType take(iterator iter)
    {
        return take(*iter);
    }
    
    TakeType takeAny()
    {
        dataLog(m_id, "->takeAny();\n");
        return m_set.takeAny();
    }
    
    template<typename OtherCollection>
    bool operator==(const OtherCollection& otherCollection) const
    {
        if (size() != otherCollection.size())
            return false;
        for (const auto& other : otherCollection) {
            if (!contains(other))
                return false;
        }
        return true;
    }
    
    template<typename OtherCollection>
    bool operator!=(const OtherCollection& other) const
    {
        return !(*this == other);
    }
    
private:
    HashSet m_set;
    LoggingHashID m_id;
};

} // namespace WTF

using WTF::LoggingHashSet;
