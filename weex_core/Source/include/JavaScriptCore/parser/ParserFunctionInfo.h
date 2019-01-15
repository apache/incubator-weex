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

template <class TreeBuilder>
struct ParserFunctionInfo {
    const Identifier* name = 0;
    typename TreeBuilder::FunctionBody body = 0;
    unsigned parameterCount = 0;
    unsigned functionLength = 0;
    unsigned startOffset = 0;
    unsigned endOffset = 0;
    int startLine = 0;
    int endLine = 0;
    unsigned parametersStartColumn = 0;
};

template <class TreeBuilder>
struct ParserClassInfo {
    const Identifier* className { nullptr };
    unsigned startOffset { 0 };
    unsigned endOffset { 0 };
    int startLine { 0 };
    unsigned startColumn { 0 };
};

}
