#ifndef IPCBYTEARRAY_H
#define IPCBYTEARRAY_H
#include <stddef.h>
struct IPCByteArray {
    uint32_t length;
    char content[1];
};
#endif /* IPCBYTEARRAY_H */
