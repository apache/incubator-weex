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
#ifndef ParallelJobsGeneric_h
#define ParallelJobsGeneric_h

#if ENABLE(THREADING_GENERIC)

#include <wtf/Condition.h>
#include <wtf/Lock.h>
#include <wtf/RefCounted.h>
#include <wtf/Threading.h>

namespace WTF {

class ParallelEnvironment {
    WTF_MAKE_FAST_ALLOCATED;
public:
    typedef void (*ThreadFunction)(void*);

    WTF_EXPORT_PRIVATE ParallelEnvironment(ThreadFunction, size_t sizeOfParameter, int requestedJobNumber);

    int numberOfJobs()
    {
        return m_numberOfJobs;
    }

    WTF_EXPORT_PRIVATE void execute(void* parameters);

    class ThreadPrivate : public RefCounted<ThreadPrivate> {
    public:
        ThreadPrivate()
            : m_threadID(0)
            , m_running(false)
            , m_parent(0)
        {
        }

        bool tryLockFor(ParallelEnvironment*);

        void execute(ThreadFunction, void*);

        void waitForFinish();

        static Ref<ThreadPrivate> create()
        {
            return adoptRef(*new ThreadPrivate());
        }

        static void workerThread(void*);

    private:
        ThreadIdentifier m_threadID;
        bool m_running;
        ParallelEnvironment* m_parent;

        mutable Lock m_mutex;
        Condition m_threadCondition;

        ThreadFunction m_threadFunction;
        void* m_parameters;
    };

private:
    ThreadFunction m_threadFunction;
    size_t m_sizeOfParameter;
    int m_numberOfJobs;

    Vector< RefPtr<ThreadPrivate> > m_threads;
    static Vector< RefPtr<ThreadPrivate> >* s_threadPool;
};

} // namespace WTF

#endif // ENABLE(THREADING_GENERIC)


#endif // ParallelJobsGeneric_h
