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
#include "JSModuleNamespaceObject.h"

#include "AbstractModuleRecord.h"
#include "Error.h"
#include "JSCInlines.h"
#include "JSModuleEnvironment.h"

namespace JSC {

const ClassInfo JSModuleNamespaceObject::s_info = { "ModuleNamespaceObject", &Base::s_info, nullptr, CREATE_METHOD_TABLE(JSModuleNamespaceObject) };

JSModuleNamespaceObject::JSModuleNamespaceObject(VM& vm, Structure* structure)
    : Base(vm, structure)
    , m_exports()
{
}

void JSModuleNamespaceObject::finishCreation(ExecState* exec, JSGlobalObject*, AbstractModuleRecord* moduleRecord, Vector<std::pair<Identifier, AbstractModuleRecord::Resolution>>&& resolutions)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    Base::finishCreation(vm);
    ASSERT(inherits(vm, info()));

    // http://www.ecma-international.org/ecma-262/6.0/#sec-module-namespace-exotic-objects
    // Quoted from the spec:
    //     A List containing the String values of the exported names exposed as own properties of this object.
    //     The list is ordered as if an Array of those String values had been sorted using Array.prototype.sort using SortCompare as comparefn.
    //
    // Sort the exported names by the code point order.
    std::sort(resolutions.begin(), resolutions.end(), [] (const auto& lhs, const auto& rhs) {
        return codePointCompare(lhs.first.impl(), rhs.first.impl()) < 0;
    });

    m_moduleRecord.set(vm, this, moduleRecord);
    {
        unsigned moduleRecordOffset = 0;
        m_names.reserveCapacity(resolutions.size());
        for (const auto& pair : resolutions) {
            moduleRecordAt(moduleRecordOffset).set(vm, this, pair.second.moduleRecord);
            m_names.append(pair.first);
            m_exports.add(pair.first.impl(), ExportEntry {
                pair.second.localName,
                moduleRecordOffset
            });
            ++moduleRecordOffset;
        }
    }

    putDirect(vm, vm.propertyNames->toStringTagSymbol, jsString(&vm, "Module"), DontEnum | DontDelete | ReadOnly);

    // http://www.ecma-international.org/ecma-262/6.0/#sec-module-namespace-exotic-objects-getprototypeof
    // http://www.ecma-international.org/ecma-262/6.0/#sec-module-namespace-exotic-objects-setprototypeof-v
    // http://www.ecma-international.org/ecma-262/6.0/#sec-module-namespace-exotic-objects-isextensible
    // http://www.ecma-international.org/ecma-262/6.0/#sec-module-namespace-exotic-objects-preventextensions
    methodTable(vm)->preventExtensions(this, exec);
    ASSERT_UNUSED(scope, !scope.exception());
}

void JSModuleNamespaceObject::destroy(JSCell* cell)
{
    JSModuleNamespaceObject* thisObject = static_cast<JSModuleNamespaceObject*>(cell);
    thisObject->JSModuleNamespaceObject::~JSModuleNamespaceObject();
}

void JSModuleNamespaceObject::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    JSModuleNamespaceObject* thisObject = jsCast<JSModuleNamespaceObject*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());
    Base::visitChildren(thisObject, visitor);
    visitor.append(thisObject->m_moduleRecord);
    for (unsigned i = 0; i < thisObject->m_names.size(); ++i)
        visitor.appendHidden(thisObject->moduleRecordAt(i));
}

static JSValue getValue(JSModuleEnvironment* environment, PropertyName localName, ScopeOffset& scopeOffset)
{
    SymbolTable* symbolTable = environment->symbolTable();
    {
        ConcurrentJSLocker locker(symbolTable->m_lock);
        auto iter = symbolTable->find(locker, localName.uid());
        ASSERT(iter != symbolTable->end(locker));
        SymbolTableEntry& entry = iter->value;
        ASSERT(!entry.isNull());
        scopeOffset = entry.scopeOffset();
    }
    return environment->variableAt(scopeOffset).get();
}

bool JSModuleNamespaceObject::getOwnPropertySlotCommon(ExecState* exec, PropertyName propertyName, PropertySlot& slot)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    // http://www.ecma-international.org/ecma-262/6.0/#sec-module-namespace-exotic-objects-getownproperty-p

    // step 1.
    // If the property name is a symbol, we don't look into the imported bindings.
    // It may return the descriptor with writable: true, but namespace objects does not allow it in [[Set]] / [[DefineOwnProperty]] side.
    if (propertyName.isSymbol())
        return JSObject::getOwnPropertySlot(this, exec, propertyName, slot);

    slot.setIsTaintedByOpaqueObject();

    auto iterator = m_exports.find(propertyName.uid());
    if (iterator == m_exports.end())
        return false;
    ExportEntry& exportEntry = iterator->value;

    switch (slot.internalMethodType()) {
    case PropertySlot::InternalMethodType::GetOwnProperty:
    case PropertySlot::InternalMethodType::Get: {
        JSModuleEnvironment* environment = moduleRecordAt(exportEntry.moduleRecordOffset)->moduleEnvironment();
        ScopeOffset scopeOffset;
        JSValue value = getValue(environment, exportEntry.localName, scopeOffset);
        // If the value is filled with TDZ value, throw a reference error.
        if (!value) {
            throwVMError(exec, scope, createTDZError(exec));
            return false;
        }

        slot.setValueModuleNamespace(this, DontDelete, value, environment, scopeOffset);
        return true;
    }

    case PropertySlot::InternalMethodType::HasProperty: {
        // Do not perform [[Get]] for [[HasProperty]].
        // [[Get]] / [[GetOwnProperty]] onto namespace object could throw an error while [[HasProperty]] just returns true here.
        // https://tc39.github.io/ecma262/#sec-module-namespace-exotic-objects-hasproperty-p
        slot.setValue(this, DontDelete, jsUndefined());
        return true;
    }

    case PropertySlot::InternalMethodType::VMInquiry:
        return false;
    }

    RELEASE_ASSERT_NOT_REACHED();
    return false;
}

bool JSModuleNamespaceObject::getOwnPropertySlot(JSObject* cell, ExecState* exec, PropertyName propertyName, PropertySlot& slot)
{
    JSModuleNamespaceObject* thisObject = jsCast<JSModuleNamespaceObject*>(cell);
    return thisObject->getOwnPropertySlotCommon(exec, propertyName, slot);
}

bool JSModuleNamespaceObject::getOwnPropertySlotByIndex(JSObject* cell, ExecState* exec, unsigned propertyName, PropertySlot& slot)
{
    JSModuleNamespaceObject* thisObject = jsCast<JSModuleNamespaceObject*>(cell);
    return thisObject->getOwnPropertySlotCommon(exec, Identifier::from(exec, propertyName), slot);
}

bool JSModuleNamespaceObject::put(JSCell*, ExecState* exec, PropertyName, JSValue, PutPropertySlot& slot)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    // http://www.ecma-international.org/ecma-262/6.0/#sec-module-namespace-exotic-objects-set-p-v-receiver
    if (slot.isStrictMode())
        throwTypeError(exec, scope, ASCIILiteral(ReadonlyPropertyWriteError));
    return false;
}

bool JSModuleNamespaceObject::putByIndex(JSCell*, ExecState* exec, unsigned, JSValue, bool shouldThrow)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (shouldThrow)
        throwTypeError(exec, scope, ASCIILiteral(ReadonlyPropertyWriteError));
    return false;
}

bool JSModuleNamespaceObject::deleteProperty(JSCell* cell, ExecState* exec, PropertyName propertyName)
{
    // http://www.ecma-international.org/ecma-262/6.0/#sec-module-namespace-exotic-objects-delete-p
    JSModuleNamespaceObject* thisObject = jsCast<JSModuleNamespaceObject*>(cell);
    if (propertyName.isSymbol())
        return JSObject::deleteProperty(thisObject, exec, propertyName);

    return !thisObject->m_exports.contains(propertyName.uid());
}

void JSModuleNamespaceObject::getOwnPropertyNames(JSObject* cell, ExecState* exec, PropertyNameArray& propertyNames, EnumerationMode mode)
{
    // http://www.ecma-international.org/ecma-262/6.0/#sec-module-namespace-exotic-objects-ownpropertykeys
    JSModuleNamespaceObject* thisObject = jsCast<JSModuleNamespaceObject*>(cell);
    for (const auto& name : thisObject->m_names)
        propertyNames.add(name.impl());
    return JSObject::getOwnPropertyNames(thisObject, exec, propertyNames, mode);
}

bool JSModuleNamespaceObject::defineOwnProperty(JSObject*, ExecState* exec, PropertyName, const PropertyDescriptor&, bool shouldThrow)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    // http://www.ecma-international.org/ecma-262/6.0/#sec-module-namespace-exotic-objects-defineownproperty-p-desc
    if (shouldThrow)
        throwTypeError(exec, scope, ASCIILiteral(NonExtensibleObjectPropertyDefineError));
    return false;
}

} // namespace JSC
