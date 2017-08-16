/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "CompareAndSwapTest.h"

#include <stdio.h>
#include <wtf/Atomics.h>
#include <wtf/Threading.h>

class Bitmap {
public:
    Bitmap() { clearAll(); }

    inline void clearAll();
    inline bool concurrentTestAndSet(size_t n);
    inline size_t numBits() const { return words * wordSize; }

private:
    static const size_t Size = 4096*10;

    static const unsigned wordSize = sizeof(uint8_t) * 8;
    static const unsigned words = (Size + wordSize - 1) / wordSize;
    static const uint8_t one = 1;

    uint8_t bits[words];
};

inline void Bitmap::clearAll()
{
    memset(&bits, 0, sizeof(bits));
}

inline bool Bitmap::concurrentTestAndSet(size_t n)
{
    uint8_t mask = one << (n % wordSize);
    size_t index = n / wordSize;
    uint8_t* wordPtr = &bits[index];
    uint8_t oldValue;
    do {
        oldValue = *wordPtr;
        if (oldValue & mask)
            return true;
    } while (!WTF::atomicCompareExchangeWeakRelaxed(wordPtr, oldValue, static_cast<uint8_t>(oldValue | mask)));
    return false;
}

struct Data {
    Bitmap* bitmap;
    int id;
    int numThreads;
};

static void setBitThreadFunc(void* p)
{
    Data* data = reinterpret_cast<Data*>(p);
    Bitmap* bitmap = data->bitmap;
    size_t numBits = bitmap->numBits();

    // The computed start index here is heuristic that seems to maximize (anecdotally)
    // the chance for the CAS issue to manifest.
    size_t start = (numBits * (data->numThreads - data->id)) / data->numThreads;

    printf("   started Thread %d\n", data->id);
    for (size_t i = start; i < numBits; i++)
        while (!bitmap->concurrentTestAndSet(i)) { }
    for (size_t i = 0; i < start; i++)
        while (!bitmap->concurrentTestAndSet(i)) { }

    printf("   finished Thread %d\n", data->id);
}

void testCompareAndSwap()
{
    Bitmap bitmap;
    const int numThreads = 5;
    ThreadIdentifier threadIDs[numThreads];
    Data data[numThreads];

    WTF::initializeThreading();
    
    printf("Starting %d threads for CompareAndSwap test.  Test should complete without hanging.\n", numThreads);
    for (int i = 0; i < numThreads; i++) {
        data[i].bitmap = &bitmap;
        data[i].id = i;
        data[i].numThreads = numThreads;
        std::function<void()> threadFunc = std::bind(setBitThreadFunc, &data[i]);
        threadIDs[i] = createThread("setBitThreadFunc", threadFunc);
    }

    printf("Waiting for %d threads to join\n", numThreads);
    for (int i = 0; i < numThreads; i++)
        waitForThreadCompletion(threadIDs[i]);

    printf("PASS: CompareAndSwap test completed without a hang\n");
}
