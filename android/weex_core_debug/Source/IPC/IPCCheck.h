#ifndef IPCCHECK_H
#define IPCCHECK_H
#include <stdlib.h>
#if defined(NDEBUG)
#define IPC_DCHECK(b)
#define IPC_UNREACHABLE() abort()
#else
void reportCheckFailed(const char* msg, const char* file, int line);
void reportUnreachable(const char* file, int line);
#define IPC_DCHECK(b)                                                                \
    if (!(b)) {                                                                      \
        reportCheckFailed("CHECK FAILED: " #b " at line %s %d", __FILE__, __LINE__); \
    }
#define IPC_UNREACHABLE() reportUnreachable(__FILE__, __LINE__);
#endif
#endif /* IPCCHECK_H */
