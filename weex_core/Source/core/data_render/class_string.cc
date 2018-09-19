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

#include <stdlib.h>
#include <algorithm>
#include "core/data_render/class_string.h"
#include "core/data_render/class.h"
#include "core/data_render/class_array.h"
#include "core/data_render/exec_state.h"
#include "core/data_render/common_error.h"
#include "core/data_render/table.h"
#include "core/data_render/string_table.h"
#include <base/LogDefines.h>

namespace weex {
namespace core {
namespace data_render {

static Value split(ExecState *exec_state);
static Value trim(ExecState* exec_state);
static Value indexOf(ExecState* exec_state);

ClassDescriptor *NewClassString() {
    ClassDescriptor *array_desc = new ClassDescriptor(nullptr);
    AddClassCFunc(array_desc, "split", split);
    AddClassCFunc(array_desc, "trim", trim);
    AddClassCFunc(array_desc, "indexOf", indexOf);
    return array_desc;
}
    
template <class Container>
void split_string(const std::string& str, Container& container, const std::string& delims = " ")
{
    std::size_t current, previous = 0;
    current = str.find(delims);
    while (current != std::string::npos) {
        container.push_back(str.substr(previous, current - previous));
        previous = current + delims.length();
        current = str.find(delims, previous);
    }
    if (str.length() > previous) {
        container.push_back(str.substr(previous, str.length() - previous));
    }
}
 
static Value split(ExecState *exec_state) {
    Value ret = exec_state->class_factory()->CreateArray();
    do {
        size_t length = exec_state->GetArgumentCount();
        if (length < 2) {
            break;
        }
        Value *string = exec_state->GetArgument(0);
        if (!IsString(string)) {
            throw VMExecError("split caller isn't a string");
        }
        Value *split = exec_state->GetArgument(1);
        if (!IsString(split)) {
            throw VMExecError("split caller isn't a string");
        }
        std::string src = CStringValue(string);
        std::string delim = CStringValue(split);
        std::vector<std::string>split_array;
        split_string<std::vector<std::string>>(src, split_array, delim);
        Array *array = ValueTo<Array>(&ret);
        for (int i = 0; i < split_array.size(); i++) {
            Value string_value = exec_state->string_table()->StringFromUTF8(split_array[i]);
            array->items.push_back(string_value);
        }
        
    } while (0);
    
    return ret;
}
    
std::string& trim(std::string &s) {
    if (s.empty()) {
        return s;
    }
    
    s.erase(0, s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
    return s;
}

static Value trim(ExecState* exec_state) {
    size_t length = exec_state->GetArgumentCount();
    if (length != 1) {
        throw VMExecError("trim caller args wrong");
    }
    Value *string = exec_state->GetArgument(0);
    if (!IsString(string)) {
        throw VMExecError("trim caller isn't a string");
    }
    
    std::string src = CStringValue(string);
    trim(src);
    
    Value string_value = exec_state->string_table()->StringFromUTF8(src);
    return string_value;
}
    
/* Converts a hex character to its integer value */
char from_hex(char ch) {
    return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

/* Converts an integer value to its hex character*/
char to_hex(char code) {
    static char hex[] = "0123456789abcdef";
    return hex[code & 15];
}

/* Returns a url-encoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char *url_encode(char *str) {
    char *pstr = str, *buf = (char *)malloc(strlen(str) * 3 + 1), *pbuf = buf;
    while (*pstr) {
        if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~')
            *pbuf++ = *pstr;
        else if (*pstr == ' ')
            *pbuf++ = '+';
        else
            *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
        pstr++;
    }
    *pbuf = '\0';
    return buf;
}
    
/* Returns a url-decoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char *url_decode(char *str) {
    char *pstr = str, *buf = (char *)malloc(strlen(str) + 1), *pbuf = buf;
    while (*pstr) {
        if (*pstr == '%') {
            if (pstr[1] && pstr[2]) {
                *pbuf++ = from_hex(pstr[1]) << 4 | from_hex(pstr[2]);
                pstr += 2;
            }
        } else if (*pstr == '+') {
            *pbuf++ = ' ';
        } else {
            *pbuf++ = *pstr;
        }
        pstr++;
    }
    *pbuf = '\0';
    return buf;
}
    
Value encodeURIComponent(ExecState *exec_state) {
    size_t length = exec_state->GetArgumentCount();
    if (length < 1) {
        throw VMExecError("trim caller args wrong");
    }
    Value *string = exec_state->GetArgument(0);
    if (!IsString(string)) {
        throw VMExecError("trim caller isn't a string");
    }
    std::string src = CStringValue(string);
    char *uri = url_encode((char *)src.c_str());
    std::string dst = src;
    if (uri) {
        dst = uri;
        free(uri);
    }
    return exec_state->string_table()->StringFromUTF8(dst);
}

Value indexOf(ExecState* exec_state) {
    size_t length = exec_state->GetArgumentCount();
    if (length != 2) {
        throw VMExecError("trim caller args wrong");
    }

    Value* string = exec_state->GetArgument(0);
    Value* subs = exec_state->GetArgument(1);
    if (!IsString(string) || !IsString(subs)) {
        throw VMExecError("trim caller isn't a string");
    }

    std::string src = CStringValue(string);
    std::string sub_src = CStringValue(subs);

    auto pos = src.find(sub_src);
    if (pos == std::string::npos) {
        return Value(-1);
    }

    return Value(static_cast<int64_t>(pos));
}
}  // namespace data_render
}  // namespace core
}  // namespace weex
