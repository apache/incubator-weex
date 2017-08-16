/*
 * Copyright (C) 2014, 2015 Apple Inc. All rights reserved.
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

#include "config.h"
#include "FTLOperations.h"

#if ENABLE(FTL_JIT)

#include "ClonedArguments.h"
#include "DirectArguments.h"
#include "FTLJITCode.h"
#include "FTLLazySlowPath.h"
#include "InlineCallFrame.h"
#include "JSAsyncFunction.h"
#include "JSCInlines.h"
#include "JSFixedArray.h"
#include "JSGeneratorFunction.h"
#include "JSLexicalEnvironment.h"

namespace JSC { namespace FTL {

using namespace JSC::DFG;

extern "C" void JIT_OPERATION operationPopulateObjectInOSR(
    ExecState* exec, ExitTimeObjectMaterialization* materialization,
    EncodedJSValue* encodedValue, EncodedJSValue* values)
{
    VM& vm = exec->vm();
    CodeBlock* codeBlock = exec->codeBlock();

    // We cannot GC. We've got pointers in evil places.
    // FIXME: We are not doing anything that can GC here, and this is
    // probably unnecessary.
    DeferGCForAWhile deferGC(vm.heap);

    switch (materialization->type()) {
    case PhantomNewObject: {
        JSFinalObject* object = jsCast<JSFinalObject*>(JSValue::decode(*encodedValue));
        Structure* structure = object->structure();

        // Figure out what the heck to populate the object with. Use
        // getPropertiesConcurrently() because that happens to be
        // lower-level and more convenient. It doesn't change the
        // materialization of the property table. We want to have
        // minimal visible effects on the system. Also, don't mind
        // that this is O(n^2). It doesn't matter. We only get here
        // from OSR exit.
        for (PropertyMapEntry entry : structure->getPropertiesConcurrently()) {
            for (unsigned i = materialization->properties().size(); i--;) {
                const ExitPropertyValue& property = materialization->properties()[i];
                if (property.location().kind() != NamedPropertyPLoc)
                    continue;
                if (codeBlock->identifier(property.location().info()).impl() != entry.key)
                    continue;

                object->putDirect(vm, entry.offset, JSValue::decode(values[i]));
            }
        }
        break;
    }

    case PhantomNewFunction:
    case PhantomNewGeneratorFunction:
    case PhantomNewAsyncFunction:
    case PhantomDirectArguments:
    case PhantomClonedArguments:
    case PhantomCreateRest:
    case PhantomSpread:
    case PhantomNewArrayWithSpread:
        // Those are completely handled by operationMaterializeObjectInOSR
        break;

    case PhantomCreateActivation: {
        JSLexicalEnvironment* activation = jsCast<JSLexicalEnvironment*>(JSValue::decode(*encodedValue));

        // Figure out what to populate the activation with
        for (unsigned i = materialization->properties().size(); i--;) {
            const ExitPropertyValue& property = materialization->properties()[i];
            if (property.location().kind() != ClosureVarPLoc)
                continue;

            activation->variableAt(ScopeOffset(property.location().info())).set(exec->vm(), activation, JSValue::decode(values[i]));
        }

        break;
    }


    default:
        RELEASE_ASSERT_NOT_REACHED();
        break;

    }
}

extern "C" JSCell* JIT_OPERATION operationMaterializeObjectInOSR(
    ExecState* exec, ExitTimeObjectMaterialization* materialization, EncodedJSValue* values)
{
    VM& vm = exec->vm();

    // We cannot GC. We've got pointers in evil places.
    DeferGCForAWhile deferGC(vm.heap);
    
    switch (materialization->type()) {
    case PhantomNewObject: {
        // Figure out what the structure is
        Structure* structure = nullptr;
        for (unsigned i = materialization->properties().size(); i--;) {
            const ExitPropertyValue& property = materialization->properties()[i];
            if (property.location() != PromotedLocationDescriptor(StructurePLoc))
                continue;

            RELEASE_ASSERT(JSValue::decode(values[i]).asCell()->inherits(vm, Structure::info()));
            structure = jsCast<Structure*>(JSValue::decode(values[i]));
            break;
        }
        RELEASE_ASSERT(structure);

        JSFinalObject* result = JSFinalObject::create(vm, structure);

        // The real values will be put subsequently by
        // operationPopulateNewObjectInOSR. We can't fill them in
        // now, because they may not be available yet (typically
        // because we have a cyclic dependency graph).

        // We put a dummy value here in order to avoid super-subtle
        // GC-and-OSR-exit crashes in case we have a bug and some
        // field is, for any reason, not filled later.
        // We use a random-ish number instead of a sensible value like
        // undefined to make possible bugs easier to track.
        for (PropertyMapEntry entry : structure->getPropertiesConcurrently())
            result->putDirect(vm, entry.offset, jsNumber(19723));

        return result;
    }

    case PhantomNewFunction:
    case PhantomNewGeneratorFunction:
    case PhantomNewAsyncFunction: {
        // Figure out what the executable and activation are
        FunctionExecutable* executable = nullptr;
        JSScope* activation = nullptr;
        for (unsigned i = materialization->properties().size(); i--;) {
            const ExitPropertyValue& property = materialization->properties()[i];
            if (property.location() == PromotedLocationDescriptor(FunctionExecutablePLoc)) {
                RELEASE_ASSERT(JSValue::decode(values[i]).asCell()->inherits(vm, FunctionExecutable::info()));
                executable = jsCast<FunctionExecutable*>(JSValue::decode(values[i]));
            }
            if (property.location() == PromotedLocationDescriptor(FunctionActivationPLoc)) {
                RELEASE_ASSERT(JSValue::decode(values[i]).asCell()->inherits(vm, JSScope::info()));
                activation = jsCast<JSScope*>(JSValue::decode(values[i]));
            }
        }
        RELEASE_ASSERT(executable && activation);

        if (materialization->type() == PhantomNewFunction)
            return JSFunction::createWithInvalidatedReallocationWatchpoint(vm, executable, activation);
        else if (materialization->type() == PhantomNewGeneratorFunction)
            return JSGeneratorFunction::createWithInvalidatedReallocationWatchpoint(vm, executable, activation);    
        ASSERT(materialization->type() == PhantomNewAsyncFunction);
        return JSAsyncFunction::createWithInvalidatedReallocationWatchpoint(vm, executable, activation);
    }

    case PhantomCreateActivation: {
        // Figure out what the scope and symbol table are
        JSScope* scope = nullptr;
        SymbolTable* table = nullptr;
        for (unsigned i = materialization->properties().size(); i--;) {
            const ExitPropertyValue& property = materialization->properties()[i];
            if (property.location() == PromotedLocationDescriptor(ActivationScopePLoc)) {
                RELEASE_ASSERT(JSValue::decode(values[i]).asCell()->inherits(vm, JSScope::info()));
                scope = jsCast<JSScope*>(JSValue::decode(values[i]));
            } else if (property.location() == PromotedLocationDescriptor(ActivationSymbolTablePLoc)) {
                RELEASE_ASSERT(JSValue::decode(values[i]).asCell()->inherits(vm, SymbolTable::info()));
                table = jsCast<SymbolTable*>(JSValue::decode(values[i]));
            }
        }
        RELEASE_ASSERT(scope);
        RELEASE_ASSERT(table);

        CodeBlock* codeBlock = baselineCodeBlockForOriginAndBaselineCodeBlock(
            materialization->origin(), exec->codeBlock());
        Structure* structure = codeBlock->globalObject()->activationStructure();

        // It doesn't matter what values we initialize as bottom values inside the activation constructor because
        // activation sinking will set bottom values for each slot.
        // FIXME: Slight optimization would be to create a constructor that doesn't initialize all slots.
        JSLexicalEnvironment* result = JSLexicalEnvironment::create(vm, structure, scope, table, jsUndefined());

        RELEASE_ASSERT(materialization->properties().size() - 2 == table->scopeSize());

        // The real values will be put subsequently by
        // operationPopulateNewObjectInOSR. See the PhantomNewObject
        // case for details.
        for (unsigned i = materialization->properties().size(); i--;) {
            const ExitPropertyValue& property = materialization->properties()[i];
            if (property.location().kind() != ClosureVarPLoc)
                continue;

            result->variableAt(ScopeOffset(property.location().info())).set(
                exec->vm(), result, jsNumber(29834));
        }

        if (validationEnabled()) {
            // Validate to make sure every slot in the scope has one value.
            ConcurrentJSLocker locker(table->m_lock);
            for (auto iter = table->begin(locker), end = table->end(locker); iter != end; ++iter) {
                bool found = false;
                for (unsigned i = materialization->properties().size(); i--;) {
                    const ExitPropertyValue& property = materialization->properties()[i];
                    if (property.location().kind() != ClosureVarPLoc)
                        continue;
                    if (ScopeOffset(property.location().info()) == iter->value.scopeOffset()) {
                        found = true;
                        break;
                    }
                }
                ASSERT_UNUSED(found, found);
            }
            unsigned numberOfClosureVarPloc = 0;
            for (unsigned i = materialization->properties().size(); i--;) {
                const ExitPropertyValue& property = materialization->properties()[i];
                if (property.location().kind() == ClosureVarPLoc)
                    numberOfClosureVarPloc++;
            }
            ASSERT(numberOfClosureVarPloc == table->scopeSize());
        }

        return result;
    }

    case PhantomCreateRest:
    case PhantomDirectArguments:
    case PhantomClonedArguments: {
        if (!materialization->origin().inlineCallFrame) {
            switch (materialization->type()) {
            case PhantomDirectArguments:
                return DirectArguments::createByCopying(exec);
            case PhantomClonedArguments:
                return ClonedArguments::createWithMachineFrame(exec, exec, ArgumentsMode::Cloned);
            case PhantomCreateRest: {
                CodeBlock* codeBlock = baselineCodeBlockForOriginAndBaselineCodeBlock(
                    materialization->origin(), exec->codeBlock());

                unsigned numberOfArgumentsToSkip = codeBlock->numParameters() - 1;
                JSGlobalObject* globalObject = codeBlock->globalObject();
                Structure* structure = globalObject->restParameterStructure();
                JSValue* argumentsToCopyRegion = exec->addressOfArgumentsStart() + numberOfArgumentsToSkip;
                unsigned arraySize = exec->argumentCount() > numberOfArgumentsToSkip ? exec->argumentCount() - numberOfArgumentsToSkip : 0;
                return constructArray(exec, structure, argumentsToCopyRegion, arraySize);
            }
            default:
                RELEASE_ASSERT_NOT_REACHED();
                return nullptr;
            }
        }

        // First figure out the argument count. If there isn't one then we represent the machine frame.
        unsigned argumentCount = 0;
        if (materialization->origin().inlineCallFrame->isVarargs()) {
            for (unsigned i = materialization->properties().size(); i--;) {
                const ExitPropertyValue& property = materialization->properties()[i];
                if (property.location() != PromotedLocationDescriptor(ArgumentCountPLoc))
                    continue;
                argumentCount = JSValue::decode(values[i]).asUInt32();
                break;
            }
        } else
            argumentCount = materialization->origin().inlineCallFrame->arguments.size();
        RELEASE_ASSERT(argumentCount);
        
        JSFunction* callee = nullptr;
        if (materialization->origin().inlineCallFrame->isClosureCall) {
            for (unsigned i = materialization->properties().size(); i--;) {
                const ExitPropertyValue& property = materialization->properties()[i];
                if (property.location() != PromotedLocationDescriptor(ArgumentsCalleePLoc))
                    continue;
                
                callee = jsCast<JSFunction*>(JSValue::decode(values[i]));
                break;
            }
        } else
            callee = materialization->origin().inlineCallFrame->calleeConstant();
        RELEASE_ASSERT(callee);
        
        CodeBlock* codeBlock = baselineCodeBlockForOriginAndBaselineCodeBlock(
            materialization->origin(), exec->codeBlock());
        
        // We have an inline frame and we have all of the data we need to recreate it.
        switch (materialization->type()) {
        case PhantomDirectArguments: {
            unsigned length = argumentCount - 1;
            unsigned capacity = std::max(length, static_cast<unsigned>(codeBlock->numParameters() - 1));
            DirectArguments* result = DirectArguments::create(
                vm, codeBlock->globalObject()->directArgumentsStructure(), length, capacity);
            result->callee().set(vm, result, callee);
            for (unsigned i = materialization->properties().size(); i--;) {
                const ExitPropertyValue& property = materialization->properties()[i];
                if (property.location().kind() != ArgumentPLoc)
                    continue;
                
                unsigned index = property.location().info();
                if (index >= capacity)
                    continue;
                
                // We don't want to use setIndexQuickly(), since that's only for the passed-in
                // arguments but sometimes the number of named arguments is greater. For
                // example:
                //
                // function foo(a, b, c) { ... }
                // foo();
                //
                // setIndexQuickly() would fail for indices 0, 1, 2 - but we need to recover
                // those here.
                result->argument(DirectArgumentsOffset(index)).set(
                    vm, result, JSValue::decode(values[i]));
            }
            return result;
        }
        case PhantomClonedArguments: {
            unsigned length = argumentCount - 1;
            ClonedArguments* result = ClonedArguments::createEmpty(
                vm, codeBlock->globalObject()->clonedArgumentsStructure(), callee, length);
            
            for (unsigned i = materialization->properties().size(); i--;) {
                const ExitPropertyValue& property = materialization->properties()[i];
                if (property.location().kind() != ArgumentPLoc)
                    continue;
                
                unsigned index = property.location().info();
                if (index >= length)
                    continue;
                result->initializeIndex(vm, index, JSValue::decode(values[i]));
            }
            
            return result;
        }
        case PhantomCreateRest: {
            unsigned numberOfArgumentsToSkip = codeBlock->numParameters() - 1;
            JSGlobalObject* globalObject = codeBlock->globalObject();
            Structure* structure = globalObject->restParameterStructure();
            ASSERT(argumentCount > 0);
            unsigned arraySize = (argumentCount - 1) > numberOfArgumentsToSkip ? argumentCount - 1 - numberOfArgumentsToSkip : 0;
            JSArray* array = JSArray::tryCreateForInitializationPrivate(vm, structure, arraySize);
            RELEASE_ASSERT(array);

            for (unsigned i = materialization->properties().size(); i--;) {
                const ExitPropertyValue& property = materialization->properties()[i];
                if (property.location().kind() != ArgumentPLoc)
                    continue;

                unsigned argIndex = property.location().info();
                if (numberOfArgumentsToSkip > argIndex)
                    continue;
                unsigned arrayIndex = argIndex - numberOfArgumentsToSkip;
                if (arrayIndex >= arraySize)
                    continue;
                array->initializeIndex(vm, arrayIndex, JSValue::decode(values[i]));
            }

#if !ASSERT_DISABLED
            // We avoid this O(n^2) loop when asserts are disabled, but the condition checked here
            // must hold to ensure the correctness of the above loop because of how we allocate the array.
            for (unsigned targetIndex = 0; targetIndex < arraySize; ++targetIndex) {
                bool found = false;
                for (unsigned i = materialization->properties().size(); i--;) {
                    const ExitPropertyValue& property = materialization->properties()[i];
                    if (property.location().kind() != ArgumentPLoc)
                        continue;

                    unsigned argIndex = property.location().info();
                    if (numberOfArgumentsToSkip > argIndex)
                        continue;
                    unsigned arrayIndex = argIndex - numberOfArgumentsToSkip;
                    if (arrayIndex >= arraySize)
                        continue;
                    if (arrayIndex == targetIndex) {
                        found = true;
                        break;
                    }
                }
                ASSERT(found);
            }
#endif
            return array;
        }

        default:
            RELEASE_ASSERT_NOT_REACHED();
            return nullptr;
        }
    }

    case PhantomSpread: {
        JSArray* array = nullptr;
        for (unsigned i = materialization->properties().size(); i--;) {
            const ExitPropertyValue& property = materialization->properties()[i];
            if (property.location().kind() == SpreadPLoc) {
                array = jsCast<JSArray*>(JSValue::decode(values[i]));
                break;
            }
        }
        RELEASE_ASSERT(array);

        // Note: it is sound for JSFixedArray::createFromArray to call getDirectIndex here
        // because we're guaranteed we won't be calling any getters. The reason for this is
        // that we only support PhantomSpread over CreateRest, which is an array we create.
        // Any attempts to put a getter on any indices on the rest array will escape the array.
        JSFixedArray* fixedArray = JSFixedArray::createFromArray(exec, vm, array);
        return fixedArray;
    }

    case PhantomNewArrayWithSpread: {
        CodeBlock* codeBlock = baselineCodeBlockForOriginAndBaselineCodeBlock(
            materialization->origin(), exec->codeBlock());
        JSGlobalObject* globalObject = codeBlock->globalObject();
        Structure* structure = globalObject->arrayStructureForIndexingTypeDuringAllocation(ArrayWithContiguous);

        unsigned arraySize = 0;
        unsigned numProperties = 0;
        for (unsigned i = materialization->properties().size(); i--;) {
            const ExitPropertyValue& property = materialization->properties()[i];
            if (property.location().kind() == NewArrayWithSpreadArgumentPLoc) {
                ++numProperties;
                JSValue value = JSValue::decode(values[i]);
                if (JSFixedArray* fixedArray = jsDynamicCast<JSFixedArray*>(vm, value))
                    arraySize += fixedArray->size();
                else
                    arraySize += 1;
            }
        }

        JSArray* result = JSArray::tryCreateForInitializationPrivate(vm, structure, arraySize);
        RELEASE_ASSERT(result);

#if !ASSERT_DISABLED
        // Ensure we see indices for everything in the range: [0, numProperties)
        for (unsigned i = 0; i < numProperties; ++i) {
            bool found = false;
            for (unsigned j = 0; j < materialization->properties().size(); ++j) {
                const ExitPropertyValue& property = materialization->properties()[j];
                if (property.location().kind() == NewArrayWithSpreadArgumentPLoc && property.location().info() == i) {
                    found = true;
                    break;
                }
            }
            ASSERT(found);
        }
#endif

        Vector<JSValue, 8> arguments;
        arguments.grow(numProperties);

        for (unsigned i = materialization->properties().size(); i--;) {
            const ExitPropertyValue& property = materialization->properties()[i];
            if (property.location().kind() == NewArrayWithSpreadArgumentPLoc) {
                JSValue value = JSValue::decode(values[i]);
                RELEASE_ASSERT(property.location().info() < numProperties);
                arguments[property.location().info()] = value;
            }
        }

        unsigned arrayIndex = 0;
        for (JSValue value : arguments) {
            if (JSFixedArray* fixedArray = jsDynamicCast<JSFixedArray*>(vm, value)) {
                for (unsigned i = 0; i < fixedArray->size(); i++) {
                    ASSERT(fixedArray->get(i));
                    result->initializeIndex(vm, arrayIndex, fixedArray->get(i));
                    ++arrayIndex;
                }
            } else {
                // We are not spreading.
                result->initializeIndex(vm, arrayIndex, value);
                ++arrayIndex;
            }
        }

        return result;
    }

        
    default:
        RELEASE_ASSERT_NOT_REACHED();
        return nullptr;
    }
}

extern "C" void* JIT_OPERATION compileFTLLazySlowPath(ExecState* exec, unsigned index)
{
    VM& vm = exec->vm();

    // We cannot GC. We've got pointers in evil places.
    DeferGCForAWhile deferGC(vm.heap);

    CodeBlock* codeBlock = exec->codeBlock();
    JITCode* jitCode = codeBlock->jitCode()->ftl();

    LazySlowPath& lazySlowPath = *jitCode->lazySlowPaths[index];
    lazySlowPath.generate(codeBlock);

    return lazySlowPath.stub().code().executableAddress();
}

} } // namespace JSC::FTL

#endif // ENABLE(FTL_JIT)

