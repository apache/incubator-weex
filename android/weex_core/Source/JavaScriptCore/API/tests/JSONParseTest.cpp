/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "JSONParseTest.h"

#include "JSCInlines.h"
#include "JSGlobalObject.h"
#include "JSONObject.h"
#include "VM.h"
#include <wtf/RefPtr.h>

using namespace JSC;

int testJSONParse()
{
    bool failed = false;

    RefPtr<VM> vm = VM::create();
    
    JSLockHolder locker(vm.get());
    JSGlobalObject* globalObject = JSGlobalObject::create(*vm, JSGlobalObject::createStructure(*vm, jsNull()));
    
    ExecState* exec = globalObject->globalExec();
    JSValue v0 = JSONParse(exec, "");
    JSValue v1 = JSONParse(exec, "#$%^");
    JSValue v2 = JSONParse(exec, String());
    UChar emptyUCharArray[1] = { '\0' };
    JSValue v3 = JSONParse(exec, String(emptyUCharArray, 0));
    JSValue v4;
    JSValue v5 = JSONParse(exec, "123");
    
    failed = failed || (v0 != v1);
    failed = failed || (v1 != v2);
    failed = failed || (v2 != v3);
    failed = failed || (v3 != v4);
    failed = failed || (v4 == v5);

    vm = nullptr;

    if (failed)
        printf("FAIL: JSONParse String test.\n");
    else
        printf("PASS: JSONParse String test.\n");

    return failed;
}
