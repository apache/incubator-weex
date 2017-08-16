/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
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

// On Mac, you can build this like so:
// xcrun clang++ -o LockSpeedTest Source/WTF/benchmarks/LockSpeedTest.cpp -O3 -W -ISource/WTF -ISource/WTF/benchmarks -LWebKitBuild/Release -lWTF -framework Foundation -licucore -std=c++11 -fvisibility=hidden

#include "config.h"

#include "ToyLocks.h"
#include <thread>
#include <unistd.h>
#include <wtf/CurrentTime.h>
#include <wtf/DataLog.h>
#include <wtf/HashMap.h>
#include <wtf/Lock.h>
#include <wtf/ParkingLot.h>
#include <wtf/StdLibExtras.h>
#include <wtf/Threading.h>
#include <wtf/ThreadingPrimitives.h>
#include <wtf/Vector.h>
#include <wtf/WordLock.h>
#include <wtf/text/CString.h>

namespace {

unsigned numThreadGroups;
unsigned numThreadsPerGroup;
unsigned workPerCriticalSection;
unsigned workBetweenCriticalSections;
double secondsPerTest;
    
NO_RETURN void usage()
{
    printf("Usage: LockSpeedTest yieldspinlock|pausespinlock|wordlock|lock|barginglock|bargingwordlock|thunderlock|thunderwordlock|cascadelock|cascadewordlockhandofflock|mutex|all <num thread groups> <num threads per group> <work per critical section> <work between critical sections> <spin limit> <seconds per test>\n");
    exit(1);
}

template<typename Type>
struct WithPadding {
    Type value;
    char buf[300]; // It's best if this isn't perfect to avoid false sharing.
};

HashMap<CString, Vector<double>> results;

void reportResult(const char* name, double value)
{
    dataLogF("%s: %.3lf KHz\n", name, value);
    results.add(name, Vector<double>()).iterator->value.append(value);
}

struct Benchmark {
    template<typename LockType>
    static void run(const char* name)
    {
        std::unique_ptr<WithPadding<LockType>[]> locks = std::make_unique<WithPadding<LockType>[]>(numThreadGroups);
        std::unique_ptr<WithPadding<double>[]> words = std::make_unique<WithPadding<double>[]>(numThreadGroups);
        std::unique_ptr<ThreadIdentifier[]> threads = std::make_unique<ThreadIdentifier[]>(numThreadGroups * numThreadsPerGroup);

        volatile bool keepGoing = true;

        double before = monotonicallyIncreasingTime();
    
        Lock numIterationsLock;
        uint64_t numIterations = 0;
    
        for (unsigned threadGroupIndex = numThreadGroups; threadGroupIndex--;) {
            words[threadGroupIndex].value = 0;

            for (unsigned threadIndex = numThreadsPerGroup; threadIndex--;) {
                threads[threadGroupIndex * numThreadsPerGroup + threadIndex] = createThread(
                    "Benchmark thread",
                    [threadGroupIndex, &locks, &words, &keepGoing, &numIterationsLock, &numIterations] () {
                        double localWord = 0;
                        double value = 1;
                        unsigned myNumIterations = 0;
                        while (keepGoing) {
                            locks[threadGroupIndex].value.lock();
                            for (unsigned j = workPerCriticalSection; j--;) {
                                words[threadGroupIndex].value += value;
                                words[threadGroupIndex].value *= 1.01;
                                value = words[threadGroupIndex].value;
                            }
                            locks[threadGroupIndex].value.unlock();
                            for (unsigned j = workBetweenCriticalSections; j--;) {
                                localWord += value;
                                localWord *= 1.01;
                                value = localWord;
                            }
                            myNumIterations++;
                        }
                        LockHolder locker(numIterationsLock);
                        numIterations += myNumIterations;
                    });
            }
        }

        sleep(secondsPerTest);
        keepGoing = false;
    
        for (unsigned threadIndex = numThreadGroups * numThreadsPerGroup; threadIndex--;)
            waitForThreadCompletion(threads[threadIndex]);

        double after = monotonicallyIncreasingTime();
    
        reportResult(name, numIterations / (after - before) / 1000);
    }
};

unsigned rangeMin;
unsigned rangeMax;
unsigned rangeStep;
unsigned* rangeVariable;

bool parseValue(const char* string, unsigned* variable)
{
    unsigned myRangeMin;
    unsigned myRangeMax;
    unsigned myRangeStep;
    if (sscanf(string, "%u-%u:%u", &myRangeMin, &myRangeMax, &myRangeStep) == 3) {
        if (rangeVariable) {
            fprintf(stderr, "Can only have one variable with a range.\n");
            return false;
        }
        
        rangeMin = myRangeMin;
        rangeMax = myRangeMax;
        rangeStep = myRangeStep;
        rangeVariable = variable;
        return true;
    }
    
    if (sscanf(string, "%u", variable) == 1)
        return true;
    
    return false;
}

} // anonymous namespace

int main(int argc, char** argv)
{
    WTF::initializeThreading();
    
    if (argc != 8
        || !parseValue(argv[2], &numThreadGroups)
        || !parseValue(argv[3], &numThreadsPerGroup)
        || !parseValue(argv[4], &workPerCriticalSection)
        || !parseValue(argv[5], &workBetweenCriticalSections)
        || !parseValue(argv[6], &toyLockSpinLimit)
        || sscanf(argv[7], "%lf", &secondsPerTest) != 1)
        usage();
    
    if (rangeVariable) {
        dataLog("Running with rangeMin = ", rangeMin, ", rangeMax = ", rangeMax, ", rangeStep = ", rangeStep, "\n");
        for (unsigned value = rangeMin; value <= rangeMax; value += rangeStep) {
            dataLog("Running with value = ", value, "\n");
            *rangeVariable = value;
            runEverything<Benchmark>(argv[1]);
        }
    } else
        runEverything<Benchmark>(argv[1]);
    
    for (auto& entry : results) {
        printf("%s = {", entry.key.data());
        bool first = true;
        for (double value : entry.value) {
            if (first)
                first = false;
            else
                printf(", ");
            printf("%.3lf", value);
        }
        printf("};\n");
    }

    return 0;
}
