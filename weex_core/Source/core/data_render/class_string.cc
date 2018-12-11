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
#include <regex>
#include <algorithm>
#include "core/data_render/class_string.h"
#include "core/data_render/class.h"
#include "core/data_render/class_array.h"
#include "core/data_render/exec_state.h"
#include "core/data_render/common_error.h"
#include "core/data_render/table.h"
#include "core/data_render/string_table.h"
#include "base/LogDefines.h"

namespace weex {
namespace core {
namespace data_render {

static Value split(ExecState *exec_state);
static Value trim(ExecState* exec_state);
static Value indexOf(ExecState* exec_state);
static Value search(ExecState* exec_state);
static Value replaceAll(ExecState *exec_state);
static Value replace(ExecState *exec_state);
static Value match(ExecState *exec_state);

static ClassInstance* to_regex_inst(ExecState *exec_state, Value* v){
  if (!IsClassInstance(v)){
    throw VMExecError("not a regex instance");
  }
  ClassInstance* instance = ValueTo<ClassInstance>(v);
  int index = exec_state->global()->IndexOf("RegExp");
  if (index < 0) {
    throw VMExecError("split lost RegExp define");
  }
  Value* reg_define = exec_state->global()->Find(index);
  if (!IsClass(reg_define)) {
    throw VMExecError("split not a RegExp define");
  }
  if (instance->p_desc_ != ValueTo<ClassDescriptor>(reg_define)) {
    throw VMExecError("split not a RegExp instance");
  }
  return instance;
}

std::vector<std::string> split_regex(const std::string& input, const std::string& regex, const std::string& flag) {
    // passing -1 as the submatch index parameter performs splitting
    std::regex_constants::syntax_option_type type = std::regex_constants::ECMAScript;
    if (flag.find('i') != std::string::npos){
        type |= std::regex_constants::icase;
    }

    std::regex re(regex,type);
    std::sregex_token_iterator first{input.begin(), input.end(), re, -1}, last;
    return {first, last};
}

ClassDescriptor *NewClassString() {
    ClassDescriptor *array_desc = new ClassDescriptor(nullptr);
    AddClassCFunc(array_desc, "split", split);
    AddClassCFunc(array_desc, "trim", trim);
    AddClassCFunc(array_desc, "indexOf", indexOf);
    AddClassCFunc(array_desc, "search", search);
    AddClassCFunc(array_desc, "replace", replace);
    AddClassCFunc(array_desc, "replaceAll", replaceAll);
    AddClassCFunc(array_desc, "match", match);
    return array_desc;
}

std::string& replace_all(std::string& str, std::string& old_value, std::string& new_value)
{
    while (true)
    {
        std::string::size_type pos(0);
        if ((pos = str.find(old_value)) != std::string::npos) {
            str.replace(pos, old_value.length(),new_value);
        }
        else {
            break;
        }
    }
    return str;
}

std::string& replace_normal(std::string& str, std::string& old_value, std::string& new_value)
{
    std::string::size_type pos(0);
    if ((pos = str.find(old_value)) != std::string::npos) {
        str.replace(pos, old_value.length(),new_value);
    }
    return str;
}

static Value replaceAll(ExecState *exec_state) {
    Value ret;
    do {
        size_t length = exec_state->GetArgumentCount();
        if (length < 3) {
            break;
        }
        Value *src = exec_state->GetArgument(0);
        if (!IsString(src)) {
            throw VMExecError("replaceAll caller isn't a string");
        }
        Value *oldValue = exec_state->GetArgument(1);
        if (!IsString(oldValue)) {
            throw VMExecError("old caller isn't a string");
        }
        Value *newValue = exec_state->GetArgument(2);
        if (!IsString(newValue)) {
            throw VMExecError("split caller isn't a string");
        }
        std::string srcstr = CStringValue(src);
        std::string oldstr = CStringValue(oldValue);
        std::string newstr = CStringValue(newValue);
        std::string dststr = replace_all(srcstr, oldstr, newstr);
        ret = exec_state->string_table()->StringFromUTF8(dststr);

    } while (0);

    return ret;
}

static Value replace(ExecState *exec_state) {
    Value ret;
    do {
        size_t length = exec_state->GetArgumentCount();
        if (length < 3) {
            break;
        }
        Value *src = exec_state->GetArgument(0);
        if (!IsString(src)) {
            throw VMExecError("replaceAll caller isn't a string");
        }
        Value *oldValue = exec_state->GetArgument(1);
        Value *newValue = exec_state->GetArgument(2);
        if (!IsString(newValue)) {
            throw VMExecError("new value isn't a string");
        }
        std::string srcstr = CStringValue(src);
        std::string newstr = CStringValue(newValue);
        if (IsString(oldValue)) {

          std::string oldstr = CStringValue(oldValue);
          std::string dststr = replace_normal(srcstr, oldstr, newstr);
          ret = exec_state->string_table()->StringFromUTF8(dststr);
        } else if (IsClassInstance(oldValue)){
          ClassInstance* instance = to_regex_inst(exec_state,oldValue);
          Value* reg = GetClassMember(instance, "_reg");
          Value* flag = GetClassMember(instance, "_flag");
          if (!IsString(reg) || !IsString(flag)){
            throw VMExecError("Regex._reg || _flag is not string");
          }
          std::string reg_str = ValueTo<String>(reg)->c_str();
          std::string flag_str = ValueTo<String>(flag)->c_str();

          std::regex_constants::syntax_option_type type = std::regex_constants::ECMAScript;
          if (flag_str.find('i') != std::string::npos){
            type |= std::regex_constants::icase;
          }
          std::regex express(reg_str, type);

          std::string dststr = std::regex_replace(srcstr, express, newstr,std::regex_constants::format_first_only);
          ret = exec_state->string_table()->StringFromUTF8(dststr);
        } else{
          throw VMExecError("old caller isn't a string or regex");
        }
    } while (0);

    return ret;
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
        if (IsString(split)) {
            std::string src = CStringValue(string);
            std::string delim = CStringValue(split);
            std::vector<std::string>split_array;
            split_string<std::vector<std::string>>(src, split_array, delim);
            Array *array = ValueTo<Array>(&ret);
            for (int i = 0; i < split_array.size(); i++) {
                Value string_value = exec_state->string_table()->StringFromUTF8(split_array[i]);
                array->items.push_back(string_value);
            }
        } else if(IsClassInstance(split)){
            ClassInstance* instance = to_regex_inst(exec_state,split);
            Value* reg = GetClassMember(instance, "_reg");
            Value* flag = GetClassMember(instance, "_flag");
            if (!IsString(reg) || !IsString(flag) || !IsString(string)){
                throw VMExecError("Regex._reg || _flag is not string");
            }
            std::string reg_str = ValueTo<String>(reg)->c_str();
            std::string flag_str = ValueTo<String>(flag)->c_str();
            std::string test_str = ValueTo<String>(string)->c_str();

            auto ret_vec = split_regex(test_str,reg_str,flag_str);

            Array *array = ValueTo<Array>(&ret);
            for(auto st: ret_vec){
                Value string_value = exec_state->string_table()->StringFromUTF8(st);
                array->items.push_back(string_value);
            }
        } else {
            throw VMExecError("split caller isn't a string or regex");
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
            *pbuf++ = '%', *pbuf++ = '2', *pbuf++ = '0';
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
    
std::string utf8chr(int cp)
{
    char c[5] = { 0x00,0x00,0x00,0x00,0x00 };
    if (cp <= 0x7F) {
        c[0] = cp;
    }
    else if (cp <= 0x7FF) {
        c[0] = (cp >> 6) + 192;
        c[1] = (cp & 63) + 128;
    }
    else if (0xd800 <= cp && cp <= 0xdfff) {
        //invalid block of utf8
    }
    else if (cp <= 0xFFFF)
    {
        c[0] = (cp >> 12) + 224;
        c[1]= ((cp >> 6) & 63) + 128;
        c[2] = (cp & 63) + 128;
    }
    else if (cp <= 0x10FFFF) {
        c[0] = (cp >> 18) + 240;
        c[1] = ((cp >> 12) & 63) + 128;
        c[2] = ((cp >> 6) & 63) + 128;
        c[3] = (cp & 63) + 128;
    }
    return std::string(c);
}
    
std::string utf8_decode(std::string &input) {
    do {
        size_t length = input.length();
        if (!length) {
            break;
        }
        const char *chars = input.c_str();
        std::string utf8str = "";
        for (int i = 0; i < length; i++) {
            char c = chars[i];
            if (c == '\\' && chars[i + 1] == 'u') {
                int cc = 0;
                for (int j = 0; j < 4; j++)
                {
                    char ch = tolower(chars[i + 2 + j]);
                    if (('0' <= ch && ch <= '9') || ('a' <= ch && ch <= 'f')) {
                        cc|= (from_hex(ch) << (3 - j) * 4);
                    }
                    else
                    {
                        cc = 0;
                        break;
                    }
                }
                if (cc) {
                    i += 5;
                    utf8str += utf8chr(cc);
                    continue;
                }
            }
            utf8str.push_back(c);
        }
        return utf8str;
        
    } while (0);
    
    return input;
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

Value search(ExecState* exec_state) {
    Value ret;
    size_t length = exec_state->GetArgumentCount();
    if (length != 2) {
        throw VMExecError("search arg count wrong");
    }

    Value *string = exec_state->GetArgument(0);
    if (!IsString(string)) {
        throw VMExecError("split caller isn't a string");
    }

    Value *search_str = exec_state->GetArgument(1);
    if(IsClassInstance(search_str)){
        ClassInstance* instance = to_regex_inst(exec_state,search_str);

        Value* reg = GetClassMember(instance, "_reg");
        Value* flag = GetClassMember(instance, "_flag");
        if (!IsString(reg) || !IsString(flag)){
            throw VMExecError("Regex._reg || _flag is not string");
        }
        std::string reg_str = ValueTo<String>(reg)->c_str();
        std::string flag_str = ValueTo<String>(flag)->c_str();
        std::string test_str = ValueTo<String>(string)->c_str();

        std::regex_constants::syntax_option_type type = std::regex_constants::ECMAScript;
        if (flag_str.find('i') != std::string::npos){
            type |= std::regex_constants::icase;
        }
        std::regex express(reg_str, type);
        std::smatch match;
        bool succ  = std::regex_search(test_str,match,express);
        if (succ){
            ret = static_cast<int64_t>(match.position(0));
        } else{
            ret = -1;
        }
    } else {
        throw VMExecError("split caller isn't a string or regex");
    }

    return ret;
}

Value match(ExecState* exec_state) {
    Value ret;
    size_t length = exec_state->GetArgumentCount();
    if (length != 2) {
        throw VMExecError("search arg count wrong");
    }

    Value *string = exec_state->GetArgument(0);
    if (!IsString(string)) {
        throw VMExecError("split caller isn't a string");
    }

    Value *search_str = exec_state->GetArgument(1);
    if(IsClassInstance(search_str)){
        ClassInstance* instance = to_regex_inst(exec_state,search_str);

        Value* reg = GetClassMember(instance, "_reg");
        Value* flag = GetClassMember(instance, "_flag");
        if (!IsString(reg) || !IsString(flag)){
            throw VMExecError("Regex._reg || _flag is not string");
        }
        std::string reg_str = ValueTo<String>(reg)->c_str();
        std::string flag_str = ValueTo<String>(flag)->c_str();
        std::string test_str = ValueTo<String>(string)->c_str();

        std::regex_constants::syntax_option_type type = std::regex_constants::ECMAScript;
        if (flag_str.find('i') != std::string::npos){
            type |= std::regex_constants::icase;
        }

        bool g_mode =flag_str.find('g') != std::string::npos;

        ret = exec_state->class_factory()->CreateArray();
        Array* arr = ValueTo<Array>(&ret);

        if (g_mode){
            std::regex express(reg_str, type);
            std::smatch match;
            std::string::const_iterator iterStart = test_str.begin();
            std::string::const_iterator iterEnd = test_str.end();
            bool succ = false;
            while (std::regex_search(iterStart,iterEnd,match,express)){
                arr->items.push_back(exec_state->string_table()->StringFromUTF8(match[0]));
                iterStart = match[0].second;
                succ = true;
            }
            if (!succ){
                ret = Value();
            }
        } else{
            std::regex express(reg_str, type);
            std::smatch match;
            bool succ  = std::regex_search(test_str,match,express);

            if (succ){
                for (size_t i = 0; i < match.size(); ++i){
                    arr->items.push_back(exec_state->string_table()->StringFromUTF8(match[i]));
                }
            } else {
                ret = Value();
            }
            // else return []
        }
    } else {
        throw VMExecError("split caller isn't a string or regex");
    }

    return ret;
}
}  // namespace data_render
}  // namespace core
}  // namespace weex
