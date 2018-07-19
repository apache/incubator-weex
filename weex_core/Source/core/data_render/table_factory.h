//
// Created by chad on 2018/7/19.
//

#ifndef DATA_RENDER_TABLEFACTORY_H
#define DATA_RENDER_TABLEFACTORY_H

#include <vector>
#include "core/data_render/object.h"

namespace weex {
namespace core {
namespace data_render {

class TableFactory {

public:

    static TableFactory* Instance();

    Value* CreateTable();

    ~TableFactory();

private:
    TableFactory() {};
    std::vector<Value*> tablePool;
};

}
}
}

#endif //DATA_RENDER_TABLEFACTORY_H
