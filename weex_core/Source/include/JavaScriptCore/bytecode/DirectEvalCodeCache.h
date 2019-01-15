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

#include "DirectEvalExecutable.h"
#include <wtf/HashMap.h>
#include <wtf/RefPtr.h>
#include <wtf/text/StringHash.h>

namespace JSC {

    class SlotVisitor;

    class DirectEvalCodeCache {
    public:
        class CacheKey {
        public:
            CacheKey(const String& source, CallSiteIndex callSiteIndex)
                : m_source(source.impl())
                , m_callSiteIndex(callSiteIndex)
            {
            }

            CacheKey(WTF::HashTableDeletedValueType)
                : m_source(WTF::HashTableDeletedValue)
            {
            }

            CacheKey() = default;

            unsigned hash() const { return m_source->hash() ^ m_callSiteIndex.bits(); }

            bool isEmptyValue() const { return !m_source; }

            bool operator==(const CacheKey& other) const
            {
                return m_callSiteIndex == other.m_callSiteIndex && WTF::equal(m_source.get(), other.m_source.get());
            }

            bool isHashTableDeletedValue() const { return m_source.isHashTableDeletedValue(); }

            struct Hash {
                static unsigned hash(const CacheKey& key)
                {
                    return key.hash();
                }
                static bool equal(const CacheKey& lhs, const CacheKey& rhs)
                {
                    return lhs == rhs;
                }
                static const bool safeToCompareToEmptyOrDeleted = false;
            };

            typedef SimpleClassHashTraits<CacheKey> HashTraits;

        private:
            RefPtr<StringImpl> m_source;
            CallSiteIndex m_callSiteIndex;
        };

        DirectEvalExecutable* tryGet(const String& evalSource, CallSiteIndex callSiteIndex)
        {
            return m_cacheMap.fastGet(CacheKey(evalSource, callSiteIndex)).get();
        }
        
        void set(ExecState* exec, JSCell* owner, const String& evalSource, CallSiteIndex callSiteIndex, DirectEvalExecutable* evalExecutable)
        {
            if (m_cacheMap.size() < maxCacheEntries)
                setSlow(exec, owner, evalSource, callSiteIndex, evalExecutable);
        }

        bool isEmpty() const { return m_cacheMap.isEmpty(); }

        void visitAggregate(SlotVisitor&);

        void clear();

    private:
        static const int maxCacheEntries = 64;

        void setSlow(ExecState*, JSCell* owner, const String& evalSource, CallSiteIndex, DirectEvalExecutable*);

        typedef HashMap<CacheKey, WriteBarrier<DirectEvalExecutable>, CacheKey::Hash, CacheKey::HashTraits> EvalCacheMap;
        EvalCacheMap m_cacheMap;
        Lock m_lock;
    };

} // namespace JSC
