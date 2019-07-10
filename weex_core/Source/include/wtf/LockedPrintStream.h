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

#pragma once

#include <wtf/PrintStream.h>
#include <wtf/RecursiveLockAdapter.h>
#include <wtf/WordLock.h>

namespace WTF {

// Makes every call to print() atomic.
class LockedPrintStream : public PrintStream {
public:
    LockedPrintStream(std::unique_ptr<PrintStream> target);
    virtual ~LockedPrintStream();
    
    void vprintf(const char* format, va_list) override WTF_ATTRIBUTE_PRINTF(2, 0);
    void flush() override;

protected:
    PrintStream& begin() override;
    void end() override;

private:
    // This needs to be a recursive lock because a printInternal or dump method could assert,
    // and that assert might want to log. Better to let it. This needs to be a WordLock so that
    // LockedPrintStream (i.e. cataLog) can be used to debug ParkingLot and Lock.
    RecursiveLockAdapter<WordLock> m_lock;
    std::unique_ptr<PrintStream> m_target;
};

} // namespace WTF

using WTF::LockedPrintStream;

