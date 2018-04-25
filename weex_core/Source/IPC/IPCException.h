#ifndef IPCEXCEPTION_H
#define IPCEXCEPTION_H
#include <memory>

class IPCException {
public:
    IPCException(const char* fmt, ...);
    ~IPCException();
    IPCException(IPCException&& ex);
    const char* msg() const;

private:
    struct IPCExceptionImpl;
    std::unique_ptr<IPCExceptionImpl> m_impl;
};
#endif /* IPCEXCEPTION_H */
