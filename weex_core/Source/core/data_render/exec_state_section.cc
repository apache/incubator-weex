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

#include "core/data_render/object.h"
#include "core/data_render/exec_state_section.h"
#include "core/data_render/exec_state_binary.h"
#include "core/data_render/common_error.h"
#include "core/data_render/exec_state.h"
#include "core/data_render/string_table.h"

namespace weex {
namespace core {
namespace data_render {
    
#define SECTION_FUNCTION_MAXARGS            256
#define SECTION_SET_LENGTH_LEN_FLAG(flag, LenLen) if (LenLen == 4) LenLen = 3; flag |= (LenLen << SECTION_LEN_OFF);
    
static uint8_t indexForU16(uint16_t index, uint32_t bytes, uint32_t len)
{
    uint8_t flags = 0;
    flags |= (index > 255 ? SECTION_TAG_MASK : 0);
    SECTION_SET_LENGTH_LEN_FLAG(flags, bytes);
    if (bytes == 0) {
        switch (len) {
            case 1:
                flags &= (~SECTION_VALUE_MASK);
                break;
            case 2:
                flags |= 0x1;
                break;
            case 4:
                flags |= 0x2;
                break;
            case 8:
                flags |= 0x3;
                break;
        }
    }
    return flags;
}

static uint32_t bytesForU32(uint32_t length) {
    if (length == 1) {
        return 0;
    }
    else if (length == 2) {
        return 0;
    }
    else if (length == 4) {
        return 0;
    }
    else if (length == 8) {
        return 0;
    }
    else if (length < 0xff) {
        return 1;
    }
    else if (length < 0xffff) {
        return 2;
    }
    else {
        return 4;
    }
}
    
static uint32_t GetFTLVLength(uint16_t flag, uint32_t length) {
    return 1 + (flag > 255 ? 2 : 1) + bytesForU32(length) + length;
}
    
fStream *Section::stream() {
    return decoder_ ? decoder_->stream_ : nullptr;
}
    
uint32_t Section::encodingValueToBuffer(uint8_t *dst_buffer, uint32_t dst_buffer_len, Value *value) {
    uint32_t total_bytes_write = 0;
    do {
        uint32_t bytes_write = 0;
        uint8_t type = ttype(value);
        if (!(bytes_write = encodingToBuffer(dst_buffer, dst_buffer_len, kValueType, sizeof(uint8_t), &type))) {
            break;
        }
        dst_buffer += bytes_write;
        dst_buffer_len -= bytes_write;
        total_bytes_write += bytes_write;
        switch (type) {
            case Value::Type::CLASS_DESC:
            {
                int32_t payload = -1;
                std::vector<ClassDescriptor *> descs = encoder_->exec_state()->class_factory()->descs();
                int index = 0;
                for (auto desc : descs) {
                    if (value->gc == reinterpret_cast<GCObject *>(desc)) {
                        payload = index;
                        break;
                    }
                    index++;
                }
                if (payload < 0) {
                    throw EncoderError("encoder value class desc can't find error");
                    break;
                }
                if (!(bytes_write = encodingToBuffer(dst_buffer, dst_buffer_len, kValuePlayload, sizeof(int32_t), &payload))) {
                    break;
                }
                break;
            }
            case Value::Type::FUNC:
            {
                int32_t payload = -1;
                FuncState *func_state = encoder_->exec_state()->func_state();
                if (func_state != value->f) {
                    const std::vector<FuncState *> &childrens = func_state->all_childrens();
                    payload++;
                    for (auto &func : childrens) {
                        if (value->f == func) {
                            break;
                        }
                        payload++;
                    }
                }
                else {
                    payload++;
                }
                if (payload < 0) {
                    throw EncoderError("encoder value function can't find error");
                    break;
                }
                if (!(bytes_write = encodingToBuffer(dst_buffer, dst_buffer_len, kValuePlayload, sizeof(int32_t), &payload))) {
                    break;
                }
                break;
            }
            case Value::Type::STRING:
            {
                int32_t payload = -1;
                int index = 0;
                StringTable *string_table = encoder_->exec_state()->string_table();
                for (auto &store : string_table->store() ) {
                    if (value->str == store.second.get()) {
                        payload = index;
                        break;
                    }
                    index++;
                }
                if (payload < 0) {
                    throw EncoderError("encoder value string can't find error");
                    break;
                }
                if (!(bytes_write = encodingToBuffer(dst_buffer, dst_buffer_len, kValuePlayload, sizeof(int32_t), &payload))) {
                    break;
                }
                break;
            }
            case Value::Type::INT:
            {
                if (!(bytes_write = encodingToBuffer(dst_buffer, dst_buffer_len, kValuePlayload, sizeof(int64_t), &value->i))) {
                    break;
                }
                break;
            }
            case Value::Type::NUMBER:
            {
                if (!(bytes_write = encodingToBuffer(dst_buffer, dst_buffer_len, kValuePlayload, sizeof(double), &value->n))) {
                    break;
                }
                break;
            }
            case Value::Type::BOOL:
            {
                if (!(bytes_write = encodingToBuffer(dst_buffer, dst_buffer_len, kValuePlayload, sizeof(uint8_t), &value->b))) {
                    break;
                }
                break;
            }
            default:
                throw EncoderError("encoder value type unkown error");
                break;
        }
        total_bytes_write += bytes_write;
        
    } while (0);
    
    return total_bytes_write;
}
    
uint32_t Section::encodingToBuffer(uint8_t *dst_buffer, uint32_t dst_buffer_len, uint16_t index, uint32_t len, void *buffer)
{
    uint32_t byte_write = 0;
    do {
        uint32_t index_bytes = index > 255 ? 2 : 1;
        uint32_t len_bytes = bytesForU32(len);
        uint8_t flags = indexForU16(index, len_bytes, len);
        if (dst_buffer_len < 1 + index_bytes + len_bytes + len) {
            break;
        }
        *(uint8_t *)dst_buffer = flags;
        dst_buffer = (uint8_t *)dst_buffer + 1;
        if (index_bytes == 1) {
            *(uint8_t *)dst_buffer = (uint8_t)index;
        }
        else {
            *(uint16_t *)dst_buffer = (uint16_t)index;
        }
        dst_buffer = (uint8_t *)dst_buffer + index_bytes;
        if (len_bytes) {
            memcpy(dst_buffer, &len, len_bytes);
        }
        dst_buffer = (uint8_t *)dst_buffer + len_bytes;
        memcpy(dst_buffer, buffer, len);
        byte_write = 1 + index_bytes + len_bytes + len;
        
    } while (0);
    
    return byte_write;
}
    
bool Section::encoding(uint16_t index, uint32_t len, void *buffer) {
    bool finished = false;
    do {
        FILE *fout = encoder_->fout_;
        if (!fout) {
            throw EncoderError("encoder can't open file");
            break;
        }
        uint32_t index_bytes = index > 255 ? 2 : 1;
        uint32_t len_bytes = bytesForU32(len);
        uint8_t flags = indexForU16(index, len_bytes, len);
        size_t bytes_encoder = sizeof(flags);
        if (fwrite(&flags, 1, 1, fout) != bytes_encoder) {
            throw EncoderError("encoder write flags error");
            break;
        }
        if (fwrite(&index, 1, index_bytes, fout) != index_bytes) {
            throw EncoderError("encoder write flags error");
            break;
        }
        if (len_bytes > 0) {
            if (fwrite(&len, 1, len_bytes, fout) != len_bytes) {
                throw EncoderError("encoder write length error");
                break;
            }
        }
        if (len > 0 && buffer) {
            if (fwrite(buffer, 1, len, fout) != len) {
                throw EncoderError("encoder write buffer error");
                break;
            }
        }
        finished = true;

    } while (0);
    
    return finished;
}
    
uint32_t Section::GetValueLength(Value *value) {
    uint32_t length = 0;
    length += GetFTLVLength(kValueType, sizeof(uint8_t));
    switch (ttype(value)) {
        case Value::Type::CLASS_DESC:
        case Value::Type::FUNC:
        case Value::Type::STRING:
        {
            length += GetFTLVLength(kValuePlayload, sizeof(int32_t));
            break;
        }
        case Value::Type::INT:
        {
            length += GetFTLVLength(kValuePlayload, sizeof(int64_t));
            break;
        }
        case Value::Type::NUMBER:
        {
            length += GetFTLVLength(kValuePlayload, sizeof(double));
            break;
        }
        case Value::Type::BOOL:
        {
            length += GetFTLVLength(kValuePlayload, sizeof(char));
            break;
        }
        default:
            throw EncoderError("encoder value type unkown error");
            break;
    }
    return length;
}
    
uint32_t SectionHeader::size() {
    return GetFTLVLength(kHeaderMagicCode, sizeof(uint32_t)) + GetFTLVLength(kHeaderVersion, sizeof(uint32_t)) + GetFTLVLength(kHeaderCompartibleVersion, sizeof(uint32_t)) + GetFTLVLength(kHeaderEncrypt, sizeof(uint32_t));
}
    
bool SectionHeader::encoding() {
    bool finished = false;
    do {
        uint32_t size = this->size();
        if (!size) {
            break;
        }
        if (!Section::encoding((uint16_t)ExecSection::EXEC_SECTION_HEADER, size)) {
            break;
        }
        uint32_t magic_code = EXEC_BINARY_MAGIC_NUMBER;
        if (!Section::encoding(kHeaderMagicCode, sizeof(uint32_t), &magic_code)) {
            break;
        }
        uint32_t version = EXEC_BINARY_CURRENT_VERSION;
        if (!Section::encoding(kHeaderVersion, sizeof(uint32_t), &version)) {
            break;
        }
        uint32_t compartibleVersion = EXEC_BINARY_COMPATIBLE_VERSION;
        if (!Section::encoding(kHeaderCompartibleVersion, sizeof(uint32_t), &compartibleVersion)) {
            break;
        }
        uint32_t encrypt = EXEC_BINARY_ENCRYPT;
        if (!Section::encoding(kHeaderEncrypt, sizeof(uint32_t), &encrypt)) {
            break;
        }
        finished = true;
        
    } while (0);
    
    return finished;
}
    
bool SectionHeader::decoding() {
    bool finished = false;
    do {
        fStream *stream = Section::stream();
        if (!stream) {
            break;
        }
        if (stream->Tell() < 0) {
            break;
        }
        int64_t seek_end = stream->Tell() + length();
        do {
            uint16_t target = 0;
            uint32_t value_length = stream->ReadTarget(&target, NULL,  NULL);
            if (!value_length) {
                throw EncoderError("decoding header target error");
                return false;
            }
            switch (target) {
                case kHeaderMagicCode:
                {
                    uint32_t magic_code = 0;
                    if (stream->Read(&magic_code, 1, value_length) != value_length || magic_code != EXEC_BINARY_MAGIC_NUMBER) {
                        throw EncoderError("decoding header target magic code error");
                    }
                    break;
                }
                case kHeaderVersion:
                {
                    if (stream->Read(&version_, 1, value_length) != value_length) {
                        throw EncoderError("decoding header target magic code error");
                    }
                    break;
                }
                case kHeaderCompartibleVersion:
                {
                    if (stream->Read(&compartible_version_, 1, value_length) != value_length) {
                        throw EncoderError("decoding header target compartible version error");
                    }
                    if (version_ < compartible_version_) {
                        throw EncoderError("decoding header target current version didn't compartible");
                    }
                    break;
                }
                case kHeaderEncrypt:
                {
                    if (stream->Read(&encrypt_, 1, value_length) != value_length) {
                        throw EncoderError("decoding header target encrypt error");
                    }
                    break;
                }
                default: {   // skip this node
                    if (stream->Seek(value_length, SEEK_CUR) < 0) {
                        return false;
                    }
                    break;
                }
            }
            if (stream->Tell() < 0) {
                return false;
            }
            
        } while (stream->Tell() < seek_end);
        
        finished = true;
        
    } while (0);
    
    return finished;
}

uint32_t SectionString::size() {
    uint32_t size = 0;
    StringTable *string_table = encoder()->exec_state()->string_table();
    const std::vector<std::pair<std::string, std::unique_ptr<String>>> &store = string_table->store();
    if (store.size() > 0) {
        size += GetFTLVLength(kValueStringCount, sizeof(uint32_t));
        for (auto &iter : store) {
            size += GetFTLVLength(kValueStringContent, (uint32_t)iter.first.length());
        }
    }
    return size;
}
    
bool SectionString::encoding() {
    bool finished = false;
    do {
        uint32_t size = this->size();
        if (!size) {
            finished = true;
            break;
        }
        if (!Section::encoding((uint16_t)ExecSection::EXEC_SECTION_STRING, size)) {
            break;
        }
        StringTable *string_table = encoder()->exec_state()->string_table();
        const std::vector<std::pair<std::string, std::unique_ptr<String>>> &store = string_table->store();
        uint32_t count = (uint32_t)store.size();
        if (!Section::encoding(kValueStringCount, sizeof(uint32_t), &count)) {
            break;
        }
        bool error = false;
        for (auto &iter : store) {
            const char *pcstr = iter.first.c_str();
            uint32_t length = (uint32_t)iter.first.length();
            if (!Section::encoding(kValueStringContent, length, (void *)pcstr)) {
                error = true;
                break;
            }
        }
        finished = !error;
        
    } while (0);
    
    return finished;
}
    
bool SectionString::decoding() {
    bool finished = false;
    do {
        fStream *stream = Section::stream();
        if (!stream) {
            break;
        }
        if (stream->Tell() < 0) {
            break;
        }
        uint16_t target = 0;
        uint32_t string_count = 0;
        uint32_t size = sizeof(uint32_t);
        uint32_t readbytes = stream->ReadTarget(&target, (uint8_t *)&string_count, &size);
        if (readbytes != size || target != kValueStringCount) {
            throw EncoderError("decoding string count error");
            break;
        }
        if (!string_count) {
            break;
        }
        StringTable *string_table = decoder()->exec_state()->string_table();
        int index = 0;
        for (index = 0; index < string_count; index++) {
            uint16_t vindex = 0;
            uint32_t varlen = 0;
            if ((varlen = stream->ReadTarget(&vindex, NULL, NULL)) == 0) {
                break;
            }
            if (vindex != kValueStringContent) {
                break;
            }
            char *pstr = (char *)malloc(varlen + 1);
            if (!pstr) {
                break;
            }
            memset(pstr, 0, varlen + 1);
            if (stream->Read(pstr, 1, varlen) != varlen) {
                throw EncoderError("decoding string content error");
            }
            string_table->StringFromUTF8(pstr);
        }
        if (index != string_count) {
            break;
        }
        finished = true;
        
    } while (0);
    
    return finished;
}
    
uint32_t SectionFunction::size() {
    uint32_t size = 0;
    std::vector<FuncState *> func_states;
    FuncState *func_state = encoder()->exec_state()->func_state();
    if (func_state) {
        func_states.push_back(func_state);
    }
    std::vector<FuncState *> childrens = func_state->all_childrens();
    for (auto &func : childrens) {
        func_states.push_back(func);
    }
    if (func_states.size() > 0) {
        size += GetFTLVLength(kValueFunctionCount, sizeof(uint32_t));
        for (auto &func_state : func_states) {
            size += GetFTLVLength(kValueFunctionSuper, sizeof(int32_t));
            size += GetFTLVLength(kValueFunctionClass, sizeof(uint8_t));
            if (func_state->argc() > 0) {
                size += GetFTLVLength(kValueFunctionArgs, sizeof(int32_t) * func_state->argc());
            }
            if (func_state->in_closure().size() > 0) {
                size += GetFTLVLength(kValueFunctionInClosure, sizeof(int32_t) * (uint32_t)func_state->in_closure().size());
            }
            if (func_state->out_closure().size() > 0) {
                size += GetFTLVLength(kValueFunctionOutClosure, sizeof(int32_t) * (uint32_t)func_state->out_closure().size());
            }
            if (func_state->instructions().size() > 0) {
                size += GetFTLVLength(kValueFunctionInstructions, sizeof(int32_t) * (uint32_t)func_state->instructions().size());
            }
            if (func_state->constants().size() > 0) {
                size += GetFTLVLength(kValueFunctionConstantCount, sizeof(int32_t));
                uint32_t constant_length = 0;
                for (int i = 0; i < func_state->constants().size(); i++) {
                    constant_length += GetValueLength(&func_state->constants()[i]);
                }
                size += GetFTLVLength(kValueFunctionConstantPayload, constant_length);
            }
        }
    }
    return size;
}
    
bool SectionFunction::encoding() {
    bool finished = false;
    do {
        uint32_t size = this->size();
        if (!size) {
            finished = true;
            break;
        }
        if (!Section::encoding((uint16_t)ExecSection::EXEC_SECTION_FUNCTION, size)) {
            break;
        }
        std::vector<FuncState *> func_states;
        FuncState *func_state = encoder()->exec_state()->func_state();
        if (func_state) {
            func_states.push_back(func_state);
        }
        std::vector<FuncState *> childrens = func_state->all_childrens();
        for (auto &func : childrens) {
            func_states.push_back(func);
        }
        uint32_t func_count = (uint32_t)func_states.size();
        if (!Section::encoding(kValueFunctionCount, sizeof(uint32_t), &func_count)) {
            break;
        }
        for (int i = 0; i < func_count; i++) {
            func_state = func_states[i];
            auto iter = std::find(func_states.begin(), func_states.end(), func_state->super_func());
            int32_t super_index = -1;
            if (iter != func_states.end()) {
                super_index = static_cast<int>(iter - func_states.begin());
            }
            else if (func_state->super_func()) {
                throw EncoderError("can't find super function error");
            }
            if (!Section::encoding(kValueFunctionSuper, sizeof(int32_t), &super_index)) {
                break;
            }
            int8_t is_class_func = func_state->is_class_func();
            if (!Section::encoding(kValueFunctionClass, sizeof(int8_t), &is_class_func)) {
                break;
            }
            if (func_state->argc() > 0) {
                int32_t args[SECTION_FUNCTION_MAXARGS] = { 0 };
                assert(func_state->argc() == func_state->args().size());
                for (int i = 0; i < func_state->argc(); i++) {
                    args[i] = (int32_t)func_state->args()[i];
                }
                if (!Section::encoding(kValueFunctionArgs, sizeof(int32_t) * func_state->argc(), args)) {
                    break;
                }
            }
            if (func_state->in_closure().size() > 0) {
                size_t size = sizeof(int32_t) * func_state->in_closure().size();
                int32_t *buffer = (int32_t *)malloc(size);
                if (!buffer) {
                    throw EncoderError("low memory error");
                    break;
                }
                memset(buffer, 0, size);
                for (int i = 0; i < func_state->in_closure().size(); i++) {
                    buffer[i] = func_state->in_closure()[i]->ref_id();
                }
                if (!Section::encoding(kValueFunctionInClosure, (uint32_t)size, buffer)) {
                    break;
                }
                free(buffer);
            }
            if (func_state->out_closure().size() > 0) {
                size_t size = sizeof(int32_t) * func_state->out_closure().size();
                int32_t *buffer = (int32_t *)malloc(size);
                if (!buffer) {
                    throw EncoderError("low memory error");
                    break;
                }
                memset(buffer, 0, size);
                for (int i = 0; i < func_state->out_closure().size(); i++) {
                    buffer[i] = func_state->out_closure()[i]->ref_id();
                }
                if (!Section::encoding(kValueFunctionOutClosure, (uint32_t)size, buffer)) {
                    break;
                }
                free(buffer);
            }
            if (func_state->instructions().size() > 0) {
                size_t size = sizeof(uint32_t) * func_state->instructions().size();
                uint32_t *buffer = (uint32_t *)malloc(size);
                if (!buffer) {
                    throw EncoderError("low memory error");
                    break;
                }
                memset(buffer, 0, size);
                for (int i = 0; i < func_state->instructions().size(); i++) {
                    buffer[i] = static_cast<uint32_t>(func_state->instructions()[i]);
                }
                if (!Section::encoding(kValueFunctionInstructions, (uint32_t)size, buffer)) {
                    break;
                }
                free(buffer);
            }
            if (func_state->constants().size() > 0) {
                uint32_t constants_size = (uint32_t)func_state->constants().size();
                if (!Section::encoding(kValueFunctionConstantCount, sizeof(uint32_t), &constants_size)) {
                    break;
                }
                uint32_t constants_value_length = 0;
                for (int i = 0; i < func_state->constants().size(); i++) {
                    constants_value_length += GetValueLength(&func_state->constants()[i]);
                }
                uint8_t *buffer = (uint8_t *)malloc(constants_value_length);
                if (!buffer) {
                    throw EncoderError("low memory error");
                    break;
                }
                uint32_t remain_length = constants_value_length;
                for (int i = 0; i < func_state->constants().size(); i++) {
                    uint32_t length = GetValueLength(&func_state->constants()[i]);
                    uint32_t bytes_write = Section::encodingValueToBuffer(buffer, remain_length, &func_state->constants()[i]);
                    if (bytes_write != length) {
                        throw EncoderError("encoding constants value error");
                        break;
                    }
                    remain_length -= bytes_write;
                    buffer += bytes_write;
                }
                if (!Section::encoding(kValueFunctionConstantPayload, sizeof(uint32_t), &constants_value_length)) {
                    break;
                }
            }
        }
        finished = true;
        
    } while (0);
    
    return finished;
}

bool SectionFunction::decoding() {
    bool finished = false;
    do {
        fStream *stream = Section::stream();
        if (!stream) {
            break;
        }
        if (stream->Tell() < 0) {
            break;
        }
        int64_t seek_end = stream->Tell() + length();
        uint16_t target = 0;
        uint32_t func_count = 0;
        uint32_t size = sizeof(uint32_t);
        uint32_t readbytes = stream->ReadTarget(&target, (uint8_t *)&func_count, &size);
        if (!readbytes || target != kValueFunctionCount) {
            break;
        }
        std::vector<FuncState *>func_states;
        for (int i = 0; i < func_count; i++) {
            FuncState *func_state = new FuncState;
            do {
                if ((readbytes = stream->ReadTarget(&target, NULL, NULL)) == 0) {
                    throw EncoderError("decoding read function target error");
                    break;
                }
                switch (target) {
                    case kValueFunctionSuper:
                    {
                        int super_index = -1;
                        if (stream->Read(&super_index, 1, readbytes) != readbytes) {
                            throw EncoderError("decoding string content error");
                        }
                        func_state->set_super_index(super_index);
                        break;
                    }
                    default:
                        break;
                }
                if (target == kValueFunctionConstantPayload) {
                    break;
                }
                
            } while (stream->Tell() < seek_end);
            
            func_states.push_back(func_state);
        }
        
        finished = true;
        
    } while (0);
    
    return finished;
}

}  // namespace data_render
}  // namespace core
}  // namespace weex
