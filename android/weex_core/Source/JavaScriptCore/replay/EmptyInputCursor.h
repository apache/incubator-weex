/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
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
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#if ENABLE(WEB_REPLAY)

#include "InputCursor.h"
#include <wtf/Assertions.h>
#include <wtf/Noncopyable.h>

namespace JSC {

class EmptyInputCursor final : public InputCursor {
    WTF_MAKE_NONCOPYABLE(EmptyInputCursor);
public:
    virtual ~EmptyInputCursor() { }

    static Ref<EmptyInputCursor> create()
    {
        return adoptRef(*new EmptyInputCursor());
    }

    bool isCapturing() const override { return false; }
    bool isReplaying() const override { return false; }

    NondeterministicInputBase* uncheckedLoadInput(InputQueue) override
    {
        ASSERT_NOT_REACHED();
        return nullptr;
    }

    void storeInput(std::unique_ptr<NondeterministicInputBase>) override
    {
        ASSERT_NOT_REACHED();
    }

protected:
    NondeterministicInputBase* loadInput(InputQueue, const String&) override
    {
        ASSERT_NOT_REACHED();
        return nullptr;
    }

private:
    EmptyInputCursor() { }
};

} // namespace JSC

using JSC::EmptyInputCursor;

#endif // ENABLE(WEB_REPLAY)
