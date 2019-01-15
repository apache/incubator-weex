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

size_t __ctype_get_mb_cur_max(void){
    return 1;
}

long double strtold(const char*p, char**end) {
  return strtod(p, end);
}

float strtof(const char* p, char** end) {
  return (float) strtod(p, end);
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

double log2(double n)
{
    // log(n)/log(2) is log2
    return log(n) / log(2);
}
