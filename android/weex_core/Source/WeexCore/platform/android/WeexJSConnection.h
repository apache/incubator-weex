#ifndef WEEXJSCONNECTION_H
#define WEEXJSCONNECTION_H

#include <memory>

class IPCSender;

class IPCHandler;

class WeexJSConnection {
public:
    WeexJSConnection();

    ~WeexJSConnection();

    IPCSender *start(IPCHandler *handler);

    void end();

private:
    struct WeexJSConnectionImpl;

    std::unique_ptr <WeexJSConnectionImpl> m_impl;
};

#endif /* WEEXJSCONNECTION_H */
