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

#include <thread>
#include <mutex>
#include "core/data_render/exec_state_binary.h"
#include "core/data_render/common_error.h"
#include "core/data_render/vm.h"
#include "core/data_render/exec_state.h"
#include "third_party/json11/json11.hpp"
#include "core/data_render/vnode/vnode_exec_env.h"
#include "base/LogDefines.h"

namespace weex {
namespace core {
namespace data_render {
    
#define EXECSTATE_ENCODING_COMPARE           0
    
std::once_flag device_little_endian;

static bool gs_device_is_little_endian = false;
static uint8_t gs_op_code_bits = 0;
    
void determine_little_endian()
{
    short i = 0x1;
    gs_device_is_little_endian = !((i >> 8) == 0x1);
    gs_op_code_bits = 1;
    uint32_t op_code_value = OP_INVALID;
    while (op_code_value / 2 > 0) {
        op_code_value = op_code_value / 2;
        gs_op_code_bits++;
    }
}
   
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
            SectionData data(this);
            if (!data.encoding()) {
                err = "data section encoding error";
                break;
            }
            SectionScript script(this);
            if (!script.encoding()) {
                err = "script section encoding error";
                break;
            }
            SectionFunction function(this, gs_op_code_bits);
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
                    case ExecSection::EXEC_SECTION_DATA:
                    {
                        SectionData data(this, section_length);
                        if (!data.decoding()) {
                            throw EncoderError("data section decoding error");
                        }
                        break;
                    }
                    case ExecSection::EXEC_SECTION_SCRIPT:
                    {
                        SectionScript script(this, section_length);
                        if (!script.decoding()) {
                            throw EncoderError("script section decoding error");
                        }
                        break;
                    }
                    case ExecSection::EXEC_SECTION_FUNCTION:
                    {
                        SectionFunction function(this, gs_op_code_bits, section_length);
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
    
#if EXECSTATE_ENCODING_COMPARE
static ExecState *gs_encoder = nullptr;
#endif
  
bool WXExecEncoder(std::string &input, std::string &path, std::string &error) {
    bool finished = false;
    VM *vm = nullptr;
    ExecState *exec_state = nullptr;
    FILE *fout = nullptr;
    do {
        std::call_once(device_little_endian, determine_little_endian);
        if (!gs_device_is_little_endian) {
            error = "device must be little endian error";
            break;
        }
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
        VNodeExecEnv::ImportExecEnv(exec_state);
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
        if (exec_state->global()->IndexOf("__weex_data__") < 0) {
            exec_state->global()->Set("__weex_data__", Value());
        }
        if (exec_state->global()->IndexOf("_init_data") < 0) {
            exec_state->global()->Set("_init_data", Value());
        }
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
#if EXECSTATE_ENCODING_COMPARE
        gs_encoder = exec_state;
#endif
        finished = true;
        
    } while (0);
    
#if ! EXECSTATE_ENCODING_COMPARE
    if (exec_state) {
        delete exec_state;
    }
#endif
    if (vm) {
        delete vm;
    }
    LOGD("encoding file size:%.02fK\n", (double)ftell(fout) / 1024);
    if (fout) {
        fclose(fout);
    }    
    return finished;
}
    
#if EXECSTATE_ENCODING_COMPARE
static void compare(ExecState *encoder, ExecState *decoder);
#endif
    
bool WXExecDecoder(ExecState *exec_state, uint8_t *buffer, size_t size, std::string &error) {
    bool finished = false;
    do {
        std::call_once(device_little_endian, determine_little_endian);
        if (!gs_device_is_little_endian) {
            error = "device must be little endian error";
            break;
        }
        std::string err;
        ExecStateDecoder decoder(exec_state, buffer, size);
        if (!decoder.decoding(err)) {
            error = err;
            break;
        }
#if EXECSTATE_ENCODING_COMPARE
        compare(gs_encoder, exec_state);
#endif
        finished = true;
        
    } while (0);
    
    return finished;
}
    
#if EXECSTATE_ENCODING_COMPARE
static void compare_value(Value *encoder_value, Value *decoder_value);
static void compare_variables(Variables *encoder_var, Variables *decoder_var, bool map_diff = true);
static void compare_value_ref(ValueRef *encoder_ref, ValueRef *decoder_ref);
static void compare_func_state(FuncState *encoder_func_state, FuncState *decoder_func_state, bool is_ref = true, bool is_children = true);
static void compare_class_desc(ClassDescriptor *encoder_desc, ClassDescriptor *decoder_desc) {
    if (encoder_desc->p_super_) {
        assert(decoder_desc->p_super_);
        compare_class_desc(encoder_desc->p_super_, decoder_desc->p_super_);
    }
    compare_variables(encoder_desc->static_funcs_.get(), decoder_desc->static_funcs_.get());
    compare_variables(encoder_desc->funcs_.get(), decoder_desc->funcs_.get());
}
    
static void compare_func_state(FuncState *encoder_func_state, FuncState *decoder_func_state, bool is_ref, bool is_children) {
    assert(encoder_func_state->is_class_func() == decoder_func_state->is_class_func());
    assert(encoder_func_state->argc() == decoder_func_state->argc());
    for (int i = 0; i < encoder_func_state->args().size(); i++) {
        assert(encoder_func_state->args()[i] == decoder_func_state->args()[i]);
    }
    assert(encoder_func_state->in_closure().size() == decoder_func_state->in_closure().size());
    if (is_ref) {
        for (int i = 0; i < encoder_func_state->in_closure().size(); i++) {
            compare_value_ref(encoder_func_state->in_closure()[i], decoder_func_state->in_closure()[i]);
        }
    }
    assert(encoder_func_state->out_closure().size() == decoder_func_state->out_closure().size());
    if (is_ref) {
        for (int i = 0; i < encoder_func_state->out_closure().size(); i++) {
            ValueRef *ref_encoder = encoder_func_state->out_closure()[i];
            ValueRef *ref_decoder = decoder_func_state->out_closure()[i];
            compare_value_ref(ref_encoder, ref_decoder);
        }
    }
    assert(encoder_func_state->instructions().size() == decoder_func_state->instructions().size());
    for (int i = 0; i < encoder_func_state->instructions().size(); i++) {
        assert(encoder_func_state->instructions()[i] == decoder_func_state->instructions()[i]);
    }
    assert(encoder_func_state->constants().size() == decoder_func_state->constants().size());
    for (int i = 0; i < encoder_func_state->constants().size(); i++) {
        Value encoder_value = encoder_func_state->constants()[i];
        Value decoder_value = decoder_func_state->constants()[i];
        compare_value(&encoder_value, &decoder_value);
    }
    if (is_children) {
        auto encoder_all_childrens = encoder_func_state->all_childrens();
        auto decoder_all_childrens = decoder_func_state->all_childrens();
        assert(encoder_all_childrens.size() == decoder_all_childrens.size());
        for (int i = 0; i < encoder_all_childrens.size(); i++) {
            compare_func_state(encoder_all_childrens[i], decoder_all_childrens[i], true, false);
        }
    }
}
    
static void compare_value(Value *encoder_value, Value *decoder_value) {
    assert(encoder_value->type == decoder_value->type);
    switch (encoder_value->type) {
        case Value::STRING:
        {
            assert(std::string(encoder_value->str->c_str()) == std::string(decoder_value->str->c_str()));
            break;
        }
        case Value::FUNC:
        {
            compare_func_state(encoder_value->f, decoder_value->f);
            break;
        }
        case Value::CLASS_DESC:
        {
            compare_class_desc(ValueTo<ClassDescriptor>(encoder_value), ValueTo<ClassDescriptor>(decoder_value));
            break;
        }
        default:
            assert(*encoder_value == *decoder_value);
            break;
    }
}
    
static void compare_variables(Variables *encoder_var, Variables *decoder_var, bool map_diff) {
    assert(encoder_var->size() == encoder_var->size());
    for (int i = 0; i < encoder_var->size(); i++) {
        Value *encoder_value = encoder_var->Find(i);
        Value *decoder_value = encoder_var->Find(i);
        compare_value(encoder_value, decoder_value);
    }
    if (map_diff) {
        std::map<std::string, int> &encoder_var_map = (std::map<std::string, int> &)encoder_var->map();
        std::map<std::string, int> &decoder_var_map = (std::map<std::string, int> &)decoder_var->map();
        assert(encoder_var_map.size() == decoder_var_map.size());
        std::map<std::string, int>::iterator encoder_iter = encoder_var_map.begin();
        std::map<std::string, int>::iterator decoder_iter = decoder_var_map.begin();
        for (int i = 0; i < encoder_var_map.size(); i++) {
            LOGD("compare_variables key:%s : %s\n", encoder_iter->first.c_str(), decoder_iter->first.c_str());
            assert(encoder_iter->first == decoder_iter->first);
            assert(encoder_iter->second == decoder_iter->second);
            encoder_iter++;
            decoder_iter++;
        }
    }
}
    
static void compare_value_ref(ValueRef *encoder_ref, ValueRef *decoder_ref) {
    int encoder_ref_id = encoder_ref->ref_id();
    int decoder_ref_id = decoder_ref->ref_id();
    assert(encoder_ref_id == decoder_ref_id);
    assert(encoder_ref->register_id() == decoder_ref->register_id());
    compare_func_state(encoder_ref->func_state(), decoder_ref->func_state(), false, false);
}
    
static void compare(ExecState *encoder, ExecState *decoder) {
    do {
        StringTable *encoder_string_table = encoder->string_table();
        StringTable *decoder_string_table = encoder->string_table();
        assert(encoder_string_table->store().size() == decoder_string_table->store().size());
        for (int i = 0; i < encoder_string_table->store().size(); i++) {
            assert(encoder_string_table->store()[i].first == decoder_string_table->store()[i].first);
            std::string encoder_string = encoder_string_table->store()[i].second->c_str();
            std::string decoder_string = decoder_string_table->store()[i].second->c_str();
            assert(encoder_string == decoder_string);
        }
        FuncState *encoder_func_state = encoder->func_state();
        FuncState *decoder_func_state = decoder->func_state();
        compare_func_state(encoder_func_state, decoder_func_state);        
        std::vector<ClassDescriptor *> encoder_descs = encoder->class_factory()->descs();
        std::vector<ClassDescriptor *> decoder_descs = decoder->class_factory()->descs();
        assert(encoder_descs.size() == decoder_descs.size());
        for (int i = 0; i < encoder_descs.size(); i++) {
            compare_class_desc(encoder_descs[i], decoder_descs[i]);
        }
        compare_variables(encoder->global(), decoder->global(), false);
        std::unordered_map<std::string, long> &encoder_global_variables = encoder->global_variables();
        std::unordered_map<std::string, long> &decoder_global_variables = decoder->global_variables();
        assert(encoder_global_variables.size() == decoder_global_variables.size());
        std::unordered_map<std::string, long>::iterator encoder_iter = encoder_global_variables.begin();
        std::unordered_map<std::string, long>::iterator decoder_iter = encoder_global_variables.begin();
        for (int i = 0; i < encoder_global_variables.size(); i++) {
            LOGD("compare map key:%s : %s\n", encoder_iter->first.c_str(), decoder_iter->first.c_str());
            assert(encoder_iter->first == decoder_iter->first);
            assert(encoder_iter->second == decoder_iter->second);
            encoder_iter++;
            decoder_iter++;
        }
        std::vector<ValueRef *> &encoder_refs = encoder->refs();
        std::vector<ValueRef *> &decoder_refs = decoder->refs();
        assert(encoder_refs.size() == decoder_refs.size());
        for (int i = 0; i < encoder_refs.size(); i++) {
            compare_value_ref(encoder_refs[i], decoder_refs[i]);
        }
        
    } while (0);
}
    
#endif
    
}  // namespace data_render
}  // namespace core
}  // namespace weex
