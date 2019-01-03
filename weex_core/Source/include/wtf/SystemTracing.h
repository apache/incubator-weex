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

#if USE(APPLE_INTERNAL_SDK)
#include <System/sys/kdebug.h>
#define HAVE_KDEBUG_H 1
#endif

// No namespaces because this file has to be includable from C and Objective-C.

// Reserved component code. Do not change this.
#define WEBKIT_COMPONENT 47

// Trace point codes can be up to 14 bits (0-16383).
// When adding or changing these codes, update Tools/Tracing/SystemTracePoints.plist to match.
enum TracePointCode {
    WTFRange = 0,

    JavaScriptRange = 2500,
    VMEntryScopeStart,
    VMEntryScopeEnd,

    WebCoreRange = 5000,
    StyleRecalcStart,
    StyleRecalcEnd,
    LayoutStart,
    LayoutEnd,
    PaintViewStart,
    PaintViewEnd,
    PaintLayerStart,
    PaintLayerEnd,
    RAFDisplayLinkScheduled,
    RAFDisplayLinkFired,
    RAFCallbackStart,
    RAFCallbackEnd,

    WebKitRange = 10000,
    WebKit2Range = 12000,

    RAFDidUpdateStart,
    RAFDidUpdateEnd,
    RAFBackingStoreFlushStart,
    RAFBackingStoreFlushEnd,
    RAFBuildTransactionStart,
    RAFBuildTransactionEnd,

    UIProcessRange = 14000,

    RAFCommitLayerTreeStart,
    RAFCommitLayerTreeEnd,
    RAFDidRefreshDisplayStart,
    RAFDidRefreshDisplayEnd,
};

#ifdef __cplusplus

namespace WTF {

inline void TracePoint(TracePointCode code, uint64_t data1 = 0, uint64_t data2 = 0, uint64_t data3 = 0, uint64_t data4 = 0)
{
#if HAVE(KDEBUG_H)
    kdebug_trace(ARIADNEDBG_CODE(WEBKIT_COMPONENT, code), data1, data2, data3, data4);
#else
    UNUSED_PARAM(code);
    UNUSED_PARAM(data1);
    UNUSED_PARAM(data2);
    UNUSED_PARAM(data3);
    UNUSED_PARAM(data4);
#endif
}

class TraceScope {
public:

    TraceScope(TracePointCode entryCode, TracePointCode exitCode)
        : m_exitCode(exitCode)
    {
        TracePoint(entryCode);
    }

    ~TraceScope()
    {
        TracePoint(m_exitCode);
    }

private:
    TracePointCode m_exitCode;
};

} // namespace WTF

using WTF::TraceScope;
using WTF::TracePoint;

#endif // __cplusplus
