#include "IPCException.h"
#include "./base/LogUtils.h"
#include "WeexJSServer.h"
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

extern "C" {
int serverMain(int argc, char** argv);
}

static unsigned long parseUL(const char* s)
{
    unsigned long val;
    errno = 0;
    val = strtoul(s, nullptr, 10);
    if (errno) {
        LOGE("failed to parse ul: %s %s", s, strerror(errno));
        exit(1);
    }
    return val;
}

struct ThreadData {
    int fd;
    bool enableTrace;
};

static void* threadEntry(void* _td)
{
    ThreadData* td = static_cast<ThreadData*>(_td);
    WeexJSServer server(static_cast<int>(td->fd), static_cast<bool>(td->enableTrace));
    nice(6);
    try {
        server.loop();
    } catch (IPCException& e) {
        LOGE("%s", e.msg());
        _exit(1);
    }
    return static_cast<void**>(nullptr);
}

int serverMain(int argc, char** argv)
{
    unsigned long fd;
    unsigned long enableTrace;
    if (argc != 3) {
        LOGE("argc is not correct");
        exit(1);
    }
    fd = parseUL(argv[1]);
    enableTrace = parseUL(argv[2]);
    pthread_attr_t threadAttr;
    pthread_attr_init(&threadAttr);
    pthread_attr_setstacksize(&threadAttr, 10 * 1024 * 1024);
    pthread_t thread;
    ThreadData td = { static_cast<int>(fd), static_cast<bool>(enableTrace) };
    pthread_create(&thread, &threadAttr, threadEntry, &td);
    void* rdata;
    pthread_join(thread, &rdata);
    return 0;
}
