#ifndef IPCSTRING_H
#define IPCSTRING_H
struct IPCString {
    uint32_t length;
    uint16_t content[1];
};
#endif /* IPCSTRING_H */
