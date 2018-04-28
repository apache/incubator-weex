#ifndef IPCARGUMENTS_H
#define IPCARGUMENTS_H
#include "IPCType.h"

struct IPCString;
struct IPCByteArray;
class IPCArguments {
public:
    virtual ~IPCArguments() = default;
    template <typename T>
    T get(int index)
    {
        return *static_cast<const T*>(getData(index));
    }

    const IPCString* getString(int index)
    {
        return static_cast<const IPCString*>(getData(index));
    }

    const IPCByteArray* getByteArray(int index)
    {
        return static_cast<const IPCByteArray*>(getData(index));
    }

    virtual const void* getData(int index) = 0;
    virtual IPCType getType(int index) = 0;
    virtual size_t getCount() = 0;
};
#endif /* IPCARGUMENTS_H */
