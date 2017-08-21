#ifndef IPCRESULT_H
#define IPCRESULT_H
#include "IPCType.h"
#include <jni.h>
#include <memory>

struct IPCString;
class IPCResult {
public:
    virtual ~IPCResult() = default;
    template <typename T>
    T get()
    {
        return *static_cast<const T*>(getData());
    }

    virtual const void* getData() = 0;
    virtual IPCType getType() = 0;
    virtual const uint16_t* getStringContent() = 0;
    virtual size_t getStringLength() = 0;
    virtual const char* getByteArrayContent() = 0;
    virtual size_t getByteArrayLength() = 0;
};

std::unique_ptr<IPCResult> createVoidResult();
std::unique_ptr<IPCResult> createInt32Result(int32_t val);
std::unique_ptr<IPCResult> createDoubleResult(double val);
std::unique_ptr<IPCResult> createStringResult(JNIEnv* env, jstring str);
std::unique_ptr<IPCResult> createJSONStringResult(JNIEnv* env, jstring str);

#endif /* IPCRESULT_H */
