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

#pragma once

#include "stdlib.h"

struct WeexString {
    uint32_t length;
    uint16_t content[1];
};

struct WeexByteArray {
    uint32_t length;
    char content[1];
};

enum class ParamsType {
    INT32 = 1,
    INT64,
    FLOAT,
    DOUBLE,
    JSONSTRING,
    STRING,
    BYTEARRAY, /* terminated with zero. */
    VOID,
    JSUNDEFINED,
    BYTEARRAYSTRING,
    BYTEARRAYJSONSTRING,
    END,
};

typedef union ExecJsParamValue {
    int32_t int32Value;
    int64_t int64Value;
    float floatValue;
    double doubleValue;
    WeexString *string;
    WeexByteArray *byteArray;
} EXEC_JS_PARAM_VALUE;

typedef struct ValueWithType {
    ParamsType type;
    EXEC_JS_PARAM_VALUE value;

    ValueWithType(): type(ParamsType::JSUNDEFINED) {}
    
    // ValueWithType((int32_t)-1) normally represents failure for call native weex jsframework
    ValueWithType(int32_t intValue): type(ParamsType::INT32), value({.int32Value = intValue}) {}
} VALUE_WITH_TYPE;
