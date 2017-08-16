/*
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2003, 2007 Apple Inc. All rights reserved.
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

#include "CallData.h"
#include "JSCJSValue.h"
#include <wtf/NakedPtr.h>

namespace JSC {

class Exception;
class ExecState;
class JSObject;
class ParserError;
class SourceCode;
class VM;
class JSInternalPromise;

JS_EXPORT_PRIVATE bool checkSyntax(VM&, const SourceCode&, ParserError&);
JS_EXPORT_PRIVATE bool checkSyntax(ExecState*, const SourceCode&, JSValue* exception = 0);
JS_EXPORT_PRIVATE bool checkModuleSyntax(ExecState*, const SourceCode&, ParserError&);

JS_EXPORT_PRIVATE JSValue evaluate(ExecState*, const SourceCode&, JSValue thisValue, NakedPtr<Exception>& returnedException);
inline JSValue evaluate(ExecState* exec, const SourceCode& sourceCode, JSValue thisValue = JSValue())
{
    NakedPtr<Exception> unused;
    return evaluate(exec, sourceCode, thisValue, unused);
}

JS_EXPORT_PRIVATE JSValue profiledEvaluate(ExecState*, ProfilingReason, const SourceCode&, JSValue thisValue, NakedPtr<Exception>& returnedException);
inline JSValue profiledEvaluate(ExecState* exec, ProfilingReason reason, const SourceCode& sourceCode, JSValue thisValue = JSValue())
{
    NakedPtr<Exception> unused;
    return profiledEvaluate(exec, reason, sourceCode, thisValue, unused);
}

JS_EXPORT_PRIVATE JSValue evaluateWithScopeExtension(ExecState*, const SourceCode&, JSObject* scopeExtension, NakedPtr<Exception>& returnedException);

// Load the module source and evaluate it.
JS_EXPORT_PRIVATE JSInternalPromise* loadAndEvaluateModule(ExecState*, const String& moduleName, JSValue scriptFetcher = jsUndefined());
JS_EXPORT_PRIVATE JSInternalPromise* loadAndEvaluateModule(ExecState*, const SourceCode&, JSValue scriptFetcher = jsUndefined());

// Fetch the module source, and instantiate the module record.
JS_EXPORT_PRIVATE JSInternalPromise* loadModule(ExecState*, const String& moduleName, JSValue scriptFetcher = jsUndefined());
JS_EXPORT_PRIVATE JSInternalPromise* loadModule(ExecState*, const SourceCode&, JSValue scriptFetcher = jsUndefined());

// Link and evaluate the already linked module. This function is called in a sync manner.
JS_EXPORT_PRIVATE JSValue linkAndEvaluateModule(ExecState*, const Identifier& moduleKey, JSValue scriptFetcher = jsUndefined());

JS_EXPORT_PRIVATE JSInternalPromise* importModule(ExecState*, const Identifier& moduleKey, JSValue scriptFetcher);

} // namespace JSC
