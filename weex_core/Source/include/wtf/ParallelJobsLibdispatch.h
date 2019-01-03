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
#ifndef ParallelJobsLibdispatch_h
#define ParallelJobsLibdispatch_h

#if ENABLE(THREADING_LIBDISPATCH)

#include <dispatch/dispatch.h>

namespace WTF {

class ParallelEnvironment {
    WTF_MAKE_FAST_ALLOCATED;
public:
    typedef void (*ThreadFunction)(void*);

    ParallelEnvironment(ThreadFunction threadFunction, size_t sizeOfParameter, int requestedJobNumber)
        : m_threadFunction(threadFunction)
        , m_sizeOfParameter(sizeOfParameter)
        , m_numberOfJobs(requestedJobNumber)
    {
        // We go with the requested number of jobs. libdispatch will distribute the work optimally.
        ASSERT_ARG(requestedJobNumber, requestedJobNumber > 0);
    }

    int numberOfJobs()
    {
        return m_numberOfJobs;
    }

    void execute(unsigned char* parameters)
    {
        static dispatch_queue_t globalQueue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);

        dispatch_apply(m_numberOfJobs, globalQueue, ^(size_t i) { (*m_threadFunction)(parameters + (m_sizeOfParameter * i)); });
    }

private:
    ThreadFunction m_threadFunction;
    size_t m_sizeOfParameter;
    int m_numberOfJobs;
};

} // namespace WTF

#endif // ENABLE(THREADING_LIBDISPATCH)

#endif // ParallelJobsLibdispatch_h
