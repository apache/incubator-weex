#ifndef WEEX_JSON_TOOLS
#define WEEX_JSON_TOOLS

#include "pointer.h"
#include <iostream>
#include <string.h>
#include "../base/fpconv.h"
#include <sstream>
#include <wtf/text/WTFString.h>
#include "../base/LogUtils.h"
#include "../base/StringUtils.h"
#include "../WeexCore/render/RenderObject.h"
#include "../WeexCore/render/RenderPage.h"

using namespace rapidjson;
using namespace std;
using namespace WeexCore;

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

    bool RawNumber(const char *, SizeType, bool);

    bool String(const char *str, SizeType length, bool);

    bool StartObject();

    bool Key(const char *str, SizeType length, bool);

    bool EndObject(SizeType);

    bool StartArray();

    bool EndArray(SizeType);

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

    Value v_;
    JsonParsingState st_;
    Reader r_;
    InsituStringStream ss_;

    static const int parseFlags = kParseDefaultFlags | kParseInsituFlag;
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

    Value *PeekValue();

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
RenderObject* json2RenderObject(char *data, RenderPage *page);

void json2SingleKeyValue(char *data, String &key, String &value);

#endif //WEEX_JSON_TOOLS
