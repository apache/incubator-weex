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

#ifdef __APPLE__
#include <Availability.h>
#include <AvailabilityMacros.h>
#include <TargetConditionals.h>
#endif

#define BPLATFORM(PLATFORM) (defined BPLATFORM_##PLATFORM && BPLATFORM_##PLATFORM)
#define BOS(OS) (defined BOS_##OS && BOS_##OS)

#ifdef __APPLE__
#define BOS_DARWIN 1
#endif

#ifdef __unix
#define BOS_UNIX 1
#endif

#if BOS(DARWIN) && ((defined(TARGET_OS_EMBEDDED) && TARGET_OS_EMBEDDED) \
    || (defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE) \
    || (defined(TARGET_IPHONE_SIMULATOR) && TARGET_IPHONE_SIMULATOR))
#define BPLATFORM_IOS 1
#if (defined(TARGET_IPHONE_SIMULATOR) && TARGET_IPHONE_SIMULATOR)
#define BPLATFORM_IOS_SIMULATOR 1
#endif
#elif BOS(DARWIN) && defined(TARGET_OS_MAC) && TARGET_OS_MAC
#define BPLATFORM_MAC 1
#endif

/* ==== Policy decision macros: these define policy choices for a particular port. ==== */

/* BUSE() - use a particular third-party library or optional OS service */
#define BUSE(FEATURE) (defined BUSE_##FEATURE && BUSE_##FEATURE)

/* ==== Platform adaptation macros: these describe properties of the target environment. ==== */

/* BCPU() - the target CPU architecture */
#define BCPU(_FEATURE) (defined BCPU_##_FEATURE  && BCPU_##_FEATURE)

/* BCPU(X86) - i386 / x86 32-bit */
#if defined(__i386__) \
|| defined(i386)     \
|| defined(_M_IX86)  \
|| defined(_X86_)    \
|| defined(__THW_INTEL)
#define BCPU_X86 1
#endif

/* BCPU(X86_64) - AMD64 / Intel64 / x86_64 64-bit */
#if defined(__x86_64__) \
|| defined(_M_X64)
#define BCPU_X86_64 1
#endif

/* BCPU(ARM64) - Apple */
#if (defined(__arm64__) && defined(__APPLE__)) || defined(__aarch64__)
#define BCPU_ARM64 1
#endif

/* BCPU(ARM) - ARM, any version*/
#define BARM_ARCH_AT_LEAST(N) (BCPU(ARM) && BARM_ARCH_VERSION >= N)

#if   defined(arm) \
|| defined(__arm__) \
|| defined(ARM) \
|| defined(_ARM_)
#define BCPU_ARM 1

/* Set BARM_ARCH_VERSION */
#if   defined(__ARM_ARCH_4__) \
|| defined(__ARM_ARCH_4T__) \
|| defined(__MARM_ARMV4__)
#define BARM_ARCH_VERSION 4

#elif defined(__ARM_ARCH_5__) \
|| defined(__ARM_ARCH_5T__) \
|| defined(__MARM_ARMV5__)
#define BARM_ARCH_VERSION 5

#elif defined(__ARM_ARCH_5E__) \
|| defined(__ARM_ARCH_5TE__) \
|| defined(__ARM_ARCH_5TEJ__)
#define BARM_ARCH_VERSION 5

#elif defined(__ARM_ARCH_6__) \
|| defined(__ARM_ARCH_6J__) \
|| defined(__ARM_ARCH_6K__) \
|| defined(__ARM_ARCH_6Z__) \
|| defined(__ARM_ARCH_6ZK__) \
|| defined(__ARM_ARCH_6T2__) \
|| defined(__ARMV6__)
#define BARM_ARCH_VERSION 6

#elif defined(__ARM_ARCH_7A__) \
|| defined(__ARM_ARCH_7K__) \
|| defined(__ARM_ARCH_7R__) \
|| defined(__ARM_ARCH_7S__)
#define BARM_ARCH_VERSION 7

#elif defined(__ARM_ARCH_8__)
#define BARM_ARCH_VERSION 8

/* MSVC sets _M_ARM */
#elif defined(_M_ARM)
#define BARM_ARCH_VERSION _M_ARM

/* RVCT sets _TARGET_ARCH_ARM */
#elif defined(__TARGET_ARCH_ARM)
#define BARM_ARCH_VERSION __TARGET_ARCH_ARM

#else
#define WTF_ARM_ARCH_VERSION 0

#endif

/* Set BTHUMB_ARCH_VERSION */
#if   defined(__ARM_ARCH_4T__)
#define BTHUMB_ARCH_VERSION 1

#elif defined(__ARM_ARCH_5T__) \
|| defined(__ARM_ARCH_5TE__) \
|| defined(__ARM_ARCH_5TEJ__)
#define BTHUMB_ARCH_VERSION 2

#elif defined(__ARM_ARCH_6J__) \
|| defined(__ARM_ARCH_6K__) \
|| defined(__ARM_ARCH_6Z__) \
|| defined(__ARM_ARCH_6ZK__) \
|| defined(__ARM_ARCH_6M__)
#define BTHUMB_ARCH_VERSION 3

#elif defined(__ARM_ARCH_6T2__) \
|| defined(__ARM_ARCH_7__) \
|| defined(__ARM_ARCH_7A__) \
|| defined(__ARM_ARCH_7K__) \
|| defined(__ARM_ARCH_7M__) \
|| defined(__ARM_ARCH_7R__) \
|| defined(__ARM_ARCH_7S__)
#define BTHUMB_ARCH_VERSION 4

/* RVCT sets __TARGET_ARCH_THUMB */
#elif defined(__TARGET_ARCH_THUMB)
#define BTHUMB_ARCH_VERSION __TARGET_ARCH_THUMB

#else
#define BTHUMB_ARCH_VERSION 0
#endif

/* BCPU(ARM_TRADITIONAL) - Thumb2 is not available, only traditional ARM (v4 or greater) */
/* BCPU(ARM_THUMB2) - Thumb2 instruction set is available */
/* Only one of these will be defined. */
#if !defined(BCPU_ARM_TRADITIONAL) && !defined(BCPU_ARM_THUMB2)
#  if defined(thumb2) || defined(__thumb2__) \
|| ((defined(__thumb) || defined(__thumb__)) && BTHUMB_ARCH_VERSION == 4)
#    define BCPU_ARM_TRADITIONAL 0
#    define BCPU_ARM_THUMB2 1
#  elif BARM_ARCH_AT_LEAST(4)
#    define BCPU_ARM_TRADITIONAL 1
#    define BCPU_ARM_THUMB2 0
#  else
#    error "Not supported ARM architecture"
#  endif
#elif BCPU(ARM_TRADITIONAL) && BCPU(ARM_THUMB2) /* Sanity Check */
#  error "Cannot use both of BCPU_ARM_TRADITIONAL and BCPU_ARM_THUMB2 platforms"
#endif /* !defined(BCPU_ARM_TRADITIONAL) && !defined(BCPU_ARM_THUMB2) */

#endif /* ARM */

#define BATTRIBUTE_PRINTF(formatStringArgument, extraArguments) __attribute__((__format__(printf, formatStringArgument, extraArguments)))

#if (BPLATFORM(MAC) && __MAC_OS_X_VERSION_MIN_REQUIRED >= 101200) || (BPLATFORM(IOS) && __IPHONE_OS_VERSION_MIN_REQUIRED >= 100000)
#define BUSE_OS_LOG 1
#endif
