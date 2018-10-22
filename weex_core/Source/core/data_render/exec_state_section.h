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

#include <stdlib.h>
#include <stdio.h>

namespace weex {
namespace core {
namespace data_render {
    
#define EXEC_BINARY_MAGIC_NUMBER                    0x6d736100
#define EXEC_BINARY_CURRENT_VERSION                 6
#define EXEC_BINARY_COMPATIBLE_VERSION              6
#define EXEC_BINARY_ENCRYPT                         0

enum ExecSection {
    EXEC_SECTION_NONE = 0,
    EXEC_SECTION_STRING,
    EXEC_SECTION_TABLE,
    EXEC_SECTION_FUNCTION,
    EXEC_SECTION_HEADER,
    EXEC_SECTION_GLOBAL_VARIABLE,
    EXEC_SECTION_STYLE,
    EXEC_SECTION_ARRAY,
    EXEC_SECTION_REFVALUE,
    EXEC_SECTION_CLASS
};

class fStream;
class ExecStateEncoder;
class ExecStateDecoder;
struct Value;
    
class Section {
public:
    enum {
        kValueType,
        kValuePlayload,
    };
    explicit Section(ExecStateEncoder *encoder) : encoder_(encoder) {}
    explicit Section(ExecStateDecoder *decoder, uint32_t length) : decoder_(decoder), length_(length) {}
    virtual ~Section() {};
    bool encoding(uint16_t index, uint32_t len, void *buffer = nullptr);
    uint32_t encodingToBuffer(uint8_t *dst_buffer, uint32_t dst_buffer_len, uint16_t index, uint32_t len, void *buffer = nullptr);
    uint32_t encodingValueToBuffer(uint8_t *dst_buffer, uint32_t dst_buffer_len, Value *value);
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
    enum Value {
        kValueStringCount,
        kValueStringContent,
    };
    SectionString(ExecStateEncoder *encoder) : Section(encoder) {}
    SectionString(ExecStateDecoder *decoder, uint32_t length) : Section(decoder, length) {}
    virtual ~SectionString() {};
    virtual bool encoding();
    virtual bool decoding();
    virtual uint32_t size();
};
    
class SectionFunction : public Section {
public:
    enum Value {
        kValueFunctionCount,
        kValueFunctionSuper,
        kValueFunctionClass,
        kValueFunctionArgs,
        kValueFunctionInClosure,
        kValueFunctionOutClosure,
        kValueFunctionInstructions,
        kValueFunctionConstantCount,
        kValueFunctionConstantPayload,
    };
    SectionFunction(ExecStateEncoder *encoder) : Section(encoder) {}
    SectionFunction(ExecStateDecoder *decoder, uint32_t length) : Section(decoder, length) {}
    virtual ~SectionFunction() {};
    virtual bool encoding();
    virtual bool decoding();
    virtual uint32_t size();
};

    
}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif // CORE_DATA_RENDER_EXEC_STATE_SECTION_H
