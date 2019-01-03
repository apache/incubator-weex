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

enum JSType : uint8_t {
    UnspecifiedType,
    UndefinedType,
    BooleanType,
    NumberType,
    NullType,

    // The CellType value must come before any JSType that is a JSCell.
    CellType,
    StringType,
    SymbolType,

    CustomGetterSetterType,
    APIValueWrapperType,

    EvalExecutableType,
    ProgramExecutableType,
    ModuleProgramExecutableType,
    FunctionExecutableType,
    WebAssemblyExecutableType,

    UnlinkedFunctionExecutableType,
    UnlinkedProgramCodeBlockType,
    UnlinkedModuleProgramCodeBlockType,
    UnlinkedEvalCodeBlockType,
    UnlinkedFunctionCodeBlockType,

    JSFixedArrayType,
    JSSourceCodeType,
    JSScriptFetcherType,

    // The ObjectType value must come before any JSType that is a subclass of JSObject.
    ObjectType,
    FinalObjectType,
    JSCalleeType,
    JSFunctionType,
    NumberObjectType,
    ErrorInstanceType,
    PureForwardingProxyType,
    ImpureProxyType,
    WithScopeType,
    DirectArgumentsType,
    ScopedArgumentsType,

    ArrayType,
    DerivedArrayType,

    Int8ArrayType,
    Int16ArrayType,
    Int32ArrayType,
    Uint8ArrayType,
    Uint8ClampedArrayType,
    Uint16ArrayType,
    Uint32ArrayType,
    Float32ArrayType,
    Float64ArrayType,
    DataViewType,

    GetterSetterType,

    // Start environment record types.
    GlobalObjectType,
    LexicalEnvironmentType,
    GlobalLexicalEnvironmentType,
    ModuleEnvironmentType,
    StrictEvalActivationType,
    // End environment record types.

    RegExpObjectType,
    ProxyObjectType,
    JSMapType,
    JSSetType,

    WebAssemblyFunctionType,

    LastJSCObjectType = WebAssemblyFunctionType,
    MaxJSType = 0b11111111,
};

static_assert(sizeof(JSType) == sizeof(uint8_t), "sizeof(JSType) is one byte.");
static_assert(LastJSCObjectType < 128, "The highest bit is reserved for embedder's extension.");

} // namespace JSC
