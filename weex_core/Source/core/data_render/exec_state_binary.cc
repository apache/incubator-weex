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

#include "core/data_render/exec_state_binary.h"
#include "core/data_render/common_error.h"
#include "core/data_render/vm.h"
#include "core/data_render/exec_state.h"
#include "third_party/json11/json11.hpp"
#include "core/data_render/vnode/vnode_exec_env.h"

namespace weex {
namespace core {
namespace data_render {
        
bool ExecStateEncoder::encoding(std::string &err) {
    bool finished = false;
    do {
        try {
            SectionHeader header(this);
            if (!header.encoding()) {
                err = "header encoding error";
                break;
            }
            SectionString string(this);
            if (!string.encoding()) {
                err = "string section encoding error";
                break;
            }
            SectionFunction function(this);
            if (!function.encoding()) {
                err = "function section encoding error";
                break;
            }
            SectionClass classDescriptor(this);
            if (!classDescriptor.encoding()) {
                err = "class encoding error";
                break;
            }
            SectionGlobalConstants constants(this);
            if (!constants.encoding()) {
                err = "global constants section encoding error";
                break;
            }
            SectionGlobalVariables variables(this);
            if (!variables.encoding()) {
                err = "global variables section encoding error";
                break;
            }
            SectionStyles styles(this);
            if (!styles.encoding()) {
                err = "style section encoding error";
                break;
            }
            SectionVaueRef valueRef(this);
            if (!valueRef.encoding()) {
                err = "value ref section encoding error";
                break;
            }
        }
        catch (std::exception &e) {
            auto error = static_cast<Error *>(&e);
            if (error) {
                err = error->what();
                std::cerr << error->what() << std::endl;
            }
            break;
        }
        finished = true;
        
    } while (0);
    
    return finished;
}
    
bool ExecStateDecoder::decoding(std::string &err) {
    bool finished = false;
    do {
        if (stream_->Seek(0, SEEK_END) <= 0) {
            err = "file length zero error";
            break;
        }
        int64_t fileSize = stream_->Tell();
        if (stream_->Seek(0, SEEK_SET) < 0) {
            err = "file length zero error";
            break;
        }
        try {
            do {
                uint16_t section = 0;
                uint32_t section_length = 0;
                if ((section_length = stream_->ReadTarget(&section, NULL, NULL)) == 0) {
                    err = "read section flag error";
                    break;
                }
                switch (section) {
                    case ExecSection::EXEC_SECTION_HEADER:
                    {
                        SectionHeader header(this, section_length);
                        if (!header.decoding()) {
                            throw EncoderError("header section decoding error");
                        }
                        break;
                    }
                    case ExecSection::EXEC_SECTION_STRING:
                    {
                        SectionString string(this, section_length);
                        if (!string.decoding()) {
                            throw EncoderError("string section decoding error");
                        }
                        break;
                    }
                    case ExecSection::EXEC_SECTION_FUNCTION:
                    {
                        SectionFunction function(this, section_length);
                        if (!function.decoding()) {
                            throw EncoderError("function section decoding error");
                        }
                        break;
                    }
                    case ExecSection::EXEC_SECTION_GLOBAL_CONSTANTS:
                    {
                        SectionGlobalConstants constants(this, section_length);
                        if (!constants.decoding()) {
                            throw EncoderError("function global constants decoding error");
                        }
                        break;
                    }
                    case ExecSection::EXEC_SECTION_GLOBAL_VARIABLES:
                    {
                        SectionGlobalVariables variables(this, section_length);
                        if (!variables.decoding()) {
                            throw EncoderError("function global variables decoding error");
                        }
                        break;
                    }
                    case ExecSection::EXEC_SECTION_STYLES:
                    {
                        SectionStyles styles(this, section_length);
                        if (!styles.decoding()) {
                            throw EncoderError("function global variables decoding error");
                        }
                        break;
                    }
                    case ExecSection::EXEC_SECTION_VALUEREF:
                    {
                        SectionVaueRef valueRef(this, section_length);
                        if (!valueRef.decoding()) {
                            throw EncoderError("value ref decoding error");
                        }
                        break;
                    }
                    case ExecSection::EXEC_SECTION_CLASS:
                    {
                        SectionClass classDescriptor(this, section_length);
                        if (!classDescriptor.decoding()) {
                            throw EncoderError("class decoding error");
                        }
                        break;
                    }
                    default:
                    {
                        if (stream_->Seek(section_length, SEEK_CUR) < 0) {
                            throw EncoderError("section decoding length error");
                        }
                        break;
                    }
                }
                
            } while (stream_->Tell() < fileSize);
        }
        catch (std::exception &e) {
            auto error = static_cast<Error *>(&e);
            if (error) {
                err = error->what();
                std::cerr << error->what() << std::endl;
            }
            break;
        }
        finished = true;
        
    } while (0);
    
    return finished;
}
    
uint32_t fStream::Read(void *buffer, uint32_t size, uint32_t cnt) {
    return Read(buffer, size * cnt) / size;
}
    
uint32_t fStream::ReadTarget(uint16_t *target, uint8_t *buffer, uint32_t *size) {
    uint32_t bytesRead = 0;
    do {
        uint32_t flags = 0;
        uint32_t target_length = 0;
        uint16_t target_value = 0;
        uint32_t size_length = 0;
        uint32_t value_length = 0;
        if (Read(&flags, 1, 1) != 1) {
            break;
        }
        if (SECTION_TEST_FLAG(flags, SECTION_EXT_MASK)) {
            if (Read((uint8_t *)&flags + 1, 1, 3) != 3) {
                break;
            }
        }
        // tag
        target_length = SECTION_TEST_FLAG(flags, SECTION_TAG_MASK) ? 2 : 1;
        if (Read(&target_value, 1, target_length) != target_length) {
            break;
        }
        // length
        SECTION_GET_LENGTH_LEN(flags, size_length);
        if (size_length == 0) {
            value_length = (flags & SECTION_VALUE_MASK) + 1;    // 0:1 1:2 2:4 3:8
            if (value_length == 3) {
                value_length = 4;
            }
            else if (value_length == 4) {
                value_length = 8;
            }
        }
        else {
            if (Read(&value_length, 1, size_length) != size_length) {
                break;
            }
        }
        if (target) {
            *target = target_value;
        }
        if (buffer == NULL || size == NULL || *size == 0 || value_length == 0) {
            bytesRead = value_length;
            break;
        }
        uint32_t buffer_length = *size;
        *size = value_length;
        if (buffer_length > value_length) {
            buffer_length = value_length;
        }
        if (Read(buffer, 1, buffer_length) != buffer_length) {
            break;
        }
        if (buffer_length < value_length) {
            Seek(value_length - buffer_length, SEEK_CUR);
        }
        bytesRead = value_length;

    } while (0);
    
    return bytesRead;
}
    
uint32_t fStream::Read(void *buffer, uint32_t size) {
    uint32_t readBytes = 0;
    do {
        int64_t left = size_ - seek_;
        if (left < 0) {
            break;
        }
        if (left < (int64_t)size) {
            size = (uint32_t)left;
        }
        memcpy(buffer, buffer_ + seek_, size);
        seek_ += size;
        readBytes = size;
        
    } while (0);
    
    return readBytes;
}
    
int64_t fStream::Seek(int64_t pos, int type) {
    switch (type) {
        case SEEK_SET:
        {
            seek_ = pos > size_ ? size_ : pos;
            break;
        }
        case SEEK_CUR:
        {
            if (seek_ + pos > size_) {
                seek_ = size_;
            }
            else {
                seek_ += pos;
            }
            break;
        }
        case SEEK_END:
        {
            if (pos >= 0) {
                seek_ = size_;
            }
            else {
                if (seek_ <= (uint64_t)(-pos)) {
                    seek_ = 0;
                }
                else {
                    seek_ -= (uint64_t)(-pos);
                }
            }
            break;
        }

    }
    return (int64_t)seek_;
}
    
bool WXExecEncoder(std::string &input, std::string &path, std::string &error) {
    bool finished = false;
    VM *vm = nullptr;
    ExecState *exec_state = nullptr;
    FILE *fout = nullptr;
    do {
        fout = fopen(path.c_str(), "wb");
        if (fout == nullptr) {
            error = "can't open " + path;
            break;
        }
        if (!input.length()) {
            error = "source is nil";
            break;
        }
        vm = new VM();
        if (!vm) {
            error = "low memory";
            break;
        }
        exec_state = new ExecState(vm);
        VNodeExecEnv::InitCFuncEnv(exec_state);
        if (!exec_state) {
            error = "low memory";
            break;
        }
        std::string err;
        json11::Json json = json11::Json::parse(input, err);
        if (!err.empty() || json.is_null()) {
            exec_state->context()->raw_source() = input;
        }
        else {
            exec_state->context()->raw_json() = json;
        }
        VNodeExecEnv::InitStyleList(exec_state);
        exec_state->Compile(err);
        if (!err.empty()) {
            error = err;
            break;
        }
        ExecStateEncoder encoder(exec_state, fout);
        if (!encoder.encoding(err)) {
            error = err;
            break;
        }
        finished = true;
        
    } while (0);
    
    if (exec_state) {
        delete exec_state;
    }
    if (vm) {
        delete vm;
    }
    if (fout) {
        fclose(fout);
    }    
    return finished;
}
    
bool WXExecDecoder(ExecState *exec_state, uint8_t *buffer, size_t size, std::string &error) {
    bool finished = false;
    do {
        std::string err;
        ExecStateDecoder decoder(exec_state, buffer, size);
        if (!decoder.decoding(err)) {
            error = err;
            break;
        }
        finished = true;
        
    } while (0);
    
    return finished;
}
    
    
}  // namespace data_render
}  // namespace core
}  // namespace weex
