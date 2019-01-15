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
#ifndef InlineASM_h
#define InlineASM_h

/* asm directive helpers */ 

#if OS(DARWIN) || (OS(WINDOWS) && CPU(X86))
#define SYMBOL_STRING(name) "_" #name
#else
#define SYMBOL_STRING(name) #name
#endif

#if OS(IOS)
#define THUMB_FUNC_PARAM(name) SYMBOL_STRING(name)
#else
#define THUMB_FUNC_PARAM(name)
#endif

#if (OS(LINUX) || OS(FREEBSD)) && CPU(X86_64)
#define GLOBAL_REFERENCE(name) #name "@plt"
#elif CPU(X86) && COMPILER(MINGW)
#define GLOBAL_REFERENCE(name) "@" #name "@4"
#elif OS(LINUX) && CPU(X86) && defined(__PIC__)
#define GLOBAL_REFERENCE(name) SYMBOL_STRING(name) "@plt"
#else
#define GLOBAL_REFERENCE(name) SYMBOL_STRING(name)
#endif

#if HAVE(INTERNAL_VISIBILITY)
#define LOCAL_REFERENCE(name) SYMBOL_STRING(name)
#else
#define LOCAL_REFERENCE(name) GLOBAL_REFERENCE(name)
#endif

#if OS(DARWIN)
    // Mach-O platform
#define HIDE_SYMBOL(name) ".private_extern _" #name
#elif OS(AIX)
    // IBM's own file format
#define HIDE_SYMBOL(name) ".lglobl " #name
#elif   OS(LINUX)               \
     || OS(FREEBSD)             \
     || OS(OPENBSD)             \
     || OS(SOLARIS)             \
     || (OS(HPUX) && CPU(IA64)) \
     || OS(NETBSD)
    // ELF platform
#define HIDE_SYMBOL(name) ".hidden " #name
#else
#define HIDE_SYMBOL(name)
#endif

// FIXME: figure out how this works on all the platforms. I know that
// on ELF, the preferred form is ".Lstuff" as opposed to "Lstuff".
// Don't know about any of the others.
#if OS(DARWIN)
#define LOCAL_LABEL_STRING(name) "L" #name
#elif   OS(LINUX)               \
    || OS(FREEBSD)             \
    || OS(OPENBSD)             \
    || OS(HURD)                \
    || OS(NETBSD)              \
    || COMPILER(MINGW)
    // GNU as-compatible syntax.
#define LOCAL_LABEL_STRING(name) ".L" #name
#endif

#if (CPU(ARM_TRADITIONAL) && (defined(thumb2) || defined(__thumb2__) || defined(__thumb) || defined(__thumb__))) || CPU(ARM_THUMB2)
#define INLINE_ARM_FUNCTION(name) ".thumb" "\n" ".thumb_func " THUMB_FUNC_PARAM(name) "\n"
#else
#define INLINE_ARM_FUNCTION(name)
#endif

#endif // InlineASM_h
