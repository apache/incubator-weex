//
// Created by chad on 2018/7/17.
//
//#include <stdlib.h>
#include "table.h"
#include "vm_mem.h"

namespace weex {
namespace core {
namespace data_render {

int SetTabIntValue(Table *t, const Value *key, const Value *val) {
    if (IsNil(val)) {
        return 0;
    }
    int index = IntValue(key);
    if (index < 0 || index >= t->sizearray) {
        return 0;
    }
    t->array[index] = *val;
    return 1;
}

int SetTabStringValue(Table *t, const Value *key, Value *val) {
    if (IsNil(val)) {
        return 0;
    }
    char *keyStr = CStringValue(key);
    if (NULL == keyStr) {
        return 0;
    }
    t->map->at(keyStr) = *val;
    return 1;
}

Value * GetTabIntValue(const Table *t, const Value *key) {
    int index = IntValue(key);
    if (index < t->sizearray) {
        return &t->array[index];
    }
    return nullptr;
}

Value * GetTabStringValue(const Table *t, const Value *key) {
    char *str = CStringValue(key);
    if (!str) {
        return &(t->map->at(str));
    }
    return nullptr;
}

Table *NewTable() {
    Table *t = reinterpret_cast<Table*>(reallocMem(NULL, sizeof(Table)));
    if (NULL == t) {
        return NULL;
    }
    t->array = new Value[0];
    t->map = new std::unordered_map<std::string, Value>(0);
    t->sizearray = 0;
    t->sizenode = 0;
    return t;
}

int ResizeTab(Table *t, size_t nasize, size_t nhsize) {

    size_t i;

    size_t oldArrSize = t->sizearray;
    size_t oldHashSize = t->sizenode;

//    Value *arr = t->array;
//    Node *oldNode = t->node;

    if (nasize != 0) {
        Value *ptr = static_cast<Value *>(reallocMem(t->array, nasize * sizeof(Value)));
        if (NULL == ptr) {
            //TODO OOM
            return 0;
        }
        if (nasize > oldArrSize) {
            for (i = oldArrSize; i < nasize; i++) {
                SetNil(&ptr[i]);
            }
        }
        t->array = ptr;
        t->sizearray = nasize;
    } else {
        return 0;
    }

    if (nhsize != 0) {
//        t->map = new std::unordered_map<std::string, Value>(nhsize);

//        Node *ptr = static_cast<Node *>(reallocMem(t->node, nhsize * sizeof(Node)));
//        if (NULL == ptr) {
//            //TODO OOM
//            return 0;
//        }
//        int *hptr = static_cast<int *>(reallocMem(t->hash, nhsize * sizeof(int)));
//        if (NULL == hptr) {
//            //TODO OOM
//            return 0;
//        }
//        if (nhsize > oldHashSize) {
//            for (i = oldHashSize; i < nhsize; i++) {
//                Node *n = &ptr[i];
//                n->next = NULL;
//                SetNil(n->key->val);
//                SetNil(n->val);
//
//                hptr[i] = -1;
//            }
//        }
//        t->hash = hptr;
//        t->node = ptr;
//        t->sizenode = nhsize;
    } else {
        return 0;
    }

    return 1;
}

Value *GetTabValue(const Table *t, const Value *key) {
    if (IsInt(key)) {
        return GetTabIntValue(t, key);
    } else if (IsString(key)) {
        return GetTabStringValue(t, key);
    }
    return nullptr;
}

int SetTabValue(Table *t, Value *key, Value *val) {
    if (IsInt(key)) {
        return SetTabIntValue(t, key, val);
    } else if (IsString(key)) {
        return SetTabStringValue(t, key, val);
    }
    return 0;
}

}
}
}