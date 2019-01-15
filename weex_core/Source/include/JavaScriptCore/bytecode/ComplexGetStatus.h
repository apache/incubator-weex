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

#include "JSCJSValue.h"
#include "ObjectPropertyConditionSet.h"
#include "PropertyOffset.h"

namespace JSC {

class CodeBlock;
class StructureChain;

// This class is useful for figuring out how to inline a cached get-like access. We
// say "get-like" because this is appropriate for loading the GetterSetter object in
// a put_by_id that hits a setter. Notably, this doesn't figure out how to call
// accessors, or even whether they should be called. What it gives us, is a way of
// determining how to load the value from the requested property (identified by a
// StringImpl* uid) from an object of the given structure in the given CodeBlock,
// assuming that such an access had already been cached by Repatch (and so Repatch had
// already done a bunch of safety checks). This doesn't reexecute any checks that
// Repatch would have executed, and for prototype chain accesses, it doesn't ask the
// objects in the prototype chain whether their getOwnPropertySlot would attempt to
// intercept the access - so this really is only appropriate if you already know that
// one of the JITOperations had OK'd this for caching and that Repatch concurred.
//
// The typical use pattern is something like:
//
//     ComplexGetStatus status = ComplexGetStatus::computeFor(...);
//     switch (status.kind()) {
//     case ComplexGetStatus::ShouldSkip:
//         // Handle the case where this kind of access is possibly safe but wouldn't
//         // pass the required safety checks. For example, if an IC gives us a list of
//         // accesses and one of them is ShouldSkip, then we should pretend as if it
//         // wasn't even there.
//         break;
//     case ComplexGetStatus::TakesSlowPath:
//         // This kind of access is not safe to inline. Bail out of any attempst to
//         // inline.
//         break;
//     case ComplexGetStatus::Inlineable:
//         // The good stuff goes here. If it's Inlineable then the other properties of
//         // the 'status' object will tell you everything you need to know about how
//         // to execute the get-like operation.
//         break;
//     }

class ComplexGetStatus {
public:
    enum Kind {
        ShouldSkip,
        TakesSlowPath,
        Inlineable
    };
    
    ComplexGetStatus()
        : m_kind(ShouldSkip)
        , m_offset(invalidOffset)
    {
    }
    
    static ComplexGetStatus skip()
    {
        return ComplexGetStatus();
    }
    
    static ComplexGetStatus takesSlowPath()
    {
        ComplexGetStatus result;
        result.m_kind = TakesSlowPath;
        return result;
    }
    
    static ComplexGetStatus computeFor(
        Structure* headStructure, const ObjectPropertyConditionSet&, UniquedStringImpl* uid);
    
    Kind kind() const { return m_kind; }
    PropertyOffset offset() const { return m_offset; }
    const ObjectPropertyConditionSet& conditionSet() const { return m_conditionSet; }
    
private:
    Kind m_kind;
    PropertyOffset m_offset;
    ObjectPropertyConditionSet m_conditionSet;
};

} // namespace JSC
