#ifndef IPCSERIALIZER_H
#define IPCSERIALIZER_H
#include <memory>
#include <stdint.h>

class IPCBuffer;
class IPCSerializer {
public:
    virtual ~IPCSerializer() = default;
    virtual void setMsg(uint32_t msg) = 0;
    virtual void add(int32_t) = 0;
    virtual void add(int64_t) = 0;
    virtual void add(float) = 0;
    virtual void add(double) = 0;
    virtual void add(const uint16_t* data, size_t len) = 0;
    virtual void addJSON(const uint16_t* data, size_t len) = 0;
    virtual void add(const char* data, size_t len) = 0;
    virtual void addJSUndefined() = 0;
    virtual void addVoid() = 0;
    virtual std::unique_ptr<IPCBuffer> finish() = 0;
};

std::unique_ptr<IPCSerializer> createIPCSerializer();
#endif /* IPCSERIALIZER_H */
