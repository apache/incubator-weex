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

#include "config.h"
#include "JSRemoteInspector.h"

#include "JSGlobalObjectConsoleClient.h"

#if ENABLE(REMOTE_INSPECTOR)
#include "RemoteInspector.h"
#endif

using namespace Inspector;

static bool remoteInspectionEnabledByDefault = true;

void JSRemoteInspectorDisableAutoStart(void)
{
#if ENABLE(REMOTE_INSPECTOR)
    RemoteInspector::startDisabled();
#endif
}

void JSRemoteInspectorStart(void)
{
#if ENABLE(REMOTE_INSPECTOR)
    RemoteInspector::singleton();
#endif
}

void JSRemoteInspectorSetParentProcessInformation(pid_t pid, const uint8_t* auditData, size_t auditLength)
{
#if ENABLE(REMOTE_INSPECTOR) && PLATFORM(COCOA)
    RetainPtr<CFDataRef> auditDataRef = adoptCF(CFDataCreate(kCFAllocatorDefault, auditData, auditLength));
    RemoteInspector::singleton().setParentProcessInformation(pid, auditDataRef);
#else
    UNUSED_PARAM(pid);
    UNUSED_PARAM(auditData);
    UNUSED_PARAM(auditLength);
#endif
}

void JSRemoteInspectorSetLogToSystemConsole(bool logToSystemConsole)
{
    JSGlobalObjectConsoleClient::setLogToSystemConsole(logToSystemConsole);
}

bool JSRemoteInspectorGetInspectionEnabledByDefault(void)
{
    return remoteInspectionEnabledByDefault;
}

void JSRemoteInspectorSetInspectionEnabledByDefault(bool enabledByDefault)
{
    remoteInspectionEnabledByDefault = enabledByDefault;
}
