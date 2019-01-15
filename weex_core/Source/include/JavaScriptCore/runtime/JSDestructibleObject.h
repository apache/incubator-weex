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

#include "JSObject.h"

namespace JSC {

struct ClassInfo;

class JSDestructibleObject : public JSNonFinalObject {
public:
    typedef JSNonFinalObject Base;

    static const bool needsDestruction = true;
    
    template<typename CellType>
    static Subspace* subspaceFor(VM& vm)
    {
        return &vm.destructibleObjectSpace;
    }

    const ClassInfo* classInfo() const { return m_classInfo; }
    
    static ptrdiff_t classInfoOffset() { return OBJECT_OFFSETOF(JSDestructibleObject, m_classInfo); }

protected:
    JSDestructibleObject(VM& vm, Structure* structure, Butterfly* butterfly = 0)
        : JSNonFinalObject(vm, structure, butterfly)
        , m_classInfo(structure->classInfo())
    {
        ASSERT(m_classInfo);
    }

private:
    const ClassInfo* m_classInfo;
};

} // namespace JSC
