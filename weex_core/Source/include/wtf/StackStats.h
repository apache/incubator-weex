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
#ifndef StackStats_h
#define StackStats_h

#include "ExportMacros.h"
#include <mutex>
#include <wtf/Lock.h>


// Define this flag to enable Stack stats collection. This feature is useful
// for getting a sample of native stack usage sizes.
//
// Enabling this will cause stats to be collected and written to a log file at
// various  instrumented points in the code. It will result in noticeable
// performance loss. Hence, this should only be enable when you want to do
// some stats location in your local build. This code is provided here as a
// convenience for collecting that data. It is not meant to be enabled by
// default on release or debug builds.

#define ENABLE_STACK_STATS 0


namespace WTF {

#if !ENABLE(STACK_STATS) 

class StackStats {
public:
    // The CheckPoint class is for marking check points corresponding
    // each location in code where a stack recursion check is being done.

    class CheckPoint {
    public:
        CheckPoint() { }
    };

    class PerThreadStats {
    public:
        PerThreadStats() { }
    };

    class LayoutCheckPoint {
    public:
        LayoutCheckPoint() { }
    };

    static void probe() { }
};

#else // ENABLE(STACK_STATS)

class StackStats {
public:
    // The CheckPoint class is for marking check points corresponding
    // each location in code where a stack recursion check is being done.

    class CheckPoint {
    public:
        CheckPoint();
        ~CheckPoint();
    private:
        CheckPoint* m_prev;
    };

    class PerThreadStats {
    public:
        PerThreadStats();

    private:
        int m_reentryDepth;
        char* m_stackStart;
        CheckPoint* m_currentCheckPoint;

        friend class CheckPoint;
        friend class StackStats;
    };

    class LayoutCheckPoint {
    public:
        WTF_EXPORT_PRIVATE LayoutCheckPoint();
        WTF_EXPORT_PRIVATE ~LayoutCheckPoint();

    private:
        LayoutCheckPoint* m_prev;
        int m_depth;
    };

    // Used for probing the stack at places where we suspect to be high
    // points of stack usage but are NOT check points where stack recursion
    // is checked.
    //
    // The more places where we add this probe, the more accurate our
    // stats data will be. However, adding too many probes will also
    // result in unnecessary performance loss. So, only add these probes
    // judiciously where appropriate.
    static void probe();

private:
    // CheckPoint management:
    static StaticLock s_sharedMutex;
    static CheckPoint* s_topCheckPoint;
    static LayoutCheckPoint* s_firstLayoutCheckPoint;
    static LayoutCheckPoint* s_topLayoutCheckPoint;

    // High watermark stats:
    static int s_maxCheckPointDiff;
    static int s_maxStackHeight;
    static int s_maxReentryDepth;

    static int s_maxLayoutCheckPointDiff;
    static int s_maxTotalLayoutCheckPointDiff;
    static int s_maxLayoutReentryDepth;

    friend class CheckPoint;
    friend class LayoutCheckPoint;
};

#endif // ENABLE(STACK_STATS) 

} // namespace WTF

using WTF::StackStats;

#endif // StackStats_h
