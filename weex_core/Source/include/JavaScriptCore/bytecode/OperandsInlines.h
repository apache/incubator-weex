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

#include "Operands.h"
#include <wtf/CommaPrinter.h>

namespace JSC {

template<typename T>
void Operands<T>::dumpInContext(PrintStream& out, DumpContext* context) const
{
    CommaPrinter comma(" ");
    for (size_t argumentIndex = numberOfArguments(); argumentIndex--;) {
        if (!argument(argumentIndex))
            continue;
        out.print(comma, "arg", argumentIndex, ":", inContext(argument(argumentIndex), context));
    }
    for (size_t localIndex = 0; localIndex < numberOfLocals(); ++localIndex) {
        if (!local(localIndex))
            continue;
        out.print(comma, "loc", localIndex, ":", inContext(local(localIndex), context));
    }
}

template<typename T>
void Operands<T>::dump(PrintStream& out) const
{
    CommaPrinter comma(" ");
    for (size_t argumentIndex = numberOfArguments(); argumentIndex--;) {
        if (!argument(argumentIndex))
            continue;
        out.print(comma, "arg", argumentIndex, ":", argument(argumentIndex));
    }
    for (size_t localIndex = 0; localIndex < numberOfLocals(); ++localIndex) {
        if (!local(localIndex))
            continue;
        out.print(comma, "loc", localIndex, ":", local(localIndex));
    }
}

} // namespace JSC
