//
// Created by chad on 2018/7/17.
//

#ifndef DATA_RENDER_TABLE_H
#define DATA_RENDER_TABLE_H

#include "object.h"

namespace weex {
namespace core {
namespace data_render {

Table *NewTable();

int ResizeTab(Table *, size_t, size_t);

Value *GetTabValue(const Table*, const Value*);

int SetTabValue(Table*, Value*, Value*);

}
}
}


#endif //DATA_RENDER_TABLE_H
