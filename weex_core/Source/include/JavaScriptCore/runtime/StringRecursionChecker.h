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

#include "Interpreter.h"
#include "VMInlines.h"
#include <wtf/StackStats.h>

namespace JSC {

class StringRecursionChecker {
    WTF_MAKE_NONCOPYABLE(StringRecursionChecker);

public:
    StringRecursionChecker(ExecState*, JSObject* thisObject);
    ~StringRecursionChecker();

    JSValue earlyReturnValue() const; // 0 if everything is OK, value to return for failure cases

private:
    JSValue throwStackOverflowError();
    JSValue emptyString();
    JSValue performCheck();

    ExecState* m_exec;
    JSObject* m_thisObject;
    JSValue m_earlyReturnValue;

    StackStats::CheckPoint stackCheckpoint;
};

inline JSValue StringRecursionChecker::performCheck()
{
    VM& vm = m_exec->vm();
    if (UNLIKELY(!vm.isSafeToRecurseSoft()))
        return throwStackOverflowError();

    bool alreadyVisited = false;
    if (!vm.stringRecursionCheckFirstObject)
        vm.stringRecursionCheckFirstObject = m_thisObject;
    else if (vm.stringRecursionCheckFirstObject == m_thisObject)
        alreadyVisited = true;
    else
        alreadyVisited = !vm.stringRecursionCheckVisitedObjects.add(m_thisObject).isNewEntry;

    if (alreadyVisited)
        return emptyString(); // Return empty string to avoid infinite recursion.
    return JSValue(); // Indicate success.
}

inline StringRecursionChecker::StringRecursionChecker(ExecState* exec, JSObject* thisObject)
    : m_exec(exec)
    , m_thisObject(thisObject)
    , m_earlyReturnValue(performCheck())
{
}

inline JSValue StringRecursionChecker::earlyReturnValue() const
{
    return m_earlyReturnValue;
}

inline StringRecursionChecker::~StringRecursionChecker()
{
    if (m_earlyReturnValue)
        return;

    VM& vm = m_exec->vm();
    if (vm.stringRecursionCheckFirstObject == m_thisObject)
        vm.stringRecursionCheckFirstObject = nullptr;
    else {
        ASSERT(vm.stringRecursionCheckVisitedObjects.contains(m_thisObject));
        vm.stringRecursionCheckVisitedObjects.remove(m_thisObject);
    }
}

} // namespace JSC
