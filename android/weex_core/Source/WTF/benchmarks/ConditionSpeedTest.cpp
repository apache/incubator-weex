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
// clang++ -o ConditionSpeedTest Source/WTF/benchmarks/ConditionSpeedTest.cpp -O3 -W -ISource/WTF -LWebKitBuild/Release -lWTF -framework Foundation -licucore -std=c++11

#include "config.h"

#include <mutex>
#include <thread>
#include <unistd.h>
#include <wtf/Condition.h>
#include <wtf/CurrentTime.h>
#include <wtf/DataLog.h>
#include <wtf/Deque.h>
#include <wtf/Lock.h>
#include <wtf/StdLibExtras.h>
#include <wtf/StringPrintStream.h>
#include <wtf/Threading.h>
#include <wtf/ThreadingPrimitives.h>
#include <wtf/Vector.h>

namespace {

const bool verbose = false;

unsigned numProducers;
unsigned numConsumers;
unsigned maxQueueSize;
unsigned numMessagesPerProducer;
    
NO_RETURN void usage()
{
    printf("Usage: ConditionSpeedTest lock|mutex|all <num producers> <num consumers> <max queue size> <num messages per producer>\n");
    exit(1);
}

template<typename Functor, typename ConditionType, typename LockType>
void wait(ConditionType& condition, LockType& lock, const Functor& predicate)
{
    while (!predicate())
        condition.wait(lock);
}

template<typename LockType, typename ConditionType, typename NotifyFunctor, typename NotifyAllFunctor>
void runTest(
    unsigned numProducers,
    unsigned numConsumers,
    unsigned maxQueueSize,
    unsigned numMessagesPerProducer,
    const NotifyFunctor& notify,
    const NotifyAllFunctor& notifyAll)
{
    Deque<unsigned> queue;
    bool shouldContinue = true;
    LockType lock;
    ConditionType emptyCondition;
    ConditionType fullCondition;

    Vector<ThreadIdentifier> consumerThreads;
    Vector<ThreadIdentifier> producerThreads;

    Vector<unsigned> received;
    LockType receivedLock;
    
    for (unsigned i = numConsumers; i--;) {
        ThreadIdentifier threadIdentifier = createThread(
            "Consumer thread",
            [&] () {
                for (;;) {
                    unsigned result;
                    unsigned mustNotify = false;
                    {
                        std::unique_lock<LockType> locker(lock);
                        wait(
                            emptyCondition, lock,
                            [&] () {
                                if (verbose)
                                    dataLog(toString(currentThread(), ": Checking consumption predicate with shouldContinue = ", shouldContinue, ", queue.size() == ", queue.size(), "\n"));
                                return !shouldContinue || !queue.isEmpty();
                            });
                        if (!shouldContinue && queue.isEmpty())
                            return;
                        mustNotify = queue.size() == maxQueueSize;
                        result = queue.takeFirst();
                    }
                    notify(fullCondition, mustNotify);

                    {
                        std::lock_guard<LockType> locker(receivedLock);
                        received.append(result);
                    }
                }
            });
        consumerThreads.append(threadIdentifier);
    }

    for (unsigned i = numProducers; i--;) {
        ThreadIdentifier threadIdentifier = createThread(
            "Producer Thread",
            [&] () {
                for (unsigned i = 0; i < numMessagesPerProducer; ++i) {
                    bool mustNotify = false;
                    {
                        std::unique_lock<LockType> locker(lock);
                        wait(
                            fullCondition, lock,
                            [&] () {
                                if (verbose)
                                    dataLog(toString(currentThread(), ": Checking production predicate with shouldContinue = ", shouldContinue, ", queue.size() == ", queue.size(), "\n"));
                                return queue.size() < maxQueueSize;
                            });
                        mustNotify = queue.isEmpty();
                        queue.append(i);
                    }
                    notify(emptyCondition, mustNotify);
                }
            });
        producerThreads.append(threadIdentifier);
    }

    for (ThreadIdentifier threadIdentifier : producerThreads)
        waitForThreadCompletion(threadIdentifier);

    {
        std::lock_guard<LockType> locker(lock);
        shouldContinue = false;
    }
    notifyAll(emptyCondition);

    for (ThreadIdentifier threadIdentifier : consumerThreads)
        waitForThreadCompletion(threadIdentifier);

    RELEASE_ASSERT(numProducers * numMessagesPerProducer == received.size());
    std::sort(received.begin(), received.end());
    for (unsigned messageIndex = 0; messageIndex < numMessagesPerProducer; ++messageIndex) {
        for (unsigned producerIndex = 0; producerIndex < numProducers; ++producerIndex)
            RELEASE_ASSERT(messageIndex == received[messageIndex * numProducers + producerIndex]);
    }
}

template<typename LockType, typename ConditionType, typename NotifyFunctor, typename NotifyAllFunctor>
void runBenchmark(
    const char* name,
    const NotifyFunctor& notify,
    const NotifyAllFunctor& notifyAll)
{
    double before = monotonicallyIncreasingTimeMS();
    
    runTest<LockType, ConditionType>(
        numProducers,
        numConsumers,
        maxQueueSize,
        numMessagesPerProducer,
        notify,
        notifyAll);

    double after = monotonicallyIncreasingTimeMS();

    printf("%s: %.3lf ms.\n", name, after - before);
}

} // anonymous namespace

int main(int argc, char** argv)
{
    WTF::initializeThreading();

    if (argc != 6
        || sscanf(argv[2], "%u", &numProducers) != 1
        || sscanf(argv[3], "%u", &numConsumers) != 1
        || sscanf(argv[4], "%u", &maxQueueSize) != 1
        || sscanf(argv[5], "%u", &numMessagesPerProducer) != 1)
        usage();

    bool didRun = false;
    if (!strcmp(argv[1], "lock") || !strcmp(argv[1], "all")) {
        runBenchmark<Lock, Condition>(
            "WTF Lock NotifyOne",
            [&] (Condition& condition, bool mustNotify) {
                condition.notifyOne();
            },
            [&] (Condition& condition) {
                condition.notifyAll();
            });
        runBenchmark<Lock, Condition>(
            "WTF Lock NotifyAll",
            [&] (Condition& condition, bool mustNotify) {
                if (mustNotify)
                    condition.notifyAll();
            },
            [&] (Condition& condition) {
                condition.notifyAll();
            });
        didRun = true;
    }
    if (!strcmp(argv[1], "mutex") || !strcmp(argv[1], "all")) {
        runBenchmark<Mutex, ThreadCondition>(
            "Platform Mutex NotifyOne",
            [&] (ThreadCondition& condition, bool mustNotify) {
                condition.signal();
            },
            [&] (ThreadCondition& condition) {
                condition.broadcast();
            });
        runBenchmark<Mutex, ThreadCondition>(
            "Platform Mutex NotifyAll",
            [&] (ThreadCondition& condition, bool mustNotify) {
                if (mustNotify)
                    condition.broadcast();
            },
            [&] (ThreadCondition& condition) {
                condition.broadcast();
            });
        didRun = true;
    }

    if (!didRun)
        usage();

    return 0;
}

