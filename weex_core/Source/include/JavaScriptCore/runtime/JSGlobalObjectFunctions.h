/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2003, 2006, 2007, 2008, 2009, 2010 Apple Inc. All rights reserved.
 *  Copyright (C) 2007 Cameron Zwarich (cwzwarich@uwaterloo.ca)
 *  Copyright (C) 2007 Maks Orlovich
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

#include "JSCJSValue.h"
#include <unicode/uchar.h>
#include <wtf/text/LChar.h>

namespace JSC {

class ArgList;
class ExecState;
class JSObject;

// FIXME: These functions should really be in JSGlobalObject.cpp, but putting them there
// is a 0.5% reduction.

EncodedJSValue JSC_HOST_CALL globalFuncEval(ExecState*);
EncodedJSValue JSC_HOST_CALL globalFuncParseInt(ExecState*);
EncodedJSValue JSC_HOST_CALL globalFuncParseFloat(ExecState*);
EncodedJSValue JSC_HOST_CALL globalFuncDecodeURI(ExecState*);
EncodedJSValue JSC_HOST_CALL globalFuncDecodeURIComponent(ExecState*);
EncodedJSValue JSC_HOST_CALL globalFuncEncodeURI(ExecState*);
EncodedJSValue JSC_HOST_CALL globalFuncEncodeURIComponent(ExecState*);
EncodedJSValue JSC_HOST_CALL globalFuncEscape(ExecState*);
EncodedJSValue JSC_HOST_CALL globalFuncUnescape(ExecState*);
EncodedJSValue JSC_HOST_CALL globalFuncThrowTypeError(ExecState*);
EncodedJSValue JSC_HOST_CALL globalFuncThrowTypeErrorArgumentsCalleeAndCaller(ExecState*);
EncodedJSValue JSC_HOST_CALL globalFuncProtoGetter(ExecState*);
EncodedJSValue JSC_HOST_CALL globalFuncProtoSetter(ExecState*);
EncodedJSValue JSC_HOST_CALL globalFuncBuiltinLog(ExecState*);
EncodedJSValue JSC_HOST_CALL privateToObject(ExecState*);
EncodedJSValue JSC_HOST_CALL globalFuncImportModule(ExecState*);

double jsToNumber(StringView);

} // namespace JSC
