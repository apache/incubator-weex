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

#if ENABLE(DFG_JIT)

#include "DFGNodeFlags.h"
#include "SpeculatedType.h"
#include <wtf/PrintStream.h>

namespace JSC { namespace DFG {

enum UseKind {
    // The DFG has 3 representations of values used:

    // 1. The JSValue representation for a JSValue that must be stored in a GP
    //    register (or a GP register pair), and follows rules for boxing and unboxing
    //    that allow the JSValue to be stored as either fully boxed JSValues, or
    //    unboxed Int32, Booleans, Cells, etc. in 32-bit as appropriate.
    UntypedUse, // UntypedUse must come first (value 0).
    Int32Use,
    KnownInt32Use,
    AnyIntUse,
    NumberUse,
    RealNumberUse,
    BooleanUse,
    KnownBooleanUse,
    CellUse,
    KnownCellUse,
    CellOrOtherUse,
    ObjectUse,
    ArrayUse,
    FunctionUse,
    FinalObjectUse,
    RegExpObjectUse,
    ProxyObjectUse,
    DerivedArrayUse,
    ObjectOrOtherUse,
    StringIdentUse,
    StringUse,
    StringOrOtherUse,
    KnownStringUse,
    KnownPrimitiveUse, // This bizarre type arises for op_strcat, which has a bytecode guarantee that it will only see primitives (i.e. not objects).
    SymbolUse,
    MapObjectUse,
    SetObjectUse,
    StringObjectUse,
    StringOrStringObjectUse,
    NotStringVarUse,
    NotCellUse,
    OtherUse,
    MiscUse,

    // 2. The Double representation for an unboxed double value that must be stored
    //    in an FP register.
    DoubleRepUse,
    DoubleRepRealUse,
    DoubleRepAnyIntUse,

    // 3. The Int52 representation for an unboxed integer value that must be stored
    //    in a GP register.
    Int52RepUse,

    LastUseKind // Must always be the last entry in the enum, as it is used to denote the number of enum elements.
};

inline SpeculatedType typeFilterFor(UseKind useKind)
{
    switch (useKind) {
    case UntypedUse:
        return SpecBytecodeTop;
    case Int32Use:
    case KnownInt32Use:
        return SpecInt32Only;
    case Int52RepUse:
        return SpecAnyInt;
    case AnyIntUse:
        return SpecInt32Only | SpecAnyIntAsDouble;
    case NumberUse:
        return SpecBytecodeNumber;
    case RealNumberUse:
        return SpecBytecodeRealNumber;
    case DoubleRepUse:
        return SpecFullDouble;
    case DoubleRepRealUse:
        return SpecDoubleReal;
    case DoubleRepAnyIntUse:
        return SpecAnyIntAsDouble;
    case BooleanUse:
    case KnownBooleanUse:
        return SpecBoolean;
    case CellUse:
    case KnownCellUse:
        return SpecCell;
    case CellOrOtherUse:
        return SpecCell | SpecOther;
    case ObjectUse:
        return SpecObject;
    case ArrayUse:
        return SpecArray;
    case FunctionUse:
        return SpecFunction;
    case FinalObjectUse:
        return SpecFinalObject;
    case RegExpObjectUse:
        return SpecRegExpObject;
    case ProxyObjectUse:
        return SpecProxyObject;
    case DerivedArrayUse:
        return SpecDerivedArray;
    case ObjectOrOtherUse:
        return SpecObject | SpecOther;
    case StringIdentUse:
        return SpecStringIdent;
    case StringUse:
    case KnownStringUse:
        return SpecString;
    case StringOrOtherUse:
        return SpecString | SpecOther;
    case KnownPrimitiveUse:
        return SpecHeapTop & ~SpecObject;
    case SymbolUse:
        return SpecSymbol;
    case MapObjectUse:
        return SpecMapObject;
    case SetObjectUse:
        return SpecSetObject;
    case StringObjectUse:
        return SpecStringObject;
    case StringOrStringObjectUse:
        return SpecString | SpecStringObject;
    case NotStringVarUse:
        return ~SpecStringVar;
    case NotCellUse:
        return ~SpecCell;
    case OtherUse:
        return SpecOther;
    case MiscUse:
        return SpecMisc;
    default:
        RELEASE_ASSERT_NOT_REACHED();
        return SpecFullTop;
    }
}

inline bool shouldNotHaveTypeCheck(UseKind kind)
{
    switch (kind) {
    case UntypedUse:
    case KnownInt32Use:
    case KnownCellUse:
    case KnownStringUse:
    case KnownPrimitiveUse:
    case KnownBooleanUse:
    case Int52RepUse:
    case DoubleRepUse:
        return true;
    default:
        return false;
    }
}

inline bool mayHaveTypeCheck(UseKind kind)
{
    return !shouldNotHaveTypeCheck(kind);
}

inline bool isNumerical(UseKind kind)
{
    switch (kind) {
    case Int32Use:
    case KnownInt32Use:
    case NumberUse:
    case RealNumberUse:
    case Int52RepUse:
    case DoubleRepUse:
    case DoubleRepRealUse:
    case AnyIntUse:
    case DoubleRepAnyIntUse:
        return true;
    default:
        return false;
    }
}

inline bool isDouble(UseKind kind)
{
    switch (kind) {
    case DoubleRepUse:
    case DoubleRepRealUse:
    case DoubleRepAnyIntUse:
        return true;
    default:
        return false;
    }
}

// Returns true if the use kind only admits cells, and is therefore appropriate for
// SpeculateCellOperand in the DFG or lowCell() in the FTL.
inline bool isCell(UseKind kind)
{
    switch (kind) {
    case CellUse:
    case KnownCellUse:
    case ObjectUse:
    case ArrayUse:
    case FunctionUse:
    case FinalObjectUse:
    case RegExpObjectUse:
    case ProxyObjectUse:
    case DerivedArrayUse:
    case StringIdentUse:
    case StringUse:
    case KnownStringUse:
    case SymbolUse:
    case StringObjectUse:
    case StringOrStringObjectUse:
    case MapObjectUse:
    case SetObjectUse:
        return true;
    default:
        return false;
    }
}

// Returns true if it uses structure in a way that could be clobbered by
// things that change the structure.
inline bool usesStructure(UseKind kind)
{
    switch (kind) {
    case StringObjectUse:
    case StringOrStringObjectUse:
        return true;
    default:
        return false;
    }
}

// Returns true if we've already guaranteed the type 
inline bool alreadyChecked(UseKind kind, SpeculatedType type)
{
    // If the check involves the structure then we need to know more than just the type to be sure
    // that the check is done.
    if (usesStructure(kind))
        return false;
    
    return !(type & ~typeFilterFor(kind));
}

inline UseKind useKindForResult(NodeFlags result)
{
    ASSERT(!(result & ~NodeResultMask));
    switch (result) {
    case NodeResultInt52:
        return Int52RepUse;
    case NodeResultDouble:
        return DoubleRepUse;
    default:
        return UntypedUse;
    }
}

} } // namespace JSC::DFG

namespace WTF {

void printInternal(PrintStream&, JSC::DFG::UseKind);

} // namespace WTF

#endif // ENABLE(DFG_JIT)
