#ifndef WEEX_JSON_TOOLS
#define WEEX_JSON_TOOLS

#include "rapidjson/pointer.h"
#include <vector>

namespace WeexCore {

  class RenderObject;

  class RenderPage;

/**
 * Use to handler json parser result
 */
  class JsonParserHandler {
  public:
    bool Null();

    bool Bool(bool b);

    bool Int(int i);

    bool Uint(unsigned u);

    bool Int64(int64_t i);

    bool Uint64(uint64_t u);

    bool Double(double d);

    bool RawNumber(const char *, rapidjson::SizeType, bool);

    bool String(const char *str, rapidjson::SizeType length, bool);

    bool StartObject();

    bool Key(const char *str, rapidjson::SizeType length, bool);

    bool EndObject(rapidjson::SizeType);

    bool StartArray();

    bool EndArray(rapidjson::SizeType);

  protected:
    JsonParserHandler(char *str);

    void ParseNext();

  protected:
    enum JsonParsingState {
      kInit,
      kError,
      kHasNull,
      kHasBool,
      kHasNumber,
      kHasString,
      kHasKey,
      kEnteringObject,
      kExitingObject,
      kEnteringArray,
      kExitingArray
    };

    rapidjson::Value v_;
    JsonParsingState st_;
    rapidjson::Reader r_;
    rapidjson::InsituStringStream ss_;

    static const int parseFlags = rapidjson::kParseDefaultFlags | rapidjson::kParseInsituFlag;
  };

/**
 * Use to parse json data
 */
  class JsonParser : protected JsonParserHandler {
  public:
    JsonParser(char *str) : JsonParserHandler(str) {}

    bool EnterObject();

    bool EnterArray();

    const char *NextObjectKey();

    bool NextArrayValue();

    int GetInt();

    double GetDouble();

    const char *GetString();

    bool GetBool();

    void GetNull();

    void SkipObject();

    void SkipArray();

    void SkipValue();

    rapidjson::Value *PeekValue();

    int PeekType(); // returns a rapidjson::Type, or -1 for no value (at end of object/array)

    bool IsValid() { return st_ != kError; }

  protected:
    void SkipOut(int depth);
  };

/**
 * Parse json data to RenderObject
 * @param data : json data (include ref/style/attr/event/children)
 * @param page : {@link RenderPage*}
 * @return {@link RenderObject*}
 */
  RenderObject *json2RenderObject(char *data);

  std::vector<std::pair<std::string, std::string> *> *
  json2Pairs(char *data);
}

#endif //WEEX_JSON_TOOLS
