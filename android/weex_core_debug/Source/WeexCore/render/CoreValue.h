#ifndef WEEX_PROJECT_COREVALUE_H
#define WEEX_PROJECT_COREVALUE_H

#include <string>

namespace WeexCore {

  enum ValueType {
    kFloat,
    kInt,
    kString
  };

  typedef struct CoreValue {
    ValueType type;
    int int_value;
    float float_value;
    std::string string_value;
  } CoreValue;
}

#endif //WEEX_PROJECT_COREVALUE_H
