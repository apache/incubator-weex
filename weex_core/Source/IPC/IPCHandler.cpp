#include "IPCHandler.h"
#include "IPCLog.h"
#include "IPCResult.h"
#include <unordered_map>

namespace {
class IPCHandlerImpl : public IPCHandler {
public:
    std::unique_ptr<IPCResult> handle(uint32_t msg, IPCArguments*) override;
    void registerHandler(int msg, const std::function<std::unique_ptr<IPCResult>(IPCArguments*)>& handler) override;

private:
    typedef std::unordered_map<int, std::function<std::unique_ptr<IPCResult>(IPCArguments*)>> MapType;
    MapType m_map;
};

std::unique_ptr<IPCResult> IPCHandlerImpl::handle(uint32_t msg, IPCArguments* arguments)
{
    auto it = m_map.find(msg);
    if (it == m_map.end()) {
        IPC_LOGE("unable to find msg: %d", msg);
        return createVoidResult();
    }
    return it->second(arguments);
}

void IPCHandlerImpl::registerHandler(int msg, const std::function<std::unique_ptr<IPCResult>(IPCArguments*)>& handler)
{
    m_map.insert(MapType::value_type(msg, handler));
}
}

std::unique_ptr<IPCHandler> createIPCHandler()
{
    return std::unique_ptr<IPCHandler>(new IPCHandlerImpl);
}
