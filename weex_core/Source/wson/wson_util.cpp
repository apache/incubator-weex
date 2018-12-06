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

#include "wson_util.h"
#include <stdio.h>


namespace wson {

    /**
     * see java jdk source to handle handle utf-16 in 4 byte
     * */
    static const u_int16_t  MIN_HIGH_SURROGATE = 0xD800;

    static const u_int16_t MAX_HIGH_SURROGATE = 0xDBFF;

    static const u_int16_t  MIN_LOW_SURROGATE  = 0xDC00;

    static const u_int16_t MAX_LOW_SURROGATE  = 0xDFFF;

    static const u_int32_t MIN_SUPPLEMENTARY_CODE_POINT = 0x010000;

    inline bool isHighSurrogate(u_int16_t ch) {
        return ch >= MIN_HIGH_SURROGATE && ch < (MAX_HIGH_SURROGATE + 1);
    }

    inline bool isLowSurrogate(u_int16_t ch) {
        return ch >= MIN_LOW_SURROGATE && ch < (MAX_LOW_SURROGATE + 1);
    }

    inline u_int32_t toCodePoint(u_int16_t high, u_int16_t low) {
        // Optimized form of:
        // return ((high - MIN_HIGH_SURROGATE) << 10)
        //         + (low - MIN_LOW_SURROGATE)
        //         + MIN_SUPPLEMENTARY_CODE_POINT;
        return ((high << 10) + low) + (MIN_SUPPLEMENTARY_CODE_POINT
                                       - (MIN_HIGH_SURROGATE << 10)
                                       - MIN_LOW_SURROGATE);
    }


    static inline int utf16_char_convert_to_utf8_cstr(u_int32_t codePoint, char* utf8){
        if (codePoint <= 0x7F)
        {
            // Plain single-byte ASCII.
            utf8[0] = ((char)codePoint);
            return 1;
        }
        else if (codePoint <= 0x7FF)
        {
            // Two bytes.
            utf8[0] = (0xC0 | (codePoint >> 6));
            utf8[1] = (0x80 | ((codePoint) & 0x3F));
            return 2;
        }
        else if (codePoint <= 0xFFFF)
        {
            // Three bytes.
            utf8[0] = (0xE0 | (codePoint >> 12));
            utf8[1] = ((0x80 | ((codePoint >> 6) & 0x3F)));
            utf8[2] = ((0x80 | ((codePoint) & 0x3F)));
            return 3;
        }
        else if (codePoint <= 0x1FFFFF)
        {
            // Four bytes.
            utf8[0] = (0xF0 | (codePoint >> 18));
            utf8[1] = (0x80 | ((codePoint >> 12) & 0x3F));
            utf8[2] = (0x80 | ((codePoint >> 6) & 0x3F));
            utf8[3] = (0x80 | ((codePoint) & 0x3F));
            return 4;
        }
        else if (codePoint <= 0x3FFFFFF)
        {
            // Five bytes.
            utf8[0] = (0xF8 | (codePoint >> 24));
            utf8[1] = (0x80 | ((codePoint >> 18) & 0x3F));
            utf8[2] = (0x80 | ((codePoint >> 12) & 0x3F));
            utf8[3] = (0x80 | ((codePoint >> 6) & 0x3F));
            utf8[4] = (0x80 | ((codePoint) & 0x3F));
            return 5;
        }
        else if (codePoint  <= 0x7FFFFFFF)
        {
            // Six bytes.
            utf8[0] = (0xFC | (codePoint >> 30));
            utf8[1] = (0x80 | ((codePoint >> 24) & 0x3F));
            utf8[2] = (0x80 | ((codePoint >> 18) & 0x3F));
            utf8[3] = (0x80 | ((codePoint >> 12) & 0x3F));
            utf8[4] = (0x80 | ((codePoint >> 6) & 0x3F));
            utf8[5] = (0x80 | ((codePoint) & 0x3F));
            return 6;
        }
        return 0;
    }

    void utf16_convert_to_utf8_string(uint16_t * utf16, int length, std::string& utf8){
        char* dest = new char[length*4 + 4];
        utf16_convert_to_utf8_string(utf16, length, dest, utf8);
        delete [] dest;
    }

    void utf16_convert_to_utf8_quote_string(uint16_t *utf16, int length, std::string& utf8){
        char* dest = new char[length*4 + 4];
        utf16_convert_to_utf8_quote_string(utf16, length, dest, utf8);
        delete [] dest;
    }


    void utf16_convert_to_utf8_string(uint16_t *utf16, int length, char* decodingBuffer, std::string& utf8){
        int count = utf16_convert_to_utf8_cstr(utf16, length, decodingBuffer);
        utf8.append(decodingBuffer, count);
    }
    void utf16_convert_to_utf8_quote_string(uint16_t *utf16, int length, char* decodingBuffer, std::string& utf8){
        int count = utf16_convert_to_utf8_quote_cstr(utf16, length, decodingBuffer);
        utf8.append(decodingBuffer, count);
    }

    int utf16_convert_to_utf8_cstr(uint16_t * utf16, int length, char* buffer){
        char* src = buffer;
        int count =0;
        for(int i=0; i<length;){
            u_int16_t c1 = utf16[i++];
            if(isHighSurrogate(c1)){
                if(i < length){
                    u_int16_t c2 = utf16[i++];
                    if (isLowSurrogate(c2)) {
                        u_int32_t codePoint =  toCodePoint(c1, c2);
                        count += utf16_char_convert_to_utf8_cstr(codePoint, src + count);
                        continue;
                    }else{
                        i--;
                    }
                }
            }
            count += utf16_char_convert_to_utf8_cstr(c1, src + count);
        }
        src[count] = '\0';
        return count;
    }

    int utf16_convert_to_utf8_quote_cstr(uint16_t *utf16, int length, char* buffer){
        int count =0;

        char* src = buffer;
        src[count++] = '"';
        for(int i=0; i<length;){
            u_int16_t c1 = utf16[i++];
            if(isHighSurrogate(c1)){
                if(i < length){
                    u_int16_t c2 = utf16[i++];
                    if (isLowSurrogate(c2)) {
                        u_int32_t codePoint =  toCodePoint(c1, c2);
                        count += utf16_char_convert_to_utf8_cstr(codePoint, src + count);
                        continue;
                    }else{
                        i--;
                    }
                }
            }
            if(c1 < 0x5D){ // 0X5C is '\'
                if(c1 == '"' || c1 == '\\'){
                    src[count++] = '\\';
                }else{
                    if(c1 <= 0x1F){ //max control latter
                        switch (c1){
                            case '\t':
                                src[count++] = '\\';
                                src[count++] = 't';
                                continue;
                            case '\r':
                                src[count++] = '\\';
                                src[count++] = 'r';
                                continue;
                            case '\n':
                                src[count++] = '\\';
                                src[count++] = 'n';
                                continue;
                            case '\f':
                                src[count++] = '\\';
                                src[count++] = 'f';
                                continue;
                            case '\b':
                                src[count++] = '\\';
                                src[count++] = 'b';
                                continue;
                        }
                    }
                }
            }
            count += utf16_char_convert_to_utf8_cstr(c1, src + count);
        }
        src[count++] = '"';
        src[count] = '\0';
        return count;
    }


    /** min size is 32 + 1 = 33 */
    inline void number_to_buffer(char* buffer, int32_t num){
        snprintf(buffer, 32,"%d", num);
    }

    /** min size is 64 + 1 = 65 */
    inline void number_to_buffer(char* buffer, float num){
        snprintf(buffer, 64, "%f", num);
    }

    /** min size is 64 + 1 = 65 */
    inline void number_to_buffer(char* buffer, double num){
        snprintf(buffer, 64, "%f", num);
    }

    /** min size is 64 + 1 = 65 */
    inline void number_to_buffer(char* buffer, int64_t num){
        snprintf(buffer, 64, "%lld", num);
    }


    void str_append_number(std::string& str, double  num){
        char src[64 + 2];
        char* buffer = src;
        number_to_buffer(buffer, num);
        str.append(src);
    }

    void str_append_number(std::string& str, float  num){
        char src[64 + 2];
        char* buffer = src;
        number_to_buffer(buffer, num);
        str.append(src);
    }

    void str_append_number(std::string& str, int32_t  num){
        char src[32 + 2];
        char* buffer = src;
        number_to_buffer(buffer, num);
        str.append(src);
    }

    void str_append_number(std::string& str, int64_t  num){
        char src[64 + 2];
        char* buffer = src;
        number_to_buffer(buffer, num);
        str.append(src);
    }


}
