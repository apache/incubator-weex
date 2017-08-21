/*
 * Copyright (C) 2008 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <ctype.h>
#include <errno.h>
#include <grp.h>
#include <math.h>
#include <mntent.h>
#include <netdb.h>
#include <pthread.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

namespace WeexCore {
int sigemptyset(sigset_t* set)
{
    if (set == NULL) {
        errno = EINVAL;
        return -1;
    }
    memset(set, 0, sizeof(sigset_t));
    return 0;
}

int sigfillset(sigset_t* set)
{
    if (set == NULL) {
        errno = EINVAL;
        return -1;
    }
    memset(set, ~0, sizeof(sigset_t));
    return 0;
}

int sigaddset(sigset_t* set, int signum)
{
    int bit = signum - 1; // Signal numbers start at 1, but bit positions start at 0.
    unsigned long* local_set = reinterpret_cast<unsigned long*>(set);
    if (set == NULL || bit < 0 || bit >= static_cast<int>(8 * sizeof(sigset_t))) {
        errno = EINVAL;
        return -1;
    }
    local_set[bit / LONG_BIT] |= 1UL << (bit % LONG_BIT);
    return 0;
}

int sigdelset(sigset_t* set, int signum)
{
    int bit = signum - 1; // Signal numbers start at 1, but bit positions start at 0.
    unsigned long* local_set = reinterpret_cast<unsigned long*>(set);
    if (set == NULL || bit < 0 || bit >= static_cast<int>(8 * sizeof(sigset_t))) {
        errno = EINVAL;
        return -1;
    }
    local_set[bit / LONG_BIT] &= ~(1UL << (bit % LONG_BIT));
    return 0;
}

int getpagesize()
{
    // We dont use sysconf(3) here because that drags in stdio, which makes static binaries fat.
    return PAGE_SIZE;
}

void srandom(unsigned int __s) { srand48(__s); }

int isinf(double n)
{
    return __builtin_isinf(n);
}

int posix_memalign(void** memptr, size_t alignment, size_t bytes)
{
    if (!memptr)
        return EINVAL;
    *memptr = memalign(alignment, bytes);
    if (!*memptr)
        return ENOMEM;
    return 0;
}

int sigismember(const sigset_t* set, int signum)
{
    int bit = signum - 1; // Signal numbers start at 1, but bit positions start at 0.
    const unsigned long* local_set = reinterpret_cast<const unsigned long*>(set);
    if (set == NULL || bit < 0 || bit >= static_cast<int>(8 * sizeof(sigset_t))) {
        errno = EINVAL;
        return -1;
    }
    return static_cast<int>((local_set[bit / LONG_BIT] >> (bit % LONG_BIT)) & 1);
}

int abs(int j)
{
	return(j < 0 ? -j : j);
}

long labs(long j)
{
	return(j < 0 ? -j : j);
}

}