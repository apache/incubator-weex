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
#ifndef ParallelJobsOpenMP_h
#define ParallelJobsOpenMP_h

#if ENABLE(THREADING_OPENMP)

#include <omp.h>

namespace WTF {

class ParallelEnvironment {
    WTF_MAKE_NONCOPYABLE(ParallelEnvironment);
public:
    typedef void (*ThreadFunction)(void*);

    ParallelEnvironment(ThreadFunction threadFunction, size_t sizeOfParameter, int requestedJobNumber) :
        m_threadFunction(threadFunction),
        m_sizeOfParameter(sizeOfParameter)
    {
        int maxNumberOfThreads = omp_get_max_threads();

        if (!requestedJobNumber || requestedJobNumber > maxNumberOfThreads)
            requestedJobNumber = maxNumberOfThreads;

        ASSERT(requestedJobNumber > 0);

        m_numberOfJobs = requestedJobNumber;

    }

    int numberOfJobs()
    {
        return m_numberOfJobs;
    }

    void execute(unsigned char* parameters)
    {
        omp_set_num_threads(m_numberOfJobs);

#pragma omp parallel for
        for (int i = 0; i < m_numberOfJobs; ++i)
            (*m_threadFunction)(parameters + i * m_sizeOfParameter);
    }

private:
    ThreadFunction m_threadFunction;
    size_t m_sizeOfParameter;
    int m_numberOfJobs;
};

} // namespace WTF

#endif // ENABLE(THREADING_OPENMP)

#endif // ParallelJobsOpenMP_h
