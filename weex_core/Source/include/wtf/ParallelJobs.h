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
#ifndef ParallelJobs_h
#define ParallelJobs_h

#include <wtf/Assertions.h>
#include <wtf/Noncopyable.h>
#include <wtf/RefPtr.h>
#include <wtf/Vector.h>

// Usage:
//
//     // Initialize parallel jobs
//     ParallelJobs<TypeOfParameter> parallelJobs(&worker [, requestedNumberOfJobs]);
//
//     // Fill the parameter array
//     for(i = 0; i < parallelJobs.numberOfJobs(); ++i) {
//       TypeOfParameter& params = parallelJobs.parameter(i);
//       params.attr1 = localVars ...
//       ...
//     }
//
//     // Execute parallel jobs
//     parallelJobs.execute();
//

#if ENABLE(THREADING_GENERIC)
#include <wtf/ParallelJobsGeneric.h>

#elif ENABLE(THREADING_OPENMP)
#include <wtf/ParallelJobsOpenMP.h>

#elif ENABLE(THREADING_LIBDISPATCH)
#include <wtf/ParallelJobsLibdispatch.h>

#else
#error "No parallel processing API for ParallelJobs"

#endif

namespace WTF {

template<typename Type>
class ParallelJobs {
    WTF_MAKE_FAST_ALLOCATED;
public:
    typedef void (*WorkerFunction)(Type*);

    ParallelJobs(WorkerFunction func, int requestedJobNumber) :
        m_parallelEnvironment(reinterpret_cast<ParallelEnvironment::ThreadFunction>(func), sizeof(Type), requestedJobNumber)
    {
        m_parameters.grow(m_parallelEnvironment.numberOfJobs());
        ASSERT(numberOfJobs() == m_parameters.size());
    }

    size_t numberOfJobs()
    {
        return m_parameters.size();
    }

    Type& parameter(size_t i)
    {
        return m_parameters[i];
    }

    void execute()
    {
        m_parallelEnvironment.execute(reinterpret_cast<unsigned char*>(m_parameters.data()));
    }

private:
    ParallelEnvironment m_parallelEnvironment;
    Vector<Type> m_parameters;
};

} // namespace WTF

using WTF::ParallelJobs;

#endif // ParallelJobs_h
