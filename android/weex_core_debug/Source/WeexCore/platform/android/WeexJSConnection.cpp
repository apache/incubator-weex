#include "WeexJSConnection.h"
#include "../../../IPC/IPCException.h"
#include "../../../IPC/IPCLog.h"
#include "../../../IPC/IPCSender.h"
#include "../../../base/LogUtils.h"

#include "../../../IPC/ashmem.h"
#include "../../../IPC/IPCFutexPageQueue.h"
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/mman.h>

static void doExec(int fd, bool traceEnable);
static void closeAllButThis(int fd);
extern const char* s_cacheDir;

struct WeexJSConnection::WeexJSConnectionImpl {
    std::unique_ptr<IPCSender> serverSender;
    std::unique_ptr<IPCFutexPageQueue> futexPageQueue;
    pid_t child{ 0 };
};

WeexJSConnection::WeexJSConnection()
    : m_impl(new WeexJSConnectionImpl)
{
}

WeexJSConnection::~WeexJSConnection()
{
    end();
}

IPCSender* WeexJSConnection::start(IPCHandler* handler)
{
    int fd = ashmem_create_region("WEEX_IPC", IPCFutexPageQueue::ipc_size);
    if (-1 == fd) {
        throw IPCException("failed to create ashmem region: %s", strerror(errno));
    }
    void* base = mmap(nullptr, IPCFutexPageQueue::ipc_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (base == MAP_FAILED) {
        int _errno = errno;
        close(fd);
        throw IPCException("failed to map ashmem region: %s", strerror(_errno));
    }
    std::unique_ptr<IPCFutexPageQueue> futexPageQueue(new IPCFutexPageQueue(base, IPCFutexPageQueue::ipc_size, 0));
    std::unique_ptr<IPCSender> sender(createIPCSender(futexPageQueue.get(), handler));
    m_impl->serverSender = std::move(sender);
    m_impl->futexPageQueue = std::move(futexPageQueue);
    pid_t child = fork();
    if (child == -1) {
        int myerrno = errno;
        munmap(base, IPCFutexPageQueue::ipc_size);
        close(fd);
        throw IPCException("failed to fork: %s", strerror(myerrno));
    } else if (child == 0) {
        // the child
        closeAllButThis(fd);
        // implements close all but handles[1]
        // do exec
        doExec(fd, true);
        LOGE("exec Failed completely.");
        // failed to exec
        _exit(1);
    } else {
        close(fd);
        m_impl->child = child;
        m_impl->futexPageQueue->spinWaitPeer();
    }
    return m_impl->serverSender.get();
}

void WeexJSConnection::end()
{
    m_impl->serverSender.reset();
    m_impl->futexPageQueue.reset();
    if (m_impl->child) {
        int wstatus;
        pid_t child;
        kill(m_impl->child, 9);
        while (true) {
            child = waitpid(m_impl->child, &wstatus, 0);
            if (child != -1)
                break;
            if (errno != EINTR)
                break;
        }
    }
}

static std::string __attribute__((noinline)) findPath();
static void findPath(std::string& executablePath, std::string& icuDataPath)
{
    unsigned long target = reinterpret_cast<unsigned long>(__builtin_return_address(0));
    FILE* f = fopen("/proc/self/maps", "r");
    if (!f) {
        return;
    }
    char buffer[256];
    char* line;
    while ((line = fgets(buffer, 256, f))) {
        if (icuDataPath.empty() && strstr(line, "icudt")) {
            icuDataPath.assign(strstr(line, "/"));
            icuDataPath = icuDataPath.substr(0, icuDataPath.length() - 1);
            continue;
        }
        char* end;
        unsigned long val;
        errno = 0;
        val = strtoul(line, &end, 16);
        if (errno)
            continue;
        if (val > target)
            continue;
        end += 1;
        errno = 0;
        val = strtoul(end, &end, 16);
        if (errno)
            continue;
        if (val > target) {
            executablePath.assign(strstr(end, "/"));
            executablePath = executablePath.substr(0, executablePath.rfind('/'));
        }
        if (!executablePath.empty()
            && !icuDataPath.empty()) {
            break;
        }
    }
    fclose(f);
    return;
}

void doExec(int fd, bool traceEnable)
{
    std::string executablePath;
    std::string icuDataPath;
    findPath(executablePath, icuDataPath);
    if (executablePath.empty()) {
        LOGE("executablePath is empty");
        return;
    }
    if (icuDataPath.empty()) {
        LOGE("icuDataPath is empty");
        return;
    }
    if (!s_cacheDir) {
        LOGE("crash log file path s_cacheDir is empty");
        return;
    }
    std::string ldLibraryPathEnv("LD_LIBRARY_PATH=");
    std::string icuDataPathEnv("ICU_DATA_PATH=");
    std::string crashFilePathEnv("CRASH_FILE_PATH=");
    ldLibraryPathEnv.append(executablePath);
    icuDataPathEnv.append(icuDataPath);
    crashFilePathEnv.append(s_cacheDir);
    crashFilePathEnv.append("/jsserver_crash");
    char fdStr[16];
    snprintf(fdStr, 16, "%d", fd);
    const char* envp[] = { ldLibraryPathEnv.c_str(),
        icuDataPathEnv.c_str(),
        crashFilePathEnv.c_str(),
        nullptr };
    {
        std::string executableName = executablePath + '/' + "libweexjsb64.so";
        chmod(executableName.c_str(), 0755);
        const char* argv[] = { executableName.c_str(), fdStr, traceEnable ? "1" : "0", nullptr };
        if (-1 == execve(argv[0], const_cast<char* const*>(&argv[0]), const_cast<char* const*>(envp))) {
            LOGE("execve failed: %s", strerror(errno));
        }
    }
    {
        std::string executableName = executablePath + '/' + "libweexjsb.so";
        chmod(executableName.c_str(), 0755);
        const char* argv[] = { executableName.c_str(), fdStr, traceEnable ? "1" : "0", nullptr };
        if (-1 == execve(argv[0], const_cast<char* const*>(&argv[0]), const_cast<char* const*>(envp))) {
            LOGE("execve failed: %s", strerror(errno));
        }
    }
}

static void closeAllButThis(int exceptfd)
{
    DIR* dir = opendir("/proc/self/fd");
    if (!dir) {
        return;
    }
    int dirFd = dirfd(dir);
    struct dirent* cur;
    while ((cur = readdir(dir))) {
        if (!strcmp(cur->d_name, ".")
            || !strcmp(cur->d_name, "..")) {
            continue;
        }
        errno = 0;
        unsigned long curFd = strtoul(cur->d_name, nullptr, 10);
        if (errno)
            continue;
        if (curFd <= 2)
            continue;
        if ((curFd != dirFd) && (curFd != exceptfd)) {
            close(curFd);
        }
    }
    closedir(dir);
}
