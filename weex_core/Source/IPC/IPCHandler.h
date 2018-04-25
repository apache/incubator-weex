#ifndef IPCHANDLER_H
#define IPCHANDLER_H
#include <functional>
#include <memory>
#include <stdint.h>

class IPCResult;
class IPCArguments;

class IPCHandler {
public:
    virtual ~IPCHandler() = default;
    virtual std::unique_ptr<IPCResult> handle(uint32_t msg, IPCArguments*) = 0;
    virtual void registerHandler(int msg, const std::function<std::unique_ptr<IPCResult>(IPCArguments*)>& handler) = 0;
};

std::unique_ptr<IPCHandler> createIPCHandler();
#endif /* IPCHANDLER_H */
