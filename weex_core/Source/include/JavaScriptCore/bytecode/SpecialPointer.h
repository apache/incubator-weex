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

namespace JSC {

class CodeBlock;
class JSGlobalObject;

namespace Special {
enum Pointer {
    CallFunction,
    ApplyFunction,
    ObjectConstructor,
    ArrayConstructor,
    TableSize // Not a real special pointer. Use this to determine the number of pointers.
};
} // namespace Special

enum class LinkTimeConstant {
    ThrowTypeErrorFunction,
};
const unsigned LinkTimeConstantCount = 1;

inline bool pointerIsFunction(Special::Pointer pointer)
{
    ASSERT_UNUSED(pointer, pointer < Special::TableSize);
    return true;
}

inline bool pointerIsCell(Special::Pointer pointer)
{
    ASSERT_UNUSED(pointer, pointer < Special::TableSize);
    return true;
}

void* actualPointerFor(JSGlobalObject*, Special::Pointer);
void* actualPointerFor(CodeBlock*, Special::Pointer);

} // namespace JSC
