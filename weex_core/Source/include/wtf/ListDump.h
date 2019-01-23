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
#ifndef ListDump_h
#define ListDump_h

#include "CommaPrinter.h"
#include "PrintStream.h"
#include "StringPrintStream.h"

namespace WTF {

template<typename T>
class ListDump {
public:
    ListDump(const T& list, const char* comma)
        : m_list(list)
        , m_comma(comma)
    {
    }
    
    void dump(PrintStream& out) const
    {
        for (auto iter = m_list.begin(); iter != m_list.end(); ++iter)
            out.print(m_comma, *iter);
    }

private:
    const T& m_list;
    CommaPrinter m_comma;
};

template<typename T>
class PointerListDump {
public:
    PointerListDump(const T& list, const char* comma)
        : m_list(list)
        , m_comma(comma)
    {
    }
    
    void dump(PrintStream& out) const
    {
        for (auto iter = m_list.begin(); iter != m_list.end(); ++iter)
            out.print(m_comma, pointerDump(*iter));
    }

private:
    const T& m_list;
    CommaPrinter m_comma;
};

template<typename T>
class MapDump {
public:
    MapDump(const T& map, const char* arrow, const char* comma)
        : m_map(map)
        , m_arrow(arrow)
        , m_comma(comma)
    {
    }
    
    void dump(PrintStream& out) const
    {
        for (auto iter = m_map.begin(); iter != m_map.end(); ++iter)
            out.print(m_comma, iter->key, m_arrow, iter->value);
    }
    
private:
    const T& m_map;
    const char* m_arrow;
    CommaPrinter m_comma;
};

template<typename T>
ListDump<T> listDump(const T& list, const char* comma = ", ")
{
    return ListDump<T>(list, comma);
}

template<typename T>
PointerListDump<T> pointerListDump(const T& list, const char* comma = ", ")
{
    return PointerListDump<T>(list, comma);
}

template<typename T, typename Comparator>
CString sortedListDump(const T& list, const Comparator& comparator, const char* comma = ", ")
{
    Vector<typename T::ValueType> myList;
    myList.appendRange(list.begin(), list.end());
    std::sort(myList.begin(), myList.end(), comparator);
    StringPrintStream out;
    CommaPrinter commaPrinter(comma);
    for (unsigned i = 0; i < myList.size(); ++i)
        out.print(commaPrinter, myList[i]);
    return out.toCString();
}

template<typename T>
CString sortedListDump(const T& list, const char* comma = ", ")
{
    return sortedListDump(list, std::less<typename T::ValueType>(), comma);
}

template<typename T>
MapDump<T> mapDump(const T& map, const char* arrow = "=>", const char* comma = ", ")
{
    return MapDump<T>(map, arrow, comma);
}

template<typename T, typename Comparator>
CString sortedMapDump(const T& map, const Comparator& comparator, const char* arrow = "=>", const char* comma = ", ")
{
    Vector<typename T::KeyType> keys;
    for (auto iter = map.begin(); iter != map.end(); ++iter)
        keys.append(iter->key);
    std::sort(keys.begin(), keys.end(), comparator);
    StringPrintStream out;
    CommaPrinter commaPrinter(comma);
    for (unsigned i = 0; i < keys.size(); ++i)
        out.print(commaPrinter, keys[i], arrow, map.get(keys[i]));
    return out.toCString();
}

template<typename T, typename U>
class ListDumpInContext {
public:
    ListDumpInContext(const T& list, U* context, const char* comma)
        : m_list(list)
        , m_context(context)
        , m_comma(comma)
    {
    }
    
    void dump(PrintStream& out) const
    {
        for (auto iter = m_list.begin(); iter != m_list.end(); ++iter)
            out.print(m_comma, inContext(*iter, m_context));
    }

private:
    const T& m_list;
    U* m_context;
    CommaPrinter m_comma;
};

template<typename T, typename U>
ListDumpInContext<T, U> listDumpInContext(
    const T& list, U* context, const char* comma = ", ")
{
    return ListDumpInContext<T, U>(list, context, comma);
}

} // namespace WTF

using WTF::listDump;
using WTF::listDumpInContext;
using WTF::mapDump;
using WTF::pointerListDump;
using WTF::sortedListDump;
using WTF::sortedMapDump;

#endif // ListDump_h

