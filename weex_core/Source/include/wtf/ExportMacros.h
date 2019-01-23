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
#ifndef ExportMacros_h
#define ExportMacros_h

#include <wtf/Platform.h>

// Different platforms have different defaults for symbol visibility. Usually
// the compiler and the linker just take care of it. However for references to
// runtime routines from JIT stubs, it matters to be able to declare a symbol as
// being local to the target being generated, and thus not subject to (e.g.) ELF
// symbol interposition rules.

#if OS(WINDOWS)
#define HAVE_INTERNAL_VISIBILITY 1
#define WTF_INTERNAL
#elif defined(__GNUC__) && !defined(__CC_ARM) && !defined(__ARMCC__)
#define HAVE_INTERNAL_VISIBILITY 1
#define WTF_INTERNAL __attribute__((visibility("hidden")))
#else
#define WTF_INTERNAL
#endif

#if OS(WINDOWS)

#define WTF_EXPORT_DECLARATION __declspec(dllexport)
#define WTF_IMPORT_DECLARATION __declspec(dllimport)
#define WTF_HIDDEN_DECLARATION

#elif defined(__GNUC__) && !defined(__CC_ARM) && !defined(__ARMCC__)
#define WTF_EXPORT_DECLARATION __attribute__((visibility("default")))
#define WTF_IMPORT_DECLARATION WTF_EXPORT_DECLARATION
#define WTF_HIDDEN_DECLARATION __attribute__((visibility("hidden")))

#else

#define WTF_EXPORT_DECLARATION
#define WTF_IMPORT_DECLARATION
#define WTF_HIDDEN_DECLARATION

#endif

#if defined(BUILDING_WTF) || defined(STATICALLY_LINKED_WITH_WTF)
#define WTF_IS_LINKED_IN_SAME_BINARY 1
#endif

// See note in wtf/Platform.h for more info on EXPORT_MACROS.
#if USE(EXPORT_MACROS)

#define WTF_EXPORT WTF_EXPORT_DECLARATION
#define WTF_IMPORT WTF_IMPORT_DECLARATION
#define WTF_HIDDEN WTF_IMPORT_DECLARATION

// FIXME: When all ports are using the export macros, we should replace
// WTF_EXPORTDATA with WTF_EXPORT_PRIVATE macros.
#if defined(WTF_IS_LINKED_IN_SAME_BINARY)
#define WTF_EXPORTDATA WTF_EXPORT
#else
#define WTF_EXPORTDATA WTF_IMPORT
#endif

#else // !USE(EXPORT_MACROS)

#if OS(WINDOWS) && !COMPILER(GCC_OR_CLANG)
#if defined(BUILDING_WTF) || defined(STATICALLY_LINKED_WITH_WTF)
#define WTF_EXPORTDATA __declspec(dllexport)
#else
#define WTF_EXPORTDATA __declspec(dllimport)
#endif
#else // !OS(WINDOWS) || COMPILER(GCC_OR_CLANG)
#define WTF_EXPORTDATA
#endif

#define WTF_EXPORTCLASS WTF_EXPORTDATA

#define WTF_EXPORT
#define WTF_IMPORT
#define WTF_HIDDEN

#endif // USE(EXPORT_MACROS)

#if defined(WTF_IS_LINKED_IN_SAME_BINARY)
#define WTF_EXPORT_PRIVATE WTF_EXPORT
#else
#define WTF_EXPORT_PRIVATE WTF_IMPORT
#endif

#define WTF_EXPORT_STRING_API WTF_EXPORT_PRIVATE

#define WTF_EXPORT_HIDDEN WTF_HIDDEN

#define HIDDEN_INLINE WTF_EXPORT_HIDDEN inline

#endif // ExportMacros_h
