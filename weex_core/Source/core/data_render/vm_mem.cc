//
// Created by chad on 2018/7/18.
//

#include <stdlib.h>
#include "vm_mem.h"

namespace weex {
namespace core {
namespace data_render {

void* reallocMem(void *block, size_t newSize) {
    if (newSize == 0) {
        free(block);
        return NULL;
    } else {
        return realloc(block, newSize);
    }
}

} //namespace weex
} //namespace core
} //namespace data_render