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

#include "CodeBlock.h"
#include "CodeSpecializationKind.h"
#include "ExceptionHelpers.h"
#include "FunctionCodeBlock.h"
#include "SlowPathReturnType.h"
#include "StackAlignment.h"
#include "Symbol.h"
#include "VMInlines.h"
#include <wtf/StdLibExtras.h>

namespace JSC {

// The purpose of this namespace is to include slow paths that are shared
// between the interpreter and baseline JIT. They are written to be agnostic
// with respect to the slow-path calling convention, but they do rely on the
// JS code being executed more-or-less directly from bytecode (so the call
// frame layout is unmodified, making it potentially awkward to use these
// from any optimizing JIT, like the DFG).

namespace CommonSlowPaths {

struct ArityCheckData {
    unsigned paddedStackSpace;
    void* thunkToCall;
};

ALWAYS_INLINE int arityCheckFor(ExecState* exec, VM& vm, CodeSpecializationKind kind)
{
    JSFunction* callee = jsCast<JSFunction*>(exec->jsCallee());
    ASSERT(!callee->isHostFunction());
    CodeBlock* newCodeBlock = callee->jsExecutable()->codeBlockFor(kind);
    int argumentCountIncludingThis = exec->argumentCountIncludingThis();
    
    ASSERT(argumentCountIncludingThis < newCodeBlock->numParameters());
    int frameSize = argumentCountIncludingThis + CallFrame::headerSizeInRegisters;
    int alignedFrameSizeForParameters = WTF::roundUpToMultipleOf(stackAlignmentRegisters(),
        newCodeBlock->numParameters() + CallFrame::headerSizeInRegisters);
    int paddedStackSpace = alignedFrameSizeForParameters - frameSize;
    
    Register* newStack = exec->registers() - WTF::roundUpToMultipleOf(stackAlignmentRegisters(), paddedStackSpace);

    if (UNLIKELY(!vm.ensureStackCapacityFor(newStack)))
        return -1;
    return paddedStackSpace;
}

inline bool opIn(ExecState* exec, JSValue baseVal, JSValue propName, ArrayProfile* arrayProfile = nullptr)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    if (!baseVal.isObject()) {
        throwException(exec, scope, createInvalidInParameterError(exec, baseVal));
        return false;
    }

    JSObject* baseObj = asObject(baseVal);
    if (arrayProfile)
        arrayProfile->observeStructure(baseObj->structure(vm));

    uint32_t i;
    if (propName.getUInt32(i)) {
        scope.release();
        return baseObj->hasProperty(exec, i);
    }

    auto property = propName.toPropertyKey(exec);
    RETURN_IF_EXCEPTION(scope, false);
    scope.release();
    return baseObj->hasProperty(exec, property);
}

inline void tryCachePutToScopeGlobal(
    ExecState* exec, CodeBlock* codeBlock, Instruction* pc, JSObject* scope,
    GetPutInfo getPutInfo, PutPropertySlot& slot, const Identifier& ident)
{
    // Covers implicit globals. Since they don't exist until they first execute, we didn't know how to cache them at compile time.
    ResolveType resolveType = getPutInfo.resolveType();
    if (resolveType != GlobalProperty && resolveType != GlobalPropertyWithVarInjectionChecks 
        && resolveType != UnresolvedProperty && resolveType != UnresolvedPropertyWithVarInjectionChecks)
        return;

    if (resolveType == UnresolvedProperty || resolveType == UnresolvedPropertyWithVarInjectionChecks) {
        if (scope->isGlobalObject()) {
            ResolveType newResolveType = resolveType == UnresolvedProperty ? GlobalProperty : GlobalPropertyWithVarInjectionChecks;
            resolveType = newResolveType;
            getPutInfo = GetPutInfo(getPutInfo.resolveMode(), newResolveType, getPutInfo.initializationMode());
            ConcurrentJSLocker locker(codeBlock->m_lock);
            pc[4].u.operand = getPutInfo.operand();
        } else if (scope->isGlobalLexicalEnvironment()) {
            JSGlobalLexicalEnvironment* globalLexicalEnvironment = jsCast<JSGlobalLexicalEnvironment*>(scope);
            ResolveType newResolveType = resolveType == UnresolvedProperty ? GlobalLexicalVar : GlobalLexicalVarWithVarInjectionChecks;
            pc[4].u.operand = GetPutInfo(getPutInfo.resolveMode(), newResolveType, getPutInfo.initializationMode()).operand();
            SymbolTableEntry entry = globalLexicalEnvironment->symbolTable()->get(ident.impl());
            ASSERT(!entry.isNull());
            ConcurrentJSLocker locker(codeBlock->m_lock);
            pc[5].u.watchpointSet = entry.watchpointSet();
            pc[6].u.pointer = static_cast<void*>(globalLexicalEnvironment->variableAt(entry.scopeOffset()).slot());
        }
    }
    
    if (resolveType == GlobalProperty || resolveType == GlobalPropertyWithVarInjectionChecks) {
        if (!slot.isCacheablePut()
            || slot.base() != scope
            || !scope->structure()->propertyAccessesAreCacheable())
            return;
        
        if (slot.type() == PutPropertySlot::NewProperty) {
            // Don't cache if we've done a transition. We want to detect the first replace so that we
            // can invalidate the watchpoint.
            return;
        }
        
        scope->structure()->didCachePropertyReplacement(exec->vm(), slot.cachedOffset());

        ConcurrentJSLocker locker(codeBlock->m_lock);
        pc[5].u.structure.set(exec->vm(), codeBlock, scope->structure());
        pc[6].u.operand = slot.cachedOffset();
    }
}

inline void tryCacheGetFromScopeGlobal(
    ExecState* exec, VM& vm, Instruction* pc, JSObject* scope, PropertySlot& slot, const Identifier& ident)
{
    GetPutInfo getPutInfo(pc[4].u.operand);
    ResolveType resolveType = getPutInfo.resolveType();

    if (resolveType == UnresolvedProperty || resolveType == UnresolvedPropertyWithVarInjectionChecks) {
        if (scope->isGlobalObject()) {
            ResolveType newResolveType = resolveType == UnresolvedProperty ? GlobalProperty : GlobalPropertyWithVarInjectionChecks;
            resolveType = newResolveType; // Allow below caching mechanism to kick in.
            ConcurrentJSLocker locker(exec->codeBlock()->m_lock);
            pc[4].u.operand = GetPutInfo(getPutInfo.resolveMode(), newResolveType, getPutInfo.initializationMode()).operand();
        } else if (scope->isGlobalLexicalEnvironment()) {
            JSGlobalLexicalEnvironment* globalLexicalEnvironment = jsCast<JSGlobalLexicalEnvironment*>(scope);
            ResolveType newResolveType = resolveType == UnresolvedProperty ? GlobalLexicalVar : GlobalLexicalVarWithVarInjectionChecks;
            SymbolTableEntry entry = globalLexicalEnvironment->symbolTable()->get(ident.impl());
            ASSERT(!entry.isNull());
            ConcurrentJSLocker locker(exec->codeBlock()->m_lock);
            pc[4].u.operand = GetPutInfo(getPutInfo.resolveMode(), newResolveType, getPutInfo.initializationMode()).operand();
            pc[5].u.watchpointSet = entry.watchpointSet();
            pc[6].u.pointer = static_cast<void*>(globalLexicalEnvironment->variableAt(entry.scopeOffset()).slot());
        }
    }

    // Covers implicit globals. Since they don't exist until they first execute, we didn't know how to cache them at compile time.
    if (slot.isCacheableValue() && slot.slotBase() == scope && scope->structure()->propertyAccessesAreCacheable()) {
        if (resolveType == GlobalProperty || resolveType == GlobalPropertyWithVarInjectionChecks) {
            CodeBlock* codeBlock = exec->codeBlock();
            Structure* structure = scope->structure(vm);
            {
                ConcurrentJSLocker locker(codeBlock->m_lock);
                pc[5].u.structure.set(exec->vm(), codeBlock, structure);
                pc[6].u.operand = slot.cachedOffset();
            }
            structure->startWatchingPropertyForReplacements(vm, slot.cachedOffset());
        }
    }
}

} // namespace CommonSlowPaths

class ExecState;
struct Instruction;

#define SLOW_PATH
    
#define SLOW_PATH_DECL(name) \
extern "C" SlowPathReturnType SLOW_PATH name(ExecState* exec, Instruction* pc)
    
#define SLOW_PATH_HIDDEN_DECL(name) \
SLOW_PATH_DECL(name) WTF_INTERNAL
    
SLOW_PATH_HIDDEN_DECL(slow_path_call_arityCheck);
SLOW_PATH_HIDDEN_DECL(slow_path_construct_arityCheck);
SLOW_PATH_HIDDEN_DECL(slow_path_create_direct_arguments);
SLOW_PATH_HIDDEN_DECL(slow_path_create_scoped_arguments);
SLOW_PATH_HIDDEN_DECL(slow_path_create_cloned_arguments);
SLOW_PATH_HIDDEN_DECL(slow_path_create_this);
SLOW_PATH_HIDDEN_DECL(slow_path_enter);
SLOW_PATH_HIDDEN_DECL(slow_path_get_callee);
SLOW_PATH_HIDDEN_DECL(slow_path_to_this);
SLOW_PATH_HIDDEN_DECL(slow_path_throw_tdz_error);
SLOW_PATH_HIDDEN_DECL(slow_path_throw_strict_mode_readonly_property_write_error);
SLOW_PATH_HIDDEN_DECL(slow_path_not);
SLOW_PATH_HIDDEN_DECL(slow_path_eq);
SLOW_PATH_HIDDEN_DECL(slow_path_neq);
SLOW_PATH_HIDDEN_DECL(slow_path_stricteq);
SLOW_PATH_HIDDEN_DECL(slow_path_nstricteq);
SLOW_PATH_HIDDEN_DECL(slow_path_less);
SLOW_PATH_HIDDEN_DECL(slow_path_lesseq);
SLOW_PATH_HIDDEN_DECL(slow_path_greater);
SLOW_PATH_HIDDEN_DECL(slow_path_greatereq);
SLOW_PATH_HIDDEN_DECL(slow_path_inc);
SLOW_PATH_HIDDEN_DECL(slow_path_dec);
SLOW_PATH_HIDDEN_DECL(slow_path_to_number);
SLOW_PATH_HIDDEN_DECL(slow_path_to_string);
SLOW_PATH_HIDDEN_DECL(slow_path_negate);
SLOW_PATH_HIDDEN_DECL(slow_path_add);
SLOW_PATH_HIDDEN_DECL(slow_path_mul);
SLOW_PATH_HIDDEN_DECL(slow_path_sub);
SLOW_PATH_HIDDEN_DECL(slow_path_div);
SLOW_PATH_HIDDEN_DECL(slow_path_mod);
SLOW_PATH_HIDDEN_DECL(slow_path_pow);
SLOW_PATH_HIDDEN_DECL(slow_path_lshift);
SLOW_PATH_HIDDEN_DECL(slow_path_rshift);
SLOW_PATH_HIDDEN_DECL(slow_path_urshift);
SLOW_PATH_HIDDEN_DECL(slow_path_unsigned);
SLOW_PATH_HIDDEN_DECL(slow_path_bitand);
SLOW_PATH_HIDDEN_DECL(slow_path_bitor);
SLOW_PATH_HIDDEN_DECL(slow_path_bitxor);
SLOW_PATH_HIDDEN_DECL(slow_path_typeof);
SLOW_PATH_HIDDEN_DECL(slow_path_is_object);
SLOW_PATH_HIDDEN_DECL(slow_path_is_object_or_null);
SLOW_PATH_HIDDEN_DECL(slow_path_is_function);
SLOW_PATH_HIDDEN_DECL(slow_path_in);
SLOW_PATH_HIDDEN_DECL(slow_path_del_by_val);
SLOW_PATH_HIDDEN_DECL(slow_path_strcat);
SLOW_PATH_HIDDEN_DECL(slow_path_to_primitive);
SLOW_PATH_HIDDEN_DECL(slow_path_get_enumerable_length);
SLOW_PATH_HIDDEN_DECL(slow_path_has_generic_property);
SLOW_PATH_HIDDEN_DECL(slow_path_has_structure_property);
SLOW_PATH_HIDDEN_DECL(slow_path_has_indexed_property);
SLOW_PATH_HIDDEN_DECL(slow_path_get_direct_pname);
SLOW_PATH_HIDDEN_DECL(slow_path_get_property_enumerator);
SLOW_PATH_HIDDEN_DECL(slow_path_next_structure_enumerator_pname);
SLOW_PATH_HIDDEN_DECL(slow_path_next_generic_enumerator_pname);
SLOW_PATH_HIDDEN_DECL(slow_path_to_index_string);
SLOW_PATH_HIDDEN_DECL(slow_path_profile_type_clear_log);
SLOW_PATH_HIDDEN_DECL(slow_path_assert);
SLOW_PATH_HIDDEN_DECL(slow_path_create_lexical_environment);
SLOW_PATH_HIDDEN_DECL(slow_path_push_with_scope);
SLOW_PATH_HIDDEN_DECL(slow_path_resolve_scope);
SLOW_PATH_HIDDEN_DECL(slow_path_create_rest);
SLOW_PATH_HIDDEN_DECL(slow_path_get_by_id_with_this);
SLOW_PATH_HIDDEN_DECL(slow_path_get_by_val_with_this);
SLOW_PATH_HIDDEN_DECL(slow_path_put_by_id_with_this);
SLOW_PATH_HIDDEN_DECL(slow_path_put_by_val_with_this);
SLOW_PATH_HIDDEN_DECL(slow_path_define_data_property);
SLOW_PATH_HIDDEN_DECL(slow_path_define_accessor_property);
SLOW_PATH_HIDDEN_DECL(slow_path_throw_static_error);
SLOW_PATH_HIDDEN_DECL(slow_path_new_array_with_spread);
SLOW_PATH_HIDDEN_DECL(slow_path_spread);

} // namespace JSC
