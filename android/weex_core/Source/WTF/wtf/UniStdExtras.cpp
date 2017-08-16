/*
 * Copyright (C) 2016 Igalia S.L.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "UniStdExtras.h"

#include <fcntl.h>

namespace WTF {

bool setCloseOnExec(int fileDescriptor)
{
    int returnValue = -1;
    do {
        int flags = fcntl(fileDescriptor, F_GETFD);
        if (flags != -1)
            returnValue = fcntl(fileDescriptor, F_SETFD, flags | FD_CLOEXEC);
    } while (returnValue == -1 && errno == EINTR);

    return returnValue != -1;
}

int dupCloseOnExec(int fileDescriptor)
{
    int duplicatedFileDescriptor = -1;
#ifdef F_DUPFD_CLOEXEC
    while ((duplicatedFileDescriptor = fcntl(fileDescriptor, F_DUPFD_CLOEXEC, 0)) == -1 && errno == EINTR) { }
    if (duplicatedFileDescriptor != -1)
        return duplicatedFileDescriptor;

#endif

    while ((duplicatedFileDescriptor = dup(fileDescriptor)) == -1 && errno == EINTR) { }
    if (duplicatedFileDescriptor == -1)
        return -1;

    if (!setCloseOnExec(duplicatedFileDescriptor)) {
        closeWithRetry(duplicatedFileDescriptor);
        return -1;
    }

    return duplicatedFileDescriptor;
}

} // namespace WTF
