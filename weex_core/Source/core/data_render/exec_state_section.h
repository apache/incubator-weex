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

#ifndef CORE_DATA_RENDER_EXEC_STATE_SECTION_H
#define CORE_DATA_RENDER_EXEC_STATE_SECTION_H

#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include "core/data_render/op_code.h"

namespace weex {
namespace core {
namespace data_render {
    
#define EXEC_BINARY_MAGIC_NUMBER                    0x6d736100
#define EXEC_BINARY_CURRENT_VERSION                 6
#define EXEC_BINARY_COMPATIBLE_VERSION              6
#define EXEC_BINARY_ENCRYPT                         0

enum ExecSection {
    EXEC_SECTION_NONE = 0,
    EXEC_SECTION_HEADER,
    EXEC_SECTION_STRING,
    EXEC_SECTION_DATA,
    EXEC_SECTION_FUNCTION,
    EXEC_SECTION_GLOBAL_CONSTANTS,
    EXEC_SECTION_GLOBAL_VARIABLES,
    EXEC_SECTION_STYLES,
    EXEC_SECTION_VALUEREF,
    EXEC_SECTION_CLASS,
    EXEC_SECTION_SCRIPT
};

class fStream;
class ExecStateEncoder;
class ExecStateDecoder;
struct Value;
    
class Section {
public:
    explicit Section(ExecStateEncoder *encoder) : encoder_(encoder) {}
    explicit Section(ExecStateDecoder *decoder, uint32_t length) : decoder_(decoder), length_(length) {}
    virtual ~Section() {};
    bool encoding(uint16_t index, uint32_t len, void *buffer = nullptr);
    uint32_t encodingToBuffer(uint8_t *dst_buffer, uint32_t dst_buffer_len, uint16_t index, uint32_t len, void *buffer = nullptr);
    uint32_t encodingToBuffer(uint8_t *dst_buffer, uint32_t dst_buffer_len, uint32_t len, void *buffer);
    uint32_t decodingFromBuffer(uint8_t *src_buffer, uint32_t src_buffer_len, uint16_t *index, uint8_t *buffer, uint32_t *len);
    uint32_t decodingFromBuffer(uint8_t *src_buffer, uint32_t src_buffer_len, uint8_t *buffer, uint32_t *len);
    uint32_t encodingValueToBuffer(uint8_t *buffer, uint32_t buffer_len, Value *pval);
    uint32_t decodingValueFromBuffer(uint8_t *buffer, uint32_t buffer_len, Value *pval);
    virtual bool encoding() { return false; };
    virtual bool decoding() { return false; };
    virtual uint32_t size() { return 0; };
    fStream *stream();
    inline uint32_t length() { return length_; }
    uint32_t GetValueLength(Value *value);
    inline ExecStateEncoder *encoder() { return encoder_; }
    inline ExecStateDecoder *decoder() { return decoder_; }
private:
    ExecStateEncoder *encoder_{nullptr};
    ExecStateDecoder *decoder_{nullptr};
    uint32_t length_{0};
};

class SectionHeader : public Section {
public:
    enum Header {
        kHeaderMagicCode,
        kHeaderVersion,
        kHeaderCompartibleVersion,
        kHeaderEncrypt,
    };
    SectionHeader(ExecStateEncoder *encoder) : Section(encoder) {}
    SectionHeader(ExecStateDecoder *decoder, uint32_t length) : Section(decoder, length) {}
    virtual ~SectionHeader() {};
    virtual bool encoding();
    virtual bool decoding();
    virtual uint32_t size();
private:
    uint32_t version_{0};
    uint32_t compartible_version_{0};
    uint32_t encrypt_{0};
};
    
class SectionString : public Section {
public:
    enum SectionKey {
        kValueStringSize,
        kValueStringPayload,
    };
    SectionString(ExecStateEncoder *encoder) : Section(encoder) {}
    SectionString(ExecStateDecoder *decoder, uint32_t length) : Section(decoder, length) {}
    virtual ~SectionString() {};
    virtual bool encoding();
    virtual bool decoding();
    virtual uint32_t size();
};
    
class SectionData : public Section {
public:
    enum SectionKey {
        kValueDataSize,
        kValueDataPayload
    };
    SectionData(ExecStateEncoder *encoder) : Section(encoder) {}
    SectionData(ExecStateDecoder *decoder, uint32_t length) : Section(decoder, length) {}
    virtual ~SectionData() {};
    virtual bool encoding();
    virtual bool decoding();
    virtual uint32_t size();
};

class SectionScript : public Section {
public:
    enum SectionKey {
        kValueScriptSize,
        kValueScriptKey,
        kValueScriptValue,
        kValueScriptItemSize,
    };
    SectionScript(ExecStateEncoder *encoder) : Section(encoder) {}
    SectionScript(ExecStateDecoder *decoder, uint32_t length) : Section(decoder, length) {}
    virtual ~SectionScript() {};
    virtual bool encoding();
    virtual bool decoding();
    virtual uint32_t size();
};
    
class SectionFunction : public Section {
public:
    enum SectionKey {
        kValueFunctionSize,
        kValueFunctionSuper,
        kValueFunctionClass,
        kValueFunctionArgs,
        kValueFunctionInClosure,
        kValueFunctionOutClosure,
        kValueFunctionInstructions,
        kValueFunctionConstantCount,
        kValueFunctionConstantPayload,
        kValueFunctionFinished = 255,
    };
    SectionFunction(ExecStateEncoder *encoder, uint8_t op_code_bits) : Section(encoder), op_code_bits_(op_code_bits)  {}
    SectionFunction(ExecStateDecoder *decoder, uint8_t op_code_bits, uint32_t length) : Section(decoder, length), op_code_bits_(op_code_bits) {}
    uint32_t GetInstructionsBytes(std::vector<Instruction>& instructions);
    uint32_t encodingInstructionsToBuffer(uint8_t *buffer, uint32_t bytes, std::vector<Instruction>& instructions);
    uint32_t decodingInstructionsFromBuffer(uint8_t *buffer, uint32_t bytes, std::vector<Instruction>& instructions);
    virtual ~SectionFunction() {};
    virtual bool encoding();
    virtual bool decoding();
    virtual uint32_t size();
private:
    uint8_t op_code_bits_{0};
};

class SectionGlobalConstants : public Section {
public:
    enum SectionKey {
        kValueGlobalConstantsSize,
        kValueGlobalConstantsPayload,
    };
    SectionGlobalConstants(ExecStateEncoder *encoder) : Section(encoder) {}
    SectionGlobalConstants(ExecStateDecoder *decoder, uint32_t length) : Section(decoder, length) {}
    virtual ~SectionGlobalConstants() {};
    virtual bool encoding();
    virtual bool decoding();
    virtual uint32_t size();
};

class SectionGlobalVariables: public Section {
public:
    enum SectionKey {
        kValueGlobalVariablesSize,
        kValueGlobalVariablesKey,
        kValueGlobalVariablesRegister
    };
    SectionGlobalVariables(ExecStateEncoder *encoder) : Section(encoder) {}
    SectionGlobalVariables(ExecStateDecoder *decoder, uint32_t length) : Section(decoder, length) {}
    virtual ~SectionGlobalVariables() {};
    virtual bool encoding();
    virtual bool decoding();
    virtual uint32_t size();
};

class SectionStyles: public Section {
public:
    enum SectionKey {
        kValueStyleSize,
        kValueStyleKey,
        kValueStyleValue,
        kValueStyleItemSize,
    };
    SectionStyles(ExecStateEncoder *encoder) : Section(encoder) {}
    SectionStyles(ExecStateDecoder *decoder, uint32_t length) : Section(decoder, length) {}
    virtual ~SectionStyles() {};
    virtual bool encoding();
    virtual bool decoding();
    virtual uint32_t size();
};

class SectionVaueRef : public Section {
public:
    enum SectionKey {
        kValueRefSize,
        kValueFuncState,
        kValueIsClosure,
        kValueRegister,
        kValueRefFinished,
    };
    SectionVaueRef(ExecStateEncoder *encoder) : Section(encoder) {}
    SectionVaueRef(ExecStateDecoder *decoder, uint32_t length) : Section(decoder, length) {}
    virtual ~SectionVaueRef() {};
    virtual bool encoding();
    virtual bool decoding();
    virtual uint32_t size();
};

class SectionClass : public Section {
public:
    enum SectionKey {
        kValueClassSize,
        kValueClassSuper,
        kValueClassFunctionKey,
        kValueClassStaticFunctionSize,
        kValueClassStaticFunction,
        kValueClassMemberFunctionSize,
        kValueClassMemberFunction,
        kValueClassFinished,
    };
    SectionClass(ExecStateEncoder *encoder) : Section(encoder) {}
    SectionClass(ExecStateDecoder *decoder, uint32_t length) : Section(decoder, length) {}
    virtual ~SectionClass() {};
    virtual bool encoding();
    virtual bool decoding();
    virtual uint32_t size();
};

}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif // CORE_DATA_RENDER_EXEC_STATE_SECTION_H
