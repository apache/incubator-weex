#include "IPCException.h"
#include <stdarg.h>
#include <string>

struct IPCException::IPCExceptionImpl {
    std::string str;
};

IPCException::IPCException(const char* fmt, ...)
    : m_impl(new IPCExceptionImpl)
{
    va_list ap;
    char buffer[256];
    va_start(ap, fmt);
    vsnprintf(buffer, 256, fmt, ap);
    va_end(ap);
    m_impl->str.assign(buffer);
}

IPCException::~IPCException()
{
}

IPCException::IPCException(IPCException&& ex)
{
    m_impl = std::move(ex.m_impl);
}

const char* IPCException::msg() const
{
    return m_impl->str.c_str();
}
