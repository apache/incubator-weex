/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "SigillCrashAnalyzer.h"

#include "CallFrame.h"
#include "CodeBlock.h"
#include "MachineContext.h"
#include "VMInspector.h"
#include <mutex>
#include <wtf/StdLibExtras.h>

#if USE(ARM64_DISASSEMBLER)
#include "A64DOpcode.h"
#endif

#if HAVE(SIGNAL_H)
#include <signal.h>
#endif

namespace JSC {

struct SignalContext;

class SigillCrashAnalyzer {
public:
    static SigillCrashAnalyzer& instance();

    enum class CrashSource {
        Unknown,
        JavaScriptCore,
        Other,
    };
    CrashSource analyze(SignalContext&);

private:
    SigillCrashAnalyzer() { }
    void dumpCodeBlock(CodeBlock*, void* machinePC);

#if USE(ARM64_DISASSEMBLER)
    A64DOpcode m_arm64Opcode;
#endif
};

#if OS(DARWIN)

#if USE(OS_LOG)

#define log(format, ...) \
    os_log_info(OS_LOG_DEFAULT, format, ##__VA_ARGS__)

#else // USE(OS_LOG)

#define log(format, ...) \
    dataLogF(format, ##__VA_ARGS__)
    
#endif // USE(OS_LOG)

struct SignalContext {
    SignalContext(mcontext_t& mcontext)
        : mcontext(mcontext)
        , machinePC(MachineContext::instructionPointer(mcontext))
        , stackPointer(MachineContext::stackPointer(mcontext))
        , framePointer(MachineContext::framePointer(mcontext))
    { }

    void dump()
    {
#if CPU(X86_64)
#define FOR_EACH_REGISTER(v) \
        v(rax) \
        v(rbx) \
        v(rcx) \
        v(rdx) \
        v(rdi) \
        v(rsi) \
        v(rbp) \
        v(rsp) \
        v(r8) \
        v(r9) \
        v(r10) \
        v(r11) \
        v(r12) \
        v(r13) \
        v(r14) \
        v(r15) \
        v(rip) \
        v(rflags) \
        v(cs) \
        v(fs) \
        v(gs)

#define DUMP_REGISTER(__reg) \
        log("Register " #__reg ": %p", reinterpret_cast<void*>(mcontext->__ss.__##__reg));
        FOR_EACH_REGISTER(DUMP_REGISTER)
#undef FOR_EACH_REGISTER

#elif CPU(ARM64)
        int i;
        for (i = 0; i < 28; i += 4) {
            log("x%d: %016llx x%d: %016llx x%d: %016llx x%d: %016llx",
                i, mcontext->__ss.__x[i],
                i+1, mcontext->__ss.__x[i+1],
                i+2, mcontext->__ss.__x[i+2],
                i+3, mcontext->__ss.__x[i+3]);
        }
        ASSERT(i < 29);
        log("x%d: %016llx fp: %016llx lr: %016llx",
            i, mcontext->__ss.__x[i], mcontext->__ss.__fp, mcontext->__ss.__lr);
        log("sp: %016llx pc: %016llx cpsr: %08x",
            mcontext->__ss.__sp, mcontext->__ss.__pc, mcontext->__ss.__cpsr);
#endif
    }

    mcontext_t& mcontext;
    void* machinePC;
    void* stackPointer;
    void* framePointer;
};

struct sigaction originalSigIllAction;

static void handleCrash(int signalNumber, siginfo_t* info, void* uap)
{
    SignalContext context(static_cast<ucontext_t*>(uap)->uc_mcontext);
    SigillCrashAnalyzer& analyzer = SigillCrashAnalyzer::instance();
    auto crashSource = analyzer.analyze(context);

    auto originalAction = originalSigIllAction.sa_sigaction;
    if (originalAction) {
        // It is always safe to just invoke the original handler using the sa_sigaction form
        // without checking for the SA_SIGINFO flag. If the original handler is of the
        // sa_handler form, it will just ignore the 2nd and 3rd arguments since sa_handler is a
        // subset of sa_sigaction. This is what the man pages says the OS does anyway.
        originalAction(signalNumber, info, uap);
    }

    if (crashSource == SigillCrashAnalyzer::CrashSource::JavaScriptCore) {
        // Restore the default handler so that we can get a core dump.
        struct sigaction defaultAction;
        defaultAction.sa_handler = SIG_DFL;
        sigfillset(&defaultAction.sa_mask);
        defaultAction.sa_flags = 0;
        sigaction(SIGILL, &defaultAction, nullptr);
    } else if (!originalAction) {
        // Pre-emptively restore the default handler but we may roll it back below.
        struct sigaction currentAction;
        struct sigaction defaultAction;
        defaultAction.sa_handler = SIG_DFL;
        sigfillset(&defaultAction.sa_mask);
        defaultAction.sa_flags = 0;
        sigaction(SIGILL, &defaultAction, &currentAction);

        if (currentAction.sa_sigaction != handleCrash) {
            // This means that there's a client handler installed after us. This also means
            // that the client handler thinks it was able to recover from the SIGILL, and
            // did not uninstall itself. We can't argue with this because the crash isn't
            // known to be from a JavaScriptCore source. Hence, restore the client handler
            // and keep going.
            sigaction(SIGILL, &currentAction, nullptr);
        }
    }
}

static void installCrashHandler()
{
#if CPU(X86_64) || CPU(ARM64)
    struct sigaction action;
    action.sa_sigaction = reinterpret_cast<void (*)(int, siginfo_t *, void *)>(handleCrash);
    sigfillset(&action.sa_mask);
    action.sa_flags = SA_SIGINFO;
    sigaction(SIGILL, &action, &originalSigIllAction);
#else
    UNUSED_PARAM(handleCrash);
#endif
}

#else // OS(DARWIN)

#define log(format, ...) do { } while (false)
    
struct SignalContext {
    SignalContext() { }

    void dump() { }

    void* machinePC;
    void* stackPointer;
    void* framePointer;
};

static void installCrashHandler()
{
    // Do nothing. Not supported for this platform.
}

#endif // OS(DARWIN)

SigillCrashAnalyzer& SigillCrashAnalyzer::instance()
{
    static SigillCrashAnalyzer* analyzer;
    static std::once_flag once;
    std::call_once(once, [] {
        installCrashHandler();
        analyzer = new SigillCrashAnalyzer;
    });
    return *analyzer;
}

void enableSigillCrashAnalyzer()
{
    // Just instantiating the SigillCrashAnalyzer will enable it.
    SigillCrashAnalyzer::instance();
}

auto SigillCrashAnalyzer::analyze(SignalContext& context) -> CrashSource
{
    CrashSource crashSource = CrashSource::Unknown;
    log("BEGIN SIGILL analysis");

    do {
        // First, dump the signal context info so that we'll at least have the same info
        // that the default crash handler would given us in case this crash analyzer
        // itself crashes.
        context.dump();

        VMInspector& inspector = VMInspector::instance();

        // Use a timeout period of 2 seconds. The client is about to crash, and we don't
        // want to turn the crash into a hang by re-trying the lock for too long.
        auto expectedLocker = inspector.lock(Seconds(2));
        if (!expectedLocker) {
            ASSERT(expectedLocker.error() == VMInspector::Error::TimedOut);
            log("ERROR: Unable to analyze SIGILL. Timed out while waiting to iterate VMs.");
            break;
        }
        auto& locker = expectedLocker.value();

        void* pc = context.machinePC;
        auto isInJITMemory = inspector.isValidExecutableMemory(locker, pc);
        if (!isInJITMemory) {
            log("ERROR: Timed out: not able to determine if pc %p is in valid JIT executable memory", pc);
            break;
        }
        if (!isInJITMemory.value()) {
            log("pc %p is NOT in valid JIT executable memory", pc);
            crashSource = CrashSource::Other;
            break;
        }
        log("pc %p is in valid JIT executable memory", pc);
        crashSource = CrashSource::JavaScriptCore;

#if CPU(ARM64)
        size_t pcAsSize = reinterpret_cast<size_t>(pc);
        if (pcAsSize != roundUpToMultipleOf<sizeof(uint32_t)>(pcAsSize)) {
            log("pc %p is NOT properly aligned", pc);
            break;
        }

        // We know it's safe to read the word at the PC because we're handling a SIGILL.
        // Otherwise, we would have crashed with a SIGBUS instead.
        uint32_t wordAtPC = *reinterpret_cast<uint32_t*>(pc);
        log("instruction bits at pc %p is: 0x%08x", pc, wordAtPC);
#endif

        auto expectedCodeBlock = inspector.codeBlockForMachinePC(locker, pc);
        if (!expectedCodeBlock) {
            if (expectedCodeBlock.error() == VMInspector::Error::TimedOut)
                log("ERROR: Timed out: not able to determine if pc %p is in a valid CodeBlock", pc);
            else
                log("The current thread does not own any VM JSLock");
            break;
        }
        CodeBlock* codeBlock = expectedCodeBlock.value();
        if (!codeBlock) {
            log("machine PC %p does not belong to any CodeBlock in the currently entered VM", pc);
            break;
        }

        log("pc %p belongs to CodeBlock %p of type %s", pc, codeBlock, JITCode::typeName(codeBlock->jitType()));

        dumpCodeBlock(codeBlock, pc);
    } while (false);

    log("END SIGILL analysis");
    return crashSource;
}

void SigillCrashAnalyzer::dumpCodeBlock(CodeBlock* codeBlock, void* machinePC)
{
#if CPU(ARM64)
    JITCode* jitCode = codeBlock->jitCode().get();

    // Dump the raw bits of the code.
    uint32_t* start = reinterpret_cast<uint32_t*>(jitCode->start());
    uint32_t* end = reinterpret_cast<uint32_t*>(jitCode->end());
    log("JITCode %p [%p-%p]:", jitCode, start, end);
    if (start < end) {
        uint32_t* p = start;
        while (p + 8 <= end) {
            log("[%p-%p]: %08x %08x %08x %08x %08x %08x %08x %08x", p, p+7, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
            p += 8;
        }
        if (p + 7 <= end)
            log("[%p-%p]: %08x %08x %08x %08x %08x %08x %08x", p, p+6, p[0], p[1], p[2], p[3], p[4], p[5], p[6]);
        else if (p + 6 <= end)
            log("[%p-%p]: %08x %08x %08x %08x %08x %08x", p, p+5, p[0], p[1], p[2], p[3], p[4], p[5]);
        else if (p + 5 <= end)
            log("[%p-%p]: %08x %08x %08x %08x %08x", p, p+4, p[0], p[1], p[2], p[3], p[4]);
        else if (p + 4 <= end)
            log("[%p-%p]: %08x %08x %08x %08x", p, p+3, p[0], p[1], p[2], p[3]);
        if (p + 3 <= end)
            log("[%p-%p]: %08x %08x %08x", p, p+2, p[0], p[1], p[2]);
        else if (p + 2 <= end)
            log("[%p-%p]: %08x %08x", p, p+1, p[0], p[1]);
        else if (p + 1 <= end)
            log("[%p-%p]: %08x", p, p, p[0]);
    }

    // Dump the disassembly of the code.
    log("Disassembly:");
    uint32_t* currentPC = reinterpret_cast<uint32_t*>(jitCode->executableAddress());
    size_t byteCount = jitCode->size();
    while (byteCount) {
        char pcString[24];
        if (currentPC == machinePC) {
            snprintf(pcString, sizeof(pcString), "* 0x%lx", reinterpret_cast<unsigned long>(currentPC));
            log("%20s: %s    <=========================", pcString, m_arm64Opcode.disassemble(currentPC));
        } else {
            snprintf(pcString, sizeof(pcString), "0x%lx", reinterpret_cast<unsigned long>(currentPC));
            log("%20s: %s", pcString, m_arm64Opcode.disassemble(currentPC));
        }
        currentPC++;
        byteCount -= sizeof(uint32_t);
    }
#else
    UNUSED_PARAM(codeBlock);
    UNUSED_PARAM(machinePC);
    // Not implemented yet.
#endif
}

} // namespace JSC
