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
#ifndef WTF_RandomNumberSeed_h
#define WTF_RandomNumberSeed_h

#include "RandomNumber.h"
#include <stdlib.h>
#include <time.h>

#if HAVE(SYS_TIME_H)
#include <sys/time.h>
#endif

#if OS(UNIX)
#include <sys/types.h>
#include <unistd.h>
#endif

namespace WTF {

inline void initializeRandomNumberGenerator()
{
#if OS(DARWIN)
    // On Darwin we use arc4random which initialises itself.
#elif COMPILER(MSVC) && defined(_CRT_RAND_S)
    // On Windows we use rand_s which initialises itself
#elif OS(UNIX)
    // srandomdev is not guaranteed to exist on linux so we use this poor seed, this should be improved
    timeval time;
    gettimeofday(&time, 0);
    srandom(static_cast<unsigned>(time.tv_usec * getpid()));
#else
    srand(static_cast<unsigned>(time(0)));
#endif

}

}

#endif
