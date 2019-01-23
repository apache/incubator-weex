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

#include "JSFunction.h"

namespace JSC {

class JSGlobalObject;
class LLIntOffsetsExtractor;
class LLIntDesiredOffsets;

class JSGeneratorFunction : public JSFunction {
    friend class JIT;
#if ENABLE(DFG_JIT)
    friend class DFG::SpeculativeJIT;
    friend class DFG::JITCompiler;
#endif
    friend class VM;
public:
    typedef JSFunction Base;

    enum class GeneratorResumeMode : int32_t {
        NormalMode = 0,
        ReturnMode = 1,
        ThrowMode = 2
    };

    enum class GeneratorState : int32_t {
        Completed = -1,
        Executing = -2,
    };

    // [this], @generator, @generatorState, @generatorValue, @generatorResumeMode, @generatorFrame.
    enum class GeneratorArgument : int32_t {
        ThisValue = 0,
        Generator = 1,
        State = 2,
        Value = 3,
        ResumeMode = 4,
        Frame = 5,
    };

    const static unsigned StructureFlags = Base::StructureFlags;

    DECLARE_EXPORT_INFO;

    static JSGeneratorFunction* create(VM&, FunctionExecutable*, JSScope*);
    static JSGeneratorFunction* create(VM&, FunctionExecutable*, JSScope*, Structure*);
    static JSGeneratorFunction* createWithInvalidatedReallocationWatchpoint(VM&, FunctionExecutable*, JSScope*);

    static size_t allocationSize(size_t inlineCapacity)
    {
        ASSERT_UNUSED(inlineCapacity, !inlineCapacity);
        return sizeof(JSGeneratorFunction);
    }

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        ASSERT(globalObject);
        return Structure::create(vm, globalObject, prototype, TypeInfo(JSFunctionType, StructureFlags), info());
    }

private:
    JSGeneratorFunction(VM&, FunctionExecutable*, JSScope*, Structure*);

    static JSGeneratorFunction* createImpl(VM&, FunctionExecutable*, JSScope*, Structure*);

    friend class LLIntOffsetsExtractor;
};

} // namespace JSC
