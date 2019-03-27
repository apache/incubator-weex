/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#include "base/crash/crash_handler.h"

#include <dlfcn.h>
#include <fcntl.h>
#include <memory>
#include <ostream>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ucontext.h>
#include <unistd.h>
#include <pthread.h>
#if defined(__arm__)
#include "base/crash/backtrace.h"
#else
#include <unwind.h>
#endif
#include "base/android/weex_log.h"

namespace crash_handler {

// global pointer
CrashHandlerInfo* g_crashHandler = NULL;

pthread_mutex_t g_handlerMutex = PTHREAD_MUTEX_INITIALIZER;

CrashHandlerInfo::SignalInfo s_hookSignals[] = {
#define DECLARE_SIGNAL(sig) { sig, #sig },
DECLARE_SIGNAL(SIGFPE)
DECLARE_SIGNAL(SIGBUS)
DECLARE_SIGNAL(SIGSEGV)
DECLARE_SIGNAL(SIGABRT)
DECLARE_SIGNAL(SIGILL)
DECLARE_SIGNAL(SIGTRAP)
{ 0, nullptr }
};

/* static functions */
static bool isFileAccess(const char* file)
{
    int ret;

    struct stat mystat;
    ret = stat(file, &mystat);
    if (ret == -1) {
        LOG(INFO) << "CrashHandlerInfo file:" << file << " not access errno:" << errno;
        return false;
    }
    return true;
}

static void crashSigAction(int num, siginfo_t* info, void* ucontext)
{
    bool handled = false;
    handled = g_crashHandler->handleSignal(num, info, ucontext);

    //If nothing to do here, we exit current process normally
    _exit(0);
}

#if defined(__arm__)
static int traceFunction(uintptr_t ip, void* arg)
{
    if (g_crashHandler->printIP(reinterpret_cast<void*>(ip)))
        return BACKTRACE_CONTINUE;
    return BACKTRACE_ABORT;
}
#else
static _Unwind_Reason_Code traceFunction(_Unwind_Context* context, void* arg)
{
    void* ip = (void*)_Unwind_GetIP(context);
    if (g_crashHandler->printIP(ip))
        return _URC_NO_REASON;
    return _URC_NORMAL_STOP;
}
#endif

/* Class CrashHandlerInfo */
CrashHandlerInfo::CrashHandlerInfo(std::string fileName)
            : m_dumpFileFd(-1),
              m_mapsFileFd(-1),
              m_crash_occurs(false),
              m_dumpFileName(fileName)
{
    if (!g_crashHandler)
        g_crashHandler = this;
}

CrashHandlerInfo::~CrashHandlerInfo()
{
    // Close fds
    if (m_dumpFileFd != -1)
        close(m_dumpFileFd);
    if (m_mapsFileFd != -1)
        close(m_mapsFileFd);

    g_crashHandler = nullptr;
}

void CrashHandlerInfo::parpareFds()
{
    // open dump file
    uint32_t flags = 0;
    if (!isFileAccess(m_dumpFileName.c_str()))
        flags = O_CREAT;
    flags |= O_WRONLY;
    uint32_t mode = S_IRUSR | S_IWUSR;
    m_dumpFileFd = ::open(m_dumpFileName.c_str(), flags, mode);
    if (m_dumpFileFd < 0) {
        LOG(ERROR) << "CrashHandlerInfo open dump file:" << m_dumpFileName.c_str() << " error: " << errno;
    }

    // open maps file
    m_mapsFileFd = ::open("/proc/self/maps", O_RDONLY);
    if (m_mapsFileFd < 0) {
        LOG(ERROR) << "CrashHandlerInfo open /proc/self/maps failed errno:" << errno;
    }
}

bool CrashHandlerInfo::initializeCrashHandler()
{
    // we open the file here because the crash maybe damage the thread stack
    parpareFds();

    // Install signal handler
    for (int i = 0; s_hookSignals[i].signum; ++i) {
        struct sigaction mysigaction = { 0 };
        mysigaction.sa_sigaction = crashSigAction;
        mysigaction.sa_flags = SA_SIGINFO;
        if (-1 == sigaction(s_hookSignals[i].signum, &mysigaction, &m_sigaction[i])) {
          LOG(ERROR) << "CrashHandlerInfo install signal(" << s_hookSignals[i].signum << ") failed errno:" << errno;
          return false;
        }
    }
    return true;
}

bool CrashHandlerInfo::handleSignal(int signum, siginfo_t* siginfo, void* ucontext)
{
    //TODO return false: add condition to filter which CrashHandlerInfo to handler the
    //signal

    m_crash_occurs = true;
    LOG(INFO) << "CrashHandlerInfo::handleSignal";
    const char* signalName = nullptr;
    for (int i = 0; s_hookSignals[i].signum; ++i) {
        if (s_hookSignals[i].signum == signum) {
            signalName = s_hookSignals[i].signame;
        }
    }
    if (!signalName)
        signalName = "unknown signal";

    printf("SIG: %s (%d), fault addr: %p\n", signalName, signum, siginfo->si_addr);

    if (siginfo->si_code == SI_USER) {
        printf("Killed by pid: %d, uid: %d\n", siginfo->si_pid, siginfo->si_uid);
    }
    m_mcontext = static_cast<ucontext_t*>(ucontext)->uc_mcontext;
    if (m_mapsFileFd < 0) {
        printf("\nmaps: read maps file error! no dump message here! \n");
    } else {
        printMaps();
        printUnwind();
        printContext();
        saveFileContent();
    }

    // close fds here
    if (m_dumpFileFd != -1)
        close(m_dumpFileFd);
    if (m_mapsFileFd != -1)
        close(m_mapsFileFd);

    return true;
}

void CrashHandlerInfo::printMaps()
{
    printf("\nmaps:\n");
    std::unique_ptr<char[]> buf(new char[CrashHandlerInfo::BUF_SIZE]);
    const char* line;
    while (true) {
        ssize_t byteRead = ::read(m_mapsFileFd, buf.get(), CrashHandlerInfo::BUF_SIZE);
        // check error.
        if (byteRead == -1 && errno != EINTR)
            return;
        // check eof.
        if (byteRead == 0)
            return;
        size_t fileContentOldSize = m_fileContent.size();
        m_fileContent.resize(fileContentOldSize + byteRead);
        memcpy(const_cast<char*>(m_fileContent.data()) + fileContentOldSize, buf.get(), byteRead);
    }
}

void CrashHandlerInfo::printContext()
{
    const mcontext_t& mcontext = m_mcontext;
#if defined(__arm__)
    printf("r0: %08lx r1: %08lx r2: %08lx r3: %08lx\n",
        static_cast<unsigned long>(mcontext.arm_r0),
        static_cast<unsigned long>(mcontext.arm_r1),
        static_cast<unsigned long>(mcontext.arm_r2),
        static_cast<unsigned long>(mcontext.arm_r3));
    printf("r4: %08lx r5: %08lx r6: %08lx r7: %08lx\n",
        static_cast<unsigned long>(mcontext.arm_r4),
        static_cast<unsigned long>(mcontext.arm_r5),
        static_cast<unsigned long>(mcontext.arm_r6),
        static_cast<unsigned long>(mcontext.arm_r7));
    printf("r8: %08lx r9: %08lx r10: %08lx fp: %08lx\n",
        static_cast<unsigned long>(mcontext.arm_r8),
        static_cast<unsigned long>(mcontext.arm_r9),
        static_cast<unsigned long>(mcontext.arm_r10),
        static_cast<unsigned long>(mcontext.arm_fp));
    printf("ip: %08lx sp: %08lx lr: %08lx pc: %08lx, cpsr: %08lx, arm_fault_addr: %08lx\n",
        static_cast<unsigned long>(mcontext.arm_ip),
        static_cast<unsigned long>(mcontext.arm_sp),
        static_cast<unsigned long>(mcontext.arm_lr),
        static_cast<unsigned long>(mcontext.arm_pc),
        static_cast<unsigned long>(mcontext.arm_cpsr),
        static_cast<unsigned long>(mcontext.fault_address));
    #define CALL_PRINT_REG_CONTENT(name) printRegContent(reinterpret_cast<void*>(mcontext.arm_##name), #name)
    printf("\n");
    CALL_PRINT_REG_CONTENT(r0);
    CALL_PRINT_REG_CONTENT(r1);
    CALL_PRINT_REG_CONTENT(r2);
    CALL_PRINT_REG_CONTENT(r3);
    CALL_PRINT_REG_CONTENT(r4);
    CALL_PRINT_REG_CONTENT(r5);
    CALL_PRINT_REG_CONTENT(r6);
    CALL_PRINT_REG_CONTENT(r7);
    CALL_PRINT_REG_CONTENT(r8);
    CALL_PRINT_REG_CONTENT(r9);
    CALL_PRINT_REG_CONTENT(r10);
    CALL_PRINT_REG_CONTENT(fp);
    CALL_PRINT_REG_CONTENT(ip);
    CALL_PRINT_REG_CONTENT(sp);
    CALL_PRINT_REG_CONTENT(lr);
    CALL_PRINT_REG_CONTENT(pc);
#elif defined(__i386__)
    printf("eax: %08lx ebx: %08lx ecx: %08lx edx: %08lx\n",
        static_cast<unsigned long>(mcontext.gregs[REG_EAX]),
        static_cast<unsigned long>(mcontext.gregs[REG_EBX]),
        static_cast<unsigned long>(mcontext.gregs[REG_ECX]),
        static_cast<unsigned long>(mcontext.gregs[REG_EDX]));

    printf("esi: %08lx edi: %08lx ebp: %08lx esp: %08lx\n",
        static_cast<unsigned long>(mcontext.gregs[REG_ESI]),
        static_cast<unsigned long>(mcontext.gregs[REG_EDI]),
        static_cast<unsigned long>(mcontext.gregs[REG_EBP]),
        static_cast<unsigned long>(mcontext.gregs[REG_ESP]));

    printf("eip: %08lx eflags: %08lx trapno: %08lx error: %08lx\n",
        static_cast<unsigned long>(mcontext.gregs[REG_EIP]),
        static_cast<unsigned long>(mcontext.gregs[REG_EFL]),
        static_cast<unsigned long>(mcontext.gregs[REG_TRAPNO]),
        static_cast<unsigned long>(mcontext.gregs[REG_ERR]));

    printf("cs: %04lx fs: %04lx gs:%04lx ss: %04lx ds: %04lx\n",
        static_cast<unsigned long>(mcontext.gregs[REG_CS]),
        static_cast<unsigned long>(mcontext.gregs[REG_FS]),
        static_cast<unsigned long>(mcontext.gregs[REG_GS]),
        static_cast<unsigned long>(mcontext.gregs[REG_SS]),
        static_cast<unsigned long>(mcontext.gregs[REG_DS]));

    #define CALL_PRINT_REG_CONTENT(name) printRegContent(reinterpret_cast<void*>(mcontext.gregs[REG_##name]), #name)
    printf("\n");
    CALL_PRINT_REG_CONTENT(EAX);
    CALL_PRINT_REG_CONTENT(EBX);
    CALL_PRINT_REG_CONTENT(ECX);
    CALL_PRINT_REG_CONTENT(EDX);
    CALL_PRINT_REG_CONTENT(ESI);
    CALL_PRINT_REG_CONTENT(EDI);
    CALL_PRINT_REG_CONTENT(EBP);
    CALL_PRINT_REG_CONTENT(ESP);
    CALL_PRINT_REG_CONTENT(EIP);
#else
    //TODO: other platform
#endif
}

void CrashHandlerInfo::printRegContent(void* addr, const char* name)
{
#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif
#define PAGE_START(addr) reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(addr) & ~(PAGE_SIZE - 1))
#define IS_PAGE_START(addr) !(reinterpret_cast<uintptr_t>(addr) & (PAGE_SIZE - 1))
    void* printStart = reinterpret_cast<void*>((reinterpret_cast<uintptr_t>(addr) - 0x20) & ~7);
    unsigned char vec;
    bool printZero = false;
    if (!IS_PAGE_START(printStart))
        printZero = mincore(PAGE_START(printStart), PAGE_SIZE, &vec);
    printf("\n%s:\n", name);
    for (int i = 0; i < 4 * 40; ++i) {
        if (!(i & 3)) {
            printf("%08lx: ", reinterpret_cast<unsigned long>(printStart));
        }
        if (IS_PAGE_START(printStart)) {
            printZero = mincore(printStart, PAGE_SIZE, &vec);
        }
        if (printZero)
            printf("%08lx", 0L);
        else
            printf("%08lx", *reinterpret_cast<unsigned long*>(printStart));
        if ((i & 3) == 3)
            printf("\n");
        else
            printf(" ");
        printStart = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(printStart) + sizeof(void*));
    }
    printf("\n");
}

void CrashHandlerInfo::printUnwind()
{
    printf("\nbacktrace:\n");
#if defined(__arm__)
    printIP(reinterpret_cast<void*>(m_mcontext.arm_pc));
    mybacktrace(traceFunction, nullptr, &m_mcontext);
#else
    _Unwind_Backtrace(traceFunction, nullptr);
#endif
    printf("\n");
}

bool CrashHandlerInfo::printIP(void* ip)
{
    Dl_info info;
    if (dladdr(ip, &info)) {
        printf("%s + %08lx(%s)\n", info.dli_fname,
            reinterpret_cast<unsigned long>(ip) - reinterpret_cast<unsigned long>(info.dli_fbase),
            info.dli_sname ? info.dli_sname : "unknown symbol");
    } else {
        printf("unknown dso: %p\n", ip);
    }

    // TODO: Does m_unwinded work?
    if (m_unwinded > maxUnwindCount)
        return false;
    return true;
}

bool CrashHandlerInfo::ensureDirectory(const char* base)
{
    int ret;
    struct stat mystat;

    ret = stat(base, &mystat);
    if (ret == -1) {
        int myerrno = errno;
        if (myerrno == ENOENT) {
            ret = mkdir(base, 0755);
            if (ret == -1)
                return false;
            return true;
        } else {
            return false;
        }
    }
    return true;
}

void CrashHandlerInfo::printf(const char* fmt, ...)
{
    va_list arg_list;
    int size;
    va_start(arg_list, fmt);
    size = vsnprintf(nullptr, 0, fmt, arg_list);
    va_end(arg_list);
    if (size > 0) {
        size_t oldSize = m_fileContent.size();
        m_fileContent.reserve(oldSize + size + 1);
        m_fileContent.resize(oldSize + size);
        va_start(arg_list, fmt);
        vsnprintf(const_cast<char*>(m_fileContent.data()) + oldSize, size + 1, fmt, arg_list);
        va_end(arg_list);
    }
}

void CrashHandlerInfo::saveFileContent()
{
    LOG(INFO) << "saveFileContent: " << std::endl <<  m_fileContent.data();
    const char* str = m_fileContent.data();
    size_t size = m_fileContent.size();
    while (size) {
        ssize_t byteWritten = ::write(m_dumpFileFd, str, size);
        if (byteWritten == -1 && errno != EINTR)
            return;
        str += byteWritten;
        size -= byteWritten;
    }
}
bool CrashHandlerInfo::is_crashed() {
    return m_crash_occurs;
}

}
