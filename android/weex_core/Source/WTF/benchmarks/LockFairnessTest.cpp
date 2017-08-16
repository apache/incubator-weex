/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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
// xcrun clang++ -o LockFairnessTest Source/WTF/benchmarks/LockFairnessTest.cpp -O3 -W -ISource/WTF -ISource/WTF/benchmarks -LWebKitBuild/Release -lWTF -framework Foundation -licucore -std=c++11 -fvisibility=hidden

#include "config.h"

#include "ToyLocks.h"
#include <thread>
#include <unistd.h>
#include <wtf/CommaPrinter.h>
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

NO_RETURN void usage()
{
    printf("Usage: LockFairnessTest yieldspinlock|pausespinlock|wordlock|lock|barginglock|bargingwordlock|thunderlock|thunderwordlock|cascadelock|cascadewordlockhandofflock|mutex|all <num threads> <seconds per test> <microseconds in critical section>\n");
    exit(1);
}

unsigned numThreads;
double secondsPerTest;
unsigned microsecondsInCriticalSection;

struct Benchmark {
    template<typename LockType>
    static void run(const char* name)
    {
        LockType lock;
        std::unique_ptr<unsigned[]> counts = std::make_unique<unsigned[]>(numThreads);
        std::unique_ptr<ThreadIdentifier[]> threads = std::make_unique<ThreadIdentifier[]>(numThreads);
    
        volatile bool keepGoing = true;
    
        lock.lock();
    
        for (unsigned threadIndex = numThreads; threadIndex--;) {
            counts[threadIndex] = 0;
            threads[threadIndex] = createThread(
                "Benchmark Thread",
                [&, threadIndex] () {
                    if (!microsecondsInCriticalSection) {
                        while (keepGoing) {
                            lock.lock();
                            counts[threadIndex]++;
                            lock.unlock();
                        }
                        return;
                    }
                    
                    while (keepGoing) {
                        lock.lock();
                        counts[threadIndex]++;
                        usleep(microsecondsInCriticalSection);
                        lock.unlock();
                    }
                });
        }
    
        sleepMS(100);
        lock.unlock();
    
        sleep(secondsPerTest);
    
        keepGoing = false;
        lock.lock();
    
        dataLog(name, ": ");
        CommaPrinter comma;
        for (unsigned threadIndex = numThreads; threadIndex--;)
            dataLog(comma, counts[threadIndex]);
        dataLog("\n");
    
        lock.unlock();
        for (unsigned threadIndex = numThreads; threadIndex--;)
            waitForThreadCompletion(threads[threadIndex]);
    }
};

} // anonymous namespace

int main(int argc, char** argv)
{
    WTF::initializeThreading();
    
    if (argc != 5
        || sscanf(argv[2], "%u", &numThreads) != 1
        || sscanf(argv[3], "%lf", &secondsPerTest) != 1
        || sscanf(argv[4], "%u", &microsecondsInCriticalSection) != 1)
        usage();
    
    runEverything<Benchmark>(argv[1]);
    
    return 0;
}
