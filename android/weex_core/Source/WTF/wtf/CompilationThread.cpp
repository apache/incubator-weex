/*
 * Copyright (C) 2013, 2016 Apple Inc. All rights reserved.
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

#include "config.h"
#include "CompilationThread.h"

#include "StdLibExtras.h"
#include "ThreadSpecific.h"
#include "Threading.h"
#include <mutex>

namespace WTF {

static ThreadSpecific<bool, CanBeGCThread::True>* s_isCompilationThread;

static void initializeCompilationThreads()
{
    static std::once_flag initializeCompilationThreadsOnceFlag;
    std::call_once(initializeCompilationThreadsOnceFlag, []{
        s_isCompilationThread = new ThreadSpecific<bool, CanBeGCThread::True>();
    });
}

bool isCompilationThread()
{
    if (!s_isCompilationThread)
        return false;
    if (!s_isCompilationThread->isSet())
        return false;
    return **s_isCompilationThread;
}

bool exchangeIsCompilationThread(bool newValue)
{
    initializeCompilationThreads();
    bool oldValue = isCompilationThread();
    **s_isCompilationThread = newValue;
    return oldValue;
}

} // namespace WTF

