/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
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
#include "JSModuleRecord.h"

#include "Error.h"
#include "Interpreter.h"
#include "JSCInlines.h"
#include "JSModuleEnvironment.h"
#include "JSModuleNamespaceObject.h"
#include "UnlinkedModuleProgramCodeBlock.h"

namespace JSC {

const ClassInfo JSModuleRecord::s_info = { "ModuleRecord", &Base::s_info, 0, CREATE_METHOD_TABLE(JSModuleRecord) };


Structure* JSModuleRecord::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
}

JSModuleRecord* JSModuleRecord::create(ExecState* exec, VM& vm, Structure* structure, const Identifier& moduleKey, const SourceCode& sourceCode, const VariableEnvironment& declaredVariables, const VariableEnvironment& lexicalVariables)
{
    JSModuleRecord* instance = new (NotNull, allocateCell<JSModuleRecord>(vm.heap)) JSModuleRecord(vm, structure, moduleKey, sourceCode, declaredVariables, lexicalVariables);
    instance->finishCreation(exec, vm);
    return instance;
}
JSModuleRecord::JSModuleRecord(VM& vm, Structure* structure, const Identifier& moduleKey, const SourceCode& sourceCode, const VariableEnvironment& declaredVariables, const VariableEnvironment& lexicalVariables)
    : Base(vm, structure, moduleKey)
    , m_sourceCode(sourceCode)
    , m_declaredVariables(declaredVariables)
    , m_lexicalVariables(lexicalVariables)
{
}

void JSModuleRecord::destroy(JSCell* cell)
{
    JSModuleRecord* thisObject = static_cast<JSModuleRecord*>(cell);
    thisObject->JSModuleRecord::~JSModuleRecord();
}

void JSModuleRecord::finishCreation(ExecState* exec, VM& vm)
{
    Base::finishCreation(exec, vm);
    ASSERT(inherits(vm, info()));
}

void JSModuleRecord::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    JSModuleRecord* thisObject = jsCast<JSModuleRecord*>(cell);
    Base::visitChildren(thisObject, visitor);
    visitor.append(thisObject->m_moduleProgramExecutable);
}

void JSModuleRecord::link(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    ModuleProgramExecutable* executable = ModuleProgramExecutable::create(exec, sourceCode());
    if (!executable) {
        throwSyntaxError(exec, scope);
        return;
    }
    instantiateDeclarations(exec, executable);
    RETURN_IF_EXCEPTION(scope, void());
    m_moduleProgramExecutable.set(vm, this, executable);
}

void JSModuleRecord::instantiateDeclarations(ExecState* exec, ModuleProgramExecutable* moduleProgramExecutable)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    // http://www.ecma-international.org/ecma-262/6.0/#sec-moduledeclarationinstantiation

    SymbolTable* symbolTable = moduleProgramExecutable->moduleEnvironmentSymbolTable();
    JSModuleEnvironment* moduleEnvironment = JSModuleEnvironment::create(vm, exec->lexicalGlobalObject(), exec->lexicalGlobalObject(), symbolTable, jsTDZValue(), this);

    // http://www.ecma-international.org/ecma-262/6.0/#sec-moduledeclarationinstantiation
    // section 15.2.1.16.4 step 9.
    // Ensure all the indirect exports are correctly resolved to unique bindings.
    // Even if we avoided duplicate exports in the parser, still ambiguous exports occur due to the star export (`export * from "mod"`).
    // When we see this type of ambiguity for the indirect exports here, throw a syntax error.
    for (const auto& pair : exportEntries()) {
        const ExportEntry& exportEntry = pair.value;
        if (exportEntry.type == JSModuleRecord::ExportEntry::Type::Indirect) {
            Resolution resolution = resolveExport(exec, exportEntry.exportName);
            switch (resolution.type) {
            case Resolution::Type::NotFound:
                throwSyntaxError(exec, scope, makeString("Indirectly exported binding name '", String(exportEntry.exportName.impl()), "' is not found."));
                return;

            case Resolution::Type::Ambiguous:
                throwSyntaxError(exec, scope, makeString("Indirectly exported binding name '", String(exportEntry.exportName.impl()), "' cannot be resolved due to ambiguous multiple bindings."));
                return;

            case Resolution::Type::Error:
                throwSyntaxError(exec, scope, makeString("Indirectly exported binding name 'default' cannot be resolved by star export entries."));
                return;

            case Resolution::Type::Resolved:
                break;
            }
        }
    }

    // http://www.ecma-international.org/ecma-262/6.0/#sec-moduledeclarationinstantiation
    // section 15.2.1.16.4 step 12.
    // Instantiate namespace objects and initialize the bindings with them if required.
    // And ensure that all the imports correctly resolved to unique bindings.
    for (const auto& pair : importEntries()) {
        const ImportEntry& importEntry = pair.value;
        AbstractModuleRecord* importedModule = hostResolveImportedModule(exec, importEntry.moduleRequest);
        if (importEntry.isNamespace(vm)) {
            JSModuleNamespaceObject* namespaceObject = importedModule->getModuleNamespace(exec);
            RETURN_IF_EXCEPTION(scope, void());
            bool putResult = false;
            symbolTablePutTouchWatchpointSet(moduleEnvironment, exec, importEntry.localName, namespaceObject, /* shouldThrowReadOnlyError */ false, /* ignoreReadOnlyErrors */ true, putResult);
        } else {
            Resolution resolution = importedModule->resolveExport(exec, importEntry.importName);
            switch (resolution.type) {
            case Resolution::Type::NotFound:
                throwSyntaxError(exec, scope, makeString("Importing binding name '", String(importEntry.importName.impl()), "' is not found."));
                return;

            case Resolution::Type::Ambiguous:
                throwSyntaxError(exec, scope, makeString("Importing binding name '", String(importEntry.importName.impl()), "' cannot be resolved due to ambiguous multiple bindings."));
                return;

            case Resolution::Type::Error:
                throwSyntaxError(exec, scope, makeString("Importing binding name 'default' cannot be resolved by star export entries."));
                return;

            case Resolution::Type::Resolved:
                break;
            }
        }
    }

    // http://www.ecma-international.org/ecma-262/6.0/#sec-moduledeclarationinstantiation
    // section 15.2.1.16.4 step 14.
    // Module environment contains the heap allocated "var", "function", "let", "const", and "class".
    // When creating the environment, we initialized all the slots with empty, it's ok for lexical values.
    // But for "var" and "function", we should initialize it with undefined. They are contained in the declared variables.
    for (const auto& variable : declaredVariables()) {
        SymbolTableEntry entry = symbolTable->get(variable.key.get());
        VarOffset offset = entry.varOffset();
        if (!offset.isStack()) {
            bool putResult = false;
            symbolTablePutTouchWatchpointSet(moduleEnvironment, exec, Identifier::fromUid(exec, variable.key.get()), jsUndefined(), /* shouldThrowReadOnlyError */ false, /* ignoreReadOnlyErrors */ true, putResult);
        }
    }

    // http://www.ecma-international.org/ecma-262/6.0/#sec-moduledeclarationinstantiation
    // section 15.2.1.16.4 step 16-a-iv.
    // Initialize heap allocated function declarations.
    // They can be called before the body of the module is executed under circular dependencies.
    UnlinkedModuleProgramCodeBlock* unlinkedCodeBlock = moduleProgramExecutable->unlinkedModuleProgramCodeBlock();
    for (size_t i = 0, numberOfFunctions = unlinkedCodeBlock->numberOfFunctionDecls(); i < numberOfFunctions; ++i) {
        UnlinkedFunctionExecutable* unlinkedFunctionExecutable = unlinkedCodeBlock->functionDecl(i);
        SymbolTableEntry entry = symbolTable->get(unlinkedFunctionExecutable->name().impl());
        VarOffset offset = entry.varOffset();
        if (!offset.isStack()) {
            ASSERT(!unlinkedFunctionExecutable->name().isEmpty());
            if (vm.typeProfiler() || vm.controlFlowProfiler()) {
                vm.functionHasExecutedCache()->insertUnexecutedRange(moduleProgramExecutable->sourceID(),
                    unlinkedFunctionExecutable->typeProfilingStartOffset(),
                    unlinkedFunctionExecutable->typeProfilingEndOffset());
            }
            JSFunction* function = JSFunction::create(vm, unlinkedFunctionExecutable->link(vm, moduleProgramExecutable->source()), moduleEnvironment);
            bool putResult = false;
            symbolTablePutTouchWatchpointSet(moduleEnvironment, exec, unlinkedFunctionExecutable->name(), function, /* shouldThrowReadOnlyError */ false, /* ignoreReadOnlyErrors */ true, putResult);
        }
    }

    m_moduleEnvironment.set(vm, this, moduleEnvironment);
}

JSValue JSModuleRecord::evaluate(ExecState* exec)
{
    if (!m_moduleProgramExecutable)
        return jsUndefined();
    JSValue result = exec->interpreter()->execute(m_moduleProgramExecutable.get(), exec, m_moduleEnvironment.get());
    m_moduleProgramExecutable.clear();
    return result;
}

} // namespace JSC
