/*
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2003-2006, 2008, 2016 Apple Inc. All rights reserved.
 *  Copyright (C) 2007 Eric Seidel (eric@webkit.org)
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
#include "Error.h"

#include "ConstructData.h"
#include "ErrorConstructor.h"
#include "ExceptionHelpers.h"
#include "FunctionPrototype.h"
#include "Interpreter.h"
#include "JSArray.h"
#include "JSFunction.h"
#include "JSGlobalObject.h"
#include "JSObject.h"
#include "JSString.h"
#include "JSCInlines.h"
#include "NativeErrorConstructor.h"
#include "SourceCode.h"
#include "StackFrame.h"

namespace JSC {

static const char* linePropertyName = "line";
static const char* sourceURLPropertyName = "sourceURL";

JSObject* createError(ExecState* exec, const String& message, ErrorInstance::SourceAppender appender)
{
    ASSERT(!message.isEmpty());
    JSGlobalObject* globalObject = exec->lexicalGlobalObject();
    return ErrorInstance::create(exec, globalObject->vm(), globalObject->errorStructure(), message, appender, TypeNothing, true);
}

JSObject* createEvalError(ExecState* exec, const String& message, ErrorInstance::SourceAppender appender)
{
    ASSERT(!message.isEmpty());
    JSGlobalObject* globalObject = exec->lexicalGlobalObject();
    return ErrorInstance::create(exec, globalObject->vm(), globalObject->evalErrorConstructor()->errorStructure(), message, appender, TypeNothing, true);
}

JSObject* createRangeError(ExecState* exec, const String& message, ErrorInstance::SourceAppender appender)
{
    ASSERT(!message.isEmpty());
    JSGlobalObject* globalObject = exec->lexicalGlobalObject();
    return ErrorInstance::create(exec, globalObject->vm(), globalObject->rangeErrorConstructor()->errorStructure(), message, appender, TypeNothing, true);
}

JSObject* createReferenceError(ExecState* exec, const String& message, ErrorInstance::SourceAppender appender)
{
    ASSERT(!message.isEmpty());
    JSGlobalObject* globalObject = exec->lexicalGlobalObject();
    return ErrorInstance::create(exec, globalObject->vm(), globalObject->referenceErrorConstructor()->errorStructure(), message, appender, TypeNothing, true);
}

JSObject* createSyntaxError(ExecState* exec, const String& message, ErrorInstance::SourceAppender appender)
{
    ASSERT(!message.isEmpty());
    JSGlobalObject* globalObject = exec->lexicalGlobalObject();
    return ErrorInstance::create(exec, globalObject->vm(), globalObject->syntaxErrorConstructor()->errorStructure(), message, appender, TypeNothing, true);
}

JSObject* createTypeError(ExecState* exec, const String& message, ErrorInstance::SourceAppender appender, RuntimeType type)
{
    ASSERT(!message.isEmpty());
    JSGlobalObject* globalObject = exec->lexicalGlobalObject();
    return ErrorInstance::create(exec, globalObject->vm(), globalObject->typeErrorConstructor()->errorStructure(), message, appender, type, true);
}

JSObject* createNotEnoughArgumentsError(ExecState* exec, ErrorInstance::SourceAppender appender)
{
    return createTypeError(exec, ASCIILiteral("Not enough arguments"), appender, TypeNothing);
}

JSObject* createURIError(ExecState* exec, const String& message, ErrorInstance::SourceAppender appender)
{
    ASSERT(!message.isEmpty());
    JSGlobalObject* globalObject = exec->lexicalGlobalObject();
    return ErrorInstance::create(exec, globalObject->vm(), globalObject->URIErrorConstructor()->errorStructure(), message, appender, TypeNothing, true);
}

JSObject* createError(ExecState* exec, ErrorType errorType, const String& message)
{
    switch (errorType) {
    case ErrorType::Error:
        return createError(exec, message);
    case ErrorType::EvalError:
        return createEvalError(exec, message);
    case ErrorType::RangeError:
        return createRangeError(exec, message);
    case ErrorType::ReferenceError:
        return createReferenceError(exec, message);
    case ErrorType::SyntaxError:
        return createSyntaxError(exec, message);
    case ErrorType::TypeError:
        return createTypeError(exec, message);
    case ErrorType::URIError:
        return createURIError(exec, message);
    }
    ASSERT_NOT_REACHED();
    return nullptr;
}

class FindFirstCallerFrameWithCodeblockFunctor {
public:
    FindFirstCallerFrameWithCodeblockFunctor(CallFrame* startCallFrame)
        : m_startCallFrame(startCallFrame)
        , m_foundCallFrame(nullptr)
        , m_foundStartCallFrame(false)
        , m_index(0)
    { }

    StackVisitor::Status operator()(StackVisitor& visitor) const
    {
        if (!m_foundStartCallFrame && (visitor->callFrame() == m_startCallFrame))
            m_foundStartCallFrame = true;

        if (m_foundStartCallFrame) {
            if (visitor->callFrame()->codeBlock()) {
                m_foundCallFrame = visitor->callFrame();
                return StackVisitor::Done;
            }
            m_index++;
        }

        return StackVisitor::Continue;
    }

    CallFrame* foundCallFrame() const { return m_foundCallFrame; }
    unsigned index() const { return m_index; }

private:
    CallFrame* m_startCallFrame;
    mutable CallFrame* m_foundCallFrame;
    mutable bool m_foundStartCallFrame;
    mutable unsigned m_index;
};

bool addErrorInfoAndGetBytecodeOffset(ExecState* exec, VM& vm, JSObject* obj, bool useCurrentFrame, CallFrame*& callFrame, unsigned* bytecodeOffset)
{
    Vector<StackFrame> stackTrace = Vector<StackFrame>();

    size_t framesToSkip = useCurrentFrame ? 0 : 1;
    vm.interpreter->getStackTrace(stackTrace, framesToSkip);
    if (!stackTrace.isEmpty()) {

        ASSERT(exec == vm.topCallFrame || exec == exec->lexicalGlobalObject()->globalExec() || exec == exec->vmEntryGlobalObject()->globalExec());

        StackFrame* firstFrameWithLineAndColumnInfo = nullptr;
        for (unsigned i = 0 ; i < stackTrace.size(); ++i) {
            firstFrameWithLineAndColumnInfo = &stackTrace.at(i);
            if (firstFrameWithLineAndColumnInfo->hasLineAndColumnInfo())
                break;
        }

        if (bytecodeOffset) {
            FindFirstCallerFrameWithCodeblockFunctor functor(exec);
            vm.topCallFrame->iterate(functor);
            callFrame = functor.foundCallFrame();
            unsigned stackIndex = functor.index();
            *bytecodeOffset = 0;
            if (stackTrace.at(stackIndex).hasBytecodeOffset())
                *bytecodeOffset = stackTrace.at(stackIndex).bytecodeOffset();
        }
        
        unsigned line;
        unsigned column;
        firstFrameWithLineAndColumnInfo->computeLineAndColumn(line, column);
        obj->putDirect(vm, vm.propertyNames->line, jsNumber(line));
        obj->putDirect(vm, vm.propertyNames->column, jsNumber(column));

        String frameSourceURL = firstFrameWithLineAndColumnInfo->sourceURL();
        if (!frameSourceURL.isEmpty())
            obj->putDirect(vm, vm.propertyNames->sourceURL, jsString(&vm, frameSourceURL));

        obj->putDirect(vm, vm.propertyNames->stack, Interpreter::stackTraceAsString(vm, stackTrace), DontEnum);

        return true;
    }
    return false;
}

void addErrorInfo(ExecState* exec, JSObject* obj, bool useCurrentFrame)
{
    CallFrame* callFrame = nullptr;
    addErrorInfoAndGetBytecodeOffset(exec, exec->vm(), obj, useCurrentFrame, callFrame);
}

JSObject* addErrorInfo(CallFrame* callFrame, JSObject* error, int line, const SourceCode& source)
{
    VM* vm = &callFrame->vm();
    const String& sourceURL = source.provider()->url();

    if (line != -1)
        error->putDirect(*vm, Identifier::fromString(vm, linePropertyName), jsNumber(line));
    if (!sourceURL.isNull())
        error->putDirect(*vm, Identifier::fromString(vm, sourceURLPropertyName), jsString(vm, sourceURL));
    return error;
}

JSObject* throwConstructorCannotBeCalledAsFunctionTypeError(ExecState* exec, ThrowScope& scope, const char* constructorName)
{
    return throwTypeError(exec, scope, makeString("calling ", constructorName, " constructor without new is invalid"));
}

JSObject* throwTypeError(ExecState* exec, ThrowScope& scope)
{
    return throwException(exec, scope, createTypeError(exec));
}

JSObject* throwTypeError(ExecState* exec, ThrowScope& scope, ASCIILiteral errorMessage)
{
    return throwTypeError(exec, scope, String(errorMessage));
}

JSObject* throwTypeError(ExecState* exec, ThrowScope& scope, const String& message)
{
    return throwException(exec, scope, createTypeError(exec, message));
}

JSObject* throwSyntaxError(ExecState* exec, ThrowScope& scope)
{
    return throwException(exec, scope, createSyntaxError(exec, ASCIILiteral("Syntax error")));
}

JSObject* throwSyntaxError(ExecState* exec, ThrowScope& scope, const String& message)
{
    return throwException(exec, scope, createSyntaxError(exec, message));
}

JSObject* createError(ExecState* exec, const String& message)
{
    return createError(exec, message, nullptr);
}

JSObject* createEvalError(ExecState* exec, const String& message)
{
    return createEvalError(exec, message, nullptr);
}

JSObject* createRangeError(ExecState* exec, const String& message)
{
    return createRangeError(exec, message, nullptr);
}

JSObject* createReferenceError(ExecState* exec, const String& message)
{
    return createReferenceError(exec, message, nullptr);
}

JSObject* createSyntaxError(ExecState* exec, const String& message)
{
    return createSyntaxError(exec, message, nullptr);
}

JSObject* createTypeError(ExecState* exec)
{
    return createTypeError(exec, ASCIILiteral("Type error"));
}

JSObject* createTypeError(ExecState* exec, const String& message)
{
    return createTypeError(exec, message, nullptr, TypeNothing);
}

JSObject* createNotEnoughArgumentsError(ExecState* exec)
{
    return createNotEnoughArgumentsError(exec, nullptr);
}

JSObject* createURIError(ExecState* exec, const String& message)
{
    return createURIError(exec, message, nullptr);
}

JSObject* createOutOfMemoryError(ExecState* exec)
{
    return createError(exec, ASCIILiteral("Out of memory"), nullptr);
}


const ClassInfo StrictModeTypeErrorFunction::s_info = { "Function", &Base::s_info, 0, CREATE_METHOD_TABLE(StrictModeTypeErrorFunction) };

void StrictModeTypeErrorFunction::destroy(JSCell* cell)
{
    static_cast<StrictModeTypeErrorFunction*>(cell)->StrictModeTypeErrorFunction::~StrictModeTypeErrorFunction();
}

} // namespace JSC

namespace WTF {

using namespace JSC;

void printInternal(PrintStream& out, JSC::ErrorType errorType)
{
    switch (errorType) {
    case JSC::ErrorType::Error:
        out.print("Error");
        break;
    case JSC::ErrorType::EvalError:
        out.print("EvalError");
        break;
    case JSC::ErrorType::RangeError:
        out.print("RangeError");
        break;
    case JSC::ErrorType::ReferenceError:
        out.print("ReferenceError");
        break;
    case JSC::ErrorType::SyntaxError:
        out.print("SyntaxError");
        break;
    case JSC::ErrorType::TypeError:
        out.print("TypeError");
        break;
    case JSC::ErrorType::URIError:
        out.print("URIError");
        break;
    }
}

} // namespace WTF
