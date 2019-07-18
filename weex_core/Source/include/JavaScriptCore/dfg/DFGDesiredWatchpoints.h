/*
 * Copyright (C) 2013-2015 Apple Inc. All rights reserved.
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
