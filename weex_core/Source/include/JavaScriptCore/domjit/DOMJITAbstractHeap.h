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

#include "DOMJITHeapRange.h"
#include <wtf/Vector.h>
#include <wtf/text/WTFString.h>

#if ENABLE(JIT)

namespace JSC { namespace DOMJIT {

class AbstractHeap {
public:
    AbstractHeap(const String& name)
        : m_name(name)
    {
    }

    void setParent(AbstractHeap* parent)
    {
        ASSERT(!m_parent);
        parent->m_children.append(this);
        m_parent = parent;
    }

    bool isRoot() const { return !m_parent; }

    JS_EXPORT_PRIVATE void compute(unsigned begin);

    bool isComputed() const { return !!m_range; }
    HeapRange range() const { return m_range; }

    JS_EXPORT_PRIVATE void dump(PrintStream&) const;
    JS_EXPORT_PRIVATE void shallowDump(PrintStream&) const;
    JS_EXPORT_PRIVATE void deepDump(PrintStream&, unsigned indent = 0) const;

private:
    String m_name;
    AbstractHeap* m_parent { nullptr };
    Vector<AbstractHeap*> m_children { };
    HeapRange m_range;
};

} }

#endif
