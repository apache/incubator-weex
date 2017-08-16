/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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

#include "Identifier.h"
#include "InitializeThreading.h"
#include "JSCInlines.h"
#include "JSCJSValue.h"
#include "JSGlobalObject.h"
#include "JSLock.h"
#include "JSObject.h"
#include "VM.h"
#include <wtf/MainThread.h>

using namespace JSC;

namespace {

StaticLock crashLock;
const char* nameFilter;
unsigned requestedIterationCount;

#define CHECK(x) do {                                                   \
        if (!!(x))                                                      \
            break;                                                      \
        crashLock.lock();                                               \
        WTFReportAssertionFailure(__FILE__, __LINE__, WTF_PRETTY_FUNCTION, #x); \
        CRASH();                                                        \
    } while (false)

template<typename Callback>
NEVER_INLINE void benchmarkImpl(const char* name, unsigned iterationCount, const Callback& callback)
{
    if (nameFilter && !strcasestr(name, nameFilter))
        return;

    if (requestedIterationCount)
        iterationCount = requestedIterationCount;
    
    double before = monotonicallyIncreasingTimeMS();
    callback(iterationCount);
    double after = monotonicallyIncreasingTimeMS();
    dataLog(name, ": ", after - before, " ms.\n");
}

} // anonymous namespace

int main(int argc, char** argv)
{
    if (argc >= 2) {
        if (argv[1][0] == '-') {
            dataLog("Usage: dynbench [<filter> [<iteration count>]]\n");
            return 1;
        }

        nameFilter = argv[1];

        if (argc >= 3) {
            if (sscanf(argv[2], "%u", &requestedIterationCount) != 1) {
                dataLog("Could not parse iteration count ", argv[2], "\n");
                return 1;
            }
        }
    }
    
    WTF::initializeMainThread();
    JSC::initializeThreading();

    VM* vm = &VM::create(LargeHeap).leakRef();
    {
        JSLockHolder locker(vm);

        JSGlobalObject* globalObject =
            JSGlobalObject::create(*vm, JSGlobalObject::createStructure(*vm, jsNull()));
        ExecState* exec = globalObject->globalExec();

        Identifier identF = Identifier::fromString(exec, "f");
        Identifier identG = Identifier::fromString(exec, "g");

        Structure* objectStructure =
            JSFinalObject::createStructure(*vm, globalObject, globalObject->objectPrototype(), 2);

        // Non-strict dynamic get by id:
        JSValue object = JSFinalObject::create(*vm, objectStructure);
        {
            PutPropertySlot slot(object, false, PutPropertySlot::PutById);
            object.putInline(exec, identF, jsNumber(42), slot);
        }
        {
            PutPropertySlot slot(object, false, PutPropertySlot::PutById);
            object.putInline(exec, identG, jsNumber(43), slot);
        }
        benchmarkImpl(
            "Non Strict Dynamic Get By Id",
            1000000,
            [&] (unsigned iterationCount) {
                for (unsigned i = iterationCount; i--;) {
                    JSValue result = object.get(exec, identF);
                    CHECK(result == jsNumber(42));
                    result = object.get(exec, identG);
                    CHECK(result == jsNumber(43));
                }
            });

        // Non-strict dynamic put by id replace:
        object = JSFinalObject::create(*vm, objectStructure);
        {
            PutPropertySlot slot(object, false, PutPropertySlot::PutById);
            object.putInline(exec, identF, jsNumber(42), slot);
        }
        {
            PutPropertySlot slot(object, false, PutPropertySlot::PutById);
            object.putInline(exec, identG, jsNumber(43), slot);
        }
        benchmarkImpl(
            "Non Strict Dynamic Put By Id Replace",
            1000000,
            [&] (unsigned iterationCount) {
                for (unsigned i = iterationCount; i--;) {
                    {
                        PutPropertySlot slot(object, false, PutPropertySlot::PutById);
                        object.putInline(exec, identF, jsNumber(i), slot);
                    }
                    {
                        PutPropertySlot slot(object, false, PutPropertySlot::PutById);
                        object.putInline(exec, identG, jsNumber(i), slot);
                    }
                }
            });

        // Non-strict dynamic put by id transition with object allocation:
        benchmarkImpl(
            "Non Strict Dynamic Allocation and Put By Id Transition",
            1000000,
            [&] (unsigned iterationCount) {
                for (unsigned i = iterationCount; i--;) {
                    JSValue object = JSFinalObject::create(*vm, objectStructure);
                    {
                        PutPropertySlot slot(object, false, PutPropertySlot::PutById);
                        object.putInline(exec, identF, jsNumber(i), slot);
                    }
                    {
                        PutPropertySlot slot(object, false, PutPropertySlot::PutById);
                        object.putInline(exec, identG, jsNumber(i), slot);
                    }
                }
            });

        // Non-strict dynamic get by id with dynamic store context:
        object = JSFinalObject::create(*vm, objectStructure);
        {
            PutPropertySlot slot(object, false);
            object.putInline(exec, identF, jsNumber(42), slot);
        }
        {
            PutPropertySlot slot(object, false);
            object.putInline(exec, identG, jsNumber(43), slot);
        }
        benchmarkImpl(
            "Non Strict Dynamic Get By Id With Dynamic Store Context",
            1000000,
            [&] (unsigned iterationCount) {
                for (unsigned i = iterationCount; i--;) {
                    JSValue result = object.get(exec, identF);
                    CHECK(result == jsNumber(42));
                    result = object.get(exec, identG);
                    CHECK(result == jsNumber(43));
                }
            });

        // Non-strict dynamic put by id replace with dynamic store context:
        object = JSFinalObject::create(*vm, objectStructure);
        {
            PutPropertySlot slot(object, false);
            object.putInline(exec, identF, jsNumber(42), slot);
        }
        {
            PutPropertySlot slot(object, false);
            object.putInline(exec, identG, jsNumber(43), slot);
        }
        benchmarkImpl(
            "Non Strict Dynamic Put By Id Replace With Dynamic Store Context",
            1000000,
            [&] (unsigned iterationCount) {
                for (unsigned i = iterationCount; i--;) {
                    {
                        PutPropertySlot slot(object, false);
                        object.putInline(exec, identF, jsNumber(i), slot);
                    }
                    {
                        PutPropertySlot slot(object, false);
                        object.putInline(exec, identG, jsNumber(i), slot);
                    }
                }
            });

        // Non-strict dynamic put by id transition with object allocation with dynamic store context:
        benchmarkImpl(
            "Non Strict Dynamic Allocation and Put By Id Transition With Dynamic Store Context",
            1000000,
            [&] (unsigned iterationCount) {
                for (unsigned i = iterationCount; i--;) {
                    JSValue object = JSFinalObject::create(*vm, objectStructure);
                    {
                        PutPropertySlot slot(object, false);
                        object.putInline(exec, identF, jsNumber(i), slot);
                    }
                    {
                        PutPropertySlot slot(object, false);
                        object.putInline(exec, identG, jsNumber(i), slot);
                    }
                }
            });
    }

    crashLock.lock();
    return 0;
}

