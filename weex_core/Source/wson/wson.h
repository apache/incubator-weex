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
// Created by furture on 2017/8/4.
//

#ifndef WSON_H
#define WSON_H

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef struct wson_buffer{
    void* data;
    uint32_t position;
    uint32_t length;
} wson_buffer;




/**
 * wson data type
 * */
#define  WSON_NULL_TYPE   '0'
#define  WSON_STRING_TYPE  's'
#define  WSON_UINT8_STRING_TYPE 'u'
#define  WSON_BOOLEAN_TYPE_TRUE 't'
#define  WSON_BOOLEAN_TYPE_FALSE 'f'
#define  WSON_NUMBER_INT_TYPE  'i'
#define  WSON_NUMBER_FLOAT_TYPE  'F'
#define  WSON_NUMBER_DOUBLE_TYPE  'd'
#define  WSON_NUMBER_LONG_TYPE  'l'
#define  WSON_NUMBER_BIG_INT_TYPE  'g'
#define  WSON_NUMBER_BIG_DECIMAL_TYPE  'e'
#define  WSON_ARRAY_TYPE  '['
#define  WSON_MAP_TYPE   '{'
#define  WSON_EXTEND_TYPE   'b'

/**
 * create wson buffer
 * */
wson_buffer* wson_buffer_new(void);

void wson_buffer_require(wson_buffer *buffer, size_t size);
/**
 * push value with type signature; 1 true, 0 false, with type WSON_BOOLEAN_TYPE
 * signature  + byte
 */
void wson_push_type_boolean(wson_buffer *buffer, uint8_t value);
void wson_push_type_int(wson_buffer *buffer, int32_t num);
void wson_push_type_long(wson_buffer *buffer, int64_t num);
void wson_push_type_double(wson_buffer *buffer, double num);
void wson_push_type_float(wson_buffer *buffer, float num);
void wson_push_type_string(wson_buffer *buffer, const void *src, int32_t length);
void wson_push_type_uint8_string(wson_buffer *buffer, const uint8_t *src, int32_t length);
void wson_push_type_null(wson_buffer *buffer);
void wson_push_type_map(wson_buffer *buffer, uint32_t size);
void wson_push_type_array(wson_buffer *buffer, uint32_t size);
void wson_push_type_extend(wson_buffer *buffer, const void *src, int32_t length);
void wson_push_ensure_size(wson_buffer *buffer, uint32_t dataSize);
void wson_push_type_string_length(wson_buffer *buffer, int32_t length);
void wson_push_property(wson_buffer *buffer, const void *src, int32_t length);
    
/**
 * push int, varint uint byte int double bts to buffer, without type signature
 * */
void wson_push_int(wson_buffer *buffer, int32_t num);
void wson_push_uint(wson_buffer *buffer, uint32_t num);
void wson_push_byte(wson_buffer *buffer, uint8_t bt);
void wson_push_type(wson_buffer *buffer, uint8_t bt);
void wson_push_double(wson_buffer *buffer, double num);
void wson_push_float(wson_buffer *buffer, float num);
void wson_push_ulong(wson_buffer *buffer, uint64_t num);
void wson_push_bytes(wson_buffer *buffer, const void *src, int32_t length);


/**
 * free  buffer
 * */
void wson_buffer_free(wson_buffer *buffer);


/**
 * parse buffer, return data from current position not include signature
 * */
int8_t wson_next_byte(wson_buffer *buffer);
int8_t wson_next_type(wson_buffer *buffer);
int32_t wson_next_int(wson_buffer *buffer);
uint32_t wson_next_uint(wson_buffer *buffer);
double wson_next_double(wson_buffer *buffer);
float wson_next_float(wson_buffer *buffer);
int64_t wson_next_long(wson_buffer *buffer);
uint64_t wson_next_ulong(wson_buffer *buffer);
uint8_t* wson_next_bts(wson_buffer *buffer, uint32_t length);
inline bool wson_has_next(wson_buffer *buffer){
    return buffer->position < buffer->length;
}

/** constructor with data */
wson_buffer* wson_buffer_from(void* data, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif //WSON_H
