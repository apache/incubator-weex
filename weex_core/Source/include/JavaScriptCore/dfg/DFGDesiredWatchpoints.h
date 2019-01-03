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

#if ENABLE(DFG_JIT)

#include "CodeOrigin.h"
#include "DFGCommonData.h"
#include "DFGDesiredInferredType.h"
#include "InferredValue.h"
#include "JSArrayBufferView.h"
#include "ObjectPropertyCondition.h"
#include "Watchpoint.h"
#include <wtf/CommaPrinter.h>
#include <wtf/HashSet.h>

namespace JSC { namespace DFG {

class Graph;

template<typename T>
struct SetPointerAdaptor {
    static void add(CodeBlock* codeBlock, T set, CommonData& common)
    {
        return set->add(common.watchpoints.add(codeBlock));
    }
    static bool hasBeenInvalidated(T set) { return set->hasBeenInvalidated(); }
    static void dumpInContext(PrintStream& out, T set, DumpContext*)
    {
        out.print(RawPointer(set));
    }
};

struct InferredValueAdaptor {
    static void add(CodeBlock*, InferredValue*, CommonData&);
    static bool hasBeenInvalidated(InferredValue* inferredValue)
    {
        return inferredValue->hasBeenInvalidated();
    }
    static void dumpInContext(PrintStream& out, InferredValue* inferredValue, DumpContext*)
    {
        out.print(RawPointer(inferredValue));
    }
};

struct ArrayBufferViewWatchpointAdaptor {
    static void add(CodeBlock*, JSArrayBufferView*, CommonData&);
    static bool hasBeenInvalidated(JSArrayBufferView* view)
    {
        return !view->length();
    }
    static void dumpInContext(PrintStream& out, JSArrayBufferView* view, DumpContext* context)
    {
        out.print(inContext(JSValue(view), context));
    }
};

struct AdaptiveStructureWatchpointAdaptor {
    static void add(CodeBlock*, const ObjectPropertyCondition&, CommonData&);
    static bool hasBeenInvalidated(const ObjectPropertyCondition& key)
    {
        return !key.isWatchable();
    }
    static void dumpInContext(
        PrintStream& out, const ObjectPropertyCondition& key, DumpContext* context)
    {
        out.print(inContext(key, context));
    }
};

struct InferredTypeAdaptor {
    static void add(CodeBlock*, const DesiredInferredType&, CommonData&);
    static bool hasBeenInvalidated(const DesiredInferredType& key)
    {
        return !key.isStillValid();
    }
    static void dumpInContext(PrintStream& out, const DesiredInferredType& key, DumpContext* context)
    {
        out.print(inContext(key, context));
    }
};

template<typename WatchpointSetType, typename Adaptor = SetPointerAdaptor<WatchpointSetType>>
class GenericDesiredWatchpoints {
#if !ASSERT_DISABLED
    typedef HashMap<WatchpointSetType, bool> StateMap;
#endif
public:
    GenericDesiredWatchpoints()
        : m_reallyAdded(false)
    {
    }
    
    void addLazily(const WatchpointSetType& set)
    {
        m_sets.add(set);
    }
    
    void reallyAdd(CodeBlock* codeBlock, CommonData& common)
    {
        RELEASE_ASSERT(!m_reallyAdded);
        
        for (auto& set : m_sets)
            Adaptor::add(codeBlock, set, common);
        
        m_reallyAdded = true;
    }
    
    bool areStillValid() const
    {
        for (auto& set : m_sets) {
            if (Adaptor::hasBeenInvalidated(set))
                return false;
        }
        
        return true;
    }
    
    bool isWatched(const WatchpointSetType& set) const
    {
        return m_sets.contains(set);
    }

    void dumpInContext(PrintStream& out, DumpContext* context) const
    {
        CommaPrinter comma;
        for (const WatchpointSetType& entry : m_sets) {
            out.print(comma);
            Adaptor::dumpInContext(out, entry, context);
        }
    }

private:
    HashSet<WatchpointSetType> m_sets;
    bool m_reallyAdded;
};

class DesiredWatchpoints {
public:
    DesiredWatchpoints();
    ~DesiredWatchpoints();
    
    void addLazily(WatchpointSet*);
    void addLazily(InlineWatchpointSet&);
    void addLazily(InferredValue*);
    void addLazily(JSArrayBufferView*);
    
    // It's recommended that you don't call this directly. Use Graph::watchCondition(), which does
    // the required GC magic as well as some other bookkeeping.
    void addLazily(const ObjectPropertyCondition&);

    // It's recommended that you don't call this directly. Use Graph::inferredTypeFor(), which does
    // the required GC magic.
    void addLazily(const DesiredInferredType&);
    
    bool consider(Structure*);
    
    void reallyAdd(CodeBlock*, CommonData&);
    
    bool areStillValid() const;
    
    bool isWatched(WatchpointSet* set)
    {
        return m_sets.isWatched(set);
    }
    bool isWatched(InlineWatchpointSet& set)
    {
        return m_inlineSets.isWatched(&set);
    }
    bool isWatched(InferredValue* inferredValue)
    {
        return m_inferredValues.isWatched(inferredValue);
    }
    bool isWatched(JSArrayBufferView* view)
    {
        return m_bufferViews.isWatched(view);
    }
    bool isWatched(const ObjectPropertyCondition& key)
    {
        return m_adaptiveStructureSets.isWatched(key);
    }
    bool isWatched(const DesiredInferredType& key)
    {
        return m_inferredTypes.isWatched(key);
    }

    void dumpInContext(PrintStream&, DumpContext*) const;
    void dump(PrintStream&) const;
    
private:
    GenericDesiredWatchpoints<WatchpointSet*> m_sets;
    GenericDesiredWatchpoints<InlineWatchpointSet*> m_inlineSets;
    GenericDesiredWatchpoints<InferredValue*, InferredValueAdaptor> m_inferredValues;
    GenericDesiredWatchpoints<JSArrayBufferView*, ArrayBufferViewWatchpointAdaptor> m_bufferViews;
    GenericDesiredWatchpoints<ObjectPropertyCondition, AdaptiveStructureWatchpointAdaptor> m_adaptiveStructureSets;
    GenericDesiredWatchpoints<DesiredInferredType, InferredTypeAdaptor> m_inferredTypes;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
