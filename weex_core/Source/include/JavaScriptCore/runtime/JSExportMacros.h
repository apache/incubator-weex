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

#include <wtf/ExportMacros.h>

// See note in wtf/Platform.h for more info on EXPORT_MACROS.
#if USE(EXPORT_MACROS)
#if defined(BUILDING_JavaScriptCore) || defined(STATICALLY_LINKED_WITH_JavaScriptCore)
#define JS_EXPORT_PRIVATE WTF_EXPORT
#else
#define JS_EXPORT_PRIVATE WTF_IMPORT
#endif

#define JS_EXPORT_HIDDEN WTF_HIDDEN
#define JS_EXPORTDATA JS_EXPORT_PRIVATE
#define JS_EXPORTCLASS JS_EXPORT_PRIVATE

#else // !USE(EXPORT_MACROS)

#if OS(WINDOWS) && !COMPILER(GCC_OR_CLANG)

#if defined(BUILDING_JavaScriptCore) || defined(STATICALLY_LINKED_WITH_JavaScriptCore)
#define JS_EXPORTDATA __declspec(dllexport)
#else
#define JS_EXPORTDATA __declspec(dllimport)
#endif

#define JS_EXPORTCLASS JS_EXPORTDATA

#else // !PLATFORM...

#define JS_EXPORTDATA
#define JS_EXPORTCLASS

#endif // !PLATFORM...

#define JS_EXPORT_PRIVATE
#define JS_EXPORT_HIDDEN

#endif // USE(EXPORT_MACROS)
