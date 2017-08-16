#ifndef IPCTYPE_H
#define IPCTYPE_H
#include <stdint.h>
struct IPCPackage {
    uint32_t packageSize;
    uint32_t msg;
    uint32_t type[1];
};

enum class IPCType {
    INT32,
    INT64,
    FLOAT,
    DOUBLE,
    JSONSTRING,
    STRING,
    BYTEARRAY, /* terminated with zero. */
    VOID,
    JSUNDEFINED,
    END,
};

static const uint32_t MSG_MASK = (1U << 31) - 1;
static const uint32_t MSG_END = static_cast<uint32_t>(-1) & MSG_MASK;
static const uint32_t MSG_TERMINATE = static_cast<uint32_t>(-2) & MSG_MASK;
static const uint32_t MSG_NOT_SET = static_cast<uint32_t>(-3) & MSG_MASK;
static const uint32_t MSG_FLAG_ASYNC = 1U << 31;
#endif /* IPCTYPE_H */
