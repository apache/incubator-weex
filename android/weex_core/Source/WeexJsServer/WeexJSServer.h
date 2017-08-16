#ifndef WEEXJSSERVER_H
#define WEEXJSSERVER_H
#include <memory>
class IPCSender;
class IPCSerializer;
class WeexJSServer {
public:
    WeexJSServer(int fd, bool enableTrace);
    ~WeexJSServer();
    void loop();
    IPCSender* getSender();
    IPCSerializer* getSerializer();

private:
    struct WeexJSServerImpl;
    std::unique_ptr<WeexJSServerImpl> m_impl;
};
#endif /* WEEXJSSERVER_H */
