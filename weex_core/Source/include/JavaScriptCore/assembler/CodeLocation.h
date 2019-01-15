/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#pragma once

#include "MacroAssemblerCodeRef.h"

#if ENABLE(ASSEMBLER)

namespace JSC {

enum NearCallMode { Regular, Tail };

class CodeLocationInstruction;
class CodeLocationLabel;
class CodeLocationJump;
class CodeLocationCall;
class CodeLocationNearCall;
class CodeLocationDataLabelCompact;
class CodeLocationDataLabel32;
class CodeLocationDataLabelPtr;
class CodeLocationConvertibleLoad;

// The CodeLocation* types are all pretty much do-nothing wrappers around
// CodePtr (or MacroAssemblerCodePtr, to give it its full name).  These
// classes only exist to provide type-safety when linking and patching code.
//
// The one new piece of functionallity introduced by these classes is the
// ability to create (or put another way, to re-discover) another CodeLocation
// at an offset from one you already know.  When patching code to optimize it
// we often want to patch a number of instructions that are short, fixed
// offsets apart.  To reduce memory overhead we will only retain a pointer to
// one of the instructions, and we will use the *AtOffset methods provided by
// CodeLocationCommon to find the other points in the code to modify.
class CodeLocationCommon : public MacroAssemblerCodePtr {
public:
    CodeLocationInstruction instructionAtOffset(int offset);
    CodeLocationLabel labelAtOffset(int offset);
    CodeLocationJump jumpAtOffset(int offset);
    CodeLocationCall callAtOffset(int offset);
    CodeLocationNearCall nearCallAtOffset(int offset, NearCallMode);
    CodeLocationDataLabelPtr dataLabelPtrAtOffset(int offset);
    CodeLocationDataLabel32 dataLabel32AtOffset(int offset);
    CodeLocationDataLabelCompact dataLabelCompactAtOffset(int offset);
    CodeLocationConvertibleLoad convertibleLoadAtOffset(int offset);

protected:
    CodeLocationCommon()
    {
    }

    CodeLocationCommon(MacroAssemblerCodePtr location)
        : MacroAssemblerCodePtr(location)
    {
    }
};

class CodeLocationInstruction : public CodeLocationCommon {
public:
    CodeLocationInstruction() {}
    explicit CodeLocationInstruction(MacroAssemblerCodePtr location)
        : CodeLocationCommon(location) {}
    explicit CodeLocationInstruction(void* location)
        : CodeLocationCommon(MacroAssemblerCodePtr(location)) {}
};

class CodeLocationLabel : public CodeLocationCommon {
public:
    CodeLocationLabel() {}
    explicit CodeLocationLabel(MacroAssemblerCodePtr location)
        : CodeLocationCommon(location) {}
    explicit CodeLocationLabel(void* location)
        : CodeLocationCommon(MacroAssemblerCodePtr(location)) {}
};

class CodeLocationJump : public CodeLocationCommon {
public:
    CodeLocationJump() {}
    explicit CodeLocationJump(MacroAssemblerCodePtr location)
        : CodeLocationCommon(location) {}
    explicit CodeLocationJump(void* location)
        : CodeLocationCommon(MacroAssemblerCodePtr(location)) {}
};

class CodeLocationCall : public CodeLocationCommon {
public:
    CodeLocationCall() {}
    explicit CodeLocationCall(MacroAssemblerCodePtr location)
        : CodeLocationCommon(location) {}
    explicit CodeLocationCall(void* location)
        : CodeLocationCommon(MacroAssemblerCodePtr(location)) {}
};

class CodeLocationNearCall : public CodeLocationCommon {
public:
    CodeLocationNearCall() {}
    explicit CodeLocationNearCall(MacroAssemblerCodePtr location, NearCallMode callMode)
        : CodeLocationCommon(location), m_callMode(callMode) { }
    explicit CodeLocationNearCall(void* location, NearCallMode callMode)
        : CodeLocationCommon(MacroAssemblerCodePtr(location)), m_callMode(callMode) { }
    NearCallMode callMode() { return m_callMode; }
private:
    NearCallMode m_callMode = NearCallMode::Regular;
};

class CodeLocationDataLabel32 : public CodeLocationCommon {
public:
    CodeLocationDataLabel32() {}
    explicit CodeLocationDataLabel32(MacroAssemblerCodePtr location)
        : CodeLocationCommon(location) {}
    explicit CodeLocationDataLabel32(void* location)
        : CodeLocationCommon(MacroAssemblerCodePtr(location)) {}
};

class CodeLocationDataLabelCompact : public CodeLocationCommon {
public:
    CodeLocationDataLabelCompact() { }
    explicit CodeLocationDataLabelCompact(MacroAssemblerCodePtr location)
        : CodeLocationCommon(location) { }
    explicit CodeLocationDataLabelCompact(void* location)
        : CodeLocationCommon(MacroAssemblerCodePtr(location)) { }
};

class CodeLocationDataLabelPtr : public CodeLocationCommon {
public:
    CodeLocationDataLabelPtr() {}
    explicit CodeLocationDataLabelPtr(MacroAssemblerCodePtr location)
        : CodeLocationCommon(location) {}
    explicit CodeLocationDataLabelPtr(void* location)
        : CodeLocationCommon(MacroAssemblerCodePtr(location)) {}
};

class CodeLocationConvertibleLoad : public CodeLocationCommon {
public:
    CodeLocationConvertibleLoad() { }
    explicit CodeLocationConvertibleLoad(MacroAssemblerCodePtr location)
        : CodeLocationCommon(location) { }
    explicit CodeLocationConvertibleLoad(void* location)
        : CodeLocationCommon(MacroAssemblerCodePtr(location)) { }
};

inline CodeLocationInstruction CodeLocationCommon::instructionAtOffset(int offset)
{
    ASSERT_VALID_CODE_OFFSET(offset);
    return CodeLocationInstruction(reinterpret_cast<char*>(dataLocation()) + offset);
}

inline CodeLocationLabel CodeLocationCommon::labelAtOffset(int offset)
{
    ASSERT_VALID_CODE_OFFSET(offset);
    return CodeLocationLabel(reinterpret_cast<char*>(dataLocation()) + offset);
}

inline CodeLocationJump CodeLocationCommon::jumpAtOffset(int offset)
{
    ASSERT_VALID_CODE_OFFSET(offset);
    return CodeLocationJump(reinterpret_cast<char*>(dataLocation()) + offset);
}

inline CodeLocationCall CodeLocationCommon::callAtOffset(int offset)
{
    ASSERT_VALID_CODE_OFFSET(offset);
    return CodeLocationCall(reinterpret_cast<char*>(dataLocation()) + offset);
}

inline CodeLocationNearCall CodeLocationCommon::nearCallAtOffset(int offset, NearCallMode callMode)
{
    ASSERT_VALID_CODE_OFFSET(offset);
    return CodeLocationNearCall(reinterpret_cast<char*>(dataLocation()) + offset, callMode);
}

inline CodeLocationDataLabelPtr CodeLocationCommon::dataLabelPtrAtOffset(int offset)
{
    ASSERT_VALID_CODE_OFFSET(offset);
    return CodeLocationDataLabelPtr(reinterpret_cast<char*>(dataLocation()) + offset);
}

inline CodeLocationDataLabel32 CodeLocationCommon::dataLabel32AtOffset(int offset)
{
    ASSERT_VALID_CODE_OFFSET(offset);
    return CodeLocationDataLabel32(reinterpret_cast<char*>(dataLocation()) + offset);
}

inline CodeLocationDataLabelCompact CodeLocationCommon::dataLabelCompactAtOffset(int offset)
{
    ASSERT_VALID_CODE_OFFSET(offset);
    return CodeLocationDataLabelCompact(reinterpret_cast<char*>(dataLocation()) + offset);
}

inline CodeLocationConvertibleLoad CodeLocationCommon::convertibleLoadAtOffset(int offset)
{
    ASSERT_VALID_CODE_OFFSET(offset);
    return CodeLocationConvertibleLoad(reinterpret_cast<char*>(dataLocation()) + offset);
}

} // namespace JSC

#endif // ENABLE(ASSEMBLER)
