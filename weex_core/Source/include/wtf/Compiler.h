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
#ifndef WTF_Compiler_h
#define WTF_Compiler_h

/* COMPILER() - the compiler being used to build the project */
#define COMPILER(WTF_FEATURE) (defined WTF_COMPILER_##WTF_FEATURE  && WTF_COMPILER_##WTF_FEATURE)

/* COMPILER_SUPPORTS() - whether the compiler being used to build the project supports the given feature. */
#define COMPILER_SUPPORTS(WTF_COMPILER_FEATURE) (defined WTF_COMPILER_SUPPORTS_##WTF_COMPILER_FEATURE  && WTF_COMPILER_SUPPORTS_##WTF_COMPILER_FEATURE)

/* COMPILER_QUIRK() - whether the compiler being used to build the project requires a given quirk. */
#define COMPILER_QUIRK(WTF_COMPILER_QUIRK) (defined WTF_COMPILER_QUIRK_##WTF_COMPILER_QUIRK  && WTF_COMPILER_QUIRK_##WTF_COMPILER_QUIRK)

/* COMPILER_HAS_CLANG_BUILTIN() - whether the compiler supports a particular clang builtin. */
#ifdef __has_builtin
#define COMPILER_HAS_CLANG_BUILTIN(x) __has_builtin(x)
#else
#define COMPILER_HAS_CLANG_BUILTIN(x) 0
#endif

/* COMPILER_HAS_CLANG_HEATURE() - whether the compiler supports a particular language or library feature. */
/* http://clang.llvm.org/docs/LanguageExtensions.html#has-feature-and-has-extension */
#ifdef __has_feature
#define COMPILER_HAS_CLANG_FEATURE(x) __has_feature(x)
#else
#define COMPILER_HAS_CLANG_FEATURE(x) 0
#endif

/* ==== COMPILER() - primary detection of the compiler being used to build the project, in alphabetical order ==== */

/* COMPILER(CLANG) - Clang  */

#if defined(__clang__)
#define WTF_COMPILER_CLANG 1
#define WTF_COMPILER_SUPPORTS_BLOCKS COMPILER_HAS_CLANG_FEATURE(blocks)
#define WTF_COMPILER_SUPPORTS_C_STATIC_ASSERT COMPILER_HAS_CLANG_FEATURE(c_static_assert)
#define WTF_COMPILER_SUPPORTS_CXX_REFERENCE_QUALIFIED_FUNCTIONS COMPILER_HAS_CLANG_FEATURE(cxx_reference_qualified_functions)
#define WTF_COMPILER_SUPPORTS_FALLTHROUGH_WARNINGS COMPILER_HAS_CLANG_FEATURE(cxx_attributes) && __has_warning("-Wimplicit-fallthrough")
#define WTF_COMPILER_SUPPORTS_CXX_EXCEPTIONS COMPILER_HAS_CLANG_FEATURE(cxx_exceptions)
#define WTF_COMPILER_SUPPORTS_BUILTIN_IS_TRIVIALLY_COPYABLE COMPILER_HAS_CLANG_FEATURE(is_trivially_copyable)

#ifdef __cplusplus
#if __cplusplus <= 201103L
#define WTF_CPP_STD_VER 11
#elif __cplusplus <= 201402L
#define WTF_CPP_STD_VER 14
#endif
#endif

#endif // defined(__clang__)

/* COMPILER(GCC_OR_CLANG) - GNU Compiler Collection or Clang */
#if defined(__GNUC__)
#define WTF_COMPILER_GCC_OR_CLANG 1
#endif

/* COMPILER(GCC) - GNU Compiler Collection */
/* Note: This section must come after the Clang section since we check !COMPILER(CLANG) here. */
#if COMPILER(GCC_OR_CLANG) && !COMPILER(CLANG)
#define WTF_COMPILER_GCC 1
#define WTF_COMPILER_SUPPORTS_CXX_REFERENCE_QUALIFIED_FUNCTIONS 1

#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#define GCC_VERSION_AT_LEAST(major, minor, patch) (GCC_VERSION >= (major * 10000 + minor * 100 + patch))

#if !GCC_VERSION_AT_LEAST(4, 9, 0)
#error "Please use a newer version of GCC. WebKit requires GCC 4.9.0 or newer to compile."
#endif

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define WTF_COMPILER_SUPPORTS_C_STATIC_ASSERT 1
#endif

#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"

#endif /* COMPILER(GCC) */

/* COMPILER(MINGW) - MinGW GCC */

#if defined(__MINGW32__)
#define WTF_COMPILER_MINGW 1
#include <_mingw.h>
#endif

/* COMPILER(MINGW64) - mingw-w64 GCC - used as additional check to exclude mingw.org specific functions */

/* Note: This section must come after the MinGW section since we check COMPILER(MINGW) here. */

#if COMPILER(MINGW) && defined(__MINGW64_VERSION_MAJOR) /* best way to check for mingw-w64 vs mingw.org */
#define WTF_COMPILER_MINGW64 1
#endif

/* COMPILER(MSVC) - Microsoft Visual C++ */

#if defined(_MSC_VER)

#define WTF_COMPILER_MSVC 1
#define WTF_COMPILER_SUPPORTS_CXX_REFERENCE_QUALIFIED_FUNCTIONS 1

#if _MSC_VER < 1900
#error "Please use a newer version of Visual Studio. WebKit requires VS2015 or newer to compile."
#endif

#endif

/* COMPILER(SUNCC) */

#if defined(__SUNPRO_CC) || defined(__SUNPRO_C)
#define WTF_COMPILER_SUNCC 1
#endif

#if !COMPILER(CLANG) && !COMPILER(MSVC)
#define WTF_COMPILER_QUIRK_CONSIDERS_UNREACHABLE_CODE 1
#endif

/* ==== COMPILER_SUPPORTS - additional compiler feature detection, in alphabetical order ==== */

/* COMPILER_SUPPORTS(EABI) */

#if defined(__ARM_EABI__) || defined(__EABI__)
#define WTF_COMPILER_SUPPORTS_EABI 1
#endif

/* RELAXED_CONSTEXPR */

#if defined(__cpp_constexpr) && __cpp_constexpr >= 201304
#define WTF_COMPILER_SUPPORTS_RELAXED_CONSTEXPR 1
#endif

#if !defined(RELAXED_CONSTEXPR)
#if COMPILER_SUPPORTS(RELAXED_CONSTEXPR)
#define RELAXED_CONSTEXPR constexpr
#else
#define RELAXED_CONSTEXPR
#endif
#endif

#define ASAN_ENABLED COMPILER_HAS_CLANG_FEATURE(address_sanitizer)

#if ASAN_ENABLED
#define SUPPRESS_ASAN __attribute__((no_sanitize_address))
#else
#define SUPPRESS_ASAN
#endif

/* ==== Compiler-independent macros for various compiler features, in alphabetical order ==== */

/* ALWAYS_INLINE */

#if !defined(ALWAYS_INLINE) && COMPILER(GCC_OR_CLANG) && defined(NDEBUG) && !COMPILER(MINGW)
#define ALWAYS_INLINE inline __attribute__((__always_inline__))
#endif

#if !defined(ALWAYS_INLINE) && COMPILER(MSVC) && defined(NDEBUG)
#define ALWAYS_INLINE __forceinline
#endif

#if !defined(ALWAYS_INLINE)
#define ALWAYS_INLINE inline
#endif

/* WTF_EXTERN_C_{BEGIN, END} */

#ifdef __cplusplus
#define WTF_EXTERN_C_BEGIN extern "C" {
#define WTF_EXTERN_C_END }
#else
#define WTF_EXTERN_C_BEGIN
#define WTF_EXTERN_C_END
#endif

/* FALLTHROUGH */

#if !defined(FALLTHROUGH) && COMPILER_SUPPORTS(FALLTHROUGH_WARNINGS) && COMPILER(CLANG)
#define FALLTHROUGH [[clang::fallthrough]]
#endif

#if !defined(FALLTHROUGH)
#define FALLTHROUGH
#endif

/* LIKELY */

#if !defined(LIKELY) && COMPILER(GCC_OR_CLANG)
#define LIKELY(x) __builtin_expect(!!(x), 1)
#endif

#if !defined(LIKELY)
#define LIKELY(x) (x)
#endif

/* NEVER_INLINE */

#if !defined(NEVER_INLINE) && COMPILER(GCC_OR_CLANG)
#define NEVER_INLINE __attribute__((__noinline__))
#endif

#if !defined(NEVER_INLINE) && COMPILER(MSVC)
#define NEVER_INLINE __declspec(noinline)
#endif

#if !defined(NEVER_INLINE)
#define NEVER_INLINE
#endif

/* NO_RETURN */

#if !defined(NO_RETURN) && COMPILER(GCC_OR_CLANG)
#define NO_RETURN __attribute((__noreturn__))
#endif

#if !defined(NO_RETURN) && COMPILER(MSVC)
#define NO_RETURN __declspec(noreturn)
#endif

#if !defined(NO_RETURN)
#define NO_RETURN
#endif

/* RETURNS_NONNULL */
#if !defined(RETURNS_NONNULL) && COMPILER(GCC_OR_CLANG)
#define RETURNS_NONNULL __attribute__((returns_nonnull))
#endif

#if !defined(RETURNS_NONNULL)
#define RETURNS_NONNULL
#endif

/* NO_RETURN_WITH_VALUE */

#if !defined(NO_RETURN_WITH_VALUE) && !COMPILER(MSVC)
#define NO_RETURN_WITH_VALUE NO_RETURN
#endif

#if !defined(NO_RETURN_WITH_VALUE)
#define NO_RETURN_WITH_VALUE
#endif

/* OBJC_CLASS */

#if !defined(OBJC_CLASS) && defined(__OBJC__)
#define OBJC_CLASS @class
#endif

#if !defined(OBJC_CLASS)
#define OBJC_CLASS class
#endif

/* PURE_FUNCTION */

#if !defined(PURE_FUNCTION) && COMPILER(GCC_OR_CLANG)
#define PURE_FUNCTION __attribute__((__pure__))
#endif

#if !defined(PURE_FUNCTION)
#define PURE_FUNCTION
#endif

/* REFERENCED_FROM_ASM */

#if !defined(REFERENCED_FROM_ASM) && COMPILER(GCC_OR_CLANG)
#define REFERENCED_FROM_ASM __attribute__((__used__))
#endif

#if !defined(REFERENCED_FROM_ASM)
#define REFERENCED_FROM_ASM
#endif

/* UNLIKELY */

#if !defined(UNLIKELY) && COMPILER(GCC_OR_CLANG)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#endif

#if !defined(UNLIKELY)
#define UNLIKELY(x) (x)
#endif

/* UNUSED_LABEL */

/* Keep the compiler from complaining for a local label that is defined but not referenced. */
/* Helpful when mixing hand-written and autogenerated code. */

#if !defined(UNUSED_LABEL) && COMPILER(MSVC)
#define UNUSED_LABEL(label) if (false) goto label
#endif

#if !defined(UNUSED_LABEL)
#define UNUSED_LABEL(label) UNUSED_PARAM(&& label)
#endif

/* UNUSED_PARAM */

#if !defined(UNUSED_PARAM) && COMPILER(MSVC)
#define UNUSED_PARAM(variable) (void)&variable
#endif

#if !defined(UNUSED_PARAM)
#define UNUSED_PARAM(variable) (void)variable
#endif

/* WARN_UNUSED_RETURN */

#if !defined(WARN_UNUSED_RETURN) && COMPILER(GCC_OR_CLANG)
#define WARN_UNUSED_RETURN __attribute__((__warn_unused_result__))
#endif

#if !defined(WARN_UNUSED_RETURN)
#define WARN_UNUSED_RETURN
#endif

#if !defined(__has_include) && COMPILER(MSVC)
#define __has_include(path) 0
#endif

#endif /* WTF_Compiler_h */
