/*
 * Copyright (C) 2009 Apple Inc. All rights reserved.
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

#ifndef VMTags_h
#define VMTags_h

// On Mac OS X, the VM subsystem allows tagging memory requested from mmap and vm_map
// in order to aid tools that inspect system memory use. 
#if OS(DARWIN)

#include <mach/vm_statistics.h>

#if defined(VM_MEMORY_TCMALLOC)
#define VM_TAG_FOR_TCMALLOC_MEMORY VM_MAKE_TAG(VM_MEMORY_TCMALLOC)
#else
#define VM_TAG_FOR_TCMALLOC_MEMORY VM_MAKE_TAG(53)
#endif // defined(VM_MEMORY_TCMALLOC)

#if defined(VM_MEMORY_JAVASCRIPT_JIT_EXECUTABLE_ALLOCATOR)
#define VM_TAG_FOR_EXECUTABLEALLOCATOR_MEMORY VM_MAKE_TAG(VM_MEMORY_JAVASCRIPT_JIT_EXECUTABLE_ALLOCATOR)
#else
#define VM_TAG_FOR_EXECUTABLEALLOCATOR_MEMORY VM_MAKE_TAG(64)
#endif // defined(VM_MEMORY_JAVASCRIPT_JIT_EXECUTABLE_ALLOCATOR)

#if defined(VM_MEMORY_JAVASCRIPT_JIT_REGISTER_FILE)
#define VM_TAG_FOR_REGISTERFILE_MEMORY VM_MAKE_TAG(VM_MEMORY_JAVASCRIPT_JIT_REGISTER_FILE)
#else
#define VM_TAG_FOR_REGISTERFILE_MEMORY VM_MAKE_TAG(65)
#endif // defined(VM_MEMORY_JAVASCRIPT_JIT_REGISTER_FILE)

#if defined(VM_MEMORY_JAVASCRIPT_CORE)
#define VM_TAG_FOR_COLLECTOR_MEMORY VM_MAKE_TAG(VM_MEMORY_JAVASCRIPT_CORE)
#else
#define VM_TAG_FOR_COLLECTOR_MEMORY VM_MAKE_TAG(63)
#endif // defined(VM_MEMORY_JAVASCRIPT_CORE)

#else // OS(DARWIN)

#define VM_TAG_FOR_TCMALLOC_MEMORY -1
#define VM_TAG_FOR_COLLECTOR_MEMORY -1
#define VM_TAG_FOR_EXECUTABLEALLOCATOR_MEMORY -1
#define VM_TAG_FOR_REGISTERFILE_MEMORY -1

#endif // OS(DARWIN)

#endif // VMTags_h
