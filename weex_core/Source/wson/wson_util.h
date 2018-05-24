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
// Created by furture on 2018/5/15.
//

#ifndef WSON_UTIL_H
#define WSON_UTIL_H

#include <cstdint>
#include <string>

namespace wson{

    /**
     *  unicode to utf8 convertor with zero dependency inspired by java sdk character source
     * */
    void utf16_convert_to_utf8_string(uint16_t *utf16, int length, char* decodingBuffer, std::string& utf8);
    void utf16_convert_to_utf8_quote_string(uint16_t *utf16, int length, char* decodingBuffer, std::string& utf8);

    /**
     *  unicode to utf8 convertor with zero dependency inspired by java sdk character source
     * */
    void utf16_convert_to_utf8_string(uint16_t *utf16, int length, std::string& utf8);
    void utf16_convert_to_utf8_quote_string(uint16_t *utf16, int length, std::string& utf8);
    /**
     * return byte count in utf8, buffer size should can contains convert values
     * */
    int utf16_convert_to_utf8_cstr(uint16_t *utf16, int length, char* buffer);
    int utf16_convert_to_utf8_quote_cstr(uint16_t *utf16, int length, char* buffer);

    /**
     * append support double float int32 int64
     * */
    void str_append_number(std::string& str, double  num);
    void str_append_number(std::string& str, float  num);
    void str_append_number(std::string& str, int32_t  num);
    void str_append_number(std::string& str, int64_t  num);
}


#endif
