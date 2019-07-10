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
//
// Created by chenpeihan on 2019/2/25.
//

#include <wson_util.h>
#include "wson/wson.h"
#include "wson/wson_parser.h"
#include "wson_for_runtime.h"
#include "base/string_util.h"

#include "js_runtime/utils/log_utils.h"



inline static std::string wson16ToString(uint16_t *src, int length) {
    std::string str;
    auto * decodeBuffer = new char[length * 2];
    wson::utf16_convert_to_utf8_string(src, length/sizeof(uint16_t), decodeBuffer, str);
    delete []decodeBuffer;
    return str;
}

namespace wson {

    unicorn::RuntimeValues *
    convertWsonToRuntimeValue(unicorn::EngineContext *context, wson_buffer *buffer) {
        uint8_t type = wson_next_type(buffer);
        switch (type) {
            case WSON_UINT8_STRING_TYPE: {
                LOG_CONVERSION("[toRunTimeValueFromWson][string][start]");
                int size = wson_next_uint(buffer);
                uint8_t *utf8 = wson_next_bts(buffer, size);
                std::string string_utf_8 = std::string(reinterpret_cast<char *>(utf8),  size);
                LOG_CONVERSION("[toRunTimeValueFro  mWson][string][end] :%s", string_utf_8.c_str());
                return new unicorn::RuntimeValues(string_utf_8);
            }
            case WSON_STRING_TYPE:
            case WSON_NUMBER_BIG_INT_TYPE:
            case WSON_NUMBER_BIG_DECIMAL_TYPE: {
                LOG_CONVERSION("[toRunTimeValueFromWson][string_utf_16][start]");
                int length = wson_next_uint(buffer);
                uint16_t * utf16 = ( uint16_t *)wson_next_bts(buffer, length);
                std::string string_utf_8 = wson16ToString(utf16,length);
//                LOGE("dyydyy [toRunTimeValueFromWson][string_utf_16][end] :%s", string_utf_8.c_str());
                return new unicorn::RuntimeValues(string_utf_8);
            }
                break;
            case WSON_ARRAY_TYPE: {
                LOG_CONVERSION("[toRunTimeValueFromWson][array][start]");
                uint32_t length = wson_next_uint(buffer);
                auto runtime_array = unicorn::Array::CreateFromNative(context, unicorn::RuntimeValues::MakeNull());
                for (uint32_t i = 0; i < length; i++) {
                    if (wson_has_next(buffer)) {
                        runtime_array->PushBack(convertWsonToRuntimeValue(context, buffer));
                    } else {
                        break;
                    }
                }
                LOG_CONVERSION("[toRunTimeValueFromWson][array][end]");
                return new unicorn::RuntimeValues(std::move(runtime_array));
            }
                break;
            case WSON_MAP_TYPE: {
                LOG_CONVERSION("[toRunTimeValueFromWson][map][start]");
                uint32_t length = wson_next_uint(buffer);
                std::unique_ptr<unicorn::Map> runtime_map = unicorn::Map::CreateFromNative(context,
                                                                                           unicorn::RuntimeValues::MakeNull());
                for (uint32_t i = 0; i < length; i++) {
                    if (wson_has_next(buffer)) {
                        int length = wson_next_uint(buffer);
                        uint16_t * utf16 = ( uint16_t *)wson_next_bts(buffer, length);
                        std::string name_utf_8 = wson16ToString(utf16,length);
                        LOG_CONVERSION("[toRunTimeValueFromWson][map][itemkey] :%s", name_utf_8.c_str());
                        runtime_map->Insert(name_utf_8, convertWsonToRuntimeValue(context, buffer));

                    } else {
                        break;
                    }
                }
                LOG_CONVERSION("[toRunTimeValueFromWson][map][end]");
                return new unicorn::RuntimeValues(std::move(runtime_map));
            }
                break;
            case WSON_NUMBER_INT_TYPE: {
                int32_t num = wson_next_int(buffer);
                return new unicorn::RuntimeValues(num);
            }
                break;
            case WSON_BOOLEAN_TYPE_TRUE: {
                return new unicorn::RuntimeValues(true);
            }
                break;
            case WSON_BOOLEAN_TYPE_FALSE: {
                return new unicorn::RuntimeValues(false);
            }
                break;
            case WSON_NUMBER_DOUBLE_TYPE: {
                double num = wson_next_double(buffer);
                return new unicorn::RuntimeValues(num);
            }
                break;
            case WSON_NUMBER_FLOAT_TYPE: {
                float num = wson_next_float(buffer);
                return new unicorn::RuntimeValues(num);
            }
                break;
            case WSON_NUMBER_LONG_TYPE: {
                int64_t num = wson_next_long(buffer);
                return new unicorn::RuntimeValues((double) num);
            }
                break;
            case WSON_NULL_TYPE: {
                return new unicorn::RuntimeValues(nullptr);
            }
            default: {
                LOGE("[WsonToRuntimeValue] unsupport data type in default case!!");
                return new unicorn::RuntimeValues(nullptr);
            }
        }
    }

    unicorn::ScopeValues toRunTimeValueFromWson(unicorn::EngineContext *context, void *data, int length) {
        wson_buffer *buffer = wson_buffer_from(data, length);
        auto ret = convertWsonToRuntimeValue(context, buffer);

#ifdef LOG_CONVERSION_SWITCH
        wson_parser parser((char *) buffer->data);
        LOG_CONVERSION("[WeexValueToRuntimeValue][wson] :%s", parser.toStringUTF8().c_str());
#endif



        buffer->data = nullptr;
        wson_buffer_free(buffer);
        return unicorn::ScopeValues(ret);
    }

    void pushStringToWsonBuffer(wson_buffer *buffer, std::string str_utf_8) {
        std::u16string s = weex::base::to_utf16(const_cast<char *>(str_utf_8.c_str()), str_utf_8.length());
        size_t length = s.length();
        wson_push_type(buffer, WSON_STRING_TYPE);
        wson_push_uint(buffer, length * sizeof(uint16_t));
        wson_push_bytes(buffer, s.c_str(), s.length() * sizeof(uint16_t));
    }

    void pushMapKeyToBuffer(wson_buffer *buffer, std::string str_utf_8) {
      std::u16string s = weex::base::to_utf16(const_cast<char *>(str_utf_8.c_str()), str_utf_8.length());
      size_t length = s.length();
      wson_push_uint(buffer, length * sizeof(uint16_t));
      wson_push_bytes(buffer, s.c_str(), s.length() * sizeof(uint16_t));
    }


    void putValuesToWson(unicorn::RuntimeValues *value, wson_buffer *buffer) {
        if (value->IsUndefined() || value->IsNull()) {
            wson_push_type_null(buffer);
        } else if (value->IsInt()) {
            int num = -1;
            value->GetAsInteger(&num);
            wson_push_type_int(buffer, num);
        } else if (value->IsDouble()) {
            double num = -1;
            value->GetAsDouble(&num);
            wson_push_type_double(buffer, num);
        } else if (value->IsBool()) {
            bool flag = false;
            value->GetAsBoolean(&flag);
            wson_push_type_boolean(buffer, flag ? 1 : 0);
        } else if (value->IsString()) {
            std::string str_utf_8;
            value->GetAsString(&str_utf_8);
            pushStringToWsonBuffer(buffer, str_utf_8);
        } else if (value->IsArray()) {
            auto array = value->GetAsArray();
            uint32_t length = array->Size();
            wson_push_type_array(buffer, length);
            for (uint32_t i = 0; i < length; i++) {
                auto item = array->atIndex(i);
                putValuesToWson(item, buffer);
            }
        } else if (value->IsMap()) {
            auto map = value->GetAsMap()->GetMap();
            uint32_t map_size = map.size();
            uint32_t undefinedOrFunctionSize = 0;
            for (auto item:map) {
                if (item.second->IsUndefined() || item.second->IsNull() || item.second->IsFunction() ||
                    item.second->IsObject()) {
                    undefinedOrFunctionSize++;
                    LOG_CONVERSION("[wson]putValuesToWson data type not match ,type :%d ", item.second->GetType());
                }
            }
            wson_push_type_map(buffer, map_size - undefinedOrFunctionSize);
            for (auto item:map) {
                if (item.second->IsUndefined() || item.second->IsNull() || item.second->IsFunction() ||
                    item.second->IsObject()) {
                    continue;
                }
                pushMapKeyToBuffer(buffer, item.first);
                putValuesToWson(item.second, buffer);
            }
        } else {
            LOGE("[wson][else] putValuesToWson data type not match ,type :%d ", value->GetType());
        }
    }


    wson_buffer *runTimeValueToWson(unicorn::RuntimeValues *value) {
        wson_buffer *buffer = wson_buffer_new();
        putValuesToWson(value, buffer);
        return buffer;
    }
}