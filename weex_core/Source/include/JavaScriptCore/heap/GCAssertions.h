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

#include <type_traits>
#include <wtf/Assertions.h>

#if ENABLE(GC_VALIDATION)
#define ASSERT_GC_OBJECT_LOOKS_VALID(cell) do { \
    RELEASE_ASSERT(cell);\
    RELEASE_ASSERT(cell->structure()->structure() == cell->structure()->structure()->structure()); \
} while (0)

#define ASSERT_GC_OBJECT_INHERITS(object, classInfo) do {\
    ASSERT_GC_OBJECT_LOOKS_VALID(object); \
    RELEASE_ASSERT(object->inherits(*object->JSC::JSCell::vm(), classInfo)); \
} while (0)

// Used to avoid triggering -Wundefined-bool-conversion.
#define ASSERT_THIS_GC_OBJECT_LOOKS_VALID() do { \
    RELEASE_ASSERT(this->structure()->structure() == this->structure()->structure()->structure()); \
} while (0)

#define ASSERT_THIS_GC_OBJECT_INHERITS(classInfo) do {\
    ASSERT_THIS_GC_OBJECT_LOOKS_VALID(); \
    RELEASE_ASSERT(this->inherits(*this->vm(), classInfo)); \
} while (0)

#else
#define ASSERT_GC_OBJECT_LOOKS_VALID(cell) do { (void)cell; } while (0)
#define ASSERT_GC_OBJECT_INHERITS(object, classInfo) do { (void)object; (void)classInfo; } while (0)
#define ASSERT_THIS_GC_OBJECT_LOOKS_VALID()
#define ASSERT_THIS_GC_OBJECT_INHERITS(classInfo) do { (void)classInfo; } while (0)
#endif

#define STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(klass) static_assert(std::is_trivially_destructible<klass>::value, #klass " must have a trivial destructor")
