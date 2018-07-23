//
// Created by chad on 2018/7/19.
//

#include "core/data_render/table_factory.h"
#include "core/data_render/table.h"

namespace weex {
namespace core {
namespace data_render {

std::unique_ptr<TableFactory> TableFactory::_instance = nullptr;

TableFactory* TableFactory::Instance() {

    if (!_instance) {
        _instance.reset(new TableFactory());
    }

    return _instance.get();
}

void TableFactory::Release() {
    if (_instance) {
        delete _instance.get();
        _instance = nullptr;
    }
}

Value* TableFactory::CreateTable() {
    Table *t = NewTable();
//    ResizeTable(t, arrSize, mapSize);
    Value *v = new Value();
    SetTValue(v, reinterpret_cast<GCObject *>(t));
    tablePool.push_back(v);
    return v;
}

TableFactory::~TableFactory() {

    std::vector<Value*>::iterator it;
    for (it = tablePool.begin(); it != tablePool.end(); it++) {
        FreeValue(*it);
    }
    tablePool.clear();
}

}
}
}