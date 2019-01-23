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
#ifndef StaticConstructors_h
#define StaticConstructors_h

// We need to avoid having static constructors. We achieve this
// with two separate methods for GCC and MSVC. Both methods prevent the static
// initializers from being registered and called on program startup. On GCC, we
// declare the global objects with a different type that can be POD default
// initialized by the linker/loader. On MSVC we use a special compiler feature
// to have the CRT ignore our static initializers. The constructors will never
// be called and the objects will be left uninitialized.
//
// With both of these approaches, we must define and explicitly call an init
// routine that uses placement new to create the objects and overwrite the
// uninitialized placeholders.
//
// This is not completely portable, but is what we have for now without
// changing how a lot of code accesses these global objects.

#ifdef SKIP_STATIC_CONSTRUCTORS_ON_MSVC
// - Assume that all includes of this header want ALL of their static
//   initializers ignored. This is currently the case. This means that if
//   a .cc includes this header (or it somehow gets included), all static
//   initializers after the include will not be executed.
// - We do this with a pragma, so that all of the static initializer pointers
//   go into our own section, and the CRT won't call them. Eventually it would
//   be nice if the section was discarded, because we don't want the pointers.
//   See: http://msdn.microsoft.com/en-us/library/7977wcck(VS.80).aspx
#pragma warning(disable:4075)
#pragma init_seg(".unwantedstaticinits")
#endif

#ifndef SKIP_STATIC_CONSTRUCTORS_ON_GCC
    // Define an global in the normal way.
#define DEFINE_GLOBAL(type, name, ...) \
    const type name;

#else
// Define an correctly-sized array of pointers to avoid static initialization.
// Use an array of pointers instead of an array of char in case there is some alignment issue.
#define DEFINE_GLOBAL(type, name, ...) \
    void * name[(sizeof(type) + sizeof(void *) - 1) / sizeof(void *)];
#endif

#endif // StaticConstructors_h
