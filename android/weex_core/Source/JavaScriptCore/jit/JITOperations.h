/*
 * Copyright (C) 2013-2017 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#pragma once

#if ENABLE(JIT)

#include "JITMathICForwards.h"
#include "MacroAssemblerCodeRef.h"
#include "PropertyOffset.h"
#include "SlowPathReturnType.h"
#include "TypedArrayType.h"
#include <wtf/Platform.h>
#include <wtf/text/UniquedStringImpl.h>

namespace JSC {

typedef int64_t EncodedJSValue;
    
class ArrayAllocationProfile;
class ArrayProfile;
class Butterfly;
class CallLinkInfo;
class CodeBlock;
class ExecState;
class JSArray;
class JSCell;
class JSFunction;
class JSGlobalObject;
class JSLexicalEnvironment;
class JSObject;
class JSScope;
class JSString;
class JSValue;
class RegExpObject;
class Register;
class Structure;
class StructureStubInfo;
class Symbol;
class SymbolTable;
class WatchpointSet;

struct ByValInfo;
struct InlineCallFrame;
struct Instruction;
struct ArithProfile;

typedef ExecState CallFrame;

extern "C" {

typedef char* UnusedPtr;

// These typedefs provide typechecking when generating calls out to helper routines;
// this helps prevent calling a helper routine with the wrong arguments!
/*
    Key:
    A: JSArray*
    Aap: ArrayAllocationProfile*
    Ap: ArrayProfile*
    Arp: ArithProfile*
    B: Butterfly*
    By: ByValInfo*
    C: JSCell*
    Cb: CodeBlock*
    Cli: CallLinkInfo*
    D: double
    E: ExecState*
    F: CallFrame*
    G: JSGlobalObject*
    I: UniquedStringImpl*
    Icf: InlineCallFrame*
    Idc: const Identifier*
    J: EncodedJSValue
    Mic: JITMathIC* (can be JITAddIC*, JITMulIC*, etc).
    Jcp: const JSValue*
    Jsc: JSScope*
    Jsf: JSFunction*
    Jss: JSString*
    L: JSLexicalEnvironment*
    O: JSObject*
    P: pointer (char*)
    Pc: Instruction* i.e. bytecode PC
    Q: int64_t
    R: Register
    Reo: RegExpObject*
    S: size_t
    Sprt: SlowPathReturnType
    Ssi: StructureStubInfo*
    St: Structure*
    Symtab: SymbolTable*
    Sym: Symbol*
    T: StringImpl*
    V: void
    Vm: VM*
    Ws: WatchpointSet*
    Z: int32_t
    Ui: uint32_t
*/

typedef CallFrame* (JIT_OPERATION *F_JITOperation_EFJZZ)(ExecState*, CallFrame*, EncodedJSValue, int32_t, int32_t);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_E)(ExecState*);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EA)(ExecState*, JSArray*);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EAZ)(ExecState*, JSArray*, int32_t);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EAapJ)(ExecState*, ArrayAllocationProfile*, EncodedJSValue);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EAapJcpZ)(ExecState*, ArrayAllocationProfile*, const JSValue*, int32_t);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EC)(ExecState*, JSCell*);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_ECC)(ExecState*, JSCell*, JSCell*);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_ECI)(ExecState*, JSCell*, UniquedStringImpl*);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_ECJ)(ExecState*, JSCell*, EncodedJSValue);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_ECZ)(ExecState*, JSCell*, int32_t);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_ECZZ)(ExecState*, JSCell*, int32_t, int32_t);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EDA)(ExecState*, double, JSArray*);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EE)(ExecState*, ExecState*);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EGReoJ)(ExecState*, JSGlobalObject*, RegExpObject*, EncodedJSValue);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EGReoJss)(ExecState*, JSGlobalObject*, RegExpObject*, JSString*);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EGJJ)(ExecState*, JSGlobalObject*, EncodedJSValue, EncodedJSValue);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EI)(ExecState*, UniquedStringImpl*);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EJ)(ExecState*, EncodedJSValue);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EJZ)(ExecState*, EncodedJSValue, int32_t);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EJC)(ExecState*, EncodedJSValue, JSCell*);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EJA)(ExecState*, EncodedJSValue, JSArray*);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EJArp)(ExecState*, EncodedJSValue, ArithProfile*);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EJI)(ExecState*, EncodedJSValue, UniquedStringImpl*);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EJJ)(ExecState*, EncodedJSValue, EncodedJSValue);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EJJJ)(ExecState*, EncodedJSValue, EncodedJSValue, EncodedJSValue);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EJJAp)(ExecState*, EncodedJSValue, EncodedJSValue, ArrayProfile*);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EJJBy)(ExecState*, EncodedJSValue, EncodedJSValue, ByValInfo*);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EJJJ)(ExecState*, EncodedJSValue, EncodedJSValue, EncodedJSValue);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EJJArp)(ExecState*, EncodedJSValue, EncodedJSValue, ArithProfile*);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EJJMic)(ExecState*, EncodedJSValue, EncodedJSValue, void*);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EJMic)(ExecState*, EncodedJSValue, void*);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EJssZ)(ExecState*, JSString*, int32_t);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EJss)(ExecState*, JSString*);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EJssReo)(ExecState*, JSString*, RegExpObject*);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EJssReoJss)(ExecState*, JSString*, RegExpObject*, JSString*);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EJP)(ExecState*, EncodedJSValue, void*);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EGP)(ExecState*, JSGlobalObject*, void*);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EP)(ExecState*, void*);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EPP)(ExecState*, void*, void*);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EPPP)(ExecState*, void*, void*, void*);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EPS)(ExecState*, void*, size_t);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EPc)(ExecState*, Instruction*);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EJscC)(ExecState*, JSScope*, JSCell*);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EJscCJ)(ExecState*, JSScope*, JSCell*, EncodedJSValue);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EReoJ)(ExecState*, RegExpObject*, EncodedJSValue);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EReoJss)(ExecState*, RegExpObject*, JSString*);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_ESS)(ExecState*, size_t, size_t);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_ESsiCI)(ExecState*, StructureStubInfo*, JSCell*, UniquedStringImpl*);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_ESsiJI)(ExecState*, StructureStubInfo*, EncodedJSValue, UniquedStringImpl*);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_ESsiJJI)(ExecState*, StructureStubInfo*, EncodedJSValue, EncodedJSValue, UniquedStringImpl*);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EZ)(ExecState*, int32_t);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EZIcfZ)(ExecState*, int32_t, InlineCallFrame*, int32_t);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EZZ)(ExecState*, int32_t, int32_t);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EZSymtabJ)(ExecState*, int32_t, SymbolTable*, EncodedJSValue);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EOIUi)(ExecState*, JSObject*, UniquedStringImpl*, uint32_t);
typedef EncodedJSValue (JIT_OPERATION *J_JITOperation_EJJI)(ExecState*, EncodedJSValue, EncodedJSValue, UniquedStringImpl*);
typedef JSCell* (JIT_OPERATION *C_JITOperation_EPUi)(ExecState*, void*, uint32_t);
typedef JSCell* (JIT_OPERATION *C_JITOperation_E)(ExecState*);
typedef JSCell* (JIT_OPERATION *C_JITOperation_EZ)(ExecState*, int32_t);
typedef JSCell* (JIT_OPERATION *C_JITOperation_EC)(ExecState*, JSCell*);
typedef JSCell* (JIT_OPERATION *C_JITOperation_ECZ)(ExecState*, JSCell*, int32_t);
typedef JSCell* (JIT_OPERATION *C_JITOperation_ECZC)(ExecState*, JSCell*, int32_t, JSCell*);
typedef JSCell* (JIT_OPERATION *C_JITOperation_ECC)(ExecState*, JSCell*, JSCell*);
typedef JSCell* (JIT_OPERATION *C_JITOperation_EGC)(ExecState*, JSGlobalObject*, JSCell*);
typedef JSCell* (JIT_OPERATION *C_JITOperation_EGJ)(ExecState*, JSGlobalObject*, EncodedJSValue);
typedef JSCell* (JIT_OPERATION *C_JITOperation_EIcf)(ExecState*, InlineCallFrame*);
typedef JSCell* (JIT_OPERATION *C_JITOperation_EJ)(ExecState*, EncodedJSValue);
typedef JSCell* (JIT_OPERATION *C_JITOperation_EJsc)(ExecState*, JSScope*);
typedef JSCell* (JIT_OPERATION *C_JITOperation_EJscC)(ExecState*, JSScope*, JSCell*);
typedef JSCell* (JIT_OPERATION *C_JITOperation_EJZ)(ExecState*, EncodedJSValue, int32_t);
typedef JSCell* (JIT_OPERATION *C_JITOperation_EJZC)(ExecState*, EncodedJSValue, int32_t, JSCell*);
typedef JSCell* (JIT_OPERATION *C_JITOperation_EJJ)(ExecState*, EncodedJSValue, EncodedJSValue);
typedef JSCell* (JIT_OPERATION *C_JITOperation_EJJC)(ExecState*, EncodedJSValue, EncodedJSValue, JSCell*);
typedef JSCell* (JIT_OPERATION *C_JITOperation_EJJJ)(ExecState*, EncodedJSValue, EncodedJSValue, EncodedJSValue);
typedef JSCell* (JIT_OPERATION *C_JITOperation_EJscZ)(ExecState*, JSScope*, int32_t);
typedef JSCell* (JIT_OPERATION *C_JITOperation_EJssSt)(ExecState*, JSString*, Structure*);
typedef JSCell* (JIT_OPERATION *C_JITOperation_EJssJss)(ExecState*, JSString*, JSString*);
typedef uintptr_t (JIT_OPERATION *C_JITOperation_B_EJssJss)(ExecState*, JSString*, JSString*);
typedef uintptr_t (JIT_OPERATION *C_JITOperation_TT)(StringImpl*, StringImpl*);
typedef JSCell* (JIT_OPERATION *C_JITOperation_EJssJssJss)(ExecState*, JSString*, JSString*, JSString*);
typedef JSCell* (JIT_OPERATION *C_JITOperation_EL)(ExecState*, JSLexicalEnvironment*);
typedef JSCell* (JIT_OPERATION *C_JITOperation_EO)(ExecState*, JSObject*);
typedef JSCell* (JIT_OPERATION *C_JITOperation_EOZ)(ExecState*, JSObject*, int32_t);
typedef JSCell* (JIT_OPERATION *C_JITOperation_ESt)(ExecState*, Structure*);
typedef JSCell* (JIT_OPERATION *C_JITOperation_EStJscSymtabJ)(ExecState*, Structure*, JSScope*, SymbolTable*, EncodedJSValue);
typedef JSCell* (JIT_OPERATION *C_JITOperation_EStRZJsfL)(ExecState*, Structure*, Register*, int32_t, JSFunction*, JSLexicalEnvironment*);
typedef JSCell* (JIT_OPERATION *C_JITOperation_EStRZJsf)(ExecState*, Structure*, Register*, int32_t, JSFunction*);
typedef JSCell* (JIT_OPERATION *C_JITOperation_EStZ)(ExecState*, Structure*, int32_t);
typedef JSCell* (JIT_OPERATION *C_JITOperation_EStZZ)(ExecState*, Structure*, int32_t, int32_t);
typedef JSCell* (JIT_OPERATION *C_JITOperation_EZ)(ExecState*, int32_t);
typedef JSCell* (JIT_OPERATION *C_JITOperation_EJscI)(ExecState*, JSScope*, UniquedStringImpl*);
typedef JSCell* (JIT_OPERATION *C_JITOperation_ECJZ)(ExecState*, JSCell*, EncodedJSValue, int32_t);
typedef double (JIT_OPERATION *D_JITOperation_D)(double);
typedef double (JIT_OPERATION *D_JITOperation_G)(JSGlobalObject*);
typedef double (JIT_OPERATION *D_JITOperation_DD)(double, double);
typedef double (JIT_OPERATION *D_JITOperation_ZZ)(int32_t, int32_t);
typedef double (JIT_OPERATION *D_JITOperation_EJ)(ExecState*, EncodedJSValue);
typedef int64_t (JIT_OPERATION *Q_JITOperation_J)(EncodedJSValue);
typedef int64_t (JIT_OPERATION *Q_JITOperation_D)(double);
typedef int32_t (JIT_OPERATION *Z_JITOperation_D)(double);
typedef int32_t (JIT_OPERATION *Z_JITOperation_E)(ExecState*);
typedef int32_t (JIT_OPERATION *Z_JITOperation_EC)(ExecState*, JSCell*);
typedef int32_t (JIT_OPERATION *Z_JITOperation_EGC)(ExecState*, JSGlobalObject*, JSCell*);
typedef int32_t (JIT_OPERATION *Z_JITOperation_ESJss)(ExecState*, size_t, JSString*);
typedef int32_t (JIT_OPERATION *Z_JITOperation_EJ)(ExecState*, EncodedJSValue);
typedef int32_t (JIT_OPERATION *Z_JITOperation_EJOJ)(ExecState*, EncodedJSValue, JSObject*, EncodedJSValue);
typedef int32_t (JIT_OPERATION *Z_JITOperation_EJZ)(ExecState*, EncodedJSValue, int32_t);
typedef int32_t (JIT_OPERATION *Z_JITOperation_EJZZ)(ExecState*, EncodedJSValue, int32_t, int32_t);
typedef int32_t (JIT_OPERATION *Z_JITOperation_EOI)(ExecState*, JSObject*, UniquedStringImpl*);
typedef int32_t (JIT_OPERATION *Z_JITOperation_EOJ)(ExecState*, JSObject*, EncodedJSValue);
typedef size_t (JIT_OPERATION *S_JITOperation_ECC)(ExecState*, JSCell*, JSCell*);
typedef size_t (JIT_OPERATION *S_JITOperation_EGC)(ExecState*, JSGlobalObject*, JSCell*);
typedef size_t (JIT_OPERATION *S_JITOperation_EGJJ)(ExecState*, JSGlobalObject*, EncodedJSValue, EncodedJSValue);
typedef size_t (JIT_OPERATION *S_JITOperation_EGReoJ)(ExecState*, JSGlobalObject*, RegExpObject*, EncodedJSValue);
typedef size_t (JIT_OPERATION *S_JITOperation_EGReoJss)(ExecState*, JSGlobalObject*, RegExpObject*, JSString*);
typedef size_t (JIT_OPERATION *S_JITOperation_EJ)(ExecState*, EncodedJSValue);
typedef size_t (JIT_OPERATION *S_JITOperation_EJI)(ExecState*, EncodedJSValue, UniquedStringImpl*);
typedef size_t (JIT_OPERATION *S_JITOperation_EJJ)(ExecState*, EncodedJSValue, EncodedJSValue);
typedef size_t (JIT_OPERATION *S_JITOperation_EOJss)(ExecState*, JSObject*, JSString*);
typedef size_t (JIT_OPERATION *S_JITOperation_EReoJ)(ExecState*, RegExpObject*, EncodedJSValue);
typedef size_t (JIT_OPERATION *S_JITOperation_EReoJss)(ExecState*, RegExpObject*, JSString*);
typedef size_t (JIT_OPERATION *S_JITOperation_J)(EncodedJSValue);
typedef SlowPathReturnType (JIT_OPERATION *Sprt_JITOperation_EZ)(ExecState*, int32_t);
typedef void (JIT_OPERATION *V_JITOperation)();
typedef void (JIT_OPERATION *V_JITOperation_E)(ExecState*);
typedef void (JIT_OPERATION *V_JITOperation_EC)(ExecState*, JSCell*);
typedef void (JIT_OPERATION *V_JITOperation_ECb)(ExecState*, CodeBlock*);
typedef void (JIT_OPERATION *V_JITOperation_ECC)(ExecState*, JSCell*, JSCell*);
typedef void (JIT_OPERATION *V_JITOperation_ECIcf)(ExecState*, JSCell*, InlineCallFrame*);
typedef void (JIT_OPERATION *V_JITOperation_ECIZC)(ExecState*, JSCell*, UniquedStringImpl*, int32_t, JSCell*);
typedef void (JIT_OPERATION *V_JITOperation_ECIZCC)(ExecState*, JSCell*, UniquedStringImpl*, int32_t, JSCell*, JSCell*);
typedef void (JIT_OPERATION *V_JITOperation_ECIZJJ)(ExecState*, JSCell*, UniquedStringImpl*, int32_t, EncodedJSValue, EncodedJSValue);
typedef void (JIT_OPERATION *V_JITOperation_ECJZC)(ExecState*, JSCell*, EncodedJSValue, int32_t, JSCell*);
typedef void (JIT_OPERATION *V_JITOperation_ECCIcf)(ExecState*, JSCell*, JSCell*, InlineCallFrame*);
typedef void (JIT_OPERATION *V_JITOperation_ECJ)(ExecState*, JSCell*, EncodedJSValue);
typedef void (JIT_OPERATION *V_JITOperation_ECJJ)(ExecState*, JSCell*, EncodedJSValue, EncodedJSValue);
typedef void (JIT_OPERATION *V_JITOperation_ECPSPS)(ExecState*, JSCell*, void*, size_t, void*, size_t);
typedef void (JIT_OPERATION *V_JITOperation_ECZ)(ExecState*, JSCell*, int32_t);
typedef void (JIT_OPERATION *V_JITOperation_ECC)(ExecState*, JSCell*, JSCell*);
typedef void (JIT_OPERATION *V_JITOperation_ECliJsf)(ExecState*, CallLinkInfo*, JSFunction*);
typedef void (JIT_OPERATION *V_JITOperation_EZSymtabJ)(ExecState*, int32_t, SymbolTable*, EncodedJSValue);
typedef void (JIT_OPERATION *V_JITOperation_EJ)(ExecState*, EncodedJSValue);
typedef void (JIT_OPERATION *V_JITOperation_EJCI)(ExecState*, EncodedJSValue, JSCell*, UniquedStringImpl*);
typedef void (JIT_OPERATION *V_JITOperation_EJJJ)(ExecState*, EncodedJSValue, EncodedJSValue, EncodedJSValue);
typedef void (JIT_OPERATION *V_JITOperation_EJJJAp)(ExecState*, EncodedJSValue, EncodedJSValue, EncodedJSValue, ArrayProfile*);
typedef void (JIT_OPERATION *V_JITOperation_EJJJBy)(ExecState*, EncodedJSValue, EncodedJSValue, EncodedJSValue, ByValInfo*);
typedef void (JIT_OPERATION *V_JITOperation_EJPP)(ExecState*, EncodedJSValue, void*, void*);
typedef void (JIT_OPERATION *V_JITOperation_EJZJ)(ExecState*, EncodedJSValue, int32_t, EncodedJSValue);
typedef void (JIT_OPERATION *V_JITOperation_EJZ)(ExecState*, EncodedJSValue, int32_t);
typedef void (JIT_OPERATION *V_JITOperation_EOZD)(ExecState*, JSObject*, int32_t, double);
typedef void (JIT_OPERATION *V_JITOperation_EOZJ)(ExecState*, JSObject*, int32_t, EncodedJSValue);
typedef void (JIT_OPERATION *V_JITOperation_EPc)(ExecState*, Instruction*);
typedef void (JIT_OPERATION *V_JITOperation_EPZJ)(ExecState*, void*, int32_t, EncodedJSValue);
typedef void (JIT_OPERATION *V_JITOperation_ESsiJJI)(ExecState*, StructureStubInfo*, EncodedJSValue, EncodedJSValue, UniquedStringImpl*);
typedef void (JIT_OPERATION *V_JITOperation_EJJJI)(ExecState*, EncodedJSValue, EncodedJSValue, EncodedJSValue, UniquedStringImpl*);
typedef void (JIT_OPERATION *V_JITOperation_EJJJJ)(ExecState*, EncodedJSValue, EncodedJSValue, EncodedJSValue, EncodedJSValue);
typedef void (JIT_OPERATION *V_JITOperation_EOJJZ)(ExecState*, JSObject*, EncodedJSValue, EncodedJSValue, int32_t);
typedef void (JIT_OPERATION *V_JITOperation_EOJssJZ)(ExecState*, JSObject*, JSString*, EncodedJSValue, int32_t);
typedef void (JIT_OPERATION *V_JITOperation_EOIJZ)(ExecState*, JSObject*, UniquedStringImpl*, EncodedJSValue, int32_t);
typedef void (JIT_OPERATION *V_JITOperation_EOSymJZ)(ExecState*, JSObject*, Symbol*, EncodedJSValue, int32_t);
typedef void (JIT_OPERATION *V_JITOperation_EOJOOZ)(ExecState*, JSObject*, EncodedJSValue, JSObject*, JSObject*, int32_t);
typedef void (JIT_OPERATION *V_JITOperation_EOJssOOZ)(ExecState*, JSObject*, JSString*, JSObject*, JSObject*, int32_t);
typedef void (JIT_OPERATION *V_JITOperation_EOIOOZ)(ExecState*, JSObject*, UniquedStringImpl*, JSObject*, JSObject*, int32_t);
typedef void (JIT_OPERATION *V_JITOperation_EOSymOOZ)(ExecState*, JSObject*, Symbol*, JSObject*, JSObject*, int32_t);
typedef void (JIT_OPERATION *V_JITOperation_EWs)(ExecState*, WatchpointSet*);
typedef void (JIT_OPERATION *V_JITOperation_EZ)(ExecState*, int32_t);
typedef void (JIT_OPERATION *V_JITOperation_EZJ)(ExecState*, int32_t, EncodedJSValue);
typedef void (JIT_OPERATION *V_JITOperation_EZJZZZ)(ExecState*, int32_t, EncodedJSValue, int32_t, int32_t, int32_t);
typedef void (JIT_OPERATION *V_JITOperation_EVm)(ExecState*, VM*);
typedef void (JIT_OPERATION *V_JITOperation_J)(EncodedJSValue);
typedef void (JIT_OPERATION *V_JITOperation_Z)(int32_t);
typedef JSCell* (JIT_OPERATION *C_JITOperation_ERUiUi)(ExecState*, Register*, uint32_t, uint32_t);
typedef void (JIT_OPERATION *V_JITOperation_EOJIUi)(ExecState*, JSObject*, EncodedJSValue, UniquedStringImpl*, uint32_t);
typedef char* (JIT_OPERATION *P_JITOperation_E)(ExecState*);
typedef char* (JIT_OPERATION *P_JITOperation_EC)(ExecState*, JSCell*);
typedef char* (JIT_OPERATION *P_JITOperation_ECli)(ExecState*, CallLinkInfo*);
typedef char* (JIT_OPERATION *P_JITOperation_EJS)(ExecState*, EncodedJSValue, size_t);
typedef char* (JIT_OPERATION *P_JITOperation_EO)(ExecState*, JSObject*);
typedef char* (JIT_OPERATION *P_JITOperation_EOS)(ExecState*, JSObject*, size_t);
typedef char* (JIT_OPERATION *P_JITOperation_EOZ)(ExecState*, JSObject*, int32_t);
typedef char* (JIT_OPERATION *P_JITOperation_EPS)(ExecState*, void*, size_t);
typedef char* (JIT_OPERATION *P_JITOperation_ES)(ExecState*, size_t);
typedef char* (JIT_OPERATION *P_JITOperation_ESJss)(ExecState*, size_t, JSString*);
typedef char* (JIT_OPERATION *P_JITOperation_ESt)(ExecState*, Structure*);
typedef char* (JIT_OPERATION *P_JITOperation_EStJ)(ExecState*, Structure*, EncodedJSValue);
typedef char* (JIT_OPERATION *P_JITOperation_EStPS)(ExecState*, Structure*, void*, size_t);
typedef char* (JIT_OPERATION *P_JITOperation_EStSS)(ExecState*, Structure*, size_t, size_t);
typedef char* (JIT_OPERATION *P_JITOperation_EStZ)(ExecState*, Structure*, int32_t);
typedef char* (JIT_OPERATION *P_JITOperation_EStZB)(ExecState*, Structure*, int32_t, Butterfly*);
typedef char* (JIT_OPERATION *P_JITOperation_EStZP)(ExecState*, Structure*, int32_t, char*);
typedef char* (JIT_OPERATION *P_JITOperation_EZZ)(ExecState*, int32_t, int32_t);
typedef SlowPathReturnType (JIT_OPERATION *Sprt_JITOperation_ECli)(ExecState*, CallLinkInfo*);
typedef StringImpl* (JIT_OPERATION *T_JITOperation_EJss)(ExecState*, JSString*);
typedef JSString* (JIT_OPERATION *Jss_JITOperation_EZ)(ExecState*, int32_t);
typedef JSString* (JIT_OPERATION *Jss_JITOperation_EJJJ)(ExecState*, EncodedJSValue, EncodedJSValue, EncodedJSValue);
typedef JSString* (JIT_OPERATION *Jss_JITOperation_EJssUi)(ExecState*, JSString*, uint32_t);

// This method is used to lookup an exception hander, keyed by faultLocation, which is
// the return location from one of the calls out to one of the helper operations above.

void JIT_OPERATION lookupExceptionHandler(VM*, ExecState*) WTF_INTERNAL;
void JIT_OPERATION lookupExceptionHandlerFromCallerFrame(VM*, ExecState*) WTF_INTERNAL;
void JIT_OPERATION operationVMHandleException(ExecState*) WTF_INTERNAL;

void JIT_OPERATION operationThrowStackOverflowError(ExecState*, CodeBlock*) WTF_INTERNAL;
#if ENABLE(WEBASSEMBLY)
void JIT_OPERATION operationThrowDivideError(ExecState*) WTF_INTERNAL;
void JIT_OPERATION operationThrowOutOfBoundsAccessError(ExecState*) WTF_INTERNAL;
#endif
int32_t JIT_OPERATION operationCallArityCheck(ExecState*) WTF_INTERNAL;
int32_t JIT_OPERATION operationConstructArityCheck(ExecState*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationTryGetById(ExecState*, StructureStubInfo*, EncodedJSValue, UniquedStringImpl*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationTryGetByIdGeneric(ExecState*, EncodedJSValue, UniquedStringImpl*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationTryGetByIdOptimize(ExecState*, StructureStubInfo*, EncodedJSValue, UniquedStringImpl*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationGetById(ExecState*, StructureStubInfo*, EncodedJSValue, UniquedStringImpl*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationGetByIdGeneric(ExecState*, EncodedJSValue, UniquedStringImpl*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationGetByIdOptimize(ExecState*, StructureStubInfo*, EncodedJSValue, UniquedStringImpl*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationGetByIdWithThisGeneric(ExecState*, StructureStubInfo*, EncodedJSValue, EncodedJSValue, UniquedStringImpl*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationGetByIdWithThisOptimize(ExecState*, StructureStubInfo*, EncodedJSValue, EncodedJSValue, UniquedStringImpl*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationInOptimize(ExecState*, StructureStubInfo*, JSCell*, UniquedStringImpl*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationIn(ExecState*, StructureStubInfo*, JSCell*, UniquedStringImpl*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationGenericIn(ExecState*, JSCell*, EncodedJSValue) WTF_INTERNAL;
void JIT_OPERATION operationPutByIdStrict(ExecState*, StructureStubInfo*, EncodedJSValue encodedValue, EncodedJSValue encodedBase, UniquedStringImpl*) WTF_INTERNAL;
void JIT_OPERATION operationPutByIdNonStrict(ExecState*, StructureStubInfo*, EncodedJSValue encodedValue, EncodedJSValue encodedBase, UniquedStringImpl*) WTF_INTERNAL;
void JIT_OPERATION operationPutByIdDirectStrict(ExecState*, StructureStubInfo*, EncodedJSValue encodedValue, EncodedJSValue encodedBase, UniquedStringImpl*) WTF_INTERNAL;
void JIT_OPERATION operationPutByIdDirectNonStrict(ExecState*, StructureStubInfo*, EncodedJSValue encodedValue, EncodedJSValue encodedBase, UniquedStringImpl*) WTF_INTERNAL;
void JIT_OPERATION operationPutByIdStrictOptimize(ExecState*, StructureStubInfo*, EncodedJSValue encodedValue, EncodedJSValue encodedBase, UniquedStringImpl*) WTF_INTERNAL;
void JIT_OPERATION operationPutByIdNonStrictOptimize(ExecState*, StructureStubInfo*, EncodedJSValue encodedValue, EncodedJSValue encodedBase, UniquedStringImpl*) WTF_INTERNAL;
void JIT_OPERATION operationPutByIdDirectStrictOptimize(ExecState*, StructureStubInfo*, EncodedJSValue encodedValue, EncodedJSValue encodedBase, UniquedStringImpl*) WTF_INTERNAL;
void JIT_OPERATION operationPutByIdDirectNonStrictOptimize(ExecState*, StructureStubInfo*, EncodedJSValue encodedValue, EncodedJSValue encodedBase, UniquedStringImpl*) WTF_INTERNAL;
void JIT_OPERATION operationPutByIdStrictBuildList(ExecState*, StructureStubInfo*, EncodedJSValue encodedValue, EncodedJSValue encodedBase, UniquedStringImpl*) WTF_INTERNAL;
void JIT_OPERATION operationPutByIdNonStrictBuildList(ExecState*, StructureStubInfo*, EncodedJSValue encodedValue, EncodedJSValue encodedBase, UniquedStringImpl*) WTF_INTERNAL;
void JIT_OPERATION operationPutByIdDirectStrictBuildList(ExecState*, StructureStubInfo*, EncodedJSValue encodedValue, EncodedJSValue encodedBase, UniquedStringImpl*) WTF_INTERNAL;
void JIT_OPERATION operationPutByIdDirectNonStrictBuildList(ExecState*, StructureStubInfo*, EncodedJSValue encodedValue, EncodedJSValue encodedBase, UniquedStringImpl*) WTF_INTERNAL;
void JIT_OPERATION operationPutByValOptimize(ExecState*, EncodedJSValue, EncodedJSValue, EncodedJSValue, ByValInfo*) WTF_INTERNAL;
void JIT_OPERATION operationDirectPutByValOptimize(ExecState*, EncodedJSValue, EncodedJSValue, EncodedJSValue, ByValInfo*) WTF_INTERNAL;
void JIT_OPERATION operationPutByValGeneric(ExecState*, EncodedJSValue, EncodedJSValue, EncodedJSValue, ByValInfo*) WTF_INTERNAL;
void JIT_OPERATION operationDirectPutByValGeneric(ExecState*, EncodedJSValue, EncodedJSValue, EncodedJSValue, ByValInfo*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationCallEval(ExecState*, ExecState*) WTF_INTERNAL;
SlowPathReturnType JIT_OPERATION operationLinkCall(ExecState*, CallLinkInfo*) WTF_INTERNAL;
void JIT_OPERATION operationLinkDirectCall(ExecState*, CallLinkInfo*, JSFunction*) WTF_INTERNAL;
SlowPathReturnType JIT_OPERATION operationLinkPolymorphicCall(ExecState*, CallLinkInfo*) WTF_INTERNAL;
SlowPathReturnType JIT_OPERATION operationVirtualCall(ExecState*, CallLinkInfo*) WTF_INTERNAL;

size_t JIT_OPERATION operationCompareLess(ExecState*, EncodedJSValue, EncodedJSValue) WTF_INTERNAL;
size_t JIT_OPERATION operationCompareLessEq(ExecState*, EncodedJSValue, EncodedJSValue) WTF_INTERNAL;
size_t JIT_OPERATION operationCompareGreater(ExecState*, EncodedJSValue, EncodedJSValue) WTF_INTERNAL;
size_t JIT_OPERATION operationCompareGreaterEq(ExecState*, EncodedJSValue, EncodedJSValue) WTF_INTERNAL;
size_t JIT_OPERATION operationCompareEq(ExecState*, EncodedJSValue, EncodedJSValue) WTF_INTERNAL;
#if USE(JSVALUE64)
EncodedJSValue JIT_OPERATION operationCompareStringEq(ExecState*, JSCell* left, JSCell* right) WTF_INTERNAL;
#else
size_t JIT_OPERATION operationCompareStringEq(ExecState*, JSCell* left, JSCell* right) WTF_INTERNAL;
#endif
EncodedJSValue JIT_OPERATION operationNewArrayWithProfile(ExecState*, ArrayAllocationProfile*, const JSValue* values, int32_t size) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationNewArrayBufferWithProfile(ExecState*, ArrayAllocationProfile*, const JSValue* values, int32_t size) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationNewArrayWithSizeAndProfile(ExecState*, ArrayAllocationProfile*, EncodedJSValue size) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationNewFunction(ExecState*, JSScope*, JSCell*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationNewFunctionWithInvalidatedReallocationWatchpoint(ExecState*, JSScope*, JSCell*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationNewGeneratorFunction(ExecState*, JSScope*, JSCell*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationNewGeneratorFunctionWithInvalidatedReallocationWatchpoint(ExecState*, JSScope*, JSCell*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationNewAsyncFunction(ExecState*, JSScope*, JSCell*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationNewAsyncFunctionWithInvalidatedReallocationWatchpoint(ExecState*, JSScope*, JSCell*) WTF_INTERNAL;
void JIT_OPERATION operationSetFunctionName(ExecState*, JSCell*, EncodedJSValue) WTF_INTERNAL;
JSCell* JIT_OPERATION operationNewObject(ExecState*, Structure*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationNewRegexp(ExecState*, void*) WTF_INTERNAL;
UnusedPtr JIT_OPERATION operationHandleTraps(ExecState*) WTF_INTERNAL;
void JIT_OPERATION operationThrow(ExecState*, EncodedJSValue) WTF_INTERNAL;
void JIT_OPERATION operationDebug(ExecState*, int32_t) WTF_INTERNAL;
#if ENABLE(DFG_JIT)
SlowPathReturnType JIT_OPERATION operationOptimize(ExecState*, int32_t) WTF_INTERNAL;
#endif
void JIT_OPERATION operationPutByIndex(ExecState*, EncodedJSValue, int32_t, EncodedJSValue);
void JIT_OPERATION operationPutGetterById(ExecState*, JSCell*, UniquedStringImpl*, int32_t options, JSCell*) WTF_INTERNAL;
void JIT_OPERATION operationPutSetterById(ExecState*, JSCell*, UniquedStringImpl*, int32_t options, JSCell*) WTF_INTERNAL;
void JIT_OPERATION operationPutGetterByVal(ExecState*, JSCell*, EncodedJSValue, int32_t attribute, JSCell*) WTF_INTERNAL;
void JIT_OPERATION operationPutSetterByVal(ExecState*, JSCell*, EncodedJSValue, int32_t attribute, JSCell*) WTF_INTERNAL;
#if USE(JSVALUE64)
void JIT_OPERATION operationPutGetterSetter(ExecState*, JSCell*, UniquedStringImpl*, int32_t attribute, EncodedJSValue, EncodedJSValue) WTF_INTERNAL;
#else
void JIT_OPERATION operationPutGetterSetter(ExecState*, JSCell*, UniquedStringImpl*, int32_t attribute, JSCell*, JSCell*) WTF_INTERNAL;
#endif
void JIT_OPERATION operationPushFunctionNameScope(ExecState*, int32_t, SymbolTable*, EncodedJSValue) WTF_INTERNAL;
void JIT_OPERATION operationPopScope(ExecState*, int32_t) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationGetByValOptimize(ExecState*, EncodedJSValue encodedBase, EncodedJSValue encodedSubscript, ByValInfo*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationGetByValGeneric(ExecState*, EncodedJSValue encodedBase, EncodedJSValue encodedSubscript, ByValInfo*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationGetByValString(ExecState*, EncodedJSValue encodedBase, EncodedJSValue encodedSubscript, ByValInfo*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationHasIndexedPropertyDefault(ExecState*, EncodedJSValue encodedBase, EncodedJSValue encodedSubscript, ByValInfo*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationHasIndexedPropertyGeneric(ExecState*, EncodedJSValue encodedBase, EncodedJSValue encodedSubscript, ByValInfo*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationDeleteByIdJSResult(ExecState*, EncodedJSValue base, UniquedStringImpl*) WTF_INTERNAL;
size_t JIT_OPERATION operationDeleteById(ExecState*, EncodedJSValue base, UniquedStringImpl*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationDeleteByValJSResult(ExecState*, EncodedJSValue base, EncodedJSValue target) WTF_INTERNAL;
size_t JIT_OPERATION operationDeleteByVal(ExecState*, EncodedJSValue base, EncodedJSValue target) WTF_INTERNAL;
JSCell* JIT_OPERATION operationGetPNames(ExecState*, JSObject*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationInstanceOf(ExecState*, EncodedJSValue, EncodedJSValue proto) WTF_INTERNAL;
int32_t JIT_OPERATION operationSizeFrameForForwardArguments(ExecState*, EncodedJSValue arguments, int32_t numUsedStackSlots, int32_t firstVarArgOffset) WTF_INTERNAL;
int32_t JIT_OPERATION operationSizeFrameForVarargs(ExecState*, EncodedJSValue arguments, int32_t numUsedStackSlots, int32_t firstVarArgOffset) WTF_INTERNAL;
CallFrame* JIT_OPERATION operationSetupForwardArgumentsFrame(ExecState*, CallFrame*, EncodedJSValue, int32_t, int32_t length) WTF_INTERNAL;
CallFrame* JIT_OPERATION operationSetupVarargsFrame(ExecState*, CallFrame*, EncodedJSValue arguments, int32_t firstVarArgOffset, int32_t length) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationToObject(ExecState*, EncodedJSValue) WTF_INTERNAL;

char* JIT_OPERATION operationSwitchCharWithUnknownKeyType(ExecState*, EncodedJSValue key, size_t tableIndex) WTF_INTERNAL;
char* JIT_OPERATION operationSwitchImmWithUnknownKeyType(ExecState*, EncodedJSValue key, size_t tableIndex) WTF_INTERNAL;
char* JIT_OPERATION operationSwitchStringWithUnknownKeyType(ExecState*, EncodedJSValue key, size_t tableIndex) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationGetFromScope(ExecState*, Instruction* bytecodePC) WTF_INTERNAL;
void JIT_OPERATION operationPutToScope(ExecState*, Instruction* bytecodePC) WTF_INTERNAL;

char* JIT_OPERATION operationReallocateButterflyToHavePropertyStorageWithInitialCapacity(ExecState*, JSObject*) WTF_INTERNAL;
char* JIT_OPERATION operationReallocateButterflyToGrowPropertyStorage(ExecState*, JSObject*, size_t newSize) WTF_INTERNAL;

void JIT_OPERATION operationWriteBarrierSlowPath(ExecState*, JSCell*);
void JIT_OPERATION operationOSRWriteBarrier(ExecState*, JSCell*);

void JIT_OPERATION operationExceptionFuzz(ExecState*);

int32_t JIT_OPERATION operationCheckIfExceptionIsUncatchableAndNotifyProfiler(ExecState*);
int32_t JIT_OPERATION operationInstanceOfCustom(ExecState*, EncodedJSValue encodedValue, JSObject* constructor, EncodedJSValue encodedHasInstance) WTF_INTERNAL;

EncodedJSValue JIT_OPERATION operationHasGenericProperty(ExecState*, EncodedJSValue, JSCell*);
EncodedJSValue JIT_OPERATION operationHasIndexedProperty(ExecState*, JSCell*, int32_t, int32_t);
JSCell* JIT_OPERATION operationGetPropertyEnumerator(ExecState*, JSCell*);
EncodedJSValue JIT_OPERATION operationNextEnumeratorPname(ExecState*, JSCell*, int32_t);
JSCell* JIT_OPERATION operationToIndexString(ExecState*, int32_t);

EncodedJSValue JIT_OPERATION operationValueAdd(ExecState*, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationValueAddProfiled(ExecState*, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2, ArithProfile*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationValueAddProfiledOptimize(ExecState*, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2, JITAddIC*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationValueAddProfiledNoOptimize(ExecState*, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2, JITAddIC*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationValueAddOptimize(ExecState*, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2, JITAddIC*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationValueAddNoOptimize(ExecState*, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2, JITAddIC*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationValueMul(ExecState*, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationValueMulOptimize(ExecState*, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2, JITMulIC*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationValueMulNoOptimize(ExecState*, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2, JITMulIC*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationValueMulProfiledOptimize(ExecState*, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2, JITMulIC*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationValueMulProfiledNoOptimize(ExecState*, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2, JITMulIC*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationValueMulProfiled(ExecState*, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2, ArithProfile*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationArithNegate(ExecState*, EncodedJSValue operand);
EncodedJSValue JIT_OPERATION operationArithNegateProfiled(ExecState*, EncodedJSValue operand, ArithProfile*);
EncodedJSValue JIT_OPERATION operationArithNegateProfiledOptimize(ExecState*, EncodedJSValue encodedOperand, JITNegIC*);
EncodedJSValue JIT_OPERATION operationArithNegateOptimize(ExecState*, EncodedJSValue encodedOperand, JITNegIC*);
EncodedJSValue JIT_OPERATION operationValueSub(ExecState*, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationValueSubProfiled(ExecState*, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2, ArithProfile*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationValueSubOptimize(ExecState*, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2, JITSubIC*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationValueSubNoOptimize(ExecState*, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2, JITSubIC*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationValueSubProfiledOptimize(ExecState*, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2, JITSubIC*) WTF_INTERNAL;
EncodedJSValue JIT_OPERATION operationValueSubProfiledNoOptimize(ExecState*, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2, JITSubIC*) WTF_INTERNAL;

void JIT_OPERATION operationProcessTypeProfilerLog(ExecState*) WTF_INTERNAL;
void JIT_OPERATION operationProcessShadowChickenLog(ExecState*) WTF_INTERNAL;

} // extern "C"

} // namespace JSC

#endif // ENABLE(JIT)
