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

#include "ConcurrentJSLock.h"
#include "Heap.h"
#include "JSArray.h"
#include "SpeculatedType.h"
#include "Structure.h"
#include "TagRegistersMode.h"
#include "WriteBarrier.h"
#include <wtf/PrintStream.h>
#include <wtf/StringPrintStream.h>

namespace JSC {

template<unsigned numberOfBucketsArgument>
struct ValueProfileBase {
    static const unsigned numberOfBuckets = numberOfBucketsArgument;
    static const unsigned numberOfSpecFailBuckets = 1;
    static const unsigned bucketIndexMask = numberOfBuckets - 1;
    static const unsigned totalNumberOfBuckets = numberOfBuckets + numberOfSpecFailBuckets;
    
    ValueProfileBase()
        : m_bytecodeOffset(-1)
        , m_prediction(SpecNone)
        , m_numberOfSamplesInPrediction(0)
    {
        for (unsigned i = 0; i < totalNumberOfBuckets; ++i)
            m_buckets[i] = JSValue::encode(JSValue());
    }
    
    ValueProfileBase(int bytecodeOffset)
        : m_bytecodeOffset(bytecodeOffset)
        , m_prediction(SpecNone)
        , m_numberOfSamplesInPrediction(0)
    {
        for (unsigned i = 0; i < totalNumberOfBuckets; ++i)
            m_buckets[i] = JSValue::encode(JSValue());
    }
    
    EncodedJSValue* specFailBucket(unsigned i)
    {
        ASSERT(numberOfBuckets + i < totalNumberOfBuckets);
        return m_buckets + numberOfBuckets + i;
    }
    
    const ClassInfo* classInfo(unsigned bucket) const
    {
#if 1
            EncodedJSValue tmp = __atomic_load_n(&m_buckets[bucket], __ATOMIC_RELAXED);
            JSValue value = JSValue::decode(tmp);
#else
            JSValue value = JSValue::decode(m_buckets[bucket]);
#endif // WTF_ARM_ARCH_VERSION == 7
        if (!!value) {
            if (!value.isCell())
                return 0;
            return value.asCell()->structure()->classInfo();
        }
        return 0;
    }
    
    unsigned numberOfSamples() const
    {
        unsigned result = 0;
        for (unsigned i = 0; i < totalNumberOfBuckets; ++i) {
#if 1
            EncodedJSValue tmp = __atomic_load_n(&m_buckets[i], __ATOMIC_RELAXED);
            JSValue value = JSValue::decode(tmp);
#else
            JSValue value = JSValue::decode(m_buckets[i]);
#endif // WTF_ARM_ARCH_VERSION == 7
            if (!!value)
                result++;
        }
        return result;
    }
    
    unsigned totalNumberOfSamples() const
    {
        return numberOfSamples() + m_numberOfSamplesInPrediction;
    }
    
    bool isLive() const
    {
        for (unsigned i = 0; i < totalNumberOfBuckets; ++i) {
#if 1
            EncodedJSValue tmp = __atomic_load_n(&m_buckets[i], __ATOMIC_RELAXED);
            JSValue value = JSValue::decode(tmp);
#else
            JSValue value = JSValue::decode(m_buckets[i]);
#endif // WTF_ARM_ARCH_VERSION == 7
            if (!!value)
                return true;
        }
        return false;
    }
    
    CString briefDescription(const ConcurrentJSLocker& locker)
    {
        computeUpdatedPrediction(locker);
        
        StringPrintStream out;
        out.print("predicting ", SpeculationDump(m_prediction));
        return out.toCString();
    }
    
    void dump(PrintStream& out)
    {
        out.print("samples = ", totalNumberOfSamples(), " prediction = ", SpeculationDump(m_prediction));
        bool first = true;
        for (unsigned i = 0; i < totalNumberOfBuckets; ++i) {
#if 1
            EncodedJSValue tmp = __atomic_load_n(&m_buckets[i], __ATOMIC_RELAXED);
            JSValue value = JSValue::decode(tmp);
#else
            JSValue value = JSValue::decode(m_buckets[i]);
#endif // WTF_ARM_ARCH_VERSION == 7
            if (!!value) {
                if (first) {
                    out.printf(": ");
                    first = false;
                } else
                    out.printf(", ");
                out.print(value);
            }
        }
    }
    
    // Updates the prediction and returns the new one. Never call this from any thread
    // that isn't executing the code.
    SpeculatedType computeUpdatedPrediction(const ConcurrentJSLocker&)
    {
        for (unsigned i = 0; i < totalNumberOfBuckets; ++i) {
#if 1
            EncodedJSValue tmp = __atomic_load_n(&m_buckets[i], __ATOMIC_RELAXED);
            JSValue value = JSValue::decode(tmp);
#else
            JSValue value = JSValue::decode(m_buckets[i]);
#endif // WTF_ARM_ARCH_VERSION == 7
            if (!value)
                continue;
            
            m_numberOfSamplesInPrediction++;
            mergeSpeculation(m_prediction, speculationFromValue(value));
#if 1
            __atomic_store_n(&m_buckets[i], JSValue::encode(JSValue()), __ATOMIC_SEQ_CST);
#else
            m_buckets[i] = JSValue::encode(JSValue());
#endif // WTF_ARM_ARCH_VERSION == 7
        }
        WTF::memoryBarrierBeforeUnlock();
        
        return m_prediction;
    }
    
    int m_bytecodeOffset; // -1 for prologue
    
    SpeculatedType m_prediction;
    unsigned m_numberOfSamplesInPrediction;
    
    EncodedJSValue m_buckets[totalNumberOfBuckets];
};

struct MinimalValueProfile : public ValueProfileBase<0> {
    MinimalValueProfile(): ValueProfileBase<0>() { }
    MinimalValueProfile(int bytecodeOffset): ValueProfileBase<0>(bytecodeOffset) { }
};

template<unsigned logNumberOfBucketsArgument>
struct ValueProfileWithLogNumberOfBuckets : public ValueProfileBase<1 << logNumberOfBucketsArgument> {
    static const unsigned logNumberOfBuckets = logNumberOfBucketsArgument;
    
    ValueProfileWithLogNumberOfBuckets()
        : ValueProfileBase<1 << logNumberOfBucketsArgument>()
    {
    }
    ValueProfileWithLogNumberOfBuckets(int bytecodeOffset)
        : ValueProfileBase<1 << logNumberOfBucketsArgument>(bytecodeOffset)
    {
    }
};

struct ValueProfile : public ValueProfileWithLogNumberOfBuckets<0> {
    ValueProfile(): ValueProfileWithLogNumberOfBuckets<0>() { }
    ValueProfile(int bytecodeOffset): ValueProfileWithLogNumberOfBuckets<0>(bytecodeOffset) { }
};

template<typename T>
inline int getValueProfileBytecodeOffset(T* valueProfile)
{
    return valueProfile->m_bytecodeOffset;
}

// This is a mini value profile to catch pathologies. It is a counter that gets
// incremented when we take the slow path on any instruction.
struct RareCaseProfile {
    RareCaseProfile(int bytecodeOffset)
        : m_bytecodeOffset(bytecodeOffset)
        , m_counter(0)
    {
    }
    
    int m_bytecodeOffset;
    uint32_t m_counter;
};

inline int getRareCaseProfileBytecodeOffset(RareCaseProfile* rareCaseProfile)
{
    return rareCaseProfile->m_bytecodeOffset;
}

} // namespace JSC
