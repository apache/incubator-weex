/*
 *  Copyright (C) 2006-2011, 2015-2016 Apple Inc. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
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
