
#include "weexjsontools.h"

using namespace std;
using namespace WeexCore;

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

  const char *result = v_.GetString();
  ParseNext();
  return result;
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

void parseJsonObject(JsonParser &r, RenderPage *page, RenderObject *root) {

  RAPIDJSON_ASSERT(r.PeekType() == kObjectType);

  r.EnterObject();
  while (const char *key = r.NextObjectKey()) {

    if (0 == strcmp(key, "ref")) {
      RAPIDJSON_ASSERT(r.PeekType() == kStringType);
      root->setRef(r.GetString());
    } else if (0 == strcmp(key, "type")) {
      RAPIDJSON_ASSERT(r.PeekType() == kStringType);
      root->setType(r.GetString());
    } else if (0 == strcmp(key, "attr") || 0 == strcmp(key, "style")) {
      RAPIDJSON_ASSERT(r.PeekType() == kObjectType);
      r.EnterObject();
      while (const char *key2 = r.NextObjectKey()) {
        char value[100 + 1];
        if (r.PeekType() == kNumberType) {
          if (0 == strcmp(key, "attr")) {
            int len = fpconv_dtoa(r.GetDouble(), value);
            value[len] = '\0';
            root->updateAttr(key2, value);
          } else if (0 == strcmp(key, "style")) {
            int len = fpconv_dtoa(r.GetDouble(), value);
            value[len] = '\0';
            root->updateStyle(key2, value);
          }
        } else if (r.PeekType() == kStringType) {
          if (0 == strcmp(key, "attr")) {
            strcpy(value, r.GetString());
            root->updateAttr(key2, value);
          } else if (0 == strcmp(key, "style")) {
            strcpy(value, r.GetString());
            root->updateStyle(key2, value);
          }
        } else {
          r.SkipValue();
        }
        memset(value, 0, sizeof(value));
      }
    } else if (0 == strcmp(key, "event")) {
      RAPIDJSON_ASSERT(r.PeekType() == kArrayType);
      r.EnterArray();
      while (r.NextArrayValue()) {
        RAPIDJSON_ASSERT(r.PeekType() == kStringType);
        root->addEvent(r.GetString());
      }
    } else if (0 == strcmp(key, "children")) {
      RAPIDJSON_ASSERT(r.PeekType() == kArrayType);
      r.EnterArray();

      int index = 0;

      while (r.NextArrayValue()) {
        RAPIDJSON_ASSERT(r.PeekType() == kObjectType);

        RenderObject *child = new RenderObject(page);
        parseJsonObject(r, page, child);

        root->addRenderObject(index, child);
        index++;
      }
    } else {
      r.SkipValue();
    }
  }
}

/**
 * Parse json data to RenderObject
 * @param data : json data (include ref/style/attr/event/children)
 * @param page : {@link RenderPage*}
 * @return {@link RenderObject*}
 */
RenderObject *json2RenderObject(char *data, RenderPage *page) {
  RenderObject *root = new RenderObject(page);
  JsonParser r(data);
  parseJsonObject(r, page, root);
  return root;
}

void json2SingleKeyValue(char *data, std::string &key, std::string &value) {
  JsonParser r(data);
  RAPIDJSON_ASSERT(r.PeekType() == kObjectType);
  r.EnterObject();
  key = r.NextObjectKey();

  if (r.PeekType() == kNumberType) {

    char buffer[24 + 1];
    int len = fpconv_dtoa(r.GetDouble(), buffer);
    buffer[len] = '\0';
    value = buffer;
  } else if (r.PeekType() == kStringType) {
    value = r.GetString();
  }
}