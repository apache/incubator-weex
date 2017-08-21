#ifndef IPCCOMMUNICATOR_H
#define IPCCOMMUNICATOR_H
#include <memory>

class IPCResult;
class IPCArguments;
class IPCBuffer;
class IPCFutexPageQueue;
class IPCCommunicator {
protected:
    explicit IPCCommunicator(IPCFutexPageQueue* futexPageQueue);
    virtual ~IPCCommunicator();

    std::unique_ptr<IPCResult> assembleResult();
    std::unique_ptr<IPCArguments> assembleArguments();
    std::unique_ptr<IPCBuffer> generateResultBuffer(IPCResult*);
    void doSendBufferOnly(IPCBuffer* buffer);
    uint32_t doReadPackage();
    const char* getBlob();
    void releaseBlob();

private:
    void doSendBufferOnly(const void* data, size_t s);
    size_t doSendBufferPage(const void* data, size_t s, size_t pageSize);
    void doRecvBufferOnly(void* data, size_t s);
    std::unique_ptr<char[]> m_package;
    // weakref to a IPCFutexPageQueue object.
    IPCFutexPageQueue* m_futexPageQueue;
};
#endif /* IPCCOMMUNICATOR_H */
