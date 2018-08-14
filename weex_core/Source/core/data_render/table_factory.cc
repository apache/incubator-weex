//
// Created by chad on 2018/7/19.
//

#include "core/data_render/table_factory.h"
#include "core/data_render/table.h"

namespace weex {
namespace core {
namespace data_render {

Value *TableFactory::CreateTable() {
  Table *t = NewTable();
  Value *v = new Value();
  SetTValue(v, reinterpret_cast<GCObject *>(t));
  tablePool.emplace_back(v);
  return v;
}

TableFactory::~TableFactory() {

  for (auto it = tablePool.begin(); it != tablePool.end(); it++) {
    FreeValue(*it);
  }
  tablePool.clear();
}

}
}
}