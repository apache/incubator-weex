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
