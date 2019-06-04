/*
 * Copyright (C) 2006-2017 Apple Inc. All rights reserved.
 * Copyright (C) 2007-2009 Torch Mobile, Inc.
 * Copyright (C) 2010, 2011 Research In Motion Limited. All rights reserved.
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

#ifndef WTF_Platform_h
#define WTF_Platform_h

/* Include compiler specific macros */
#include <wtf/Compiler.h>

/* ==== PLATFORM handles OS, operating environment, graphics API, and
   CPU. This macro will be phased out in favor of platform adaptation
   macros, policy decision macros, and top-level port definitions. ==== */
#define PLATFORM(WTF_FEATURE) (defined WTF_PLATFORM_##WTF_FEATURE  && WTF_PLATFORM_##WTF_FEATURE)


/* ==== Platform adaptation macros: these describe properties of the target environment. ==== */

/* CPU() - the target CPU architecture */
#define CPU(WTF_FEATURE) (defined WTF_CPU_##WTF_FEATURE  && WTF_CPU_##WTF_FEATURE)
/* HAVE() - specific system features (headers, functions or similar) that are present or not */
#define HAVE(WTF_FEATURE) (defined HAVE_##WTF_FEATURE  && HAVE_##WTF_FEATURE)
/* OS() - underlying operating system; only to be used for mandated low-level services like 
   virtual memory, not to choose a GUI toolkit */
#define OS(WTF_FEATURE) (defined WTF_OS_##WTF_FEATURE  && WTF_OS_##WTF_FEATURE)


/* ==== Policy decision macros: these define policy choices for a particular port. ==== */

/* USE() - use a particular third-party library or optional OS service */
#define USE(WTF_FEATURE) (defined USE_##WTF_FEATURE  && USE_##WTF_FEATURE)
/* ENABLE() - turn on a specific feature of WebKit */
#define ENABLE(WTF_FEATURE) (defined ENABLE_##WTF_FEATURE  && ENABLE_##WTF_FEATURE)


/* ==== CPU() - the target CPU architecture ==== */

/* This also defines CPU(BIG_ENDIAN) or CPU(MIDDLE_ENDIAN) or neither, as appropriate. */

/* CPU(ALPHA) - DEC Alpha */
#if defined(__alpha__)
#define WTF_CPU_ALPHA 1
#endif

/* CPU(HPPA) - HP PA-RISC */
#if defined(__hppa__) || defined(__hppa64__)
#define WTF_CPU_HPPA 1
#define WTF_CPU_BIG_ENDIAN 1
#endif

/* CPU(IA64) - Itanium / IA-64 */
#if defined(__ia64__)
#define WTF_CPU_IA64 1
/* 32-bit mode on Itanium */
#if !defined(__LP64__)
#define WTF_CPU_IA64_32 1
#endif
#endif

/* CPU(MIPS) - MIPS 32-bit and 64-bit */
#if (defined(mips) || defined(__mips__) || defined(MIPS) || defined(_MIPS_) || defined(__mips64))
#if defined(_ABI64) && (_MIPS_SIM == _ABI64)
#define WTF_CPU_MIPS64 1
#define WTF_MIPS_ARCH __mips64
#else
#define WTF_CPU_MIPS 1
#define WTF_MIPS_ARCH __mips
#endif
#if defined(__MIPSEB__)
#define WTF_CPU_BIG_ENDIAN 1
#endif
#define WTF_MIPS_PIC (defined __PIC__)
#define WTF_MIPS_ISA(v) (defined WTF_MIPS_ARCH && WTF_MIPS_ARCH == v)
#define WTF_MIPS_ISA_AT_LEAST(v) (defined WTF_MIPS_ARCH && WTF_MIPS_ARCH >= v)
#define WTF_MIPS_ARCH_REV __mips_isa_rev
#define WTF_MIPS_ISA_REV(v) (defined WTF_MIPS_ARCH_REV && WTF_MIPS_ARCH_REV == v)
#define WTF_MIPS_DOUBLE_FLOAT (defined __mips_hard_float && !defined __mips_single_float)
#define WTF_MIPS_FP64 (defined __mips_fpr && __mips_fpr == 64)
/* MIPS requires allocators to use aligned memory */
#define USE_ARENA_ALLOC_ALIGNMENT_INTEGER 1
#endif /* MIPS */

/* CPU(PPC64) - PowerPC 64-bit Big Endian */
#if (  defined(__ppc64__)      \
    || defined(__PPC64__))     \
    && defined(__BYTE_ORDER__) \
    && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define WTF_CPU_PPC64 1
#define WTF_CPU_BIG_ENDIAN 1
#endif

/* CPU(PPC64) - PowerPC 64-bit Little Endian */
#if (   defined(__ppc64__)     \
    || defined(__PPC64__)      \
    || defined(__ppc64le__)    \
    || defined(__PPC64LE__))   \
    && defined(__BYTE_ORDER__) \
    && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define WTF_CPU_PPC64LE 1
#endif

/* CPU(PPC) - PowerPC 32-bit */
#if (  defined(__ppc__)        \
    || defined(__PPC__)        \
    || defined(__powerpc__)    \
    || defined(__powerpc)      \
    || defined(__POWERPC__)    \
    || defined(_M_PPC)         \
    || defined(__PPC))         \
    && !CPU(PPC64)             \
    && defined(__BYTE_ORDER__) \
    && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define WTF_CPU_PPC 1
#define WTF_CPU_BIG_ENDIAN 1
#endif

/* CPU(SH4) - SuperH SH-4 */
#if defined(__SH4__)
#define WTF_CPU_SH4 1
#endif

/* CPU(S390X) - S390 64-bit */
#if defined(__s390x__)
#define WTF_CPU_S390X 1
#define WTF_CPU_BIG_ENDIAN 1
#endif

/* CPU(S390) - S390 32-bit */
#if (  defined(__s390__)        \
    && !CPU(S390X))
#define WTF_CPU_S390 1
#define WTF_CPU_BIG_ENDIAN 1
#endif

/* CPU(X86) - i386 / x86 32-bit */
#if   defined(__i386__) \
    || defined(i386)     \
    || defined(_M_IX86)  \
    || defined(_X86_)    \
    || defined(__THW_INTEL)
#define WTF_CPU_X86 1

#if defined(__SSE2__) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
#define WTF_CPU_X86_SSE2 1
#endif

#endif

/* CPU(X86_64) - AMD64 / Intel64 / x86_64 64-bit */
#if   defined(__x86_64__) \
    || defined(_M_X64)
#define WTF_CPU_X86_64 1
#define WTF_CPU_X86_SSE2 1
#endif

/* CPU(ARM64) - Apple */
#if (defined(__arm64__) && defined(__APPLE__)) || defined(__aarch64__)
#define WTF_CPU_ARM64 1
#endif

/* CPU(ARM) - ARM, any version*/
#define WTF_ARM_ARCH_AT_LEAST(N) (CPU(ARM) && WTF_ARM_ARCH_VERSION >= N)

#if   defined(arm) \
    || defined(__arm__) \
    || defined(ARM) \
    || defined(_ARM_)
#define WTF_CPU_ARM 1

#if defined(__ARM_PCS_VFP)
#define WTF_CPU_ARM_HARDFP 1
#endif

#if defined(__ARMEB__)
#define WTF_CPU_BIG_ENDIAN 1

#elif !defined(__ARM_EABI__) \
    && !defined(__EABI__) \
    && !defined(__VFP_FP__) \
    && !defined(_WIN32_WCE)
#define WTF_CPU_MIDDLE_ENDIAN 1

#endif

/* Set WTF_ARM_ARCH_VERSION */
#if   defined(__ARM_ARCH_4__) \
    || defined(__ARM_ARCH_4T__) \
    || defined(__MARM_ARMV4__)
#define WTF_ARM_ARCH_VERSION 4

#elif defined(__ARM_ARCH_5__) \
    || defined(__ARM_ARCH_5T__) \
    || defined(__MARM_ARMV5__)
#define WTF_ARM_ARCH_VERSION 5

#elif defined(__ARM_ARCH_5E__) \
    || defined(__ARM_ARCH_5TE__) \
    || defined(__ARM_ARCH_5TEJ__)
#define WTF_ARM_ARCH_VERSION 5
/*ARMv5TE requires allocators to use aligned memory*/
#define USE_ARENA_ALLOC_ALIGNMENT_INTEGER 1

#elif defined(__ARM_ARCH_6__) \
    || defined(__ARM_ARCH_6J__) \
    || defined(__ARM_ARCH_6K__) \
    || defined(__ARM_ARCH_6Z__) \
    || defined(__ARM_ARCH_6ZK__) \
    || defined(__ARM_ARCH_6T2__) \
    || defined(__ARMV6__)
#define WTF_ARM_ARCH_VERSION 6

#elif defined(__ARM_ARCH_7A__) \
    || defined(__ARM_ARCH_7K__) \
    || defined(__ARM_ARCH_7R__) \
    || defined(__ARM_ARCH_7S__)
#define WTF_ARM_ARCH_VERSION 7

#elif defined(__ARM_ARCH_8__)
#define WTF_ARM_ARCH_VERSION 8

/* MSVC sets _M_ARM */
#elif defined(_M_ARM)
#define WTF_ARM_ARCH_VERSION _M_ARM

/* RVCT sets _TARGET_ARCH_ARM */
#elif defined(__TARGET_ARCH_ARM)
#define WTF_ARM_ARCH_VERSION __TARGET_ARCH_ARM

#if defined(__TARGET_ARCH_5E) \
    || defined(__TARGET_ARCH_5TE) \
    || defined(__TARGET_ARCH_5TEJ)
/*ARMv5TE requires allocators to use aligned memory*/
#define USE_ARENA_ALLOC_ALIGNMENT_INTEGER 1
#endif

#else
#define WTF_ARM_ARCH_VERSION 0

#endif

/* Set WTF_THUMB_ARCH_VERSION */
#if   defined(__ARM_ARCH_4T__)
#define WTF_THUMB_ARCH_VERSION 1

#elif defined(__ARM_ARCH_5T__) \
    || defined(__ARM_ARCH_5TE__) \
    || defined(__ARM_ARCH_5TEJ__)
#define WTF_THUMB_ARCH_VERSION 2

#elif defined(__ARM_ARCH_6J__) \
    || defined(__ARM_ARCH_6K__) \
    || defined(__ARM_ARCH_6Z__) \
    || defined(__ARM_ARCH_6ZK__) \
    || defined(__ARM_ARCH_6M__)
#define WTF_THUMB_ARCH_VERSION 3

#elif defined(__ARM_ARCH_6T2__) \
    || defined(__ARM_ARCH_7__) \
    || defined(__ARM_ARCH_7A__) \
    || defined(__ARM_ARCH_7K__) \
    || defined(__ARM_ARCH_7M__) \
    || defined(__ARM_ARCH_7R__) \
    || defined(__ARM_ARCH_7S__)
#define WTF_THUMB_ARCH_VERSION 4

/* RVCT sets __TARGET_ARCH_THUMB */
#elif defined(__TARGET_ARCH_THUMB)
#define WTF_THUMB_ARCH_VERSION __TARGET_ARCH_THUMB

#else
#define WTF_THUMB_ARCH_VERSION 0
#endif


/* CPU(ARMV5_OR_LOWER) - ARM instruction set v5 or earlier */
/* On ARMv5 and below the natural alignment is required. 
   And there are some other differences for v5 or earlier. */
#if !defined(ARMV5_OR_LOWER) && !WTF_ARM_ARCH_AT_LEAST(6)
#define WTF_CPU_ARMV5_OR_LOWER 1
#endif


/* CPU(ARM_TRADITIONAL) - Thumb2 is not available, only traditional ARM (v4 or greater) */
/* CPU(ARM_THUMB2) - Thumb2 instruction set is available */
/* Only one of these will be defined. */
#if !defined(WTF_CPU_ARM_TRADITIONAL) && !defined(WTF_CPU_ARM_THUMB2)
#  if defined(thumb2) || defined(__thumb2__) \
    || ((defined(__thumb) || defined(__thumb__)) && WTF_THUMB_ARCH_VERSION == 4)
#    define WTF_CPU_ARM_TRADITIONAL 0
#    define WTF_CPU_ARM_THUMB2 1
#  elif WTF_ARM_ARCH_AT_LEAST(4)
#    define WTF_CPU_ARM_TRADITIONAL 1
#    define WTF_CPU_ARM_THUMB2 0
#  else
#    error "Not supported ARM architecture"
#  endif
#elif CPU(ARM_TRADITIONAL) && CPU(ARM_THUMB2) /* Sanity Check */
#  error "Cannot use both of WTF_CPU_ARM_TRADITIONAL and WTF_CPU_ARM_THUMB2 platforms"
#endif /* !defined(WTF_CPU_ARM_TRADITIONAL) && !defined(WTF_CPU_ARM_THUMB2) */

#if defined(__ARM_NEON__) && !defined(WTF_CPU_ARM_NEON)
#define WTF_CPU_ARM_NEON 1
#endif

#if CPU(ARM_NEON)
/* All NEON intrinsics usage can be disabled by this macro. */
#define HAVE_ARM_NEON_INTRINSICS 1
#endif

#if (defined(__VFP_FP__) && !defined(__SOFTFP__))
#define WTF_CPU_ARM_VFP 1
#endif

#if defined(__ARM_ARCH_7K__)
#define WTF_CPU_APPLE_ARMV7K 1
#endif

#if defined(__ARM_ARCH_7S__)
#define WTF_CPU_APPLE_ARMV7S 1
#endif

#if defined(__ARM_ARCH_EXT_IDIV__) || CPU(APPLE_ARMV7S)
#define HAVE_ARM_IDIV_INSTRUCTIONS 1
#endif

#endif /* ARM */

#if CPU(ARM) || CPU(MIPS) || CPU(SH4)
#define WTF_CPU_NEEDS_ALIGNED_ACCESS 1
#endif

/* ==== OS() - underlying operating system; only to be used for mandated low-level services like 
   virtual memory, not to choose a GUI toolkit ==== */

/* OS(AIX) - AIX */
#ifdef _AIX
#define WTF_OS_AIX 1
#endif

/* OS(DARWIN) - Any Darwin-based OS, including Mac OS X and iPhone OS */
#ifdef __APPLE__
#define WTF_OS_DARWIN 1

#include <Availability.h>
#include <AvailabilityMacros.h>
#include <TargetConditionals.h>
#endif

/* OS(IOS) - iOS */
/* OS(MAC_OS_X) - Mac OS X (not including iOS) */
#if OS(DARWIN) && ((defined(TARGET_OS_EMBEDDED) && TARGET_OS_EMBEDDED) \
    || (defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE)                 \
    || (defined(TARGET_IPHONE_SIMULATOR) && TARGET_IPHONE_SIMULATOR))
#define WTF_OS_IOS 1
#elif OS(DARWIN) && defined(TARGET_OS_MAC) && TARGET_OS_MAC
#define WTF_OS_MAC_OS_X 1
#endif

/* OS(FREEBSD) - FreeBSD */
#if defined(__FreeBSD__) || defined(__DragonFly__) || defined(__FreeBSD_kernel__)
#define WTF_OS_FREEBSD 1
#endif

/* OS(HURD) - GNU/Hurd */
#ifdef __GNU__
#define WTF_OS_HURD 1
#endif

/* OS(LINUX) - Linux */
#ifdef __linux__
#define WTF_OS_LINUX 1
#endif

/* OS(NETBSD) - NetBSD */
#if defined(__NetBSD__)
#define WTF_OS_NETBSD 1
#endif

/* OS(OPENBSD) - OpenBSD */
#ifdef __OpenBSD__
#define WTF_OS_OPENBSD 1
#endif

/* OS(SOLARIS) - Solaris */
#if defined(sun) || defined(__sun)
#define WTF_OS_SOLARIS 1
#endif

/* OS(WINDOWS) - Any version of Windows */
#if defined(WIN32) || defined(_WIN32)
#define WTF_OS_WINDOWS 1
#endif

#define WTF_OS_WIN ERROR "USE WINDOWS WITH OS NOT WIN"
#define WTF_OS_MAC ERROR "USE MAC_OS_X WITH OS NOT MAC"

/* OS(UNIX) - Any Unix-like system */
#if    OS(AIX)              \
    || OS(DARWIN)           \
    || OS(FREEBSD)          \
    || OS(HURD)             \
    || OS(LINUX)            \
    || OS(NETBSD)           \
    || OS(OPENBSD)          \
    || OS(SOLARIS)          \
    || defined(unix)        \
    || defined(__unix)      \
    || defined(__unix__)
#define WTF_OS_UNIX 1
#endif

/* Operating environments */

/* Standard libraries */
#if defined(HAVE_FEATURES_H) && HAVE_FEATURES_H
/* If the included features.h is glibc's one, __GLIBC__ is defined. */
#include <features.h>
#endif

/* FIXME: these are all mixes of OS, operating environment and policy choices. */
/* PLATFORM(GTK) */
/* PLATFORM(MAC) */
/* PLATFORM(IOS) */
/* PLATFORM(IOS_SIMULATOR) */
/* PLATFORM(WIN) */
#if defined(BUILDING_GTK__)
#define WTF_PLATFORM_GTK 1
#elif defined(BUILDING_JSCONLY__)
/* JSCOnly does not provide PLATFORM() macro */
#elif OS(MAC_OS_X)
#define WTF_PLATFORM_MAC 1
#elif OS(IOS)
#define WTF_PLATFORM_IOS 1
#if defined(TARGET_IPHONE_SIMULATOR) && TARGET_IPHONE_SIMULATOR
#define WTF_PLATFORM_IOS_SIMULATOR 1
#endif
#elif OS(WINDOWS)
#define WTF_PLATFORM_WIN 1
#endif

/* PLATFORM(COCOA) */
#if PLATFORM(MAC) || PLATFORM(IOS)
#define WTF_PLATFORM_COCOA 1
#endif

#if PLATFORM(COCOA)
#if defined __has_include && __has_include(<CoreFoundation/CFPriv.h>)
#define USE_APPLE_INTERNAL_SDK 1
#endif
#endif

/* PLATFORM(APPLETV) */
#if defined(TARGET_OS_TV) && TARGET_OS_TV
#define WTF_PLATFORM_APPLETV 1
#endif

/* PLATFORM(WATCHOS) */
#if defined(TARGET_OS_WATCH) && TARGET_OS_WATCH
#define WTF_PLATFORM_WATCHOS 1
#endif

/* Graphics engines */

/* USE(CG) and PLATFORM(CI) */
#if PLATFORM(COCOA) || (PLATFORM(WIN) && !USE(WINGDI) && !PLATFORM(WIN_CAIRO) && !USE(DIRECT2D))
#define USE_CG 1
#endif
#if PLATFORM(COCOA) || (PLATFORM(WIN) && USE(CG) && !USE(DIRECT2D))
#define USE_CA 1
#endif

#if PLATFORM(GTK)
#define USE_CAIRO 1
#define USE_GLIB 1
#define USE_FREETYPE 1
#define USE_HARFBUZZ 1
#define USE_SOUP 1
#define USE_WEBP 1
#endif

#if PLATFORM(GTK)
#define GLIB_VERSION_MIN_REQUIRED GLIB_VERSION_2_36
#endif

#if PLATFORM(GTK) && !defined(GTK_API_VERSION_2)
#define GDK_VERSION_MIN_REQUIRED GDK_VERSION_3_6
#endif

#if USE(SOUP)
#define SOUP_VERSION_MIN_REQUIRED SOUP_VERSION_2_42
#endif

/* On Windows, use QueryPerformanceCounter by default */
#if OS(WINDOWS)
#define USE_QUERY_PERFORMANCE_COUNTER  1
#endif

#if PLATFORM(COCOA)

#define USE_CF 1
#define USE_FOUNDATION 1
#define USE_NETWORK_CFDATA_ARRAY_CALLBACK 1
#define ENABLE_USER_MESSAGE_HANDLERS 1
#define HAVE_OUT_OF_PROCESS_LAYER_HOSTING 1
#define HAVE_DTRACE 0

#if !PLATFORM(WATCHOS) && !PLATFORM(APPLETV)
#define HAVE_AVKIT 1
#define HAVE_PARENTAL_CONTROLS 1
#endif

#endif

#if PLATFORM(MAC)

#if __MAC_OS_X_VERSION_MIN_REQUIRED < 101200
#define USE_QTKIT 1
#else
#define USE_QTKIT 0
#endif

#define USE_APPKIT 1
#define HAVE_RUNLOOP_TIMER 1
#define HAVE_SEC_IDENTITY 1
#define HAVE_SEC_KEYCHAIN 1

#if CPU(X86_64)
#define HAVE_NETWORK_EXTENSION 1
#define USE_PLUGIN_HOST_PROCESS 1
#endif

#if __MAC_OS_X_VERSION_MIN_REQUIRED >= 101100
#define HAVE_NSSCROLLING_FILTERS 1
#else
#define HAVE_NSSCROLLING_FILTERS 0
#endif

/* OS X defines a series of platform macros for debugging. */
/* Some of them are really annoying because they use common names (e.g. check()). */
/* Disable those macros so that we are not limited in how we name methods and functions. */
#undef __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES
#define __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES 0

#endif /* PLATFORM(MAC) */

#if PLATFORM(IOS)

#if USE(APPLE_INTERNAL_SDK) \
    && ((TARGET_OS_IOS && __IPHONE_OS_VERSION_MAX_ALLOWED < 100000) \
     || (PLATFORM(APPLETV) && __TV_OS_VERSION_MAX_ALLOWED < 100000) \
     || (PLATFORM(WATCHOS) && __WATCH_OS_VERSION_MAX_ALLOWED < 30000))
#define USE_CFURLCONNECTION 1
#endif

#define HAVE_NETWORK_EXTENSION 1
#define HAVE_READLINE 1
#define USE_UIKIT_EDITING 1
#define USE_WEB_THREAD 1

#if !PLATFORM(WATCHOS) && !PLATFORM(APPLETV)
#define USE_QUICK_LOOK 1
#endif

#if TARGET_OS_IOS
#define HAVE_APP_LINKS 1
#endif

#if CPU(ARM64)
#define ENABLE_JIT_CONSTANT_BLINDING 0
#endif

#if CPU(ARM_NEON)
#undef HAVE_ARM_NEON_INTRINSICS
#define HAVE_ARM_NEON_INTRINSICS 0
#endif

#endif /* PLATFORM(IOS) */

#if PLATFORM(WIN) && !USE(WINGDI)
#define USE_CF 1
#endif

#if PLATFORM(WIN) && !USE(WINGDI) && !PLATFORM(WIN_CAIRO)
#define USE_CFURLCONNECTION 1
#endif

#if !defined(HAVE_ACCESSIBILITY)
#if PLATFORM(COCOA) || PLATFORM(WIN) || PLATFORM(GTK)
#define HAVE_ACCESSIBILITY 1
#endif
#endif /* !defined(HAVE_ACCESSIBILITY) */

/* FIXME: Remove after CMake build enabled on Darwin */
#if OS(DARWIN)
#define HAVE_ERRNO_H 1
#define HAVE_LANGINFO_H 1
#define HAVE_LOCALTIME_R 1
#define HAVE_MMAP 1
#define HAVE_SIGNAL_H 1
#define HAVE_STAT_BIRTHTIME 1
#define HAVE_STRINGS_H 1
#define HAVE_STRNSTR 1
#define HAVE_SYS_PARAM_H 1
#define HAVE_SYS_TIME_H 1 
#define HAVE_TM_GMTOFF 1
#define HAVE_TM_ZONE 1
#define HAVE_TIMEGM 1
#endif /* OS(DARWIN) */

#if OS(UNIX)
#define USE_PTHREADS 1
#endif /* OS(UNIX) */

#if OS(DARWIN)
#define HAVE_DISPATCH_H 1
#define HAVE_MADV_FREE 1
#define HAVE_MADV_FREE_REUSE 1
#define HAVE_MADV_DONTNEED 1
#define HAVE_MERGESORT 1
#define HAVE_PTHREAD_SETNAME_NP 1
#define HAVE_READLINE 1
#define HAVE_SYS_TIMEB_H 1

#if !PLATFORM(GTK)
#define USE_ACCELERATE 1
#endif
#if !PLATFORM(IOS)
#define HAVE_HOSTED_CORE_ANIMATION 1
#endif

#endif /* OS(DARWIN) */

#if (PLATFORM(MAC) && __MAC_OS_X_VERSION_MIN_REQUIRED >= 101200) || (PLATFORM(IOS) && __IPHONE_OS_VERSION_MIN_REQUIRED >= 100000)
#define HAVE_CFNETWORK_STORAGE_PARTITIONING 1
#endif

/* ENABLE macro defaults */

/* FIXME: move out all ENABLE() defines from here to FeatureDefines.h */

/* Include feature macros */
#include <wtf/FeatureDefines.h>

#if USE(APPLE_INTERNAL_SDK) && __has_include(<WebKitAdditions/AdditionalFeatureDefines.h>)
#include <WebKitAdditions/AdditionalFeatureDefines.h>
#endif

#if OS(WINDOWS)
#define USE_SYSTEM_MALLOC 1
#endif

#if !defined(USE_JSVALUE64) && !defined(USE_JSVALUE32_64)
#if (CPU(X86_64) && (OS(UNIX) || OS(WINDOWS))) \
    || (CPU(IA64) && !CPU(IA64_32)) \
    || CPU(ALPHA) \
    || CPU(ARM64) \
    || CPU(S390X) \
    || CPU(MIPS64) \
    || CPU(PPC64) \
    || CPU(PPC64LE)
#define USE_JSVALUE64 1
#else
#define USE_JSVALUE32_64 1
#endif
#endif /* !defined(USE_JSVALUE64) && !defined(USE_JSVALUE32_64) */

/* The JIT is enabled by default on all x86, x86-64, ARM & MIPS platforms except ARMv7k. */
#if !defined(ENABLE_JIT) \
    && (CPU(X86) || CPU(X86_64) || CPU(ARM) || CPU(ARM64) || CPU(MIPS)) \
    && !CPU(APPLE_ARMV7K)
#define ENABLE_JIT 1
#endif

/* The FTL *does not* work on 32-bit platforms. Disable it even if someone asked us to enable it. */
#if USE(JSVALUE32_64)
#undef ENABLE_FTL_JIT
#define ENABLE_FTL_JIT 0
#endif

/* The FTL is disabled on the iOS simulator, mostly for simplicity. */
#if PLATFORM(IOS_SIMULATOR)
#undef ENABLE_FTL_JIT
#define ENABLE_FTL_JIT 0
#endif

/* If possible, try to enable a disassembler. This is optional. We proceed in two
   steps: first we try to find some disassembler that we can use, and then we
   decide if the high-level disassembler API can be enabled. */
#if !defined(USE_UDIS86) && ENABLE(JIT) && ((OS(DARWIN) && !PLATFORM(GTK)) || (OS(LINUX) && PLATFORM(GTK))) \
    && (CPU(X86) || CPU(X86_64))
#define USE_UDIS86 1
#endif

#if !defined(ENABLE_DISASSEMBLER) && USE(UDIS86)
#define ENABLE_DISASSEMBLER 1
#endif

#if !defined(USE_ARM64_DISASSEMBLER) && ENABLE(JIT) && CPU(ARM64)
#define USE_ARM64_DISASSEMBLER 1
#endif

#if !defined(USE_ARMV7_DISASSEMBLER) && ENABLE(JIT) && CPU(ARM_THUMB2)
#define USE_ARMV7_DISASSEMBLER 1
#endif

#if !defined(USE_ARM_LLVM_DISASSEMBLER) && ENABLE(JIT) && CPU(ARM_TRADITIONAL) && HAVE(LLVM)
#define USE_ARM_LLVM_DISASSEMBLER 1
#endif

#if !defined(ENABLE_DISASSEMBLER) && (USE(UDIS86) || USE(ARMV7_DISASSEMBLER) || USE(ARM64_DISASSEMBLER) || USE(ARM_LLVM_DISASSEMBLER))
#define ENABLE_DISASSEMBLER 1
#endif

#if !defined(ENABLE_DFG_JIT) && ENABLE(JIT)
/* Enable the DFG JIT on X86 and X86_64. */
#if (CPU(X86) || CPU(X86_64)) && (OS(DARWIN) || OS(LINUX) || OS(FREEBSD) || OS(WINDOWS) || OS(HURD))
#define ENABLE_DFG_JIT 1
#endif
/* Enable the DFG JIT on ARMv7.  Only tested on iOS and Qt/GTK+ Linux. */
#if (CPU(ARM_THUMB2) || CPU(ARM64)) && (PLATFORM(IOS) || PLATFORM(GTK))
#define ENABLE_DFG_JIT 1
#endif
/* Enable the DFG JIT on ARM and MIPS. */
#if CPU(ARM_TRADITIONAL) || CPU(MIPS)
#define ENABLE_DFG_JIT 1
#endif
#endif

/* Concurrent JS only works on 64-bit platforms because it requires that
   values get stored to atomically. This is trivially true on 64-bit platforms,
   but not true at all on 32-bit platforms where values are composed of two
   separate sub-values. */
#if ENABLE(DFG_JIT) && (USE(JSVALUE64) || defined(__ANDROID__))
#define ENABLE_CONCURRENT_JS 1
#endif

/* FIXME: Enable it on Linux once https://bugs.webkit.org/show_bug.cgi?id=169510 is fixed. */
#if CPU(ARM64) && OS(DARWIN)
#define HAVE_LL_SC 1
#endif // CPU(ARM64) && OS(DARWIN)

#if __has_include(<System/pthread_machdep.h>)
#define HAVE_FAST_TLS 1
#endif

#if (CPU(X86_64) || CPU(ARM64)) && HAVE(FAST_TLS)
#define ENABLE_FAST_TLS_JIT 1
#endif

/* This controls whether B3 is built. B3 is needed for FTL JIT and WebAssembly */
#if ENABLE(FTL_JIT) || ENABLE(WEBASSEMBLY)
#define ENABLE_B3_JIT 1
#endif

/* If the baseline jit is not available, then disable upper tiers as well: */
#if !ENABLE(JIT)
#undef ENABLE_DFG_JIT
#undef ENABLE_FTL_JIT
#undef ENABLE_B3_JIT
#define ENABLE_DFG_JIT 0
#define ENABLE_FTL_JIT 0
#define ENABLE_B3_JIT 0
#endif

/* The SamplingProfiler is the probabilistic and low-overhead profiler used by
 * JSC to measure where time is spent inside a JavaScript program.
 * In configurations other than Windows and Darwin, because layout of mcontext_t depends on standard libraries (like glibc),
 * sampling profiler is enabled if WebKit uses pthreads and glibc. */
#if !defined(ENABLE_SAMPLING_PROFILER)
#if (OS(DARWIN) || OS(WINDOWS) || PLATFORM(GTK)) && ENABLE(JIT)
#define ENABLE_SAMPLING_PROFILER 1
#else
#define ENABLE_SAMPLING_PROFILER 0
#endif
#endif

/* Counts uses of write barriers using sampling counters. Be sure to also
   set ENABLE_SAMPLING_COUNTERS to 1. */
#if !defined(ENABLE_WRITE_BARRIER_PROFILING)
#define ENABLE_WRITE_BARRIER_PROFILING 0
#endif

/* Logs all allocation-related activity that goes through fastMalloc or the
   JSC GC (both cells and butterflies). Also logs marking. Note that this
   isn't a completely accurate view of the heap since it doesn't include all
   butterfly resize operations, doesn't tell you what is going on with weak
   references (other than to tell you when they're marked), and doesn't
   track direct mmap() allocations or things like JIT allocation. */
#if !defined(ENABLE_ALLOCATION_LOGGING)
#define ENABLE_ALLOCATION_LOGGING 0
#endif

/* Enable verification that that register allocations are not made within generated control flow.
   Turned on for debug builds. */
#if !defined(ENABLE_DFG_REGISTER_ALLOCATION_VALIDATION) && ENABLE(DFG_JIT)
#if !defined(NDEBUG)
#define ENABLE_DFG_REGISTER_ALLOCATION_VALIDATION 1
#else
#define ENABLE_DFG_REGISTER_ALLOCATION_VALIDATION 0
#endif
#endif

/* Configure the JIT */
#if CPU(X86) && COMPILER(MSVC)
#define JSC_HOST_CALL __fastcall
#elif CPU(X86) && COMPILER(GCC_OR_CLANG)
#define JSC_HOST_CALL __attribute__ ((fastcall))
#else
#define JSC_HOST_CALL
#endif

#if CPU(X86) && OS(WINDOWS)
#define CALLING_CONVENTION_IS_STDCALL 1
#ifndef CDECL
#if COMPILER(MSVC)
#define CDECL __cdecl
#else
#define CDECL __attribute__ ((__cdecl))
#endif
#endif
#else
#define CALLING_CONVENTION_IS_STDCALL 0
#endif

#if CPU(X86)
#define WTF_COMPILER_SUPPORTS_FASTCALL_CALLING_CONVENTION 1
#ifndef FASTCALL
#if COMPILER(MSVC)
#define FASTCALL __fastcall
#else
#define FASTCALL  __attribute__ ((fastcall))
#endif
#endif
#else
#define WTF_COMPILER_SUPPORTS_FASTCALL_CALLING_CONVENTION 0
#endif

#if ENABLE(JIT) && CALLING_CONVENTION_IS_STDCALL
#define JIT_OPERATION CDECL
#else
#define JIT_OPERATION
#endif

/* Configure the interpreter */
#if COMPILER(GCC_OR_CLANG)
#define HAVE_COMPUTED_GOTO 1
#endif

/* Determine if we need to enable Computed Goto Opcodes or not: */
#if HAVE(COMPUTED_GOTO) || ENABLE(JIT)
#define ENABLE_COMPUTED_GOTO_OPCODES 1
#endif

/* Regular Expression Tracing - Set to 1 to trace RegExp's in jsc.  Results dumped at exit */
#define ENABLE_REGEXP_TRACING 0

/* Yet Another Regex Runtime - turned on by default for JIT enabled ports. */
#if !defined(ENABLE_YARR_JIT) && ENABLE(JIT)
#define ENABLE_YARR_JIT 1

/* Setting this flag compares JIT results with interpreter results. */
#define ENABLE_YARR_JIT_DEBUG 0
#endif

/* If either the JIT or the RegExp JIT is enabled, then the Assembler must be
   enabled as well: */
#if ENABLE(JIT) || ENABLE(YARR_JIT)
#if defined(ENABLE_ASSEMBLER) && !ENABLE_ASSEMBLER
#error "Cannot enable the JIT or RegExp JIT without enabling the Assembler"
#else
#undef ENABLE_ASSEMBLER
#define ENABLE_ASSEMBLER 1
#endif
#endif

/* If the Disassembler is enabled, then the Assembler must be enabled as well: */
#if ENABLE(DISASSEMBLER)
#if defined(ENABLE_ASSEMBLER) && !ENABLE_ASSEMBLER
#error "Cannot enable the Disassembler without enabling the Assembler"
#else
#undef ENABLE_ASSEMBLER
#define ENABLE_ASSEMBLER 1
#endif
#endif

/* Enable the following if you want to use the MacroAssembler::probe() facility
   to do JIT debugging. */
#if (CPU(X86) || CPU(X86_64) || CPU(ARM64) || (CPU(ARM_THUMB2) && PLATFORM(IOS))) && ENABLE(JIT) && OS(DARWIN)
#define ENABLE_MASM_PROBE 1
#else
#define ENABLE_MASM_PROBE 0
#endif

#ifndef ENABLE_EXCEPTION_SCOPE_VERIFICATION
#ifdef NDEBUG
#define ENABLE_EXCEPTION_SCOPE_VERIFICATION 0
#else
#define ENABLE_EXCEPTION_SCOPE_VERIFICATION 1
#endif
#endif

#if (OS(DARWIN) || OS(LINUX) || OS(FREEBSD)) && ENABLE(JIT)
#if !defined(__ANDROID__)
#define ENABLE_SIGNAL_BASED_VM_TRAPS 1
#endif
#endif

/* CSS Selector JIT Compiler */
#if !defined(ENABLE_CSS_SELECTOR_JIT)
#if (CPU(X86_64) || CPU(ARM64) || (CPU(ARM_THUMB2) && PLATFORM(IOS))) && ENABLE(JIT) && (OS(DARWIN) || PLATFORM(GTK))
#define ENABLE_CSS_SELECTOR_JIT 1
#else
#define ENABLE_CSS_SELECTOR_JIT 0
#endif
#endif

#if ENABLE(WEBGL) && PLATFORM(WIN)
#define USE_OPENGL 1
#define USE_OPENGL_ES_2 1
#define USE_EGL 1
#endif

#if ENABLE(VIDEO) && PLATFORM(WIN_CAIRO)
#if ENABLE(GSTREAMER_WINCAIRO)
#define USE_MEDIA_FOUNDATION 0
#define USE_GLIB 1
#define USE_GSTREAMER 1
#else
#define USE_MEDIA_FOUNDATION 1
#endif
#endif

#if PLATFORM(WIN_CAIRO)
#define USE_TEXTURE_MAPPER 1
#endif

#if USE(TEXTURE_MAPPER) && ENABLE(GRAPHICS_CONTEXT_3D) && !defined(USE_TEXTURE_MAPPER_GL)
#define USE_TEXTURE_MAPPER_GL 1
#endif

#if PLATFORM(COCOA)
#define USE_PROTECTION_SPACE_AUTH_CALLBACK 1
#endif

#if PLATFORM(COCOA) && HAVE(ACCESSIBILITY)
#define USE_ACCESSIBILITY_CONTEXT_MENUS 1
#endif

#if CPU(ARM_THUMB2) || CPU(ARM64)
#define ENABLE_BRANCH_COMPACTION 1
#endif

#if !defined(ENABLE_THREADING_LIBDISPATCH) && HAVE(DISPATCH_H)
#define ENABLE_THREADING_LIBDISPATCH 1
#elif !defined(ENABLE_THREADING_OPENMP) && defined(_OPENMP)
#define ENABLE_THREADING_OPENMP 1
#elif !defined(THREADING_GENERIC)
#define ENABLE_THREADING_GENERIC 1
#endif

#if USE(GLIB)
#include <wtf/glib/GTypedefs.h>
#endif

/* FIXME: This define won't be needed once #27551 is fully landed. However,
   since most ports try to support sub-project independence, adding new headers
   to WTF causes many ports to break, and so this way we can address the build
   breakages one port at a time. */
#if !defined(USE_EXPORT_MACROS) && (PLATFORM(COCOA) || PLATFORM(WIN))
#define USE_EXPORT_MACROS 1
#endif

#if !defined(USE_EXPORT_MACROS_FOR_TESTING) && (PLATFORM(GTK) || PLATFORM(WIN))
#define USE_EXPORT_MACROS_FOR_TESTING 1
#endif

#if PLATFORM(GTK)
#define USE_UNIX_DOMAIN_SOCKETS 1
#endif

#if !defined(USE_IMLANG_FONT_LINK2)
#define USE_IMLANG_FONT_LINK2 1
#endif

#if !defined(ENABLE_GC_VALIDATION) && !defined(NDEBUG)
#define ENABLE_GC_VALIDATION 1
#endif

#if !defined(ENABLE_BINDING_INTEGRITY) && !OS(WINDOWS)
#define ENABLE_BINDING_INTEGRITY 1
#endif

#if PLATFORM(COCOA)
#define USE_AVFOUNDATION 1
#endif

#if !defined(ENABLE_TREE_DEBUGGING)
#if !defined(NDEBUG)
#define ENABLE_TREE_DEBUGGING 1
#else
#define ENABLE_TREE_DEBUGGING 0
#endif
#endif

#if PLATFORM(IOS) || PLATFORM(MAC)
#define USE_COREMEDIA 1
#define HAVE_AVFOUNDATION_VIDEO_OUTPUT 1
#endif

#if PLATFORM(IOS) || PLATFORM(MAC) || (OS(WINDOWS) && USE(CG))
#define HAVE_AVFOUNDATION_MEDIA_SELECTION_GROUP 1
#endif

#if PLATFORM(IOS) || PLATFORM(MAC) || (OS(WINDOWS) && USE(CG))
#define HAVE_AVFOUNDATION_LEGIBLE_OUTPUT_SUPPORT 1
#define HAVE_MEDIA_ACCESSIBILITY_FRAMEWORK 1
#endif

#if PLATFORM(IOS) || PLATFORM(MAC)
#define HAVE_AVFOUNDATION_LOADER_DELEGATE 1
#endif

#if PLATFORM(MAC) || (PLATFORM(IOS) && ENABLE(WEB_RTC))
#define USE_VIDEOTOOLBOX 1
#endif

#if PLATFORM(COCOA) || PLATFORM(GTK) || (PLATFORM(WIN) && !USE(WINGDI))
#define USE_REQUEST_ANIMATION_FRAME_TIMER 1
#endif

#if PLATFORM(COCOA)
#define USE_REQUEST_ANIMATION_FRAME_DISPLAY_MONITOR 1
#endif

#if PLATFORM(MAC)
#define USE_COREAUDIO 1
#endif

#if !defined(USE_ZLIB)
#define USE_ZLIB 1
#endif

#ifndef HAVE_QOS_CLASSES
#if PLATFORM(COCOA)
#define HAVE_QOS_CLASSES 1
#endif
#endif

#ifndef HAVE_VOUCHERS
#if PLATFORM(COCOA)
#define HAVE_VOUCHERS 1
#endif
#endif

#define USE_GRAMMAR_CHECKING 1

#if PLATFORM(COCOA) || PLATFORM(GTK)
#define USE_UNIFIED_TEXT_CHECKING 1
#endif
#if PLATFORM(MAC)
#define USE_AUTOMATIC_TEXT_REPLACEMENT 1
#endif

#if PLATFORM(MAC)
/* Some platforms provide UI for suggesting autocorrection. */
#define USE_AUTOCORRECTION_PANEL 1
#endif

#if PLATFORM(COCOA)
/* Some platforms use spelling and autocorrection markers to provide visual cue. On such platform, if word with marker is edited, we need to remove the marker. */
#define USE_MARKER_REMOVAL_UPON_EDITING 1
#endif

#if PLATFORM(MAC)
#define USE_INSERTION_UNDO_GROUPING 1
#endif

#if (PLATFORM(MAC) && __MAC_OS_X_VERSION_MIN_REQUIRED >= 101100) || PLATFORM(IOS)
#define HAVE_TIMINGDATAOPTIONS 1
#endif

#if PLATFORM(COCOA)
#define USE_AUDIO_SESSION 1
#endif

#if PLATFORM(COCOA) && !PLATFORM(IOS_SIMULATOR)
#define USE_IOSURFACE 1
#endif

#if PLATFORM(COCOA)
#define ENABLE_RESOURCE_USAGE 1
#endif

#if PLATFORM(GTK)
#undef ENABLE_OPENTYPE_VERTICAL
#define ENABLE_OPENTYPE_VERTICAL 1
#define ENABLE_CSS3_TEXT_DECORATION_SKIP_INK 1
#endif

#if PLATFORM(GTK)
#define USE_WOFF2 1
#endif

#if PLATFORM(COCOA)
#define ENABLE_CSS3_TEXT_DECORATION_SKIP_INK 1
#endif

#if COMPILER(MSVC)
#undef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#undef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#if PLATFORM(MAC)
#define HAVE_NS_ACTIVITY 1
#endif

#if (OS(DARWIN) && USE(CG)) || (USE(FREETYPE) && !PLATFORM(GTK)) || (PLATFORM(WIN) && (USE(CG) || USE(CAIRO)))
#undef ENABLE_OPENTYPE_MATH
#define ENABLE_OPENTYPE_MATH 1
#endif

/* Set TARGET_OS_IPHONE to 0 by default to allow using it as a guard 
 * in cross-platform the same way as it is used in OS(DARWIN) code. */ 
#if !defined(TARGET_OS_IPHONE) && !OS(DARWIN)
#define TARGET_OS_IPHONE 0
#endif

#if PLATFORM(COCOA)
#define USE_MEDIATOOLBOX 1
#endif

/* While 10.10 has support for fences, it is missing some API important for our integration of them. */
#if PLATFORM(IOS) || (PLATFORM(MAC) && __MAC_OS_X_VERSION_MIN_REQUIRED >= 101100)
#define HAVE_COREANIMATION_FENCES 1
#endif

/* FIXME: Enable USE_OS_LOG when building with the public iOS 10 SDK once we fix <rdar://problem/27758343>. */
#if (PLATFORM(MAC) && __MAC_OS_X_VERSION_MIN_REQUIRED >= 101200) || (PLATFORM(IOS) && __IPHONE_OS_VERSION_MIN_REQUIRED >= 100000 && USE(APPLE_INTERNAL_SDK))
#define USE_OS_LOG 1
#if USE(APPLE_INTERNAL_SDK)
#define USE_OS_STATE 1
#endif
#endif

#if (PLATFORM(MAC) && __MAC_OS_X_VERSION_MIN_REQUIRED >= 101200) || (PLATFORM(IOS) && __IPHONE_OS_VERSION_MIN_REQUIRED >= 100000)
#define HAVE_SEC_TRUST_SERIALIZATION 1
#endif

#if !defined(WTF_DEFAULT_EVENT_LOOP)
#define WTF_DEFAULT_EVENT_LOOP 1
#endif

#if WTF_DEFAULT_EVENT_LOOP
#if PLATFORM(WIN)
/* Use Windows message pump abstraction.
 * Even if the port is AppleWin, we use the Windows message pump system for the event loop,
 * so that USE(WINDOWS_EVENT_LOOP) && USE(CF) can be true.
 * And PLATFORM(WIN) and PLATFORM(GTK) are exclusive. If the port is GTK,
 * PLATFORM(WIN) should be false. And in that case, GLib's event loop is used.
 */
#define USE_WINDOWS_EVENT_LOOP 1
#elif PLATFORM(COCOA)
/* OS X and IOS. Use CoreFoundation & GCD abstraction. */
#define USE_COCOA_EVENT_LOOP 1
#elif USE(GLIB)
/* Use GLib's event loop abstraction. Primarily GTK port uses it. */
#define USE_GLIB_EVENT_LOOP 1
#else
#define USE_GENERIC_EVENT_LOOP 1
#endif
#endif

#if PLATFORM(MAC) && __MAC_OS_X_VERSION_MIN_REQUIRED >= 101200
#define USE_MEDIAREMOTE 1
#endif

#if COMPILER(MSVC)
/* Enable strict runtime stack buffer checks. */
#pragma strict_gs_check(on)
#endif

#if PLATFORM(MAC) && __MAC_OS_X_VERSION_MAX_ALLOWED >= 101201 && __MAC_OS_X_VERSION_MIN_REQUIRED >= 101200
#define HAVE_TOUCH_BAR 1
#define HAVE_ADVANCED_SPELL_CHECKING 1

#if defined(__LP64__)
#define ENABLE_WEB_PLAYBACK_CONTROLS_MANAGER 1
#endif
#endif /* PLATFORM(MAC) && __MAC_OS_X_VERSION_MAX_ALLOWED >= 101201 && __MAC_OS_X_VERSION_MIN_REQUIRED >= 101200 */

#if PLATFORM(COCOA) && ENABLE(WEB_RTC)
#define USE_LIBWEBRTC 1
#endif

#endif /* WTF_Platform_h */
