#ifndef IPCLISTENER_H
#define IPCLISTENER_H
#include <memory>
class IPCHandler;
class IPCFutexPageQueue;

class IPCListener {
public:
    virtual ~IPCListener() = default;
    virtual void listen() = 0;
};

std::unique_ptr<IPCListener> createIPCListener(IPCFutexPageQueue*, IPCHandler* handler);
#endif /* IPCLISTENER_H */
