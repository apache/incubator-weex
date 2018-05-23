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

#include "dom_parser.h"
#include <core/render/node/render_object.h>
#include <core/render/page/render_page.h>
#include <core/render/node/factory/render_creator.h>
#include <base/ViewUtils.h>

using namespace std;
using namespace rapidjson;

namespace WeexCore {

  static inline int matchNum(std::string temp, std::string key, int startIndex, int endIndex) {
    int num = 0;
    while (startIndex < endIndex) {
      int index = temp.find(key, startIndex);
      if (index >= 0 && index < endIndex) {
        num++;
        startIndex = index + 1;
      } else {
        break;
      }
    }
    return num;
  }

  static inline int GetSplitIndex(std::string temp, const char * leftChar, const char * rightChar){
    int endIndex = temp.find(rightChar);
    if(endIndex > 0) {
      // has found!
      int temp_length = temp.length();
      int startIndex = 0;
      int leftMatchSize = matchNum(temp, leftChar, startIndex, endIndex) - 1;
      while (leftMatchSize > 0 && startIndex <= endIndex && startIndex < temp_length) {
        startIndex = endIndex + 1;
        int markIndex = temp.find(rightChar, startIndex);
        if(markIndex > 0) {
          endIndex = markIndex;
          leftMatchSize--;
        }
        leftMatchSize += matchNum(temp, leftChar, startIndex, endIndex);
      }
    }
    return endIndex;
  }

  bool JsonParserHandler::Null() {
    st_ = kHasNull;
    v_.SetNull();
    return true;
  }

  bool JsonParserHandler::Bool(bool b) {
    st_ = kHasBool;
    v_.SetBool(b);
    return true;
  }

  bool JsonParserHandler::Int(int i) {
    st_ = kHasNumber;
    v_.SetInt(i);
    return true;
  }

  bool JsonParserHandler::Uint(unsigned u) {
    st_ = kHasNumber;
    v_.SetUint(u);
    return true;
  }

  bool JsonParserHandler::Int64(int64_t i) {
    st_ = kHasNumber;
    v_.SetInt64(i);
    return true;
  }

  bool JsonParserHandler::Uint64(uint64_t u) {
    st_ = kHasNumber;
    v_.SetUint64(u);
    return true;
  }

  bool JsonParserHandler::Double(double d) {
    st_ = kHasNumber;
    v_.SetDouble(d);
    return true;
  }

  bool JsonParserHandler::RawNumber(const char *, SizeType, bool) { return false; }

  bool JsonParserHandler::String(const char *str, SizeType length, bool) {
    st_ = kHasString;
    v_.SetString(str, length);
    return true;
  }

  bool JsonParserHandler::StartObject() {
    st_ = kEnteringObject;
    return true;
  }

  bool JsonParserHandler::Key(const char *str, SizeType length, bool) {
    st_ = kHasKey;
    v_.SetString(str, length);
    return true;
  }

  bool JsonParserHandler::EndObject(SizeType) {
    st_ = kExitingObject;
    return true;
  }

  bool JsonParserHandler::StartArray() {
    st_ = kEnteringArray;
    return true;
  }

  bool JsonParserHandler::EndArray(SizeType) {
    st_ = kExitingArray;
    return true;
  }

  JsonParserHandler::JsonParserHandler(char *str) : v_(), st_(kInit), r_(), ss_(str) {
    r_.IterativeParseInit();
    ParseNext();
  }

  void JsonParserHandler::ParseNext() {
    if (r_.HasParseError()) {
      st_ = kError;
      return;
    }

    r_.IterativeParseNext<parseFlags>(ss_, *this);
  }

  bool JsonParser::EnterObject() {
    if (st_ != kEnteringObject) {
      st_ = kError;
      return false;
    }

    ParseNext();
    return true;
  }

  bool JsonParser::EnterArray() {
    if (st_ != kEnteringArray) {
      st_ = kError;
      return false;
    }

    ParseNext();
    return true;
  }

  const char *JsonParser::NextObjectKey() {
    if (st_ == kHasKey) {
      const char *result = v_.GetString();
      ParseNext();
      return result;
    }

    if (st_ != kExitingObject) {
      st_ = kError;
      return 0;
    }

    ParseNext();
    return 0;
  }

  bool JsonParser::NextArrayValue() {
    if (st_ == kExitingArray) {
      ParseNext();
      return false;
    }

    if (st_ == kError || st_ == kExitingObject || st_ == kHasKey) {
      st_ = kError;
      return false;
    }

    return true;
  }

  int JsonParser::GetInt() {
    if (st_ != kHasNumber || !v_.IsInt()) {
      st_ = kError;
      return 0;
    }

    int result = v_.GetInt();
    ParseNext();
    return result;
  }

  double JsonParser::GetDouble() {
    if (st_ != kHasNumber) {
      st_ = kError;
      return 0.;
    }

    double result = v_.GetDouble();
    ParseNext();
    return result;
  }

  bool JsonParser::GetBool() {
    if (st_ != kHasBool) {
      st_ = kError;
      return false;
    }

    bool result = v_.GetBool();
    ParseNext();
    return result;
  }

  void JsonParser::GetNull() {
    if (st_ != kHasNull) {
      st_ = kError;
      return;
    }

    ParseNext();
  }

  const char *JsonParser::GetString() {
    if (st_ != kHasString) {
      st_ = kError;
      return 0;
    }

    const char *temp = v_.GetString();
    int len = v_.GetStringLength();
    char *result = new char[len + 1];
    strcpy(result, temp);
    result[len] = '\0';
    ParseNext();
    return result;
  }

  const std::string JsonParser::GetObjectStr() {
      std::string temp = "{";
      temp.append(Stringify());
      int endIndex = GetSplitIndex(temp, "{", "}");
      SkipValue();
      return temp.substr(0, endIndex + 1);
  }

  const std::string JsonParser::GetArrayStr() {
      std::string temp = "[";
      temp.append(Stringify());
      int endIndex = GetSplitIndex(temp, "[", "]");
      SkipValue();
      return temp.substr(0, endIndex + 1);
  }

  const char *JsonParser::Stringify() {
    return ss_.src_;
  }

  void JsonParser::SkipOut(int depth) {
    do {
      if (st_ == kEnteringArray || st_ == kEnteringObject) {
        ++depth;
      } else if (st_ == kExitingArray || st_ == kExitingObject) {
        --depth;
      } else if (st_ == kError) {
        return;
      }

      ParseNext();
    } while (depth > 0);
  }

  void JsonParser::SkipValue() {
    SkipOut(0);
  }

  void JsonParser::SkipArray() {
    SkipOut(1);
  }

  void JsonParser::SkipObject() {
    SkipOut(1);
  }

  Value *JsonParser::PeekValue() {
    if (st_ >= kHasNull && st_ <= kHasKey) {
      return &v_;
    }

    return 0;
  }

  int JsonParser::PeekType() {
    if (st_ >= kHasNull && st_ <= kHasKey) {
      return v_.GetType();
    }

    if (st_ == kEnteringArray) {
      return kArrayType;
    }

    if (st_ == kEnteringObject) {
      return kObjectType;
    }

    return -1;
  }

  RenderObject *
  ParseJsonObject(JsonParser &r, RenderObject *parent, int index, const std::string &pageId) {

    RAPIDJSON_ASSERT(r.PeekType() == kObjectType);
    r.EnterObject();

    RenderObject *render;
    std::string ref;

    while (const char *key = r.NextObjectKey()) {

      if (0 == strcmp(key, "ref")) {
        RAPIDJSON_ASSERT(r.PeekType() == kStringType);
        const char *temp_ref = r.GetString();
        ref = temp_ref;
        if (temp_ref != nullptr) {
          delete[]temp_ref;
          temp_ref = nullptr;
        }
      } else if (0 == strcmp(key, "type")) {
        RAPIDJSON_ASSERT(r.PeekType() == kStringType);
        const char *temp_type = r.GetString();
        render = (RenderObject *) RenderCreator::GetInstance()->CreateRender(temp_type, ref);
        render->SetPageId(pageId);
        if (parent != nullptr)
          parent->AddRenderObject(index, render);
        if (temp_type != nullptr) {
          delete[]temp_type;
          temp_type = nullptr;
        }
      } else if (0 == strcmp(key, "attr") || 0 == strcmp(key, "style")) {
        RAPIDJSON_ASSERT(r.PeekType() == kObjectType);
        r.EnterObject();
        while (const char *key2 = r.NextObjectKey()) {
          if (r.PeekType() == kNumberType) {
            RAPIDJSON_ASSERT(r.PeekType() == kNumberType);
            if (0 == strcmp(key, "attr")) {
              render->AddAttr(key2, to_string(r.GetDouble()));
            } else if (0 == strcmp(key, "style")) {
              render->AddStyle(key2, to_string(r.GetDouble()));
            }
          } else if (r.PeekType() == kStringType) {
            RAPIDJSON_ASSERT(r.PeekType() == kStringType);
            const char *temp_str = r.GetString();
            if (0 == strcmp(key, "attr")) {
              render->AddAttr(key2, temp_str);
            } else if (0 == strcmp(key, "style")) {
              render->AddStyle(key2, temp_str);
            }
            if (temp_str != nullptr) {
              delete[]temp_str;
              temp_str = nullptr;
            }
          } else if (r.PeekType() == kArrayType) {
            RAPIDJSON_ASSERT(r.PeekType() == kArrayType);
            std::string value = r.GetArrayStr();
            if (0 == strcmp(key, "attr")) {
              render->AddAttr(key2, value);
            } else if (0 == strcmp(key, "style")) {
              render->AddStyle(key2, value);
            }
          } else if (r.PeekType() == kTrueType) {
            RAPIDJSON_ASSERT(r.PeekType() == kTrueType);
            if (0 == strcmp(key, "attr")) {
              render->AddAttr(key2, "true");
            } else if (0 == strcmp(key, "style")) {
              render->AddStyle(key2, "true");
            }
            r.SkipValue();
          } else if (r.PeekType() == kFalseType) {
            RAPIDJSON_ASSERT(r.PeekType() == kFalseType);
            if (0 == strcmp(key, "attr")) {
              render->AddAttr(key2, "false");
            } else if (0 == strcmp(key, "style")) {
              render->AddStyle(key2, "false");
            }
            r.SkipValue();
          } else if (r.PeekType() == kObjectType) {
            RAPIDJSON_ASSERT(r.PeekType() == kObjectType);
            std::string value = r.GetObjectStr();
            if (0 == strcmp(key, "attr")) {
              render->AddAttr(key2, value);
            } else if (0 == strcmp(key, "style")) {
              render->AddStyle(key2, value);
            }
          } else {
            r.SkipValue();
          }
        }
      } else if (0 == strcmp(key, "event")) {
          if (r.PeekType() == kArrayType) {
              RAPIDJSON_ASSERT(r.PeekType() == kArrayType);
              r.EnterArray();
              while (r.NextArrayValue()) {
                  std::string temp_event_str;
                  if(r.PeekType() == kStringType) {
                      RAPIDJSON_ASSERT(r.PeekType() == kStringType);
                      const char *temp_event = r.GetString();
                      temp_event_str.append(temp_event);
                      delete[]temp_event;
                  } else if(r.PeekType() == kObjectType){
                      RAPIDJSON_ASSERT(r.PeekType() == kObjectType);
                      temp_event_str = r.GetObjectStr();
                  } else {
                      r.SkipValue();
                  }

                  if (temp_event_str.length() > 0) {
                      render->AddEvent(temp_event_str);
                  }
              }
          }

      } else if (0 == strcmp(key, "children")) {
        RAPIDJSON_ASSERT(r.PeekType() == kArrayType);
        r.EnterArray();

        int index = 0;

        while (r.NextArrayValue()) {
          RAPIDJSON_ASSERT(r.PeekType() == kObjectType);
          ParseJsonObject(r, render, index, pageId);
          index++;
        }
      } else {
        r.SkipValue();
      }
    }

    if (render != nullptr) {
      render->ApplyDefaultStyle();
      render->ApplyDefaultAttr();
    }

    return render;
  }

/**
 * Parse json data to RenderObject
 * @param data : json data (include ref/style/attr/event/children)
 * @param page : {@link RenderPage}
 * @return {@link RenderObject*}
 */
  RenderObject *Json2RenderObject(char *data, const std::string &pageId) {
    JsonParser r(data);
    return ParseJsonObject(r, nullptr, 0, pageId);
  }

  std::vector<std::pair<std::string, std::string>> *Json2Pairs(char *data) {
    std::vector<std::pair<std::string, std::string>> *pairs = nullptr;
    JsonParser r(data);
    RAPIDJSON_ASSERT(r.PeekType() == kObjectType);
    r.EnterObject();
    pairs = new std::vector<std::pair<std::string, std::string>>();
    while (const char *key = r.NextObjectKey()) {
      if (r.PeekType() == kNumberType) {
        RAPIDJSON_ASSERT(r.PeekType() == kNumberType);
        std::pair<std::string, std::string> myPair(key, to_string(r.GetDouble()));
        pairs->insert(pairs->end(), myPair);
      } else if (r.PeekType() == kStringType) {
        RAPIDJSON_ASSERT(r.PeekType() == kStringType);
        const char *value = r.GetString();
        std::pair<std::string, std::string> myPair(key, value);
        pairs->insert(pairs->end(), myPair);
        if (value != nullptr) {
          delete value;
          value = nullptr;
        }
      } else if (r.PeekType() == kArrayType) {
        RAPIDJSON_ASSERT(r.PeekType() == kArrayType);
        std::string value = r.GetArrayStr();
        std::pair<std::string, std::string> myPair(key, value);
        pairs->insert(pairs->end(), myPair);
      } else if (r.PeekType() == kTrueType) {
        RAPIDJSON_ASSERT(r.PeekType() == kTrueType);
        std::pair<std::string, std::string> myPair(key, "true");
        pairs->insert(pairs->end(), myPair);
        r.SkipValue();
      } else if (r.PeekType() == kFalseType) {
        RAPIDJSON_ASSERT(r.PeekType() == kFalseType);
        std::pair<std::string, std::string> myPair(key, "false");
        pairs->insert(pairs->end(), myPair);
        r.SkipValue();
      } else if (r.PeekType() == kNullType) {
        RAPIDJSON_ASSERT(r.PeekType() == kNullType);
        std::pair<std::string, std::string> myPair(key, "");
        pairs->insert(pairs->end(), myPair);
        r.SkipValue();
      } else if (r.PeekType() == kObjectType) {
        RAPIDJSON_ASSERT(r.PeekType() == kObjectType);
        std::string value = r.GetObjectStr();
        std::pair<std::string, std::string> myPair(key, value);
        pairs->insert(pairs->end(), myPair);
      } else {
        r.SkipValue();
      }
    }
    return pairs;
  }
}

