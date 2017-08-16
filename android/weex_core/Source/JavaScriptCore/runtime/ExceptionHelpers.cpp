/*
 * Copyright (C) 2008-2009, 2016 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "ExceptionHelpers.h"

#include "CodeBlock.h"
#include "CallFrame.h"
#include "ErrorHandlingScope.h"
#include "Exception.h"
#include "JSGlobalObjectFunctions.h"
#include "Interpreter.h"
#include "Nodes.h"
#include "JSCInlines.h"
#include "RuntimeType.h"
#include <wtf/text/StringBuilder.h>
#include <wtf/text/StringView.h>

namespace JSC {

STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(TerminatedExecutionError);

const ClassInfo TerminatedExecutionError::s_info = { "TerminatedExecutionError", &Base::s_info, 0, CREATE_METHOD_TABLE(TerminatedExecutionError) };

JSValue TerminatedExecutionError::defaultValue(const JSObject*, ExecState* exec, PreferredPrimitiveType hint)
{
    if (hint == PreferString)
        return jsNontrivialString(exec, String(ASCIILiteral("JavaScript execution terminated.")));
    return JSValue(PNaN);
}

JSObject* createTerminatedExecutionException(VM* vm)
{
    return TerminatedExecutionError::create(*vm);
}

bool isTerminatedExecutionException(VM& vm, Exception* exception)
{
    if (!exception->value().isObject())
        return false;

    return exception->value().inherits(vm, TerminatedExecutionError::info());
}

JSObject* createStackOverflowError(ExecState* exec)
{
    return createRangeError(exec, ASCIILiteral("Maximum call stack size exceeded."));
}

JSObject* createUndefinedVariableError(ExecState* exec, const Identifier& ident)
{
    if (exec->propertyNames().isPrivateName(ident)) {
        String message(makeString("Can't find private variable: @", exec->propertyNames().lookUpPublicName(ident).string()));
        return createReferenceError(exec, message);
    }
    String message(makeString("Can't find variable: ", ident.string()));
    return createReferenceError(exec, message);
}
    
JSString* errorDescriptionForValue(ExecState* exec, JSValue v)
{
    if (v.isString())
        return jsNontrivialString(exec, makeString('"', asString(v)->value(exec), '"'));
    if (v.isSymbol())
        return jsNontrivialString(exec, asSymbol(v)->descriptiveString());
    if (v.isObject()) {
        CallData callData;
        JSObject* object = asObject(v);
        if (object->methodTable()->getCallData(object, callData) != CallType::None)
            return exec->vm().smallStrings.functionString();
        return jsString(exec, JSObject::calculatedClassName(object));
    }
    return v.toString(exec);
}
    
static String defaultApproximateSourceError(const String& originalMessage, const String& sourceText)
{
    return makeString(originalMessage, " (near '...", sourceText, "...')");
}

String defaultSourceAppender(const String& originalMessage, const String& sourceText, RuntimeType, ErrorInstance::SourceTextWhereErrorOccurred occurrence)
{
    if (occurrence == ErrorInstance::FoundApproximateSource)
        return defaultApproximateSourceError(originalMessage, sourceText);

    ASSERT(occurrence == ErrorInstance::FoundExactSource);
    return makeString(originalMessage, " (evaluating '", sourceText, "')");
}

static String functionCallBase(const String& sourceText)
{ 
    // This function retrieves the 'foo.bar' substring from 'foo.bar(baz)'.
    // FIXME: This function has simple processing of /* */ style comments.
    // It doesn't properly handle embedded comments of string literals that contain
    // parenthesis or comment constructs, e.g. foo.bar("/abc\)*/").
    // https://bugs.webkit.org/show_bug.cgi?id=146304

    unsigned sourceLength = sourceText.length();
    unsigned idx = sourceLength - 1;
    if (sourceLength < 2 || sourceText[idx] != ')') {
        // For function calls that have many new lines in between their open parenthesis
        // and their closing parenthesis, the text range passed into the message appender 
        // will not inlcude the text in between these parentheses, it will just be the desired
        // text that precedes the parentheses.
        return sourceText;
    }

    unsigned parenStack = 1;
    bool isInMultiLineComment = false;
    idx -= 1;
    // Note that we're scanning text right to left instead of the more common left to right, 
    // so syntax detection is backwards.
    while (parenStack > 0) {
        UChar curChar = sourceText[idx];
        if (isInMultiLineComment)  {
            if (idx > 0 && curChar == '*' && sourceText[idx - 1] == '/') {
                isInMultiLineComment = false;
                idx -= 1;
            }
        } else if (curChar == '(')
            parenStack -= 1;
        else if (curChar == ')')
            parenStack += 1;
        else if (idx > 0 && curChar == '/' && sourceText[idx - 1] == '*') {
            isInMultiLineComment = true;
            idx -= 1;
        }

        if (!idx)
            break;

        idx -= 1;
    }

    return sourceText.left(idx + 1);
}

static String notAFunctionSourceAppender(const String& originalMessage, const String& sourceText, RuntimeType type, ErrorInstance::SourceTextWhereErrorOccurred occurrence)
{
    ASSERT(type != TypeFunction);

    if (occurrence == ErrorInstance::FoundApproximateSource)
        return defaultApproximateSourceError(originalMessage, sourceText);

    ASSERT(occurrence == ErrorInstance::FoundExactSource);
    auto notAFunctionIndex = originalMessage.reverseFind("is not a function");
    RELEASE_ASSERT(notAFunctionIndex != notFound);
    StringView displayValue;
    if (originalMessage.is8Bit()) 
        displayValue = StringView(originalMessage.characters8(), notAFunctionIndex - 1);
    else
        displayValue = StringView(originalMessage.characters16(), notAFunctionIndex - 1);

    String base = functionCallBase(sourceText);
    StringBuilder builder;
    builder.append(base);
    builder.appendLiteral(" is not a function. (In '");
    builder.append(sourceText);
    builder.appendLiteral("', '");
    builder.append(base);
    builder.appendLiteral("' is ");
    if (type == TypeSymbol)
        builder.appendLiteral("a Symbol");
    else {
        if (type == TypeObject)
            builder.appendLiteral("an instance of ");
        builder.append(displayValue);
    }
    builder.append(')');

    return builder.toString();
}

static String invalidParameterInSourceAppender(const String& originalMessage, const String& sourceText, RuntimeType type, ErrorInstance::SourceTextWhereErrorOccurred occurrence)
{
    ASSERT_UNUSED(type, type != TypeObject);

    if (occurrence == ErrorInstance::FoundApproximateSource)
        return defaultApproximateSourceError(originalMessage, sourceText);

    ASSERT(occurrence == ErrorInstance::FoundExactSource);
    auto inIndex = sourceText.reverseFind("in");
    RELEASE_ASSERT(inIndex != notFound);
    if (sourceText.find("in") != inIndex)
        return makeString(originalMessage, " (evaluating '", sourceText, "')");

    static const unsigned inLength = 2;
    String rightHandSide = sourceText.substring(inIndex + inLength).simplifyWhiteSpace();
    return makeString(rightHandSide, " is not an Object. (evaluating '", sourceText, "')");
}

inline String invalidParameterInstanceofSourceAppender(const String& content, const String& originalMessage, const String& sourceText, RuntimeType, ErrorInstance::SourceTextWhereErrorOccurred occurrence)
{
    if (occurrence == ErrorInstance::FoundApproximateSource)
        return defaultApproximateSourceError(originalMessage, sourceText);

    ASSERT(occurrence == ErrorInstance::FoundExactSource);
    auto instanceofIndex = sourceText.reverseFind("instanceof");
    RELEASE_ASSERT(instanceofIndex != notFound);
    if (sourceText.find("instanceof") != instanceofIndex)
        return makeString(originalMessage, " (evaluating '", sourceText, "')");

    static const unsigned instanceofLength = 10;
    String rightHandSide = sourceText.substring(instanceofIndex + instanceofLength).simplifyWhiteSpace();
    return makeString(rightHandSide, content, ". (evaluating '", sourceText, "')");
}

static String invalidParameterInstanceofNotFunctionSourceAppender(const String& originalMessage, const String& sourceText, RuntimeType runtimeType, ErrorInstance::SourceTextWhereErrorOccurred occurrence)
{
    return invalidParameterInstanceofSourceAppender(WTF::makeString(" is not a function"), originalMessage, sourceText, runtimeType, occurrence);
}

static String invalidParameterInstanceofhasInstanceValueNotFunctionSourceAppender(const String& originalMessage, const String& sourceText, RuntimeType runtimeType, ErrorInstance::SourceTextWhereErrorOccurred occurrence)
{
    return invalidParameterInstanceofSourceAppender(WTF::makeString("[Symbol.hasInstance] is not a function, undefined, or null"), originalMessage, sourceText, runtimeType, occurrence);
}

JSObject* createError(ExecState* exec, JSValue value, const String& message, ErrorInstance::SourceAppender appender)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_CATCH_SCOPE(vm);

    String errorMessage = makeString(errorDescriptionForValue(exec, value)->value(exec), ' ', message);
    ASSERT_UNUSED(scope, !scope.exception());
    JSObject* exception = createTypeError(exec, errorMessage, appender, runtimeTypeForValue(value));
    ASSERT(exception->isErrorInstance());
    return exception;
}

JSObject* createInvalidFunctionApplyParameterError(ExecState* exec, JSValue value)
{
    JSObject* exception = createTypeError(exec, makeString("second argument to Function.prototype.apply must be an Array-like object"), defaultSourceAppender, runtimeTypeForValue(value));
    ASSERT(exception->isErrorInstance());
    return exception;
}

JSObject* createInvalidInParameterError(ExecState* exec, JSValue value)
{
    return createError(exec, value, makeString("is not an Object."), invalidParameterInSourceAppender);
}

JSObject* createInvalidInstanceofParameterErrorNotFunction(ExecState* exec, JSValue value)
{
    return createError(exec, value, makeString(" is not a function"), invalidParameterInstanceofNotFunctionSourceAppender);
}

JSObject* createInvalidInstanceofParameterErrorhasInstanceValueNotFunction(ExecState* exec, JSValue value)
{
    return createError(exec, value, makeString("[Symbol.hasInstance] is not a function, undefined, or null"), invalidParameterInstanceofhasInstanceValueNotFunctionSourceAppender);
}

JSObject* createNotAConstructorError(ExecState* exec, JSValue value)
{
    return createError(exec, value, ASCIILiteral("is not a constructor"), defaultSourceAppender);
}

JSObject* createNotAFunctionError(ExecState* exec, JSValue value)
{
    return createError(exec, value, ASCIILiteral("is not a function"), notAFunctionSourceAppender);
}

JSObject* createNotAnObjectError(ExecState* exec, JSValue value)
{
    return createError(exec, value, ASCIILiteral("is not an object"), defaultSourceAppender);
}

JSObject* createErrorForInvalidGlobalAssignment(ExecState* exec, const String& propertyName)
{
    return createReferenceError(exec, makeString("Strict mode forbids implicit creation of global property '", propertyName, '\''));
}

JSObject* createTDZError(ExecState* exec)
{
    return createReferenceError(exec, "Cannot access uninitialized variable.");
}

JSObject* throwOutOfMemoryError(ExecState* exec, ThrowScope& scope)
{
    return throwException(exec, scope, createOutOfMemoryError(exec));
}

JSObject* throwStackOverflowError(ExecState* exec, ThrowScope& scope)
{
    VM& vm = exec->vm();
    ErrorHandlingScope errorScope(vm);
    return throwException(exec, scope, createStackOverflowError(exec));
}

JSObject* throwTerminatedExecutionException(ExecState* exec, ThrowScope& scope)
{
    VM& vm = exec->vm();
    ErrorHandlingScope errorScope(vm);
    return throwException(exec, scope, createTerminatedExecutionException(&vm));
}

} // namespace JSC
