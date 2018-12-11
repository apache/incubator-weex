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

#include <math.h>
#include "core/data_render/object.h"
#include "core/data_render/exec_state_section.h"
#include "core/data_render/exec_state_binary.h"
#include "core/data_render/common_error.h"
#include "core/data_render/exec_state.h"
#include "core/data_render/string_table.h"
#include "core/data_render/table.h"
#include "base/LogDefines.h"

namespace weex {
namespace core {
namespace data_render {
    
#define SECTION_FUNCTION_MAXARGS            256
#define SECTION_VALUE_FINISHED              0xff
#define SECTION_SET_LENGTH_LEN_FLAG(flag, LenLen) if (LenLen == 4) LenLen = 3; flag |= (LenLen << SECTION_LEN_OFF);
#define SECTION_REGISTER_MAX                255
    
class CBitsReader {
public:
    CBitsReader(std::uint8_t *data, std::uint32_t size)
    : data_(data),
    size_(size),
    byteOffset_(0),
    currentValue_(0),
    currentBits_(0) {}
    std::uint32_t remainingBytes() {
        return size_ - byteOffset_;
    }
    std::uint32_t remainingBits() {
        return 8 * (size_ - byteOffset_) + currentBits_;
    }
    std::uint32_t readStream(std::uint8_t *buffer, std::uint32_t size) {
        std::uint32_t bytes_reading = 0;
        do {
            if (size > size_ - byteOffset_) {
                break;
            }
            memcpy(buffer, data_ + byteOffset_, size);
            byteOffset_ += size;
            bytes_reading += size;
            
        } while (0);
        
        return bytes_reading;
    }
    std::uint32_t nextBits(std::uint32_t numBits) {
        assert(numBits <= remainingBits() && "attempt to read more bits than available");
        // Collect bytes until we have enough bits:
        while (currentBits_ < numBits) {
            currentValue_ = (currentValue_ << 8) + (std::uint32_t)(data_[byteOffset_]);
            currentBits_ = currentBits_ + 8;
            byteOffset_ = byteOffset_ + 1;
        }
        // Extract result:
        std::uint32_t remaining = currentBits_ - numBits;
        std::uint32_t result = currentValue_ >> remaining;
        // Update remaining bits:
        currentValue_ = currentValue_ & ((1 << remaining) - 1);
        currentBits_ = remaining;
        return result;
    }
private:
    std::uint8_t *data_;
    std::uint32_t byteOffset_;
    std::uint32_t currentValue_;
    std::uint32_t currentBits_;
    std::uint32_t size_;
};
    
class CBitsWriter {
public:
    CBitsWriter(std::uint8_t *data, std::uint32_t size) : data_(data), size_(size) {}
    ~CBitsWriter() {}
    virtual void WriteBits(std::uint32_t data, std::uint32_t numBits);
    void WriteByte(std::uint8_t uByte);
    virtual void Finish();
    std::uint8_t *GetBuffer();
    inline std::uint32_t Seek() { return seek_; };
    inline std::uint32_t Size() { return size_; };
    inline std::uint32_t TotalBits() { return totalBits_; }
    void Reset();
    bool WriteStream(const std::uint8_t *data, std::uint32_t size);
private:
    bool bFinish_{false};
    std::uint32_t size_;
    std::uint8_t *data_;
    std::uint32_t totalBits_{0};
    std::uint32_t seek_{0};
    std::uint32_t bytesTemp_{0};
    std::uint32_t currentBits_{0};
};
    
void InvBits4(std::uint8_t& uChar)
{
    static std::uint8_t table[16] = {0x00, 0x08, 0x04, 0x0C, 0x02, 0x0A, 0x06, 0x0E, 0x01, 0x09, 0x05, 0x0D, 0x03, 0x0B, 0x07, 0x0F};
    uChar = table[uChar];
}

void InvBits(std::uint8_t& uChar)
{
    std::uint8_t uH4 = uChar >> 4;
    std::uint8_t uL4 = uChar & 0xFF;
    InvBits4(uH4);
    InvBits4(uL4);
    uChar = uL4 << 4 | uH4;
}

void CBitsWriter::WriteBits(std::uint32_t data, std::uint32_t numBits)
{
    do {
        if (!numBits) {
            break;
        }
        assert(numBits < 24);
        const std::uint32_t maxBitCount = sizeof(bytesTemp_) * 8;
        bytesTemp_ |= data << ((maxBitCount - currentBits_) - numBits) ;
        currentBits_ += numBits;
        while (currentBits_ >= 8)
        {
            std::uint8_t byte = (std::uint8_t)(bytesTemp_ >> (maxBitCount - 8));
            WriteByte(byte);
            bytesTemp_ <<= 8;
            currentBits_ -= 8;
        }
        totalBits_ += numBits;
        
    } while (0);
}

void CBitsWriter::WriteByte(std::uint8_t uByte)
{
    if (seek_ < size_)
    {
        data_[seek_++] = uByte;
    }
    else {
        throw EncoderError("encoding bits not bits buffer error");
    }
}

void CBitsWriter::Finish()
{
    if(!bFinish_)
    {
        // fill bits for byte
        if(currentBits_ > 0 && currentBits_ < 8)
        {
            unsigned int kMaxBits = sizeof(bytesTemp_) * 8 ;
            unsigned char h8 = (unsigned char)(bytesTemp_ >> (kMaxBits - 8));//24
            totalBits_ += (8 - currentBits_);
            WriteByte(h8);
        }
        currentBits_ = 0;
        bytesTemp_ = 0;
        bFinish_ = true;
        seek_ = 0;
    }
}

std::uint8_t *CBitsWriter::GetBuffer()
{
    return data_;
}
    
void CBitsWriter::Reset()
{
    seek_ = 0;
    bFinish_ = false;
    totalBits_ = 0;
}

bool CBitsWriter::WriteStream(const std::uint8_t *data, std::uint32_t size)
{
    for (unsigned int i = 0; i < size; i++)
        WriteByte(data[i]);
    totalBits_ += size * 8;
    return true;
}
    
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
    
uint32_t Section::decodingValueFromBuffer(uint8_t *buffer, uint32_t remain, Value *pval) {
    uint32_t total_bytes_read = 0;
    do {
        uint32_t bytes_read = 0;
        uint32_t bytes_decoding = sizeof(uint8_t);
        int8_t type = 0;
        if (!(bytes_read = decodingFromBuffer(buffer, remain, (uint8_t *)&type, &bytes_decoding))) {
            break;
        }
        total_bytes_read += bytes_read;
        buffer += bytes_read;
        remain -= bytes_read;
        if (type < Value::Type::NIL && type > Value::Type::FUNC_INST) {
            break;
        }
        switch (type) {
            case Value::Type::CLASS_DESC:
            {
                int32_t payload = -1;
                uint16_t u16_payload = 0;
                bytes_decoding = sizeof(uint16_t);
                if (!(bytes_read = decodingFromBuffer(buffer, remain, (uint8_t *)&u16_payload, &bytes_decoding))) {
                    break;
                }
                payload = u16_payload;
                std::vector<ClassDescriptor *> descs = decoder()->exec_state()->class_factory()->descs();
                if (payload < 0 || payload >= descs.size()) {
                    throw DecoderError("decoding value payload class desc error");
                    break;
                }
                SetCDValue(pval, reinterpret_cast<GCObject *>(descs[payload]));
                LOGD("decoding class desc value:%i\n", payload);
                total_bytes_read += bytes_read;
                break;
            }
            case Value::Type::FUNC:
            {
                int32_t payload = -1;
                uint16_t u16_payload = 0;
                bytes_decoding = sizeof(uint16_t);
                if (!(bytes_read = decodingFromBuffer(buffer, remain, (uint8_t *)&u16_payload, &bytes_decoding))) {
                    break;
                }
                payload = u16_payload;
                if (payload < 0) {
                    throw DecoderError("decoding value payload function error");
                    break;
                }
                FuncState *func_state = decoder()->exec_state()->func_state();
                if (payload == 0) {
                    pval->f = func_state;
                }
                else if (payload - 1 >= func_state->all_childrens().size()) {
                    throw DecoderError("decoding value payload function error");
                    break;
                }
                else {
                    pval->f = func_state->all_childrens()[payload - 1];
                }
                pval->type = Value::Type::FUNC;
                LOGD("decoding function value:%i\n", payload);
                total_bytes_read += bytes_read;
                break;
            }
            case Value::Type::STRING:
            {
                int32_t payload = -1;
                uint16_t u16_payload = 0;
                bytes_decoding = sizeof(uint16_t);
                if (!(bytes_read = decodingFromBuffer(buffer, remain, (uint8_t *)&u16_payload, &bytes_decoding))) {
                    break;
                }
                payload = u16_payload;
                if (payload < 0) {
                    throw DecoderError("decoding value payload string error");
                    break;
                }
                const std::vector<std::pair<std::string, std::unique_ptr<String>>>& store = decoder()->exec_state()->string_table()->store();
                if (payload >= store.size()) {
                    throw DecoderError("decoding value payload string error");
                    break;
                }
                pval->str = store[payload].second.get();
                LOGD("decoding string value:[%i] %s\n", payload, pval->str->c_str());
                pval->type = Value::Type::STRING;
                total_bytes_read += bytes_read;
                break;
            }
            case Value::Type::INT:
            {
                int32_t i32_payload = 0;
                bytes_decoding = sizeof(int32_t);
                if (!(bytes_read = decodingFromBuffer(buffer, remain, (uint8_t *)&i32_payload, &bytes_decoding))) {
                    break;
                }
                pval->i = i32_payload;
                pval->type = Value::Type::INT;
                LOGD("decoding int value:%i\n", (int32_t)pval->i);
                total_bytes_read += bytes_read;
                break;
            }
            case Value::Type::NUMBER:
            {
                bytes_decoding = sizeof(double);
                if (!(bytes_read = decodingFromBuffer(buffer, remain, (uint8_t *)&pval->n, &bytes_decoding))) {
                    break;
                }
                pval->type = Value::Type::NUMBER;
                LOGD("decoding number value:%f\n", pval->n);
                total_bytes_read += bytes_read;
                break;
            }
            case Value::Type::BOOL:
            {
                bytes_decoding = sizeof(uint8_t);
                if (!(bytes_read = decodingFromBuffer(buffer, remain, (uint8_t *)&pval->b, &bytes_decoding))) {
                    break;
                }
                pval->type = Value::Type::BOOL;
                LOGD("decoding bool value:%i\n", pval->b);
                total_bytes_read += bytes_read;
                break;
            }
            case Value::Type::ARRAY:
            {
                uint8_t item_size = 0;
                bytes_decoding = sizeof(uint8_t);
                if (!(bytes_read = decodingFromBuffer(buffer, remain, (uint8_t *)&item_size, &bytes_decoding))) {
                    break;
                }
                total_bytes_read += bytes_read;
                buffer += bytes_read;
                remain -= bytes_read;
                ClassFactory *class_factory = decoder()->exec_state()->class_factory();
                Value array_value = class_factory->CreateArray();
                if (!item_size) {
                    break;
                }
                Array *array = ValueTo<Array>(&array_value);
                for (int i = 0; i < item_size; i++) {
                    Value item;
                    if (!(bytes_read = decodingValueFromBuffer(buffer, remain, &item))) {
                        throw DecoderError("decoding array item payload error");
                        break;
                    }
                    total_bytes_read += bytes_read;
                    buffer += bytes_read;
                    remain -= bytes_read;
                    array->items.push_back(item);
                }
                *pval = array_value;
                LOGD("decoding array:%s\n", ArrayToString(array).c_str());
                break;
            }
            case Value::Type::TABLE:
            {
                uint8_t keys_size = 0;
                bytes_decoding = sizeof(uint8_t);
                if (!(bytes_read = decodingFromBuffer(buffer, remain, (uint8_t *)&keys_size, &bytes_decoding))) {
                    break;
                }
                total_bytes_read += bytes_read;
                buffer += bytes_read;
                remain -= bytes_read;
                ClassFactory *class_factory = decoder()->exec_state()->class_factory();
                Value table_value = class_factory->CreateTable();
                if (!keys_size) {
                    break;
                }
                Table *table = ValueTo<Table>(&table_value);
                for (int i = 0; i < keys_size; i++) {
                    Value key;
                    if (!(bytes_read = decodingValueFromBuffer(buffer, remain, &key))) {
                        throw DecoderError("decoding table key payload error");
                        break;
                    }
                    if (!IsString(&key)) {
                        throw DecoderError("decoding table key payload error");
                        break;
                    }
                    total_bytes_read += bytes_read;
                    buffer += bytes_read;
                    remain -= bytes_read;
                    Value val;
                    if (!(bytes_read = decodingValueFromBuffer(buffer, remain, &val))) {
                        throw DecoderError("decoding table value payload error");
                        break;
                    }
                    total_bytes_read += bytes_read;
                    buffer += bytes_read;
                    remain -= bytes_read;
                    table->map.insert(std::make_pair(CStringValue(&key), val));
                }
                *pval = table_value;
                LOGD("decoding table:%s\n", TableToString(table).c_str());
                break;
            }
            default:
                throw EncoderError("decoding value type unkown error");
                break;
        }
        
    } while (0);
    
    return total_bytes_read;
}
    
uint32_t Section::encodingValueToBuffer(uint8_t *buffer, uint32_t buffer_len, Value *pval) {
    uint32_t total_bytes_write = 0;
    do {
        uint32_t bytes_write = 0;
        uint8_t type = ttype(pval);
        if (!(bytes_write = encodingToBuffer(buffer, buffer_len, sizeof(uint8_t), &type))) {
            break;
        }
        buffer += bytes_write;
        buffer_len -= bytes_write;
        total_bytes_write += bytes_write;
        switch (type) {
            case Value::Type::CLASS_DESC:
            {
                int32_t payload = -1;
                std::vector<ClassDescriptor *> descs = encoder_->exec_state()->class_factory()->descs();
                int index = 0;
                for (auto desc : descs) {
                    if (pval->gc == reinterpret_cast<GCObject *>(desc)) {
                        payload = index;
                        break;
                    }
                    index++;
                }
                if (payload < 0) {
                    throw EncoderError("encoder value class desc can't find error");
                    break;
                }
                uint16_t u16_payload = static_cast<uint16_t>(payload);
                if (!(bytes_write = encodingToBuffer(buffer, buffer_len, sizeof(uint16_t), &u16_payload))) {
                    break;
                }
                LOGD("encoding class value:%i\n", payload);
                total_bytes_write += bytes_write;
                break;
            }
            case Value::Type::FUNC:
            {
                int32_t payload = -1;
                FuncState *func_state = encoder_->exec_state()->func_state();
                if (func_state == pval->f) {
                    payload++;
                }
                else {
                    const std::vector<FuncState *> &childrens = func_state->all_childrens();
                    int index = 0;
                    for (auto &func : childrens) {
                        if (pval->f == func) {
                            payload = index + 1;
                            break;
                        }
                        index++;
                    }
                }
                if (payload < 0) {
                    throw EncoderError("encoder value function can't find error");
                    break;
                }
                uint16_t u16_payload = static_cast<uint16_t>(payload);
                if (!(bytes_write = encodingToBuffer(buffer, buffer_len, sizeof(uint16_t), &u16_payload))) {
                    break;
                }
                LOGD("encoding function value:%i\n", payload);
                total_bytes_write += bytes_write;
                break;
            }
            case Value::Type::STRING:
            {
                int32_t payload = -1;
                int index = 0;
                StringTable *string_table = encoder_->exec_state()->string_table();
                for (auto &store : string_table->store() ) {
                    if (pval->str == store.second.get()) {
                        payload = index;
                        break;
                    }
                    index++;
                }
                if (payload < 0) {
                    throw EncoderError("encoder value string can't find error");
                    break;
                }
                uint16_t u16_payload = static_cast<uint16_t>(payload);
                if (!(bytes_write = encodingToBuffer(buffer, buffer_len, sizeof(uint16_t), &u16_payload))) {
                    break;
                }
                LOGD("encoding string value:[%i] %s\n", payload, pval->str->c_str());
                total_bytes_write += bytes_write;
                break;
            }
            case Value::Type::INT:
            {
                int32_t i32_payload = static_cast<int32_t>(pval->i);
                if (!(bytes_write = encodingToBuffer(buffer, buffer_len, sizeof(int32_t), &i32_payload))) {
                    break;
                }
                LOGD("encoding int value:%i\n", (int32_t)pval->i);
                total_bytes_write += bytes_write;
                break;
            }
            case Value::Type::NUMBER:
            {
                if (!(bytes_write = encodingToBuffer(buffer, buffer_len, sizeof(double), &pval->n))) {
                    break;
                }
                LOGD("encoding int value:%f\n", pval->n);
                total_bytes_write += bytes_write;
                break;
            }
            case Value::Type::BOOL:
            {
                if (!(bytes_write = encodingToBuffer(buffer, buffer_len, sizeof(uint8_t), &pval->b))) {
                    break;
                }
                LOGD("encoding bool value:%i\n", pval->b);
                total_bytes_write += bytes_write;
                break;
            }
            case Value::Type::ARRAY:
            {
                Array *array = ValueTo<Array>(pval);
                uint8_t items_size = static_cast<uint8_t>(array->items.size());
                if (!(bytes_write = encodingToBuffer(buffer, buffer_len, sizeof(uint8_t), &items_size))) {
                    throw EncoderError("encoder array item size error");
                    break;
                }
                buffer += bytes_write;
                buffer_len -= bytes_write;
                total_bytes_write += bytes_write;
                for (auto &item : array->items) {
                    uint32_t length = GetValueLength(&item);
                    uint32_t bytes_write = Section::encodingValueToBuffer(buffer, buffer_len, &item);
                    if (bytes_write != length) {
                        throw EncoderError("encoding array payload error");
                        break;
                    }
                    buffer += bytes_write;
                    buffer_len -= bytes_write;
                    total_bytes_write += bytes_write;
                }
                LOGD("encoding array:%s\n", ArrayToString(array).c_str());
                break;
            }
            case Value::Type::TABLE:
            {
                StringTable *string_table = encoder()->exec_state()->string_table();
                Table *table = ValueTo<Table>(pval);
                uint8_t keys_size = static_cast<uint8_t>(table->map.size());
                if (!(bytes_write = encodingToBuffer(buffer, buffer_len, sizeof(uint8_t), &keys_size))) {
                    throw EncoderError("encoder table key size error");
                    break;
                }
                buffer += bytes_write;
                buffer_len -= bytes_write;
                total_bytes_write += bytes_write;
                for (auto &map : table->map) {
                    Value key(string_table->StringFromUTF8(map.first));
                    Value val = map.second;
                    uint32_t length = GetValueLength(&key);
                    uint32_t bytes_write = Section::encodingValueToBuffer(buffer, buffer_len, &key);
                    if (bytes_write != length) {
                        throw EncoderError("encoding table key payload error");
                        break;
                    }
                    buffer += bytes_write;
                    buffer_len -= bytes_write;
                    total_bytes_write += bytes_write;
                    length = GetValueLength(&val);
                    bytes_write = Section::encodingValueToBuffer(buffer, buffer_len, &val);
                    if (bytes_write != length) {
                        throw EncoderError("encoding table key value payload error");
                        break;
                    }
                    buffer += bytes_write;
                    buffer_len -= bytes_write;
                    total_bytes_write += bytes_write;
                }
                LOGD("encoding table:%s\n", TableToString(table).c_str());
                break;
            }
            default:
                throw EncoderError("encoder value type unkown error");
                break;
        }
        
    } while (0);
    
    return total_bytes_write;
}
    
uint32_t Section::decodingFromBuffer(uint8_t *src_buffer, uint32_t src_buffer_len, uint8_t *buffer, uint32_t *len) {
    uint32_t bytes_reader = 0;
    do {
        if (!buffer || !len) {
            break;
        }
        if (*len > src_buffer_len) {
            break;
        }
        memcpy(buffer, src_buffer, *len);
        bytes_reader = *len;
        
    } while (0);
    
    return bytes_reader;
}
    
uint32_t Section::decodingFromBuffer(uint8_t *src_buffer, uint32_t src_buffer_len, uint16_t *target, uint8_t *buffer, uint32_t *len) {
    
    uint32_t bytes_reader = 0;
    do {
        uint32_t flags = src_buffer[0];
        uint32_t target_length = 0;
        uint16_t target_value = 0;
        uint32_t value_length_bytes = 0;
        uint32_t value_length = 0;
        if (SECTION_TEST_FLAG(flags, SECTION_EXT_MASK)) {
            flags = *(uint32_t *)src_buffer;
        }
        // tag
        if (SECTION_TEST_FLAG(flags, SECTION_TAG_MASK)) {
            target_length = 2;
            target_value = *(uint16_t *) (src_buffer + 1);
        }
        else {
            target_length = 1;
            target_value = *(src_buffer + 1);
        }
        // length
        if (SECTION_TEST_FLAG(flags, SECTION_LEN_MASK)) {
            value_length_bytes = ((flags & SECTION_LEN_MASK) >> SECTION_LEN_OFF);
            switch (value_length_bytes) {
                case 1:
                    value_length = *(src_buffer + 1 + target_length);
                    break;
                case 2:
                    value_length = *(uint16_t *) (src_buffer + 1 + target_length);
                    break;
                case 3:
                    value_length = *(uint32_t *) (src_buffer + 1 + target_length);
                    break;
            }
        }
        else {
            value_length_bytes = 0;
            value_length = (flags & SECTION_VALUE_MASK) + 1;
            if (value_length == 3) {
                value_length = 4;
            }
            else if (value_length == 4) {
                value_length = 8;
            }
        }
        // value
        // if no out buffer specified, just out put the tag, and return value length
        if (target) {
            *target = target_value;
        }
        bytes_reader += 1 + target_length + value_length_bytes;
        if (buffer == NULL || value_length == 0 || *len == 0) {
            if (len) {
                *len = value_length_bytes;
            }
            break;
        }
        // copy value data
        uint32_t buffer_length = *len;
        *len = value_length;
        if (buffer_length > value_length) {
            buffer_length = value_length;
        }
        memcpy(buffer, src_buffer + 1 + target_length + value_length_bytes, buffer_length);
        bytes_reader += buffer_length;
        
    } while (0);
    
    return bytes_reader;    
}
    
uint32_t Section::encodingToBuffer(uint8_t *dst_buffer, uint32_t dst_buffer_len, uint32_t len, void *buffer)
{
    uint32_t byte_write = 0;
    do {
        if (dst_buffer_len < len || !buffer) {
            break;
        }
        memcpy(dst_buffer, buffer, len);
        byte_write = len;
        
    } while (0);
    
    return byte_write;
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
    uint32_t length = sizeof(uint8_t);
    switch (ttype(value)) {
        case Value::Type::CLASS_DESC:
        case Value::Type::FUNC:
        case Value::Type::STRING:
        {
            length += sizeof(uint16_t);
            break;
        }
        case Value::Type::INT:
        {
            length += sizeof(int32_t);
            break;
        }
        case Value::Type::NUMBER:
        {
            length += sizeof(double);
            break;
        }
        case Value::Type::BOOL:
        {
            length += sizeof(uint8_t);
            break;
        }
        case Value::Type::ARRAY:
        {
            length += sizeof(uint8_t);
            Array *array = ValueTo<Array>(value);
            for (auto item : array->items) {
                length += GetValueLength(&item);
            }
            break;
        }
        case Value::Type::TABLE:
        {
            StringTable *string_table = encoder()->exec_state()->string_table();
            length += sizeof(uint8_t);
            Table *table = ValueTo<Table>(value);
            for (auto &map : table->map) {
                Value key(string_table->StringFromUTF8(map.first));
                Value val = map.second;
                length += GetValueLength(&key);
                length += GetValueLength(&val);
            }
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
    ExecState *exec_state = encoder()->exec_state();
    StringTable *string_table = exec_state->string_table();
    ClassFactory *class_factory = exec_state->class_factory();
    std::vector<ClassDescriptor *> descs = class_factory->descs();
    int32_t class_compile_index = static_cast<int32_t>(exec_state->class_compile_index());
    int32_t descs_size = static_cast<int32_t>(descs.size()) - class_compile_index;
    if (descs_size > 0) {
        // import decs string to table
        for (int i = class_compile_index; i < descs.size(); i++) {
            ClassDescriptor *desc = descs[i];
            uint32_t static_func_size = static_cast<uint32_t>(desc->statics_->size());
            if (static_func_size > 0) {
                for (int j = 0; j < static_func_size; j++) {
                    Value *func = desc->statics_->Find(j);
                    if (!func) {
                        continue;
                    }
                    for (auto &item : desc->statics_->map()) {
                        if (item.second == j) {
                            string_table->StringFromUTF8(item.first);
                            break;
                        }
                    }
                }
            }
            uint32_t class_func_size = static_cast<uint32_t>(desc->funcs_->size());
            if (class_func_size > 0) {
                for (int j = 0; j < class_func_size; j++) {
                    Value *func = desc->funcs_->Find(j);
                    if (!func) {
                        continue;
                    }
                    for (auto &item : desc->funcs_->map()) {
                        if (item.second == j) {
                            string_table->StringFromUTF8(item.first);
                            break;
                        }
                    }
                }
            }
        }
    }
    uint32_t size = 0;
    const std::vector<std::pair<std::string, std::unique_ptr<String>>> &store = string_table->store();
    int32_t string_compile_index = exec_state->string_compile_index();
    int32_t count = static_cast<int32_t>(store.size()) - string_compile_index;
    if (count > 0) {
        size += GetFTLVLength(kValueStringSize, sizeof(uint32_t));
        for (auto iter = store.begin() + string_compile_index; iter != store.end(); iter++) {
            size += GetFTLVLength(kValueStringPayload, (uint32_t)iter->first.length());
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
        ExecState *exec_state = encoder()->exec_state();
        StringTable *string_table = exec_state->string_table();
        const std::vector<std::pair<std::string, std::unique_ptr<String>>> &store = string_table->store();
        int32_t string_compile_index = exec_state->string_compile_index();
        int32_t count = static_cast<int32_t>(store.size()) - string_compile_index;
        if (!Section::encoding(kValueStringSize, sizeof(uint32_t), &count)) {
            break;
        }
        bool error = false;
        int index = 0;
        for (auto iter = store.begin() + string_compile_index; iter != store.end(); iter++) {
            const char *pcstr = iter->first.c_str();
            uint32_t length = (uint32_t)iter->first.length();
            if (!Section::encoding(kValueStringPayload, length, (void *)pcstr)) {
                error = true;
                break;
            }
            index++;
            LOGD("encoding string:[%i] %s\n", index, pcstr);
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
        if (readbytes != size || target != kValueStringSize) {
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
            uint32_t varlen = stream->ReadTarget(&vindex, NULL, NULL);
            if (vindex != kValueStringPayload) {
                break;
            }
            if (varlen == 0) {
                std::string str = "";
                string_table->StringFromUTF8(str);
                continue;
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
            LOGD("decoding string:%s\n", pstr);
            free(pstr);
        }
        if (index != string_count) {
            break;
        }
        finished = true;
        
    } while (0);
    
    return finished;
}
 
uint32_t SectionData::size() {
    ExecState *exec_state = encoder()->exec_state();
    ClassFactory *class_factory = exec_state->class_factory();
    std::vector<Value> constants = class_factory->constants();
    uint32_t data_size = static_cast<uint32_t>(constants.size());
    uint32_t size = 0;
    if (data_size > 0) {
        size += GetFTLVLength(kValueDataSize, sizeof(uint32_t));
        uint32_t constants_length = 0;
        for (auto constant : constants) {
            constants_length += GetValueLength(&constant);
        }
        size += GetFTLVLength(kValueDataPayload, constants_length);
    }
    return size;
}
    
bool SectionData::encoding() {
    bool finished = false;
    do {
        uint32_t size = this->size();
        if (!size) {
            finished = true;
            break;
        }
        if (!Section::encoding((uint16_t)ExecSection::EXEC_SECTION_DATA, size)) {
            break;
        }
        ExecState *exec_state = encoder()->exec_state();
        ClassFactory *class_factory = exec_state->class_factory();
        std::vector<Value> constants = class_factory->constants();
        uint32_t data_size = static_cast<uint32_t>(constants.size());
        if (!Section::encoding(kValueDataSize, sizeof(uint32_t), &data_size)) {
            break;
        }
        uint32_t payload_length = 0;
        for (int i = 0; i < constants.size(); i++) {
            payload_length += GetValueLength(&constants[i]);
        }
        uint8_t *buffer = (uint8_t *)malloc(payload_length);
        if (!buffer) {
            throw EncoderError("low memory error");
            break;
        }
        uint8_t *write_buffer = buffer;
        uint32_t remain_length = payload_length;
        for (int i = 0; i < constants.size(); i++) {
            uint32_t length = GetValueLength(&constants[i]);
            uint32_t bytes_write = Section::encodingValueToBuffer(write_buffer, remain_length, &constants[i]);
            if (bytes_write != length) {
                throw EncoderError("encoding data payload error");
                break;
            }
            remain_length -= bytes_write;
            write_buffer += bytes_write;
        }
        if (remain_length) {
            free(buffer);
            throw EncoderError("encoding data payload error");
            break;
        }
        if (!Section::encoding(kValueDataPayload, payload_length, buffer)) {
            free(buffer);
            throw EncoderError("encoding data payload error");
            break;
        }
        free(buffer);
        finished = true;
        
    } while (0);
    
    return finished;
}
    
bool SectionData::decoding() {
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
        uint32_t data_size = 0;
        uint32_t size = sizeof(uint32_t);
        uint32_t readbytes = stream->ReadTarget(&target, (uint8_t *)&data_size, &size);
        if (readbytes != size || target != kValueDataSize) {
            throw EncoderError("decoding data size error");
            break;
        }
        if (!data_size) {
            break;
        }
        if ((readbytes = stream->ReadTarget(&target, NULL, NULL)) == 0) {
            throw DecoderError("decoding data payload error");
            break;
        }
        if (target != kValueDataPayload) {
            throw DecoderError("decoding data payload target error");
            break;
        }
        uint8_t *buffer = (uint8_t *)malloc(readbytes);
        if (!buffer) {
            throw DecoderError("low memory error");
            break;
        }
        if (stream->Read(buffer, 1, readbytes) != readbytes) {
            free(buffer);
            throw DecoderError("decoding data payload error");
            break;
        }
        uint8_t *read_buffer = buffer;
        uint32_t remain_length = readbytes;
        uint32_t bytes_read = 0;
        for (int i = 0; i < data_size; i++) {
            Value data;
            if (!(bytes_read = decodingValueFromBuffer(read_buffer, remain_length, &data))) {
                throw DecoderError("decoding data payload error");
                break;
            }
            read_buffer += bytes_read;
            remain_length -= bytes_read;
        }
        if (remain_length) {
            free(buffer);
            throw DecoderError("decoding array item payload error");
            break;
        }
        free(buffer);
        finished = true;
        
    } while (0);
    
    return finished;
}

uint32_t SectionScript::size() {
    uint32_t size = 0;
    do {
        const json11::Json& scripts = encoder()->exec_state()->context()->script_json();
        if (!scripts.array_items().size()) {
            break;
        }
        size += GetFTLVLength(kValueScriptSize, sizeof(uint32_t));
        for (auto script : scripts.array_items()) {
            const json11::Json::object& items = script.object_items();
            size += GetFTLVLength(kValueScriptItemSize, sizeof(uint32_t));
            for (auto iter = items.begin(); iter != items.end(); iter++) {
                uint32_t key_length = static_cast<uint32_t>(iter->first.length());
                size += GetFTLVLength(kValueScriptKey, key_length);
                uint32_t val_length = static_cast<uint32_t>(iter->second.string_value().length());
                size += GetFTLVLength(kValueScriptValue, val_length);
            }
        }
    } while (0);

    return size;
}

bool SectionScript::encoding() {
    bool finished = false;
    do {
        uint32_t size = this->size();
        if (!size) {
            finished = true;
            break;
        }
        if (!Section::encoding((uint16_t)ExecSection::EXEC_SECTION_SCRIPT, size)) {
            break;
        }
        const json11::Json& scripts = encoder()->exec_state()->context()->script_json();
        if (!scripts.is_array()) {
            break;
        }
        uint32_t scripts_size = (uint32_t)scripts.array_items().size();
        if (!Section::encoding(kValueScriptSize, sizeof(uint32_t), &scripts_size)) {
            break;
        }
        for (auto it = scripts.array_items().begin(); it != scripts.array_items().end(); it++) {
            uint32_t script_item_size = (uint32_t)it->object_items().size();
            if (!Section::encoding(kValueScriptItemSize, sizeof(uint32_t), &script_item_size)) {
                throw DecoderError("decoding script item size error");
                break;
            }
            for (auto item : it->object_items()) {
                uint32_t length = static_cast<uint32_t>(item.first.length());
                uint8_t *pstr = (uint8_t *)item.first.c_str();
                if (!Section::encoding(kValueScriptKey, length, (uint8_t *)pstr)) {
                    throw DecoderError("decoding script item key error");
                    break;
                }
                length = static_cast<uint32_t>(item.second.string_value().length());
                const char *pstr_val = item.second.string_value().c_str();
                if (!Section::encoding(kValueScriptValue, length, (uint8_t *)pstr_val)) {
                    throw DecoderError("decoding script item value error");
                    break;
                }
            }
        }
        finished = true;
    } while (0);

    return finished;
}

bool SectionScript::decoding() {
    bool finished = false;
    do {
        fStream *stream = Section::stream();
        if (!stream) {
            break;
        }
        if (stream->Tell() < 0) {
            break;
        }
        std::vector<json11::Json> scripts;
        uint16_t target = 0;
        uint32_t script_size = 0;
        uint32_t size = sizeof(uint32_t);
        uint32_t readbytes = stream->ReadTarget(&target, (uint8_t *)&script_size, &size);
        if (!readbytes || target != kValueScriptSize) {
            break;
        }
        for (uint32_t i = 0; i < script_size; i++) {
            size = sizeof(uint32_t);
            uint32_t items_size = 0;
            readbytes = stream->ReadTarget(&target, (uint8_t *)&items_size, &size);
            if (!readbytes || target != kValueScriptItemSize) {
                throw DecoderError("decoding script items size error");
                break;
            }
            std::unordered_map<std::string, json11::Json> items;
            for (uint32_t j = 0; j < items_size; j++) {
                if ((readbytes = stream->ReadTarget(&target, NULL, NULL)) == 0) {
                    throw DecoderError("decoding script target error");
                    break;
                }
                if (target != kValueScriptKey) {
                    throw DecoderError("decoding script target error");
                    break;
                }
                char *pstr_key = (char *)malloc(readbytes + 1);
                if (!pstr_key) {
                    throw DecoderError("decoding script low memory error");
                    break;
                }
                memset(pstr_key, 0, readbytes + 1);
                if (stream->Read(pstr_key, 1, readbytes) != readbytes) {
                    throw DecoderError("decoding script key error");
                    break;
                }
                if ((readbytes = stream->ReadTarget(&target, NULL, NULL)) == 0) {
                    throw DecoderError("decoding script target error");
                    break;
                }
                if (target != kValueScriptValue) {
                    throw DecoderError("decoding script target error");
                    break;
                }
                char *pstr_value = (char *)malloc(readbytes + 1);
                if (!pstr_value) {
                    throw DecoderError("decoding script low memory error");
                    break;
                }
                memset(pstr_value, 0, readbytes + 1);
                if (stream->Read(pstr_value, 1, readbytes) != readbytes) {
                    throw DecoderError("decoding script value error");
                    break;
                }
                json11::Json json(pstr_value);
                items.insert(std::make_pair(pstr_key, json));
                free(pstr_key);
                free(pstr_value);
            }
            scripts.push_back(std::move(items));
        }
        decoder()->exec_state()->context()->set_script_json(scripts);
        finished = true;

    } while (0);

    return finished;
}
    
uint32_t SectionFunction::GetInstructionsBytes(std::vector<Instruction>& instructions) {
    uint32_t numBits = 0;
    do {
        if (!instructions.size()) {
            break;
        }
        numBits += sizeof(uint32_t) * 8;
        uint8_t half_bits_op_code = op_code_bits_ / 2;
        int half_bits_op_code_value = pow(2, half_bits_op_code);
        for (int i = 0; i < instructions.size(); i++) {
            Instruction instruction = instructions[i];
            numBits += 1;
            OPCode code = GET_OP_CODE(instruction);
            numBits += code >= half_bits_op_code_value ? op_code_bits_ : half_bits_op_code;
            int ops = OPUtil::ops(code);
            if (code == OP_GOTO) {
                long A = GET_ARG_Ax(instruction);
                numBits += 1;
                numBits += A > 2047 ? 24 : 12;
            }
            else if (code == OP_JMP) {
                long A = GET_ARG_A(instruction);
                long B = GET_ARG_Bx(instruction);
                numBits += 1;
                numBits += A > 15 ? 8 : 4;
                numBits += 1;
                numBits += B > 2047 ? 24 : 12;
            }
            else {
                switch (ops) {
                    case 1:
                    {
                        long A = GET_ARG_Ax(instruction);
                        numBits += 1;
                        numBits += A > 15 ? 8 : 4;
                        if (A > SECTION_REGISTER_MAX) {
                            throw EncoderError("register overflow error， please decrease function body");
                        }
                        break;
                    }
                    case 2:
                    {
                        long A = GET_ARG_A(instruction);
                        long B = GET_ARG_Bx(instruction);
                        numBits += 1;
                        numBits += A > 15 ? 8 : 4;
                        numBits += 1;
                        numBits += B > 15 ? 8 : 4;
                        if (A > SECTION_REGISTER_MAX || B > SECTION_REGISTER_MAX) {
                            throw EncoderError("register overflow error， please decrease function body");
                        }
                        break;
                    }
                    case 3:
                    {
                        long A = GET_ARG_A(instruction);
                        long B = GET_ARG_B(instruction);
                        long C = GET_ARG_C(instruction);
                        numBits += 1;
                        numBits += A > 15 ? 8 : 4;
                        numBits += 1;
                        numBits += B > 15 ? 8 : 4;
                        numBits += 1;
                        numBits += C > 15 ? 8 : 4;
                        if (A > SECTION_REGISTER_MAX || B > SECTION_REGISTER_MAX || C > SECTION_REGISTER_MAX)
                        {
                            throw EncoderError("register overflow error， please decrease function body");
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
        }
        
    } while (0);
    
    return (numBits + 7) / 8;
}
    
uint32_t SectionFunction::decodingInstructionsFromBuffer(uint8_t *buffer, uint32_t bytes, std::vector<Instruction>& instructions) {
    uint32_t bytes_reading = 0;
    do {
        CBitsReader reader(buffer, bytes);
        uint32_t count = 0;
        if (reader.readStream((uint8_t *)&count, sizeof(uint32_t)) != sizeof(uint32_t) || !count) {
            throw DecoderError("decoding instructions buffer count zero error");
            break;
        }
        uint8_t half_bits_op_code = op_code_bits_ / 2;
        for (int i = 0; i < count; i++) {
            uint8_t using_full_bits_op_code = reader.nextBits(1);
            uint32_t codeBits = reader.nextBits(using_full_bits_op_code ? op_code_bits_ : half_bits_op_code);
            if (codeBits >= OP_INVALID) {
                throw DecoderError("decoding instructions code error");
                break;
            }
            uint32_t A = 0, B = 0, C = 0;
            uint8_t using8bits = 0, using24bits = 0;
            OPCode code = static_cast<OPCode>(codeBits);
            int ops = OPUtil::ops(code);
            Instruction instruction;
            if (code == OP_GOTO) {
                using24bits = reader.nextBits(1);
                A = reader.nextBits(using24bits ? 24 : 12);
                instruction = CREATE_Ax(code, A);
            }
            else if (code == OP_JMP) {
                using8bits = reader.nextBits(1);
                A = reader.nextBits(using8bits ? 8 : 4);
                using24bits = reader.nextBits(1);
                B = reader.nextBits(using24bits ? 24 : 12);
                instruction = CREATE_ABx(code, A, B);
            }
            else {
                switch (ops) {
                    case 1:
                    {
                        using8bits = reader.nextBits(1);
                        A = reader.nextBits(using8bits ? 8 : 4);
                        instruction = CREATE_Ax(code, A);
                        break;
                    }
                    case 2:
                    {
                        using8bits = reader.nextBits(1);
                        A = reader.nextBits(using8bits ? 8 : 4);
                        using8bits = reader.nextBits(1);
                        B = reader.nextBits(using8bits ? 8 : 4);
                        instruction = CREATE_ABx(code, A, B);
                        break;
                    }
                    case 3:
                    {
                        using8bits = reader.nextBits(1);
                        A = reader.nextBits(using8bits ? 8 : 4);
                        using8bits = reader.nextBits(1);
                        B = reader.nextBits(using8bits ? 8 : 4);
                        using8bits = reader.nextBits(1);
                        C = reader.nextBits(using8bits ? 8 : 4);
                        instruction = CREATE_ABC(code, A, B, C);
                        break;
                    }
                    default:
                        instruction = CREATE_Ax(code, 0);
                        break;
                }
            }
            LOGD("decoding Instructions:%ld\n", instruction);
            instructions.push_back(instruction);
        }
        if (reader.remainingBytes()) {
            throw DecoderError("decoding instructions code error");
            break;
        }
        bytes_reading = bytes;
        
    } while (0);
    
    return bytes_reading;
}
    
uint32_t SectionFunction::encodingInstructionsToBuffer(uint8_t *buffer, uint32_t bytes, std::vector<Instruction>& instructions) {
    uint32_t bytes_writed = 0;
    do {
        if (!instructions.size()) {
            break;
        }
        CBitsWriter writer(buffer, bytes);
        uint32_t count = static_cast<uint32_t>(instructions.size());
        if (!writer.WriteStream((const uint8_t *)&count, sizeof(uint32_t))) {
            throw EncoderError("write instructions size error");
            break;
        }
        uint8_t half_bits_op_code = op_code_bits_ / 2;
        int half_bits_op_code_value = pow(2, half_bits_op_code);
        for (int i = 0; i < count; i++) {
            Instruction instruction = instructions[i];
            OPCode code = GET_OP_CODE(instruction);
            std::uint32_t bits_data = code >= half_bits_op_code_value ? 1 : 0;
            writer.WriteBits(bits_data, 1);
            std::uint32_t numBits = bits_data ? op_code_bits_ : half_bits_op_code;
            bits_data = code;
            writer.WriteBits(bits_data, numBits);
            int ops = OPUtil::ops(code);
            bool using8bits = false;
            bool using24bits = false;
            if (code == OP_GOTO) {
                long A = GET_ARG_Ax(instruction);
                using24bits = A > 2047 ? true : false;
                bits_data = using24bits ? 1 : 0;
                writer.WriteBits(bits_data, 1);
                bits_data = static_cast<std::uint32_t>(A);
                writer.WriteBits(bits_data, using24bits ? 24 : 12);
            }
            else if (code == OP_JMP) {
                long A = GET_ARG_A(instruction);
                long B = GET_ARG_Bx(instruction);
                using8bits = A > 15 ? true : false;
                bits_data = using8bits ? 1 : 0;
                writer.WriteBits(bits_data, 1);
                bits_data = static_cast<std::uint32_t>(A);
                writer.WriteBits(bits_data, using8bits ? 8 : 4);
                using24bits = B > 2047 ? true : false;
                bits_data = using24bits ? 1 : 0;
                writer.WriteBits(bits_data, 1);
                bits_data = static_cast<std::uint32_t>(B);
                writer.WriteBits(bits_data, using24bits ? 24 : 12);
            }
            else {
                switch (ops) {
                    case 1:
                    {
                        long A = GET_ARG_Ax(instruction);
                        using8bits = A > 15 ? true : false;
                        bits_data = using8bits ? 1 : 0;
                        writer.WriteBits(bits_data, 1);
                        bits_data = static_cast<std::uint32_t>(A);
                        writer.WriteBits(bits_data, using8bits ? 8 : 4);
                        break;
                    }
                    case 2:
                    {
                        long A = GET_ARG_A(instruction);
                        long B = GET_ARG_Bx(instruction);
                        using8bits = A > 15 ? true : false;
                        bits_data = using8bits ? 1 : 0;
                        writer.WriteBits(bits_data, 1);
                        bits_data = static_cast<std::uint32_t>(A);
                        writer.WriteBits(bits_data, using8bits ? 8 : 4);
                        using8bits = B > 15 ? true : false;
                        bits_data = using8bits ? 1 : 0;
                        writer.WriteBits(bits_data, 1);
                        bits_data = static_cast<std::uint32_t>(B);
                        writer.WriteBits(bits_data, using8bits ? 8 : 4);
                        break;
                    }
                    case 3:
                    {
                        long A = GET_ARG_A(instruction);
                        long B = GET_ARG_B(instruction);
                        long C = GET_ARG_C(instruction);
                        using8bits = A > 15 ? true : false;
                        bits_data = using8bits ? 1 : 0;
                        writer.WriteBits(bits_data, 1);
                        bits_data = static_cast<std::uint32_t>(A);
                        writer.WriteBits(bits_data, using8bits ? 8 : 4);
                        using8bits = B > 15 ? true : false;
                        bits_data = using8bits ? 1 : 0;
                        writer.WriteBits(bits_data, 1);
                        bits_data = static_cast<std::uint32_t>(B);
                        writer.WriteBits(bits_data, using8bits ? 8 : 4);
                        using8bits = C > 15 ? true : false;
                        bits_data = using8bits ? 1 : 0;
                        writer.WriteBits(bits_data, 1);
                        bits_data = static_cast<std::uint32_t>(C);
                        writer.WriteBits(bits_data, using8bits ? 8 : 4);
                        break;
                    }
                    default:
                        break;
                }
            }
            LOGD("encoding Instructions:%ld\n", instruction);
        }
        writer.Finish();
        LOGD("Writer Instructions: numBits:%i\n", (int)writer.TotalBits());
        if (writer.TotalBits() / 8 != bytes) {
            throw EncoderError("encoding instructions buffer size error");
            break;
        }
        bytes_writed += bytes;
        
    } while (0);
    
    return bytes_writed;
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
    uint32_t total_constancts_bytes = 0;
    uint32_t total_instructions_bytes = 0;
    if (func_states.size() > 0) {
        size += GetFTLVLength(kValueFunctionSize, sizeof(uint32_t));
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
                uint32_t instructions_bytes = GetInstructionsBytes(func_state->instructions());
                instructions_bytes = GetFTLVLength(kValueFunctionInstructions, instructions_bytes);
                total_instructions_bytes += instructions_bytes;
                size += instructions_bytes;
            }
            if (func_state->constants().size() > 0) {
                size += GetFTLVLength(kValueFunctionConstantCount, sizeof(int32_t));
                uint32_t constant_bytes = 0;
                for (int i = 0; i < func_state->constants().size(); i++) {
                    constant_bytes += GetValueLength(&func_state->constants()[i]);
                }
                constant_bytes = GetFTLVLength(kValueFunctionConstantPayload, constant_bytes);
                total_constancts_bytes += constant_bytes;
                size += constant_bytes;
            }
            size += GetFTLVLength(kValueFunctionFinished, sizeof(uint8_t));
        }
    }
    LOGD("[WXExecEncoder]: instructions:%.02f K\n", (double)total_instructions_bytes / 1024);
    LOGD("[WXExecEncoder]: constancts:%.02f K\n", (double)total_constancts_bytes / 1024);
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
        if (!Section::encoding(kValueFunctionSize, sizeof(uint32_t), &func_count)) {
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
            uint8_t is_class_func = func_state->is_class_func();
            if (!Section::encoding(kValueFunctionClass, sizeof(uint8_t), &is_class_func)) {
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
                    LOGD("encoding in_closure:%i\n", buffer[i]);
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
                    LOGD("encoding out_closure:%i\n", buffer[i]);
                }
                if (!Section::encoding(kValueFunctionOutClosure, (uint32_t)size, buffer)) {
                    break;
                }
                free(buffer);
            }
            if (func_state->instructions().size() > 0) {
                uint32_t instructions_bytes = GetInstructionsBytes(func_state->instructions());
                uint8_t *buffer = (uint8_t *)malloc(instructions_bytes);
                if (!buffer) {
                    throw EncoderError("low memory error");
                    break;
                }
                memset(buffer, 0, instructions_bytes);
                if (encodingInstructionsToBuffer(buffer, instructions_bytes, func_state->instructions()) != instructions_bytes) {
                    LOGD("encoding instructions error\n");
                    break;
                }
                if (!Section::encoding(kValueFunctionInstructions, (uint32_t)instructions_bytes, buffer)) {
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
                uint8_t *write_buffer = buffer;
                uint32_t remain_length = constants_value_length;
                for (int i = 0; i < func_state->constants().size(); i++) {
                    uint32_t length = GetValueLength(&func_state->constants()[i]);
                    uint32_t bytes_write = Section::encodingValueToBuffer(write_buffer, remain_length, &func_state->constants()[i]);
                    if (bytes_write != length) {
                        throw EncoderError("encoding constants value error");
                        break;
                    }
                    remain_length -= bytes_write;
                    write_buffer += bytes_write;
                }
                if (!Section::encoding(kValueFunctionConstantPayload, constants_value_length, buffer)) {
                    break;
                }
                free(buffer);
            }
            uint8_t function_finished = SECTION_VALUE_FINISHED;
            if (!Section::encoding(kValueFunctionFinished, sizeof(uint8_t), &function_finished)) {
                break;
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
        if (!readbytes || target != kValueFunctionSize || !func_count) {
            break;
        }
        std::vector<FuncState *>func_states;
        for (int i = 0; i < func_count; i++) {
            FuncState *func_state = new FuncState;
            uint32_t constants_count = 0;
            do {
                if ((readbytes = stream->ReadTarget(&target, NULL, NULL)) == 0) {
                    throw DecoderError("decoding read function target error");
                    break;
                }
                switch (target) {
                    case kValueFunctionSuper:
                    {
                        int super_index = -1;
                        if (stream->Read(&super_index, 1, readbytes) != readbytes) {
                            throw DecoderError("decoding read super function error");
                        }
                        func_state->set_super_index(super_index);
                        break;
                    }
                    case kValueFunctionClass:
                    {
                        uint8_t is_class_func = 0;
                        if (stream->Read(&is_class_func, 1, readbytes) != readbytes) {
                            throw DecoderError("decoding read class function error");
                        }
                        func_state->set_is_class_func(is_class_func);
                        break;
                    }
                    case kValueFunctionArgs:
                    {
                        int32_t args[SECTION_FUNCTION_MAXARGS] = { 0 };
                        int readcnt = readbytes / sizeof(int32_t);
                        if (stream->Read(args, sizeof(int32_t), readcnt) != readcnt) {
                            throw DecoderError("decoding read function args error");
                            break;
                        }
                        for (int i = 0; i < readcnt; i++) {
                            func_state->args().push_back(args[i]);
                        }
                        func_state->argc() = readcnt;
                        break;
                    }
                    case kValueFunctionInstructions:
                    {
                        uint8_t *buffer = (uint8_t *)malloc(readbytes);
                        if (!buffer) {
                            throw DecoderError("decoding read function low memory error");
                            break;
                        }
                        if (stream->Read(buffer, 1, readbytes) != readbytes) {
                            throw DecoderError("decoding read function instructions error");
                            break;
                        }
                        if (decodingInstructionsFromBuffer(buffer, readbytes, func_state->instructions())  != readbytes) {
                            throw DecoderError("decoding read function instructions error");
                            break;
                        }
                        free(buffer);
                        break;
                    }
                    case kValueFunctionConstantCount:
                    {
                        if (stream->Read(&constants_count, 1, readbytes) != readbytes) {
                            throw DecoderError("decoding read function constants count error");
                            break;
                        }
                        break;
                    }
                    case kValueFunctionConstantPayload:
                    {
                        if (!constants_count) {
                            throw DecoderError("decoding read function constants count error");
                            break;
                        }
                        uint8_t *buffer = (uint8_t *)malloc(readbytes);
                        if (!buffer) {
                            throw DecoderError("low memory error");
                            break;
                        }
                        if (stream->Read(buffer, 1, readbytes) != readbytes) {
                            throw DecoderError("decoding function constants payload error");
                            break;
                        }
                        uint8_t *read_buffer = buffer;
                        uint32_t remain_length = readbytes;
                        uint32_t bytes_read = 0;
                        for (int i = 0; i < constants_count; i++) {
                            Value value;
                            if (!(bytes_read = decodingValueFromBuffer(read_buffer, remain_length, &value))) {
                                throw DecoderError("decoding function constants payload error");
                                break;
                            }
                            read_buffer += bytes_read;
                            remain_length -= bytes_read;
                            func_state->AddConstant(value);
                        }
                        free(buffer);
                        break;
                    }
                    case kValueFunctionOutClosure:
                    {
                        int32_t *buffer = (int32_t *)malloc(readbytes);
                        if (!buffer) {
                            throw DecoderError("decoding low memory error");
                            break;
                        }
                        if (stream->Read(buffer, 1, readbytes) != readbytes) {
                            throw DecoderError("decoding function out closure error");
                            break;
                        }
                        int32_t size = readbytes / sizeof(int32_t);
                        for (int i = 0; i < size; i++) {
                            func_state->out_closure_refs().push_back(buffer[i]);
                            LOGD("decoding function out closure:%i\n", buffer[i]);
                        }
                        break;
                    }
                    case kValueFunctionInClosure:
                    {
                        int32_t *buffer = (int32_t *)malloc(readbytes);
                        if (!buffer) {
                            throw DecoderError("decoding low memory error");
                            break;
                        }
                        if (stream->Read(buffer, 1, readbytes) != readbytes) {
                            throw DecoderError("decoding function in closure error");
                            break;
                        }
                        int32_t size = readbytes / sizeof(int32_t);
                        for (int i = 0; i < size; i++) {
                            func_state->in_closure_refs().push_back(buffer[i]);
                            LOGD("decoding function in closure:%i\n", buffer[i]);
                        }
                        break;
                    }
                    case kValueFunctionFinished:
                    {
                        uint8_t function_finshed = 0;
                        if (stream->Read(&function_finshed, 1, readbytes) != readbytes) {
                            throw DecoderError("decoding function finished error");
                            break;
                        }
                        if (function_finshed != SECTION_VALUE_FINISHED) {
                            throw DecoderError("decoding function finished error");
                            break;
                        }
                        break;
                    }
                    default:
                    {
                        if (stream->Seek(readbytes, SEEK_CUR) < 0) {
                            throw DecoderError("function decoding length error");
                        }
                        break;
                    }
                }
                if (target == SECTION_VALUE_FINISHED) {
                    break;
                }
                
            } while (stream->Tell() < seek_end);
            
            func_states.push_back(func_state);
        }
        if (!func_states.size()) {
            break;
        }
        decoder()->exec_state()->reset(func_states[0]);
        for (auto func : func_states) {
            int index = func->super_index();
            if (index > -1) {
                func_states[index]->AddChild(func);
            }
        }        
        finished = true;
        
    } while (0);
    
    return finished;
}

uint32_t SectionGlobalConstants::size() {
    uint32_t size = 0;
    do {
        Variables *global = encoder()->exec_state()->global();
        uint32_t compile_index = encoder()->exec_state()->global_compile_index();
        if (global->size() <= compile_index) {
            break;
        }
        size += GetFTLVLength(kValueGlobalConstantsSize, sizeof(int32_t));
        uint32_t constant_length = 0;
        for (uint32_t i = compile_index; i < global->size(); i++) {
            Value *value = global->Find(compile_index);
            constant_length += GetValueLength(value);
        }
        size += GetFTLVLength(kValueGlobalConstantsPayload, constant_length);
        
    } while (0);

    return size;
}
    
bool SectionGlobalConstants::encoding() {
    bool finished = false;
    do {
        uint32_t size = this->size();
        if (!size) {
            finished = true;
            break;
        }
        if (!Section::encoding((uint16_t)ExecSection::EXEC_SECTION_GLOBAL_CONSTANTS, size)) {
            break;
        }
        Variables *global = encoder()->exec_state()->global();
        uint32_t compile_index = encoder()->exec_state()->global_compile_index();
        uint32_t constants_count = (uint32_t)(global->size() - compile_index);
        if (!Section::encoding(kValueGlobalConstantsSize, sizeof(uint32_t), &constants_count)) {
            break;
        }
        uint32_t constants_payload_length = 0;
        for (int i = 0; i < constants_count; i++) {
            Value *value = global->Find(compile_index + i);
            constants_payload_length += GetValueLength(value);
        }
        uint8_t *buffer = (uint8_t *)malloc(constants_payload_length);
        if (!buffer) {
            throw EncoderError("low memory error");
            break;
        }
        uint8_t *write_buffer = buffer;
        uint32_t remain_length = constants_payload_length;
        for (int i = 0; i < constants_count; i++) {
            Value *value = global->Find(compile_index + i);
            uint32_t length = GetValueLength(value);
            uint32_t bytes_write = Section::encodingValueToBuffer(write_buffer, remain_length, value);
            if (bytes_write != length) {
                throw EncoderError("encoding global constants value error");
                break;
            }
            remain_length -= bytes_write;
            write_buffer += bytes_write;
        }
        if (!Section::encoding(kValueGlobalConstantsPayload, constants_payload_length, buffer)) {
            break;
        }
        free(buffer);
        finished = true;
        
    } while (0);
    
    return finished;
}

bool SectionGlobalConstants::decoding() {
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
        uint32_t constants_count = 0;
        uint32_t size = sizeof(uint32_t);
        uint32_t readbytes = stream->ReadTarget(&target, (uint8_t *)&constants_count, &size);
        if (!readbytes || target != kValueGlobalConstantsSize || !constants_count) {
            break;
        }
        if ((readbytes = stream->ReadTarget(&target, NULL, NULL)) == 0) {
            throw DecoderError("decoding global constants target error");
            break;
        }
        if (target != kValueGlobalConstantsPayload) {
            break;
        }
        uint8_t *buffer = (uint8_t *)malloc(readbytes);
        if (!buffer) {
            throw DecoderError("low memory error");
            break;
        }
        if (stream->Read(buffer, 1, readbytes) != readbytes) {
            throw DecoderError("decoding global constants payload error");
            break;
        }
        uint8_t *read_buffer = buffer;
        uint32_t remain_length = readbytes;
        uint32_t bytes_read = 0;
        Variables *global = decoder()->exec_state()->global();
        if (global->IndexOf("__weex_data__") < 0) {
            global->Set("__weex_data__", Value());
        }
        if (global->IndexOf("_init_data") < 0) {
            global->Set("_init_data", Value());
        }
        for (int i = 0; i < constants_count; i++) {
            Value value;
            if (!(bytes_read = decodingValueFromBuffer(read_buffer, remain_length, &value))) {
                throw DecoderError("decoding function constants payload error");
                break;
            }
            read_buffer += bytes_read;
            remain_length -= bytes_read;
            global->Add(value);
        }
        if (remain_length != 0) {
            throw DecoderError("decoding function constants payload error");
            break;
        }
        free(buffer);
        finished = true;
        
    } while (0);
    
    return finished;
}

uint32_t SectionGlobalVariables::size() {
    uint32_t size = 0;
    do {
        std::unordered_map<std::string, long> global_variables = encoder()->exec_state()->global_variables();
        if (!global_variables.size()) {
            break;
        }
        size += GetFTLVLength(kValueGlobalVariablesSize, sizeof(int32_t));
        for (auto variable : global_variables) {
            uint32_t length = static_cast<uint32_t>(variable.first.length());
            size += GetFTLVLength(kValueGlobalVariablesKey, length);
            size += GetFTLVLength(kValueGlobalVariablesRegister, sizeof(int32_t));
        }
        
    } while (0);
    
    return size;
}

bool SectionGlobalVariables::encoding() {
    bool finished = false;
    do {
        uint32_t size = this->size();
        if (!size) {
            finished = true;
            break;
        }
        if (!Section::encoding((uint16_t)ExecSection::EXEC_SECTION_GLOBAL_VARIABLES, size)) {
            break;
        }
        std::unordered_map<std::string, long> global_variables = encoder()->exec_state()->global_variables();
        uint32_t global_variables_size = (uint32_t)global_variables.size();
        if (!Section::encoding(kValueGlobalVariablesSize, sizeof(uint32_t), &global_variables_size)) {
            break;
        }
        for (auto variable : global_variables) {
            uint32_t length = static_cast<uint32_t>(variable.first.length());
            uint8_t *pstr = (uint8_t *)variable.first.c_str();
            if (!Section::encoding(kValueGlobalVariablesKey, length, pstr)) {
                throw DecoderError("decoding global variables key error");
                break;
            }
            int32_t register_id = (int32_t)variable.second;
            if (!Section::encoding(kValueGlobalVariablesRegister, sizeof(int32_t), &register_id)) {
                throw DecoderError("decoding global variables register error");
                break;
            }
        }
        finished = true;
        
    } while (0);
    
    return finished;
}

bool SectionGlobalVariables::decoding() {
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
        uint32_t global_variables_size = 0;
        uint32_t size = sizeof(uint32_t);
        uint32_t readbytes = stream->ReadTarget(&target, (uint8_t *)&global_variables_size, &size);
        if (!readbytes || target != kValueGlobalVariablesSize || !global_variables_size) {
            break;
        }
        std::unordered_map<std::string, long> &global_variables = decoder()->exec_state()->global_variables();
        for (int i = 0; i < global_variables_size; i++) {
            if ((readbytes = stream->ReadTarget(&target, NULL, NULL)) == 0) {
                throw DecoderError("decoding global variables target error");
                break;
            }
            if (target != kValueGlobalVariablesKey) {
                throw DecoderError("decoding global variables target error");
                break;
            }
            char *pstr = (char *)malloc(readbytes + 1);
            if (!pstr) {
                throw DecoderError("decoding global variables low memory error");
                break;
            }
            memset(pstr, 0, readbytes + 1);
            if (stream->Read(pstr, 1, readbytes) != readbytes) {
                throw DecoderError("decoding global variables key error");
                break;
            }
            int32_t register_id = -1;
            uint32_t size = sizeof(uint32_t);
            readbytes = stream->ReadTarget(&target, (uint8_t *)&register_id, &size);
            if (readbytes != size || target != kValueGlobalVariablesRegister) {
                throw EncoderError("decoding global variables register error");
                break;
            }
            global_variables.insert(std::pair<std::string, long>(pstr, register_id));
            free(pstr);
        }
        finished = true;
        
    } while (0);
    
    return finished;
}

uint32_t SectionStyles::size() {
    uint32_t size = 0;
    do {
        std::map<std::string, json11::Json>& styles = encoder()->exec_state()->context()->style_json();
        if (!styles.size()) {
            break;
        }
        size += GetFTLVLength(kValueStyleSize, sizeof(uint32_t));
        for (auto style : styles) {
            uint32_t key_length = (uint32_t)style.first.length();
            size += GetFTLVLength(kValueStyleKey, key_length);
            const json11::Json::object& items = style.second.object_items();
            size += GetFTLVLength(kValueStyleItemSize, sizeof(uint32_t));
            for (auto iter = items.begin(); iter != items.end(); iter++) {
                key_length = static_cast<uint32_t>(iter->first.length());
                size += GetFTLVLength(kValueStyleKey, key_length);
                uint32_t val_length = static_cast<uint32_t>(iter->second.string_value().length());
                size += GetFTLVLength(kValueStyleValue, val_length);
            }
        }
        
    } while (0);
    
    return size;
}

bool SectionStyles::encoding() {
    bool finished = false;
    do {
        uint32_t size = this->size();
        if (!size) {
            finished = true;
            break;
        }
        if (!Section::encoding((uint16_t)ExecSection::EXEC_SECTION_STYLES, size)) {
            break;
        }
        std::map<std::string, json11::Json>& styles = encoder()->exec_state()->context()->style_json();
        uint32_t styles_size = (uint32_t)styles.size();
        if (!Section::encoding(kValueStyleSize, sizeof(uint32_t), &styles_size)) {
            break;
        }
        for (auto style : styles) {
            uint32_t length = (uint32_t)style.first.length();
            uint8_t *pstr = (uint8_t *)style.first.c_str();
            if (!Section::encoding(kValueStyleKey, length, pstr)) {
                throw DecoderError("decoding style key error");
                break;
            }
            const json11::Json::object& items = style.second.object_items();
            uint32_t items_size = static_cast<uint32_t>(items.size());
            if (!Section::encoding(kValueStyleItemSize, sizeof(uint32_t), &items_size)) {
                throw DecoderError("decoding style items size error");
                break;
            }
            for (auto iter = items.begin(); iter != items.end(); iter++) {
                uint32_t length = static_cast<uint32_t>(iter->first.length());
                uint8_t *pstr = (uint8_t *)iter->first.c_str();
                if (!Section::encoding(kValueStyleKey, length, pstr)) {
                    throw DecoderError("decoding style item key error");
                    break;
                }
                length = static_cast<uint32_t>(iter->second.string_value().length());
                const char *pstr_val = iter->second.string_value().c_str();
                if (!Section::encoding(kValueStyleValue, length, (uint8_t *)pstr_val)) {
                    throw DecoderError("decoding style item value error");
                    break;
                }
            }
        }
        finished = true;
        
    } while (0);
    
    return finished;
}

bool SectionStyles::decoding() {
    bool finished = false;
    do {
        fStream *stream = Section::stream();
        if (!stream) {
            break;
        }
        if (stream->Tell() < 0) {
            break;
        }
        std::map<std::string, json11::Json>& styles = decoder()->exec_state()->context()->style_json();
        uint16_t target = 0;
        uint32_t styles_size = 0;
        uint32_t size = sizeof(uint32_t);
        uint32_t readbytes = stream->ReadTarget(&target, (uint8_t *)&styles_size, &size);
        if (!readbytes || target != kValueStyleSize || !styles_size) {
            break;
        }
        for (uint32_t i = 0; i < styles_size; i++) {
            if ((readbytes = stream->ReadTarget(&target, NULL, NULL)) == 0) {
                throw DecoderError("decoding styles target error");
                break;
            }
            if (target != kValueStyleKey) {
                throw DecoderError("decoding styles target error");
                break;
            }
            char *pstr = (char *)malloc(readbytes + 1);
            if (!pstr) {
                throw DecoderError("decoding styles low memory error");
                break;
            }
            memset(pstr, 0, readbytes + 1);
            if (stream->Read(pstr, 1, readbytes) != readbytes) {
                throw DecoderError("decoding styles key error");
                break;
            }
            size = sizeof(uint32_t);
            uint32_t items_size = 0;
            readbytes = stream->ReadTarget(&target, (uint8_t *)&items_size, &size);
            if (!readbytes || target != kValueStyleItemSize) {
                throw DecoderError("decoding styles items size error");
                break;
            }
            std::unordered_map<std::string, json11::Json> items;
            for (uint32_t j = 0; j < items_size; j++) {
                if ((readbytes = stream->ReadTarget(&target, NULL, NULL)) == 0) {
                    throw DecoderError("decoding styles target error");
                    break;
                }
                if (target != kValueStyleKey) {
                    throw DecoderError("decoding styles target error");
                    break;
                }
                char *pstr_key = (char *)malloc(readbytes + 1);
                if (!pstr_key) {
                    throw DecoderError("decoding styles low memory error");
                    break;
                }
                memset(pstr_key, 0, readbytes + 1);
                if (stream->Read(pstr_key, 1, readbytes) != readbytes) {
                    throw DecoderError("decoding styles key error");
                    break;
                }
                if ((readbytes = stream->ReadTarget(&target, NULL, NULL)) == 0) {
                    throw DecoderError("decoding styles target error");
                    break;
                }
                if (target != kValueStyleValue) {
                    throw DecoderError("decoding styles target error");
                    break;
                }
                char *pstr_value = (char *)malloc(readbytes + 1);
                if (!pstr_value) {
                    throw DecoderError("decoding styles low memory error");
                    break;
                }
                memset(pstr_value, 0, readbytes + 1);
                if (stream->Read(pstr_value, 1, readbytes) != readbytes) {
                    throw DecoderError("decoding styles key error");
                    break;
                }
                json11::Json json(pstr_value);
                items.insert(std::make_pair(pstr_key, json));
                free(pstr_key);
                free(pstr_value);
            }
            json11::Json json(std::move(items));
            styles.insert(std::make_pair(pstr, std::move(json)));
            free(pstr);
        }
        finished = true;
        
    } while (0);
    
    return finished;
}

uint32_t SectionVaueRef::size() {
    uint32_t size = 0;
    do {
        std::vector<ValueRef *> &refs = encoder()->exec_state()->refs();
        if (!refs.size()) {
            break;
        }
        size += GetFTLVLength(kValueRefSize, sizeof(uint32_t));
        for (auto ref : refs) {
            Value value;
            value.type = Value::Type::FUNC;
            value.f = ref->func_state();
            uint32_t func_state_length = GetValueLength(&value);
            size += GetFTLVLength(kValueFuncState, func_state_length);
            size += GetFTLVLength(kValueRegister, sizeof(int32_t));
            size += GetFTLVLength(kValueIsClosure, sizeof(uint8_t));
            size += GetFTLVLength(kValueRefFinished, sizeof(uint8_t));
        }
        
    } while (0);
    
    return size;
}

bool SectionVaueRef::encoding() {
    bool finished = false;
    do {
        uint32_t size = this->size();
        if (!size) {
            finished = true;
            break;
        }
        if (!Section::encoding((uint16_t)ExecSection::EXEC_SECTION_VALUEREF, size)) {
            break;
        }
        std::vector<ValueRef *> &refs = encoder()->exec_state()->refs();
        uint32_t refs_size = (uint32_t)refs.size();
        if (!Section::encoding(kValueRefSize, sizeof(uint32_t), &refs_size)) {
            break;
        }
        for (auto ref : refs) {
            Value value;
            value.type = Value::Type::FUNC;
            value.f = ref->func_state();
            uint32_t length = GetValueLength(&value);
            uint8_t *buffer = (uint8_t *)malloc(length);
            if (!buffer) {
                throw EncoderError("encoding low memory error");
                break;
            }
            uint32_t bytes_write = Section::encodingValueToBuffer(buffer, length, &value);
            if (bytes_write != length) {
                throw EncoderError("encoding ref value error");
                break;
            }
            if (!Section::encoding(kValueFuncState, length, buffer)) {
                throw DecoderError("decoding ref value function state error");
                break;
            }
            free(buffer);
            int32_t register_id = static_cast<int32_t>(ref->register_id());
            if (!Section::encoding(kValueRegister, sizeof(int32_t), &register_id)) {
                throw DecoderError("decoding ref value register id error");
                break;
            }
            uint8_t is_closure = ref->is_closure();
            if (!Section::encoding(kValueIsClosure, sizeof(uint8_t), &is_closure)) {
                throw DecoderError("decoding ref value is closure error");
                break;
            }
            uint8_t ref_finished = SECTION_VALUE_FINISHED;
            if (!Section::encoding(kValueRefFinished, sizeof(uint8_t), &ref_finished)) {
                throw DecoderError("decoding ref value finished error");
                break;
            }
        }
        finished = true;
        
    } while (0);
    
    return finished;
}

bool SectionVaueRef::decoding() {
    bool finished = false;
    do {
        fStream *stream = Section::stream();
        if (!stream) {
            break;
        }
        if (stream->Tell() < 0) {
            break;
        }
        std::vector<ValueRef *> &refs = decoder()->exec_state()->refs();
        uint16_t target = 0;
        uint32_t refs_size = 0;
        uint32_t size = sizeof(uint32_t);
        uint32_t readbytes = stream->ReadTarget(&target, (uint8_t *)&refs_size, &size);
        if (!readbytes || target != kValueRefSize || !refs_size) {
            break;
        }
        int64_t seek_end = stream->Tell() + length();
        ValueRef::reset();
        for (uint32_t i = 0; i < refs_size; i++) {
            FuncState *func_state = nullptr;
            ValueRef *ref = nullptr;
            long register_id = -1;
            do {
                if ((readbytes = stream->ReadTarget(&target, NULL, NULL)) == 0) {
                    throw DecoderError("decoding value ref target error");
                    break;
                }
                switch (target) {
                    case kValueFuncState:
                    {
                        uint8_t *buffer = (uint8_t *)malloc(readbytes);
                        if (!buffer) {
                            throw DecoderError("low memory error");
                            break;
                        }
                        if (stream->Read(buffer, 1, readbytes) != readbytes) {
                            throw DecoderError("decoding value ref function state error");
                            break;
                        }
                        Value value;
                        if (!(readbytes = decodingValueFromBuffer(buffer, readbytes, &value)))
                        {
                            throw DecoderError("decoding value ref function state error");
                            break;
                        }
                        if (value.type != Value::Type::FUNC || !value.f) {
                            throw DecoderError("decoding value ref function state error");
                            break;
                        }
                        func_state = value.f;
                        break;
                    }
                    case kValueRegister: {
                        int32_t register_id_reader = -1;
                        if (stream->Read(&register_id_reader, 1, readbytes) != readbytes) {
                            throw DecoderError("decoding value register id error");
                            break;
                        }
                        if (register_id_reader < 0) {
                            throw DecoderError("decoding value register id error");
                        }
                        register_id = register_id_reader;
                        if (func_state && register_id >= 0) {
                            ref = new ValueRef(func_state, register_id);
                        }
                        break;
                    }
                    case kValueIsClosure:
                    {
                        uint8_t is_closure = 0;
                        if (stream->Read(&is_closure, 1, readbytes) != readbytes) {
                            throw DecoderError("decoding value ref is closure error");
                            break;
                        }
                        if (!ref) {
                            throw DecoderError("decoding value ref is closure error");
                        }
                        ref->is_closure() = is_closure;
                        break;
                    }
                    case kValueRefFinished:
                    {
                        uint8_t finished = 0;
                        if (stream->Read(&finished, 1, readbytes) != readbytes) {
                            throw DecoderError("decoding value ref finished error");
                            break;
                        }
                        if (finished != SECTION_VALUE_FINISHED) {
                            throw DecoderError("decoding value ref finished error");
                            break;
                        }
                        break;
                    }
                    default:
                    {
                        if (stream->Seek(readbytes, SEEK_CUR) < 0) {
                            throw DecoderError("decoding value ref length error");
                        }
                        break;
                    }
                }
                if (target == kValueRefFinished) {
                    break;
                }
                
            } while (stream->Tell() < seek_end);
            
            if (ref) {
                refs.push_back(ref);
            }
        }
        std::vector<FuncState *> func_states;
        FuncState *func_state = decoder()->exec_state()->func_state();
        if (func_state) {
            func_states.push_back(func_state);
        }
        std::vector<FuncState *> childrens = func_state->all_childrens();
        for (auto &func : childrens) {
            func_states.push_back(func);
        }
        for (auto &func : func_states) {
            if (func->in_closure_refs().size() > 0) {
                for (int i = 0; i < func->in_closure_refs().size(); i++) {
                    int32_t ref_id = func->in_closure_refs()[i];
                    if (ref_id >= refs.size()) {
                        throw DecoderError("decoding value ref finder error");
                    }
                    func->AddInClosure(refs[ref_id]);
                }
            }
            if (func->out_closure_refs().size() > 0) {
                for (int i = 0; i < func->out_closure_refs().size(); i++) {
                    int32_t ref_id = func->out_closure_refs()[i];
                    if (ref_id >= refs.size()) {
                        throw DecoderError("decoding value ref finder error");
                    }
                    func->AddOutClosure(refs[ref_id]);
                }
            }
        }
        finished = true;
        
    } while (0);
    
    return finished;
}

uint32_t SectionClass::size() {
    uint32_t size = 0;
    do {
        ExecState *exec_state = encoder()->exec_state();
        ClassFactory *class_factory = exec_state->class_factory();
        StringTable *string_table = exec_state->string_table();
        std::vector<ClassDescriptor *> descs = class_factory->descs();
        int32_t class_compile_index = static_cast<int32_t>(exec_state->class_compile_index());
        int32_t descs_size = static_cast<int32_t>(descs.size()) - class_compile_index;
        if (descs_size <= 0) {
            break;
        }
        size += GetFTLVLength(kValueClassSize, sizeof(uint32_t));
        for (int i = class_compile_index; i < descs.size(); i++) {
            ClassDescriptor *desc = descs[i];
            if (desc->p_super_) {
                size += GetFTLVLength(kValueClassSuper, sizeof(int32_t));
            }
            uint32_t static_func_size = static_cast<uint32_t>(desc->statics_->size());
            if (static_func_size > 0) {
                size += GetFTLVLength(kValueClassStaticFunctionSize, sizeof(uint32_t));
                uint32_t static_func_length = 0;
                for (int j = 0; j < static_func_size; j++) {
                    Value *func = desc->statics_->Find(j);
                    if (!func) {
                        throw EncoderError("can't find class static funcs error");
                        break;
                    }
                    static_func_length += GetValueLength(func);
                    static_func_length += GetFTLVLength(kValueClassFunctionKey, sizeof(uint8_t));
                    for (auto &item : desc->statics_->map()) {
                        if (item.second == j) {
                            Value key(string_table->StringFromUTF8(item.first));
                            static_func_length += GetValueLength(&key);
                            break;
                        }
                    }
                }
                size += GetFTLVLength(kValueClassStaticFunction, static_func_length);
            }
            uint32_t class_func_size = static_cast<uint32_t>(desc->funcs_->size());
            if (class_func_size > 0) {
                size += GetFTLVLength(kValueClassMemberFunctionSize, sizeof(uint32_t));
                uint32_t class_func_length = 0;
                for (int j = 0; j < class_func_size; j++) {
                    Value *func = desc->funcs_->Find(j);
                    if (!func) {
                        throw EncoderError("can't find class funcs error");
                        break;
                    }
                    class_func_length += GetValueLength(func);
                    class_func_length += GetFTLVLength(kValueClassFunctionKey, sizeof(uint8_t));
                    for (auto &item : desc->funcs_->map()) {
                        if (item.second == j) {
                            Value key(string_table->StringFromUTF8(item.first));
                            class_func_length += GetValueLength(&key);
                            break;
                        }
                    }
                }
                size += GetFTLVLength(kValueClassMemberFunction, class_func_length);
            }
            size += GetFTLVLength(kValueClassFinished, sizeof(uint8_t));
        }
        
    } while (0);
    
    return size;
}

bool SectionClass::encoding() {
    bool finished = false;
    do {
        uint32_t size = this->size();
        if (!size) {
            finished = true;
            break;
        }
        if (!Section::encoding((uint16_t)ExecSection::EXEC_SECTION_CLASS, size)) {
            break;
        }
        ExecState *exec_state = encoder()->exec_state();
        ClassFactory *class_factory = exec_state->class_factory();
        StringTable *string_table = exec_state->string_table();
        std::vector<ClassDescriptor *> descs = class_factory->descs();
        int32_t class_compile_index = static_cast<int32_t>(exec_state->class_compile_index());
        int32_t descs_size = static_cast<int32_t>(descs.size()) - class_compile_index;
        if (descs_size <= 0) {
            break;
        }
        if (!Section::encoding(kValueClassSize, sizeof(int32_t), &descs_size)) {
            break;
        }
        for (int i = class_compile_index; i < descs.size(); i++) {
            ClassDescriptor *desc = descs[i];
            if (desc->p_super_) {
                int32_t super_index = class_factory->Find(desc->p_super_);
                if (!Section::encoding(kValueClassSuper, sizeof(int32_t), &super_index)) {
                    break;
                }
                LOGD("encoding class super index:%i\n", super_index);
            }
            uint32_t static_func_size = static_cast<uint32_t>(desc->statics_->size());
            if (static_func_size > 0) {
                if (!Section::encoding(kValueClassStaticFunctionSize, sizeof(uint32_t), &static_func_size)) {
                    break;
                }
                LOGD("encoding class static function size:%d\n", static_func_size);
                uint32_t static_func_length = 0;
                for (int j = 0; j < static_func_size; j++) {
                    Value *func = desc->statics_->Find(j);
                    static_func_length += GetValueLength(func);
                    static_func_length += GetFTLVLength(kValueClassFunctionKey, sizeof(uint8_t));
                    for (auto &item : desc->statics_->map()) {
                        if (item.second == j) {
                            Value key(string_table->StringFromUTF8(item.first));
                            static_func_length += GetValueLength(&key);
                            break;
                        }
                    }
                }
                uint8_t *buffer = (uint8_t *)malloc(static_func_length);
                if (!buffer) {
                    throw EncoderError("low memory error");
                    break;
                }
                uint8_t *write_buffer = buffer;
                uint32_t remain_length = static_func_length;
                for (int j = 0; j < static_func_size; j++) {
                    Value *func = desc->statics_->Find(j);
                    uint32_t length = GetValueLength(func);
                    uint32_t bytes_write = Section::encodingValueToBuffer(write_buffer, remain_length, func);
                    if (bytes_write != length) {
                        throw EncoderError("encoding class static func value error");
                        break;
                    }
                    write_buffer += bytes_write;
                    remain_length -= bytes_write;
                    uint8_t has_key_value = false;
                    Value key;
                    for (auto &item : desc->statics_->map()) {
                        if (item.second == j) {
                            LOGD("encoding class static function key:%s\n", item.first.c_str());
                            key = Value(string_table->StringFromUTF8(item.first));
                            has_key_value = true;
                            break;
                        }
                    }
                    if (!(bytes_write = Section::encodingToBuffer(write_buffer, remain_length, kValueClassFunctionKey, sizeof(uint8_t), &has_key_value))) {
                        break;
                    }
                    write_buffer += bytes_write;
                    remain_length -= bytes_write;
                    if (has_key_value) {
                        length = GetValueLength(&key);
                        bytes_write = Section::encodingValueToBuffer(write_buffer, remain_length, &key);
                        if (bytes_write != length) {
                            throw EncoderError("encoding class static func value error");
                            break;
                        }
                        write_buffer += bytes_write;
                        remain_length -= bytes_write;
                    }
                }
                if (!Section::encoding(kValueClassStaticFunction, static_func_length, buffer)) {
                    break;
                }
                free(buffer);
            }
            uint32_t class_func_size = static_cast<uint32_t>(desc->funcs_->size());
            if (class_func_size > 0) {
                if (!Section::encoding(kValueClassMemberFunctionSize, sizeof(uint32_t), &class_func_size)) {
                    break;
                }
                LOGD("encoding class function size:%d\n", class_func_size);
                uint32_t class_func_length = 0;
                for (int j = 0; j < class_func_size; j++) {
                    Value *func = desc->funcs_->Find(j);
                    class_func_length += GetValueLength(func);
                    class_func_length += GetFTLVLength(kValueClassFunctionKey, sizeof(uint8_t));
                    for (auto &item : desc->funcs_->map()) {
                        if (item.second == j) {
                            Value key(string_table->StringFromUTF8(item.first));
                            class_func_length += GetValueLength(&key);
                            break;
                        }
                    }
                }
                uint8_t *buffer = (uint8_t *)malloc(class_func_length);
                if (!buffer) {
                    throw EncoderError("low memory error");
                    break;
                }
                uint8_t *write_buffer = buffer;
                uint32_t remain_length = class_func_length;
                for (int j = 0; j < class_func_size; j++) {
                    Value *func = desc->funcs_->Find(j);
                    uint32_t length = GetValueLength(func);
                    uint32_t bytes_write = Section::encodingValueToBuffer(write_buffer, remain_length, func);
                    if (bytes_write != length) {
                        throw EncoderError("encoding class func value error");
                        break;
                    }
                    write_buffer += bytes_write;
                    remain_length -= bytes_write;
                    uint8_t has_key_value = false;
                    Value key;
                    for (auto &item : desc->funcs_->map()) {
                        if (item.second == j) {
                            key = Value(string_table->StringFromUTF8(item.first));
                            has_key_value = true;
                            break;
                        }
                    }
                    if (!(bytes_write = Section::encodingToBuffer(write_buffer, remain_length, kValueClassFunctionKey, sizeof(uint8_t), &has_key_value))) {
                        break;
                    }
                    write_buffer += bytes_write;
                    remain_length -= bytes_write;
                    if (has_key_value) {
                        length = GetValueLength(&key);
                        bytes_write = Section::encodingValueToBuffer(write_buffer, remain_length, &key);
                        if (bytes_write != length) {
                            throw EncoderError("encoding class func value key error");
                            break;
                        }
                        write_buffer += bytes_write;
                        remain_length -= bytes_write;
                        LOGD("encoding class function key:%s\n", key.str->c_str());
                    }
                }
                if (!Section::encoding(kValueClassMemberFunction, class_func_length, buffer)) {
                    break;
                }
                free(buffer);
            }
            uint8_t class_finished = SECTION_VALUE_FINISHED;
            if (!Section::encoding(kValueClassFinished, sizeof(uint8_t), &class_finished)) {
                throw DecoderError("encoding class finished error");
                break;
            }
        }
        finished = true;
        
    } while (0);
    
    return finished;
}

bool SectionClass::decoding() {
    bool finished = false;
    do {
        fStream *stream = Section::stream();
        if (!stream) {
            break;
        }
        if (stream->Tell() < 0) {
            break;
        }
        ClassFactory *class_factory = decoder()->exec_state()->class_factory();
        uint16_t target = 0;
        uint32_t descs_size = 0;
        uint32_t size = sizeof(uint32_t);
        uint32_t readbytes = stream->ReadTarget(&target, (uint8_t *)&descs_size, &size);
        if (!readbytes || target != kValueClassSize || !descs_size) {
            break;
        }
        int64_t seek_end = stream->Tell() + length();
        std::vector<ClassDescriptor *> descs = class_factory->descs();
        for (uint32_t i = 0; i < descs_size; i++) {
            Value value = class_factory->CreateClassDescriptor(nullptr);
            ClassDescriptor *desc = ValueTo<ClassDescriptor>(&value);
            descs.push_back(desc);
            uint32_t class_funcs_size = 0;
            uint32_t class_static_funcs_size = 0;
            do {
                if ((readbytes = stream->ReadTarget(&target, NULL, NULL)) == 0) {
                    throw DecoderError("decoding class target error");
                    break;
                }
                switch (target) {
                    case kValueClassSuper: {
                        int32_t super_index = -1;
                        if (stream->Read(&super_index, 1, readbytes) != readbytes) {
                            throw DecoderError("decoding class super error");
                            break;
                        }
                        if (super_index < 0 || super_index >= descs.size()) {
                            throw DecoderError("decoding class super error");
                        }
                        desc->p_super_ = descs[super_index];
                        LOGD("decoding class super index:%i\n", super_index);
                        break;
                    }
                    case kValueClassStaticFunctionSize:
                    {
                        if (readbytes != sizeof(uint32_t)) {
                            throw DecoderError("decoding class static funcs size error");
                            break;
                        }
                        if (stream->Read(&class_static_funcs_size, 1, readbytes) != readbytes) {
                            throw DecoderError("decoding class static funcs size error");
                            break;
                        }
                        LOGD("decoding class static function size:%d\n", class_static_funcs_size);
                        break;
                    }
                    case kValueClassStaticFunction:
                    {
                        if (!class_static_funcs_size) {
                            throw DecoderError("decoding class static funcs size error");
                            break;
                        }
                        uint8_t *buffer = (uint8_t *)malloc(readbytes);
                        if (!buffer) {
                            throw DecoderError("low memory error");
                            break;
                        }
                        if (stream->Read(buffer, 1, readbytes) != readbytes) {
                            throw DecoderError("decoding class static func error");
                            break;
                        }
                        uint8_t *read_buffer = buffer;
                        uint32_t remain_length = readbytes;
                        uint32_t bytes_read = 0;
                        for (int j = 0; j < class_static_funcs_size; j++) {
                            Value func, key;
                            if (!(bytes_read = decodingValueFromBuffer(read_buffer, remain_length, &func))) {
                                throw DecoderError("decoding class static func payload error");
                                break;
                            }
                            read_buffer += bytes_read;
                            remain_length -= bytes_read;
                            uint8_t has_key_value = false;
                            uint32_t bytes_decoding = sizeof(uint8_t);
                            if (!(bytes_read = decodingFromBuffer(read_buffer, remain_length, &target, (uint8_t *)&has_key_value, &bytes_decoding))) {
                                break;
                            }
                            if (target != kValueClassFunctionKey) {
                                throw DecoderError("decoding class static func key error");
                                break;
                            }
                            read_buffer += bytes_read;
                            remain_length -= bytes_read;
                            if (has_key_value) {
                                if (!(bytes_read = decodingValueFromBuffer(read_buffer, remain_length, &key))) {
                                    throw DecoderError("decoding class static func key error");
                                    break;
                                }
                                read_buffer += bytes_read;
                                remain_length -= bytes_read;
                                if (!IsString(&key)) {
                                    throw DecoderError("decoding class static func key error");
                                    break;
                                }
                                desc->statics_->Add(key.str->c_str(), func);
                            }
                            else {
                                desc->statics_->Add(func);
                            }
                        }
                        free(buffer);
                        break;
                    }
                    case kValueClassMemberFunctionSize:
                    {
                        if (readbytes != sizeof(uint32_t)) {
                            throw DecoderError("decoding class funcs size error");
                            break;
                        }
                        if (stream->Read(&class_funcs_size, 1, readbytes) != readbytes) {
                            throw DecoderError("decoding class funcs size error");
                            break;
                        }
                        LOGD("decoding class function size:%d\n", class_funcs_size);
                        break;
                    }
                    case kValueClassMemberFunction:
                    {
                        if (!class_funcs_size) {
                            throw DecoderError("decoding class funcs size error");
                            break;
                        }
                        uint8_t *buffer = (uint8_t *)malloc(readbytes);
                        if (!buffer) {
                            throw DecoderError("low memory error");
                            break;
                        }
                        if (stream->Read(buffer, 1, readbytes) != readbytes) {
                            throw DecoderError("decoding class func error");
                            break;
                        }
                        uint8_t *read_buffer = buffer;
                        uint32_t remain_length = readbytes;
                        uint32_t bytes_read = 0;
                        for (int j = 0; j < class_funcs_size; j++) {
                            Value func, key;
                            if (!(bytes_read = decodingValueFromBuffer(read_buffer, remain_length, &func))) {
                                throw DecoderError("decoding class func payload error");
                                break;
                            }
                            if (func.type != Value::Type::FUNC) {
                                throw DecoderError("decoding class func payload error");
                                break;
                            }
                            read_buffer += bytes_read;
                            remain_length -= bytes_read;
                            uint8_t has_key_value = false;
                            uint32_t bytes_decoding = sizeof(uint8_t);
                            if (!(bytes_read = decodingFromBuffer(read_buffer, remain_length, &target, (uint8_t *)&has_key_value, &bytes_decoding))) {
                                break;
                            }
                            read_buffer += bytes_read;
                            remain_length -= bytes_read;
                            if (target != kValueClassFunctionKey) {
                                throw DecoderError("decoding class func key error");
                                break;
                            }
                            if (has_key_value) {
                                if (!(bytes_read = decodingValueFromBuffer(read_buffer, remain_length, &key))) {
                                    throw DecoderError("decoding class func key error");
                                    break;
                                }
                                if (!IsString(&key)) {
                                    throw DecoderError("decoding class static func key error");
                                    break;
                                }
                                read_buffer += bytes_read;
                                remain_length -= bytes_read;
                                desc->funcs_->Add(key.str->c_str(), func);
                            }
                            else {
                                desc->funcs_->Add(func);
                            }
                        }
                        free(buffer);
                        break;
                    }
                    case kValueClassFinished:
                    {
                        uint8_t class_finshed = 0;
                        if (stream->Read(&class_finshed, 1, readbytes) != readbytes) {
                            throw DecoderError("decoding class finished error");
                            break;
                        }
                        if (class_finshed != SECTION_VALUE_FINISHED) {
                            throw DecoderError("decoding class finished error");
                            break;
                        }
                        break;
                    }
                    default:
                    {
                        if (stream->Seek(readbytes, SEEK_CUR) < 0) {
                            throw DecoderError("decoding value ref length error");
                        }
                        break;
                    }
                }
                if (target == kValueClassFinished) {
                    break;
                }
                
            } while (stream->Tell() < seek_end);
            
        }
        finished = true;
        
    } while (0);
    
    return finished;
}
    
}  // namespace data_render
}  // namespace core
}  // namespace weex
