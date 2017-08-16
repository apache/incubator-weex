/*
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2003, 2007, 2008 Apple Inc. All rights reserved.
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

#include "config.h"
#include "JSCell.h"

#include "ArrayBufferView.h"
#include "JSCInlines.h"
#include "JSFunction.h"
#include "JSString.h"
#include "JSObject.h"
#include "JSWebAssemblyCallee.h"
#include "NumberObject.h"
#include "WebAssemblyToJSCallee.h"
#include <wtf/MathExtras.h>

namespace JSC {

COMPILE_ASSERT(sizeof(JSCell) == sizeof(uint64_t), jscell_is_eight_bytes);
STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(JSCell);

void JSCell::destroy(JSCell* cell)
{
    cell->JSCell::~JSCell();
}

void JSCell::dump(PrintStream& out) const
{
    methodTable()->dumpToStream(this, out);
}

void JSCell::dumpToStream(const JSCell* cell, PrintStream& out)
{
    out.printf("<%p, %s>", cell, cell->className(*cell->vm()));
}

size_t JSCell::estimatedSizeInBytes() const
{
    return methodTable()->estimatedSize(const_cast<JSCell*>(this));
}

size_t JSCell::estimatedSize(JSCell* cell)
{
    return cell->cellSize();
}

void JSCell::heapSnapshot(JSCell*, HeapSnapshotBuilder&)
{
}

bool JSCell::getString(ExecState* exec, String& stringValue) const
{
    if (!isString())
        return false;
    stringValue = static_cast<const JSString*>(this)->value(exec);
    return true;
}

String JSCell::getString(ExecState* exec) const
{
    return isString() ? static_cast<const JSString*>(this)->value(exec) : String();
}

JSObject* JSCell::getObject()
{
    return isObject() ? asObject(this) : 0;
}

const JSObject* JSCell::getObject() const
{
    return isObject() ? static_cast<const JSObject*>(this) : 0;
}

CallType JSCell::getCallData(JSCell*, CallData& callData)
{
    callData.js.functionExecutable = 0;
    callData.js.scope = 0;
    callData.native.function = 0;
    return CallType::None;
}

ConstructType JSCell::getConstructData(JSCell*, ConstructData& constructData)
{
    constructData.js.functionExecutable = 0;
    constructData.js.scope = 0;
    constructData.native.function = 0;
    return ConstructType::None;
}

bool JSCell::put(JSCell* cell, ExecState* exec, PropertyName identifier, JSValue value, PutPropertySlot& slot)
{
    if (cell->isString() || cell->isSymbol())
        return JSValue(cell).putToPrimitive(exec, identifier, value, slot);

    JSObject* thisObject = cell->toObject(exec, exec->lexicalGlobalObject());
    return thisObject->methodTable(exec->vm())->put(thisObject, exec, identifier, value, slot);
}

bool JSCell::putByIndex(JSCell* cell, ExecState* exec, unsigned identifier, JSValue value, bool shouldThrow)
{
    if (cell->isString() || cell->isSymbol()) {
        PutPropertySlot slot(cell, shouldThrow);
        return JSValue(cell).putToPrimitive(exec, Identifier::from(exec, identifier), value, slot);
    }
    JSObject* thisObject = cell->toObject(exec, exec->lexicalGlobalObject());
    return thisObject->methodTable(exec->vm())->putByIndex(thisObject, exec, identifier, value, shouldThrow);
}

bool JSCell::deleteProperty(JSCell* cell, ExecState* exec, PropertyName identifier)
{
    JSObject* thisObject = cell->toObject(exec, exec->lexicalGlobalObject());
    return thisObject->methodTable(exec->vm())->deleteProperty(thisObject, exec, identifier);
}

bool JSCell::deletePropertyByIndex(JSCell* cell, ExecState* exec, unsigned identifier)
{
    JSObject* thisObject = cell->toObject(exec, exec->lexicalGlobalObject());
    return thisObject->methodTable(exec->vm())->deletePropertyByIndex(thisObject, exec, identifier);
}

JSValue JSCell::toThis(JSCell* cell, ExecState* exec, ECMAMode ecmaMode)
{
    if (ecmaMode == StrictMode)
        return cell;
    return cell->toObject(exec, exec->lexicalGlobalObject());
}

JSValue JSCell::toPrimitive(ExecState* exec, PreferredPrimitiveType preferredType) const
{
    if (isString())
        return static_cast<const JSString*>(this)->toPrimitive(exec, preferredType);
    if (isSymbol())
        return static_cast<const Symbol*>(this)->toPrimitive(exec, preferredType);
    return static_cast<const JSObject*>(this)->toPrimitive(exec, preferredType);
}

bool JSCell::getPrimitiveNumber(ExecState* exec, double& number, JSValue& value) const
{
    if (isString())
        return static_cast<const JSString*>(this)->getPrimitiveNumber(exec, number, value);
    if (isSymbol())
        return static_cast<const Symbol*>(this)->getPrimitiveNumber(exec, number, value);
    return static_cast<const JSObject*>(this)->getPrimitiveNumber(exec, number, value);
}

double JSCell::toNumber(ExecState* exec) const
{
    if (isString())
        return static_cast<const JSString*>(this)->toNumber(exec);
    if (isSymbol())
        return static_cast<const Symbol*>(this)->toNumber(exec);
    return static_cast<const JSObject*>(this)->toNumber(exec);
}

JSObject* JSCell::toObjectSlow(ExecState* exec, JSGlobalObject* globalObject) const
{
    ASSERT(!isObject());
    if (isString())
        return static_cast<const JSString*>(this)->toObject(exec, globalObject);
    ASSERT(isSymbol());
    return static_cast<const Symbol*>(this)->toObject(exec, globalObject);
}

void slowValidateCell(JSCell* cell)
{
    ASSERT_GC_OBJECT_LOOKS_VALID(cell);
}

JSValue JSCell::defaultValue(const JSObject*, ExecState*, PreferredPrimitiveType)
{
    RELEASE_ASSERT_NOT_REACHED();
    return jsUndefined();
}

bool JSCell::getOwnPropertySlot(JSObject*, ExecState*, PropertyName, PropertySlot&)
{
    RELEASE_ASSERT_NOT_REACHED();
    return false;
}

bool JSCell::getOwnPropertySlotByIndex(JSObject*, ExecState*, unsigned, PropertySlot&)
{
    RELEASE_ASSERT_NOT_REACHED();
    return false;
}

void JSCell::getOwnPropertyNames(JSObject*, ExecState*, PropertyNameArray&, EnumerationMode)
{
    RELEASE_ASSERT_NOT_REACHED();
}

void JSCell::getOwnNonIndexPropertyNames(JSObject*, ExecState*, PropertyNameArray&, EnumerationMode)
{
    RELEASE_ASSERT_NOT_REACHED();
}

String JSCell::className(const JSObject*)
{
    RELEASE_ASSERT_NOT_REACHED();
    return String();
}

String JSCell::toStringName(const JSObject*, ExecState*)
{
    RELEASE_ASSERT_NOT_REACHED();
    return String();
}

const char* JSCell::className(VM& vm) const
{
    return classInfo(vm)->className;
}

void JSCell::getPropertyNames(JSObject*, ExecState*, PropertyNameArray&, EnumerationMode)
{
    RELEASE_ASSERT_NOT_REACHED();
}

bool JSCell::customHasInstance(JSObject*, ExecState*, JSValue)
{
    RELEASE_ASSERT_NOT_REACHED();
    return false;
}

bool JSCell::defineOwnProperty(JSObject*, ExecState*, PropertyName, const PropertyDescriptor&, bool)
{
    RELEASE_ASSERT_NOT_REACHED();
    return false;
}

ArrayBuffer* JSCell::slowDownAndWasteMemory(JSArrayBufferView*)
{
    RELEASE_ASSERT_NOT_REACHED();
    return nullptr;
}

RefPtr<ArrayBufferView> JSCell::getTypedArrayImpl(JSArrayBufferView*)
{
    RELEASE_ASSERT_NOT_REACHED();
    return nullptr;
}

uint32_t JSCell::getEnumerableLength(ExecState*, JSObject*)
{
    RELEASE_ASSERT_NOT_REACHED();
    return 0;
}

void JSCell::getStructurePropertyNames(JSObject*, ExecState*, PropertyNameArray&, EnumerationMode)
{
    RELEASE_ASSERT_NOT_REACHED();
}

void JSCell::getGenericPropertyNames(JSObject*, ExecState*, PropertyNameArray&, EnumerationMode)
{
    RELEASE_ASSERT_NOT_REACHED();
}

bool JSCell::preventExtensions(JSObject*, ExecState*)
{
    RELEASE_ASSERT_NOT_REACHED();
}

bool JSCell::isExtensible(JSObject*, ExecState*)
{
    RELEASE_ASSERT_NOT_REACHED();
}

bool JSCell::setPrototype(JSObject*, ExecState*, JSValue, bool)
{
    RELEASE_ASSERT_NOT_REACHED();
}

JSValue JSCell::getPrototype(JSObject*, ExecState*)
{
    RELEASE_ASSERT_NOT_REACHED();
}

bool JSCell::isAnyWasmCallee(VM& vm) const
{
#if ENABLE(WEBASSEMBLY)
    return inherits(vm, JSWebAssemblyCallee::info()) || inherits(vm, WebAssemblyToJSCallee::info());
#else
    UNUSED_PARAM(vm);
    return false;
#endif

}

} // namespace JSC
