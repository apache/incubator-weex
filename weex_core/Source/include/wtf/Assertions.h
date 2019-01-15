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
#ifndef WTF_Assertions_h
#define WTF_Assertions_h

#include <wtf/Platform.h>

/*
   no namespaces because this file has to be includable from C and Objective-C

   Note, this file uses many GCC extensions, but it should be compatible with
   C, Objective C, C++, and Objective C++.

   For non-debug builds, everything is disabled by default except for "always
   on" logging. Defining any of the symbols explicitly prevents this from
   having any effect.
*/

#undef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <wtf/ExportMacros.h>

#if USE(OS_LOG)
#include <os/log.h>
#endif

#ifdef NDEBUG
/* Disable ASSERT* macros in release mode. */
#define ASSERTIONS_DISABLED_DEFAULT 1
#else
#define ASSERTIONS_DISABLED_DEFAULT 0
#endif

#ifndef BACKTRACE_DISABLED
#define BACKTRACE_DISABLED ASSERTIONS_DISABLED_DEFAULT
#endif

#ifndef ASSERT_DISABLED
#define ASSERT_DISABLED ASSERTIONS_DISABLED_DEFAULT
#endif

#ifndef ASSERT_MSG_DISABLED
#define ASSERT_MSG_DISABLED ASSERTIONS_DISABLED_DEFAULT
#endif

#ifndef ASSERT_ARG_DISABLED
#define ASSERT_ARG_DISABLED ASSERTIONS_DISABLED_DEFAULT
#endif

#ifndef FATAL_DISABLED
#define FATAL_DISABLED ASSERTIONS_DISABLED_DEFAULT
#endif

#ifndef ERROR_DISABLED
#define ERROR_DISABLED ASSERTIONS_DISABLED_DEFAULT
#endif

#ifndef LOG_DISABLED
#define LOG_DISABLED ASSERTIONS_DISABLED_DEFAULT
#endif

#ifndef RELEASE_LOG_DISABLED
#define RELEASE_LOG_DISABLED !(USE(OS_LOG))
#endif

#if COMPILER(GCC_OR_CLANG)
#define WTF_PRETTY_FUNCTION __PRETTY_FUNCTION__
#else
#define WTF_PRETTY_FUNCTION __FUNCTION__
#endif

#if COMPILER(MINGW)
/* By default MinGW emits warnings when C99 format attributes are used, even if __USE_MINGW_ANSI_STDIO is defined */
#define WTF_ATTRIBUTE_PRINTF(formatStringArgument, extraArguments) __attribute__((__format__(gnu_printf, formatStringArgument, extraArguments)))
#elif COMPILER(GCC_OR_CLANG) && !defined(__OBJC__)
/* WTF logging functions can process %@ in the format string to log a NSObject* but the printf format attribute
   emits a warning when %@ is used in the format string.  Until <rdar://problem/5195437> is resolved we can't include
   the attribute when being used from Objective-C code in case it decides to use %@. */
#define WTF_ATTRIBUTE_PRINTF(formatStringArgument, extraArguments) __attribute__((__format__(printf, formatStringArgument, extraArguments)))
#else
#define WTF_ATTRIBUTE_PRINTF(formatStringArgument, extraArguments)
#endif

#if PLATFORM(IOS)
/* For a project that uses WTF but has no config.h, we need to explicitly set the export defines here. */
#ifndef WTF_EXPORT_PRIVATE
#define WTF_EXPORT_PRIVATE
#endif
#endif // PLATFORM(IOS)

/* These helper functions are always declared, but not necessarily always defined if the corresponding function is disabled. */

#ifdef __cplusplus
extern "C" {
#endif

/* CRASH() - Raises a fatal error resulting in program termination and triggering either the debugger or the crash reporter.

   Use CRASH() in response to known, unrecoverable errors like out-of-memory.
   Macro is enabled in both debug and release mode.
   To test for unknown errors and verify assumptions, use ASSERT instead, to avoid impacting performance in release builds.

   Signals are ignored by the crash reporter on OS X so we must do better.
*/
#if COMPILER(GCC_OR_CLANG) || COMPILER(MSVC)
#define NO_RETURN_DUE_TO_CRASH NO_RETURN
#else
#define NO_RETURN_DUE_TO_CRASH
#endif

typedef enum { WTFLogChannelOff, WTFLogChannelOn, WTFLogChannelOnWithAccumulation } WTFLogChannelState;

typedef struct {
    WTFLogChannelState state;
    const char* name;
#if !RELEASE_LOG_DISABLED
    const char* subsystem;
    __unsafe_unretained os_log_t osLogChannel;
#endif
} WTFLogChannel;

#define LOG_CHANNEL(name) JOIN_LOG_CHANNEL_WITH_PREFIX(LOG_CHANNEL_PREFIX, name)
#define LOG_CHANNEL_ADDRESS(name) &LOG_CHANNEL(name),
#define JOIN_LOG_CHANNEL_WITH_PREFIX(prefix, channel) JOIN_LOG_CHANNEL_WITH_PREFIX_LEVEL_2(prefix, channel)
#define JOIN_LOG_CHANNEL_WITH_PREFIX_LEVEL_2(prefix, channel) prefix ## channel

#define LOG_CHANNEL_WEBKIT_SUBSYSTEM "com.apple.WebKit"

#define DECLARE_LOG_CHANNEL(name) \
    extern WTFLogChannel LOG_CHANNEL(name);

#if !defined(DEFINE_LOG_CHANNEL)
#if RELEASE_LOG_DISABLED
#define DEFINE_LOG_CHANNEL(name, subsystem) \
    WTFLogChannel LOG_CHANNEL(name) = { WTFLogChannelOff, #name };
#else
#define DEFINE_LOG_CHANNEL(name, subsystem) \
    WTFLogChannel LOG_CHANNEL(name) = { WTFLogChannelOff, #name, subsystem, OS_LOG_DEFAULT };
#endif
#endif

WTF_EXPORT_PRIVATE void WTFReportAssertionFailure(const char* file, int line, const char* function, const char* assertion);
WTF_EXPORT_PRIVATE void WTFReportAssertionFailureWithMessage(const char* file, int line, const char* function, const char* assertion, const char* format, ...) WTF_ATTRIBUTE_PRINTF(5, 6);
WTF_EXPORT_PRIVATE void WTFReportArgumentAssertionFailure(const char* file, int line, const char* function, const char* argName, const char* assertion);
WTF_EXPORT_PRIVATE void WTFReportFatalError(const char* file, int line, const char* function, const char* format, ...) WTF_ATTRIBUTE_PRINTF(4, 5);
WTF_EXPORT_PRIVATE void WTFReportError(const char* file, int line, const char* function, const char* format, ...) WTF_ATTRIBUTE_PRINTF(4, 5);
WTF_EXPORT_PRIVATE void WTFLog(WTFLogChannel*, const char* format, ...) WTF_ATTRIBUTE_PRINTF(2, 3);
WTF_EXPORT_PRIVATE void WTFLogVerbose(const char* file, int line, const char* function, WTFLogChannel*, const char* format, ...) WTF_ATTRIBUTE_PRINTF(5, 6);
WTF_EXPORT_PRIVATE void WTFLogAlwaysV(const char* format, va_list);
WTF_EXPORT_PRIVATE void WTFLogAlways(const char* format, ...) WTF_ATTRIBUTE_PRINTF(1, 2);
WTF_EXPORT_PRIVATE NO_RETURN_DUE_TO_CRASH void WTFLogAlwaysAndCrash(const char* format, ...) WTF_ATTRIBUTE_PRINTF(1, 2);
WTF_EXPORT_PRIVATE WTFLogChannel* WTFLogChannelByName(WTFLogChannel*[], size_t count, const char*);
WTF_EXPORT_PRIVATE void WTFInitializeLogChannelStatesFromString(WTFLogChannel*[], size_t count, const char*);

WTF_EXPORT_PRIVATE void WTFGetBacktrace(void** stack, int* size);
WTF_EXPORT_PRIVATE void WTFReportBacktrace();
WTF_EXPORT_PRIVATE void WTFPrintBacktrace(void** stack, int size);

typedef void (*WTFCrashHookFunction)();
WTF_EXPORT_PRIVATE void WTFSetCrashHook(WTFCrashHookFunction);
WTF_EXPORT_PRIVATE void WTFInstallReportBacktraceOnCrashHook();

WTF_EXPORT_PRIVATE bool WTFIsDebuggerAttached();

#ifndef CRASH

#if defined(NDEBUG) && OS(DARWIN)
#if CPU(X86_64) || CPU(X86)
#define WTFBreakpointTrap()  __asm__ volatile ("int3")
#elif CPU(ARM_THUMB2)
#define WTFBreakpointTrap()  __asm__ volatile ("bkpt #0")
#elif CPU(ARM64)
#define WTFBreakpointTrap()  __asm__ volatile ("brk #0")
#else
#error "Unsupported CPU".
#endif

// Crash with a SIGTRAP i.e EXC_BREAKPOINT.
// We are not using __builtin_trap because it is only guaranteed to abort, but not necessarily
// trigger a SIGTRAP. Instead, we use inline asm to ensure that we trigger the SIGTRAP.
#define CRASH() do { \
    WTFBreakpointTrap(); \
    __builtin_unreachable(); \
} while (0)
#else
#define CRASH() WTFCrash()
#endif

#endif // CRASH

WTF_EXPORT_PRIVATE NO_RETURN_DUE_TO_CRASH void WTFCrash();

#ifndef CRASH_WITH_SECURITY_IMPLICATION
#define CRASH_WITH_SECURITY_IMPLICATION() WTFCrashWithSecurityImplication()
#endif

WTF_EXPORT_PRIVATE NO_RETURN_DUE_TO_CRASH void WTFCrashWithSecurityImplication();

#ifdef __cplusplus
}
#endif

/* BACKTRACE

  Print a backtrace to the same location as ASSERT messages.
*/

#if BACKTRACE_DISABLED

#define BACKTRACE() ((void)0)

#else

#define BACKTRACE() do { \
    WTFReportBacktrace(); \
} while(false)

#endif

/* ASSERT, ASSERT_NOT_REACHED, ASSERT_UNUSED

  These macros are compiled out of release builds.
  Expressions inside them are evaluated in debug builds only.
*/

#if OS(WINDOWS)
/* FIXME: Change to use something other than ASSERT to avoid this conflict with the underlying platform */
#undef ASSERT
#endif

#if ASSERT_DISABLED

#define ASSERT(assertion) ((void)0)
#define ASSERT_AT(assertion, file, line, function) ((void)0)
#define ASSERT_NOT_REACHED() ((void)0)
#define ASSERT_IMPLIES(condition, assertion) ((void)0)
#define NO_RETURN_DUE_TO_ASSERT

#define ASSERT_UNUSED(variable, assertion) ((void)variable)

#if ENABLE(SECURITY_ASSERTIONS)
#define ASSERT_WITH_SECURITY_IMPLICATION(assertion) \
    (!(assertion) ? \
        (WTFReportAssertionFailure(__FILE__, __LINE__, WTF_PRETTY_FUNCTION, #assertion), \
         CRASH_WITH_SECURITY_IMPLICATION()) : \
        (void)0)

#define ASSERT_WITH_SECURITY_IMPLICATION_DISABLED 0
#else
#define ASSERT_WITH_SECURITY_IMPLICATION(assertion) ((void)0)
#define ASSERT_WITH_SECURITY_IMPLICATION_DISABLED 1
#endif

#else

#define ASSERT(assertion) do { \
    if (!(assertion)) { \
        WTFReportAssertionFailure(__FILE__, __LINE__, WTF_PRETTY_FUNCTION, #assertion); \
        CRASH(); \
    } \
} while (0)

#define ASSERT_AT(assertion, file, line, function) do { \
    if (!(assertion)) { \
        WTFReportAssertionFailure(file, line, function, #assertion); \
        CRASH(); \
    } \
} while (0)

#define ASSERT_NOT_REACHED() do { \
    WTFReportAssertionFailure(__FILE__, __LINE__, WTF_PRETTY_FUNCTION, 0); \
    CRASH(); \
} while (0)

#define ASSERT_IMPLIES(condition, assertion) do { \
    if ((condition) && !(assertion)) { \
        WTFReportAssertionFailure(__FILE__, __LINE__, WTF_PRETTY_FUNCTION, #condition " => " #assertion); \
        CRASH(); \
    } \
} while (0)

#define ASSERT_UNUSED(variable, assertion) ASSERT(assertion)

#define NO_RETURN_DUE_TO_ASSERT NO_RETURN_DUE_TO_CRASH

/* ASSERT_WITH_SECURITY_IMPLICATION
 
   Failure of this assertion indicates a possible security vulnerability.
   Class of vulnerabilities that it tests include bad casts, out of bounds
   accesses, use-after-frees, etc. Please file a bug using the security
   template - https://bugs.webkit.org/enter_bug.cgi?product=Security.
 
*/
#define ASSERT_WITH_SECURITY_IMPLICATION(assertion) \
    (!(assertion) ? \
        (WTFReportAssertionFailure(__FILE__, __LINE__, WTF_PRETTY_FUNCTION, #assertion), \
         CRASH_WITH_SECURITY_IMPLICATION()) : \
        (void)0)
#define ASSERT_WITH_SECURITY_IMPLICATION_DISABLED 0
#endif

/* ASSERT_WITH_MESSAGE */

#if ASSERT_MSG_DISABLED
#define ASSERT_WITH_MESSAGE(assertion, ...) ((void)0)
#else
#define ASSERT_WITH_MESSAGE(assertion, ...) do { \
    if (!(assertion)) { \
        WTFReportAssertionFailureWithMessage(__FILE__, __LINE__, WTF_PRETTY_FUNCTION, #assertion, __VA_ARGS__); \
        CRASH(); \
    } \
} while (0)
#endif

/* ASSERT_WITH_MESSAGE_UNUSED */

#if ASSERT_MSG_DISABLED
#define ASSERT_WITH_MESSAGE_UNUSED(variable, assertion, ...) ((void)variable)
#else
#define ASSERT_WITH_MESSAGE_UNUSED(variable, assertion, ...) do { \
    if (!(assertion)) { \
        WTFReportAssertionFailureWithMessage(__FILE__, __LINE__, WTF_PRETTY_FUNCTION, #assertion, __VA_ARGS__); \
        CRASH(); \
    } \
} while (0)
#endif
                        
                        
/* ASSERT_ARG */

#if ASSERT_ARG_DISABLED

#define ASSERT_ARG(argName, assertion) ((void)0)

#else

#define ASSERT_ARG(argName, assertion) do { \
    if (!(assertion)) { \
        WTFReportArgumentAssertionFailure(__FILE__, __LINE__, WTF_PRETTY_FUNCTION, #argName, #assertion); \
        CRASH(); \
    } \
} while (0)

#endif

/* COMPILE_ASSERT */
#ifndef COMPILE_ASSERT
#if COMPILER_SUPPORTS(C_STATIC_ASSERT)
/* Unlike static_assert below, this also works in plain C code. */
#define COMPILE_ASSERT(exp, name) _Static_assert((exp), #name)
#else
#define COMPILE_ASSERT(exp, name) static_assert((exp), #name)
#endif
#endif

/* FATAL */

#if FATAL_DISABLED
#define FATAL(...) ((void)0)
#else
#define FATAL(...) do { \
    WTFReportFatalError(__FILE__, __LINE__, WTF_PRETTY_FUNCTION, __VA_ARGS__); \
    CRASH(); \
} while (0)
#endif

/* LOG_ERROR */

#if ERROR_DISABLED
#define LOG_ERROR(...) ((void)0)
#else
#define LOG_ERROR(...) WTFReportError(__FILE__, __LINE__, WTF_PRETTY_FUNCTION, __VA_ARGS__)
#endif

/* LOG */

#if LOG_DISABLED
#define LOG(channel, ...) ((void)0)
#else
#define LOG(channel, ...) WTFLog(&LOG_CHANNEL(channel), __VA_ARGS__)
#endif

/* LOG_VERBOSE */

#if LOG_DISABLED
#define LOG_VERBOSE(channel, ...) ((void)0)
#else
#define LOG_VERBOSE(channel, ...) WTFLogVerbose(__FILE__, __LINE__, WTF_PRETTY_FUNCTION, &LOG_CHANNEL(channel), __VA_ARGS__)
#endif

/* RELEASE_LOG */

#if RELEASE_LOG_DISABLED
#define RELEASE_LOG(      channel, format, ...) ((void)0)
#define RELEASE_LOG_ERROR(channel, format, ...) LOG_ERROR(format, ##__VA_ARGS__)

#define RELEASE_LOG_IF(      isAllowed, channel, format, ...) ((void)0)
#define RELEASE_LOG_ERROR_IF(isAllowed, channel, format, ...) do { if (isAllowed) RELEASE_LOG_ERROR(channel, format, ##__VA_ARGS__); } while (0)
#else
#define RELEASE_LOG(      channel, format, ...) os_log(      LOG_CHANNEL(channel).osLogChannel, format, ##__VA_ARGS__)
#define RELEASE_LOG_ERROR(channel, format, ...) os_log_error(LOG_CHANNEL(channel).osLogChannel, format, ##__VA_ARGS__)

#define RELEASE_LOG_IF(      isAllowed, channel, format, ...) do { if (isAllowed) RELEASE_LOG(      channel, format, ##__VA_ARGS__); } while (0)
#define RELEASE_LOG_ERROR_IF(isAllowed, channel, format, ...) do { if (isAllowed) RELEASE_LOG_ERROR(channel, format, ##__VA_ARGS__); } while (0)
#endif


/* RELEASE_ASSERT */

#if ASSERT_DISABLED
#define RELEASE_ASSERT(assertion) do { \
    if (UNLIKELY(!(assertion))) \
        CRASH(); \
} while (0)
#define RELEASE_ASSERT_WITH_MESSAGE(assertion, ...) RELEASE_ASSERT(assertion)
#define RELEASE_ASSERT_NOT_REACHED() CRASH()
#else
#define RELEASE_ASSERT(assertion) ASSERT(assertion)
#define RELEASE_ASSERT_WITH_MESSAGE(assertion, ...) ASSERT_WITH_MESSAGE(assertion, __VA_ARGS__)
#define RELEASE_ASSERT_NOT_REACHED() ASSERT_NOT_REACHED()
#endif

/* UNREACHABLE_FOR_PLATFORM */

#if COMPILER(CLANG)
// This would be a macro except that its use of #pragma works best around
// a function. Hence it uses macro naming convention.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
static inline void UNREACHABLE_FOR_PLATFORM()
{
    RELEASE_ASSERT_NOT_REACHED();
}
#pragma clang diagnostic pop
#else
#define UNREACHABLE_FOR_PLATFORM() RELEASE_ASSERT_NOT_REACHED()
#endif


#endif /* WTF_Assertions_h */
