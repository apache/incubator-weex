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
#ifndef CRASHHANDLER_H
#define CRASHHANDLER_H

#include <signal.h>
#include <string>
#include <sys/ucontext.h>

namespace crash_handler {

class CrashHandlerInfo {
public:
    CrashHandlerInfo(std::string fileName);
    ~CrashHandlerInfo();

    void parpareFds();
    bool initializeCrashHandler();
    bool handleSignal(int signum, siginfo_t* siginfo, void* ucontext);
    bool printIP(void* addr);
    bool is_crashed();
    struct SignalInfo {
        int signum;
        const char* signame;
    };
    void setEnableAlarmSignal(bool handle){handle_alarm_signal = handle;}

private:
    void printContext();
    void printMaps();
    void printRegContent(void* addr, const char* name);
    void printUnwind();
    bool ensureDirectory(const char* base);
    void printf(const char* fmt, ...);
    void saveFileContent();

    /* static members */
    static const size_t BUF_SIZE = 1024;
    static const int maxUnwindCount = 32;

    /* members */
    struct sigaction m_sigaction[16];

    // File descriptior to store crash dump message
    int m_dumpFileFd;

    // File descriptior for /prco/self/maps
    int m_mapsFileFd;
    int m_unwinded;

    std::string m_dumpFileName;
    std::string m_fileContent;
    mcontext_t m_mcontext;
    volatile bool m_crash_occurs;
    volatile bool handle_alarm_signal = true;
};
}
#endif /* CRASHHANDLER_H */
