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

#include <wtf/StackBounds.h>
#include <wtf/StackStats.h>
#include <wtf/Vector.h>

namespace JSC {

class JSGlobalObject;
class VM;

class VMEntryScope {
public:
    JS_EXPORT_PRIVATE VMEntryScope(VM&, JSGlobalObject*);
    JS_EXPORT_PRIVATE ~VMEntryScope();

    VM& vm() const { return m_vm; }
    JSGlobalObject* globalObject() const { return m_globalObject; }

    void addDidPopListener(std::function<void ()>);

private:
    VM& m_vm;
    JSGlobalObject* m_globalObject;
    Vector<std::function<void ()>> m_didPopListeners;
};

} // namespace JSC
