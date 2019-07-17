/*
 * Copyright (C) 2008-2017 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer. 
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution. 
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "CallData.h"
#include "ConcurrentJSLock.h"
#include "ControlFlowProfiler.h"
#include "DateInstanceCache.h"
#include "DeleteAllCodeEffort.h"
#include "ExceptionEventLocation.h"
#include "ExecutableAllocator.h"
#include "FunctionHasExecutedCache.h"
#include "Heap.h"
#include "Intrinsic.h"
#include "JITThunks.h"
#include "JSCJSValue.h"
#include "JSDestructibleObjectSubspace.h"
#include "JSLock.h"
#include "JSSegmentedVariableObjectSubspace.h"
#include "JSStringSubspace.h"
#include "MacroAssemblerCodeRef.h"
#include "Microtask.h"
#include "NumericStrings.h"
#include "PrivateName.h"
#include "PrototypeMap.h"
#include "SmallStrings.h"
#include "SourceCode.h"
#include "Strong.h"
#include "Subspace.h"
#include "TemplateRegistryKeyTable.h"
#include "ThunkGenerators.h"
#include "VMEntryRecord.h"
#include "VMTraps.h"
#include "Watchpoint.h"
#include <wtf/Bag.h>
#include <wtf/BumpPointerAllocator.h>
#include <wtf/DateMath.h>
#include <wtf/Deque.h>
#include <wtf/DoublyLinkedList.h>
#include <wtf/Forward.h>
#include <wtf/HashMap.h>
#include <wtf/HashSet.h>
#include <wtf/StackBounds.h>
#include <wtf/Stopwatch.h>
#include <wtf/ThreadSafeRefCounted.h>
#include <wtf/ThreadSpecific.h>
#include <wtf/WTFThreadData.h>
#include <wtf/text/SymbolRegistry.h>
#include <wtf/text/WTFString.h>
#if ENABLE(REGEXP_TRACING)
#include <wtf/ListHashSet.h>
#endif

namespace WTF {
class SimpleStats;
} // namespace WTF
using WTF::SimpleStats;

namespace JSC {

class BuiltinExecutables;
class BytecodeIntrinsicRegistry;
class CodeBlock;
class CodeCache;
class CommonIdentifiers;
class CustomGetterSetter;
class ExecState;
class Exception;
class ExceptionScope;
class HandleStack;
class TypeProfiler;
class TypeProfilerLog;
class HasOwnPropertyCache;
class HeapProfiler;
class Identifier;
class Interpreter;
class JSCustomGetterSetterFunction;
class JSGlobalObject;
class JSObject;
class JSWebAssemblyInstance;
class LLIntOffsetsExtractor;
class NativeExecutable;
class RegExpCache;
class Register;
class RegisterAtOffsetList;
#if ENABLE(SAMPLING_PROFILER)
class SamplingProfiler;
#endif
class ShadowChicken;
class ScriptExecutable;
class SourceProvider;
class SourceProviderCache;
class StackFrame;
class Structure;
#if ENABLE(REGEXP_TRACING)
class RegExp;
#endif
class Symbol;
class TypedArrayController;
class UnlinkedCodeBlock;
class UnlinkedEvalCodeBlock;
class UnlinkedFunctionExecutable;
class UnlinkedProgramCodeBlock;
class UnlinkedModuleProgramCodeBlock;
class VirtualRegister;
class VMEntryScope;
class Watchdog;
class Watchpoint;
class WatchpointSet;

#if ENABLE(DFG_JIT)
namespace DFG {
class LongLivedState;
}
#endif // ENABLE(DFG_JIT)
#if ENABLE(FTL_JIT)
namespace FTL {
class Thunks;
}
#endif // ENABLE(FTL_JIT)
namespace CommonSlowPaths {
struct ArityCheckData;
}
namespace Profiler {
class Database;
}
namespace DOMJIT {
class Signature;
}
#if ENABLE(WEBASSEMBLY)
namespace Wasm {
class SignatureInformation;
}
#endif

struct HashTable;
struct Instruction;

struct LocalTimeOffsetCache {
    LocalTimeOffsetCache()
        : start(0.0)
        , end(-1.0)
        , increment(0.0)
        , timeType(WTF::UTCTime)
    {
    }

    void reset()
    {
        offset = LocalTimeOffset();
        start = 0.0;
        end = -1.0;
        increment = 0.0;
        timeType = WTF::UTCTime;
    }

    LocalTimeOffset offset;
    double start;
    double end;
    double increment;
    WTF::TimeType timeType;
};

class QueuedTask {
    WTF_MAKE_NONCOPYABLE(QueuedTask);
    WTF_MAKE_FAST_ALLOCATED;
public:
    void run();

    QueuedTask(VM& vm, JSGlobalObject* globalObject, Ref<Microtask>&& microtask)
        : m_globalObject(vm, globalObject)
        , m_microtask(WTFMove(microtask))
    {
    }

private:
    Strong<JSGlobalObject> m_globalObject;
    Ref<Microtask> m_microtask;
};

class ConservativeRoots;

#if COMPILER(MSVC)
#pragma warning(push)
#pragma warning(disable: 4200) // Disable "zero-sized array in struct/union" warning
#endif
struct ScratchBuffer {
    ScratchBuffer()
    {
        u.m_activeLength = 0;
    }

    static ScratchBuffer* create(size_t size)
    {
        ScratchBuffer* result = new (fastMalloc(ScratchBuffer::allocationSize(size))) ScratchBuffer;

        return result;
    }

    static size_t allocationSize(size_t bufferSize) { return sizeof(ScratchBuffer) + bufferSize; }
    void setActiveLength(size_t activeLength) { u.m_activeLength = activeLength; }
    size_t activeLength() const { return u.m_activeLength; };
    size_t* activeLengthPtr() { return &u.m_activeLength; };
    void* dataBuffer() { return m_buffer; }

    union {
        size_t m_activeLength;
        double pad; // Make sure m_buffer is double aligned.
    } u;
#if CPU(MIPS) && (defined WTF_MIPS_ARCH_REV && WTF_MIPS_ARCH_REV == 2)
    void* m_buffer[0] __attribute__((aligned(8)));
#else
    void* m_buffer[0];
#endif
};
#if COMPILER(MSVC)
#pragma warning(pop)
#endif

class VM : public ThreadSafeRefCounted<VM>, public DoublyLinkedListNode<VM> {
public:
    // WebCore has a one-to-one mapping of threads to VMs;
    // either create() or createLeaked() should only be called once
    // on a thread, this is the 'default' VM (it uses the
    // thread's default string uniquing table from wtfThreadData).
    // API contexts created using the new context group aware interface
    // create APIContextGroup objects which require less locking of JSC
    // than the old singleton APIShared VM created for use by
    // the original API.
    enum VMType { Default, APIContextGroup, APIShared };

    struct ClientData {
        JS_EXPORT_PRIVATE virtual ~ClientData() = 0;
    };

    bool isSharedInstance() { return vmType == APIShared; }
    bool usingAPI() { return vmType != Default; }
    JS_EXPORT_PRIVATE static bool sharedInstanceExists();
    JS_EXPORT_PRIVATE static VM& sharedInstance();

    JS_EXPORT_PRIVATE static Ref<VM> create(HeapType = SmallHeap);
    JS_EXPORT_PRIVATE static Ref<VM> createLeaked(HeapType = SmallHeap);
    static Ref<VM> createContextGroup(HeapType = SmallHeap);
    JS_EXPORT_PRIVATE ~VM();

    Watchdog& ensureWatchdog();
    Watchdog* watchdog() { return m_watchdog.get(); }

    HeapProfiler* heapProfiler() const { return m_heapProfiler.get(); }
    JS_EXPORT_PRIVATE HeapProfiler& ensureHeapProfiler();

#if ENABLE(SAMPLING_PROFILER)
    SamplingProfiler* samplingProfiler() { return m_samplingProfiler.get(); }
    JS_EXPORT_PRIVATE SamplingProfiler& ensureSamplingProfiler(RefPtr<Stopwatch>&&);
#endif

private:
    RefPtr<JSLock> m_apiLock;

public:
#if ENABLE(ASSEMBLER)
    // executableAllocator should be destructed after the heap, as the heap can call executableAllocator
    // in its destructor.
    ExecutableAllocator executableAllocator;
#endif

    // The heap should be just after executableAllocator and before other members to ensure that it's
    // destructed after all the objects that reference it.
    Heap heap;
    
    Subspace auxiliarySpace;
    
    // Whenever possible, use subspaceFor<CellType>(vm) to get one of these subspaces.
    Subspace cellSpace;
    Subspace destructibleCellSpace;
    JSStringSubspace stringSpace;
    JSDestructibleObjectSubspace destructibleObjectSpace;
    JSSegmentedVariableObjectSubspace segmentedVariableObjectSpace;

#if ENABLE(DFG_JIT)
    std::unique_ptr<DFG::LongLivedState> dfgState;
#endif // ENABLE(DFG_JIT)

    VMType vmType;
    ClientData* clientData;
    VMEntryFrame* topVMEntryFrame;
    // NOTE: When throwing an exception while rolling back the call frame, this may be equal to
    // topVMEntryFrame.
    // FIXME: This should be a void*, because it might not point to a CallFrame.
    // https://bugs.webkit.org/show_bug.cgi?id=160441
    ExecState* topCallFrame { nullptr };
    JSWebAssemblyInstance* topJSWebAssemblyInstance;
    Strong<Structure> structureStructure;
    Strong<Structure> structureRareDataStructure;
    Strong<Structure> terminatedExecutionErrorStructure;
    Strong<Structure> stringStructure;
    Strong<Structure> propertyNameIteratorStructure;
    Strong<Structure> propertyNameEnumeratorStructure;
    Strong<Structure> customGetterSetterStructure;
    Strong<Structure> scopedArgumentsTableStructure;
    Strong<Structure> apiWrapperStructure;
    Strong<Structure> JSScopeStructure;
    Strong<Structure> executableStructure;
    Strong<Structure> nativeExecutableStructure;
    Strong<Structure> evalExecutableStructure;
    Strong<Structure> programExecutableStructure;
    Strong<Structure> functionExecutableStructure;
#if ENABLE(WEBASSEMBLY)
    Strong<Structure> webAssemblyCalleeStructure;
    Strong<Structure> webAssemblyToJSCalleeStructure;
    Strong<Structure> webAssemblyCodeBlockStructure;
    Strong<JSCell> webAssemblyToJSCallee;
#endif
    Strong<Structure> moduleProgramExecutableStructure;
    Strong<Structure> regExpStructure;
    Strong<Structure> symbolStructure;
    Strong<Structure> symbolTableStructure;
    Strong<Structure> fixedArrayStructure;
    Strong<Structure> sourceCodeStructure;
    Strong<Structure> scriptFetcherStructure;
    Strong<Structure> structureChainStructure;
    Strong<Structure> sparseArrayValueMapStructure;
    Strong<Structure> templateRegistryKeyStructure;
    Strong<Structure> arrayBufferNeuteringWatchpointStructure;
    Strong<Structure> unlinkedFunctionExecutableStructure;
    Strong<Structure> unlinkedProgramCodeBlockStructure;
    Strong<Structure> unlinkedEvalCodeBlockStructure;
    Strong<Structure> unlinkedFunctionCodeBlockStructure;
    Strong<Structure> unlinkedModuleProgramCodeBlockStructure;
    Strong<Structure> propertyTableStructure;
    Strong<Structure> weakMapDataStructure;
    Strong<Structure> inferredValueStructure;
    Strong<Structure> inferredTypeStructure;
    Strong<Structure> inferredTypeTableStructure;
    Strong<Structure> functionRareDataStructure;
    Strong<Structure> exceptionStructure;
    Strong<Structure> promiseDeferredStructure;
    Strong<Structure> internalPromiseDeferredStructure;
    Strong<Structure> nativeStdFunctionCellStructure;
    Strong<Structure> programCodeBlockStructure;
    Strong<Structure> moduleProgramCodeBlockStructure;
    Strong<Structure> evalCodeBlockStructure;
    Strong<Structure> functionCodeBlockStructure;
    Strong<Structure> hashMapBucketSetStructure;
    Strong<Structure> hashMapBucketMapStructure;
    Strong<Structure> hashMapImplSetStructure;
    Strong<Structure> hashMapImplMapStructure;

    Strong<JSCell> iterationTerminator;
    Strong<JSCell> emptyPropertyNameEnumerator;

#if ENABLE(WEBASSEMBLY)
    std::once_flag m_wasmSignatureInformationOnceFlag;
    std::unique_ptr<Wasm::SignatureInformation> m_wasmSignatureInformation;
#endif
    
    JSCell* currentlyDestructingCallbackObject;
    const ClassInfo* currentlyDestructingCallbackObjectClassInfo;

    AtomicStringTable* m_atomicStringTable;
    WTF::SymbolRegistry m_symbolRegistry;
    TemplateRegistryKeyTable m_templateRegistryKeytable;
    CommonIdentifiers* propertyNames;
    const ArgList* emptyList;
    SmallStrings smallStrings;
    NumericStrings numericStrings;
    DateInstanceCache dateInstanceCache;
    std::unique_ptr<SimpleStats> machineCodeBytesPerBytecodeWordForBaselineJIT;
    WeakGCMap<std::pair<CustomGetterSetter*, int>, JSCustomGetterSetterFunction> customGetterSetterFunctionMap;
    WeakGCMap<StringImpl*, JSString, PtrHash<StringImpl*>> stringCache;
    Strong<JSString> lastCachedString;

    AtomicStringTable* atomicStringTable() const { return m_atomicStringTable; }
    WTF::SymbolRegistry& symbolRegistry() { return m_symbolRegistry; }

    TemplateRegistryKeyTable& templateRegistryKeyTable() { return m_templateRegistryKeytable; }

    WeakGCMap<SymbolImpl*, Symbol, PtrHash<SymbolImpl*>> symbolImplToSymbolMap;

    enum class DeletePropertyMode {
        // Default behaviour of deleteProperty, matching the spec.
        Default,
        // This setting causes deleteProperty to force deletion of all
        // properties including those that are non-configurable (DontDelete).
        IgnoreConfigurable
    };

    DeletePropertyMode deletePropertyMode()
    {
        return m_deletePropertyMode;
    }

    class DeletePropertyModeScope {
    public:
        DeletePropertyModeScope(VM& vm, DeletePropertyMode mode)
            : m_vm(vm)
            , m_previousMode(vm.m_deletePropertyMode)
        {
            m_vm.m_deletePropertyMode = mode;
        }

        ~DeletePropertyModeScope()
        {
            m_vm.m_deletePropertyMode = m_previousMode;
        }

    private:
        VM& m_vm;
        DeletePropertyMode m_previousMode;
    };

#if ENABLE(JIT)
    bool canUseJIT() { return m_canUseJIT; }
#else
    bool canUseJIT() { return false; } // interpreter only
#endif

#if ENABLE(YARR_JIT)
    bool canUseRegExpJIT() { return m_canUseRegExpJIT; }
#else
    bool canUseRegExpJIT() { return false; } // interpreter only
#endif

    SourceProviderCache* addSourceProviderCache(SourceProvider*);
    void clearSourceProviderCaches();

    PrototypeMap prototypeMap;

    typedef HashMap<RefPtr<SourceProvider>, RefPtr<SourceProviderCache>> SourceProviderCacheMap;
    SourceProviderCacheMap sourceProviderCacheMap;
    Interpreter* interpreter;
#if ENABLE(JIT)
    std::unique_ptr<JITThunks> jitStubs;
    MacroAssemblerCodeRef getCTIStub(ThunkGenerator generator)
    {
        return jitStubs->ctiStub(this, generator);
    }
    
    std::unique_ptr<RegisterAtOffsetList> allCalleeSaveRegisterOffsets;
    
    RegisterAtOffsetList* getAllCalleeSaveRegisterOffsets() { return allCalleeSaveRegisterOffsets.get(); }

#endif // ENABLE(JIT)
    std::unique_ptr<CommonSlowPaths::ArityCheckData> arityCheckData;
#if ENABLE(FTL_JIT)
    std::unique_ptr<FTL::Thunks> ftlThunks;
#endif
    NativeExecutable* getHostFunction(NativeFunction, NativeFunction constructor, const String& name);
    NativeExecutable* getHostFunction(NativeFunction, Intrinsic, NativeFunction constructor, const DOMJIT::Signature*, const String& name);

    static ptrdiff_t exceptionOffset()
    {
        return OBJECT_OFFSETOF(VM, m_exception);
    }

    static ptrdiff_t callFrameForCatchOffset()
    {
        return OBJECT_OFFSETOF(VM, callFrameForCatch);
    }

    static ptrdiff_t targetMachinePCForThrowOffset()
    {
        return OBJECT_OFFSETOF(VM, targetMachinePCForThrow);
    }

    void restorePreviousException(Exception* exception) { setException(exception); }

    void clearLastException() { m_lastException = nullptr; }

    ExecState** addressOfCallFrameForCatch() { return &callFrameForCatch; }

    JSCell** addressOfException() { return reinterpret_cast<JSCell**>(&m_exception); }

    Exception* lastException() const { return m_lastException; }
    JSCell** addressOfLastException() { return reinterpret_cast<JSCell**>(&m_lastException); }

    void setFailNextNewCodeBlock() { m_failNextNewCodeBlock = true; }
    bool getAndClearFailNextNewCodeBlock()
    {
        bool result = m_failNextNewCodeBlock;
        m_failNextNewCodeBlock = false;
        return result;
    }
    
    ALWAYS_INLINE Structure* getStructure(StructureID id)
    {
        return heap.structureIDTable().get(decontaminate(id));
    }
    
    void* stackPointerAtVMEntry() const { return m_stackPointerAtVMEntry; }
    void setStackPointerAtVMEntry(void*);

    size_t softReservedZoneSize() const { return m_currentSoftReservedZoneSize; }
    size_t updateSoftReservedZoneSize(size_t softReservedZoneSize);
    
    static size_t committedStackByteCount();
    inline bool ensureStackCapacityFor(Register* newTopOfStack);

    void* stackLimit() { return m_stackLimit; }
    void* softStackLimit() { return m_softStackLimit; }
    void** addressOfSoftStackLimit() { return &m_softStackLimit; }
#if !ENABLE(JIT)
    void* cloopStackLimit() { return m_cloopStackLimit; }
    void setCLoopStackLimit(void* limit) { m_cloopStackLimit = limit; }
#endif

    inline bool isSafeToRecurseSoft() const;
    bool isSafeToRecurse() const
    {
        return isSafeToRecurse(m_stackLimit);
    }

    void* lastStackTop() { return m_lastStackTop; }
    void setLastStackTop(void*);

    const ClassInfo* const jsArrayClassInfo;
    const ClassInfo* const jsFinalObjectClassInfo;

    JSValue hostCallReturnValue;
    unsigned varargsLength;
    ExecState* newCallFrameReturnValue;
    ExecState* callFrameForCatch;
    void* targetMachinePCForThrow;
    Instruction* targetInterpreterPCForThrow;
    uint32_t osrExitIndex;
    void* osrExitJumpDestination;
    Vector<ScratchBuffer*> scratchBuffers;
    size_t sizeOfLastScratchBuffer;

    bool isExecutingInRegExpJIT { false };

    ScratchBuffer* scratchBufferForSize(size_t size)
    {
        if (!size)
            return 0;

        if (size > sizeOfLastScratchBuffer) {
            // Protect against a N^2 memory usage pathology by ensuring
            // that at worst, we get a geometric series, meaning that the
            // total memory usage is somewhere around
            // max(scratch buffer size) * 4.
            sizeOfLastScratchBuffer = size * 2;

            ScratchBuffer* newBuffer = ScratchBuffer::create(sizeOfLastScratchBuffer);
            RELEASE_ASSERT(newBuffer);
            scratchBuffers.append(newBuffer);
        }

        ScratchBuffer* result = scratchBuffers.last();
        result->setActiveLength(0);
        return result;
    }

    EncodedJSValue* exceptionFuzzingBuffer(size_t size)
    {
        ASSERT(Options::useExceptionFuzz());
        if (!m_exceptionFuzzBuffer)
            m_exceptionFuzzBuffer = MallocPtr<EncodedJSValue>::malloc(size);
        return m_exceptionFuzzBuffer.get();
    }

    void gatherConservativeRoots(ConservativeRoots&);

    VMEntryScope* entryScope;

    JSObject* stringRecursionCheckFirstObject { nullptr };
    HashSet<JSObject*> stringRecursionCheckVisitedObjects;

    LocalTimeOffsetCache localTimeOffsetCache;

    String cachedDateString;
    double cachedDateStringValue;

    std::unique_ptr<Profiler::Database> m_perBytecodeProfiler;
    RefPtr<TypedArrayController> m_typedArrayController;
    RegExpCache* m_regExpCache;
    BumpPointerAllocator m_regExpAllocator;
    ConcurrentJSLock m_regExpAllocatorLock;

    std::unique_ptr<HasOwnPropertyCache> m_hasOwnPropertyCache;
    ALWAYS_INLINE HasOwnPropertyCache* hasOwnPropertyCache() { return m_hasOwnPropertyCache.get(); }
    HasOwnPropertyCache* ensureHasOwnPropertyCache();

#if ENABLE(REGEXP_TRACING)
    typedef ListHashSet<RegExp*> RTTraceList;
    RTTraceList* m_rtTraceList;
#endif

    JS_EXPORT_PRIVATE void resetDateCache();

    RegExpCache* regExpCache() { return m_regExpCache; }
#if ENABLE(REGEXP_TRACING)
    void addRegExpToTrace(RegExp*);
#endif
    JS_EXPORT_PRIVATE void dumpRegExpTrace();

    bool isCollectorBusyOnCurrentThread() { return heap.isCurrentThreadBusy(); }

#if ENABLE(GC_VALIDATION)
    bool isInitializingObject() const; 
    void setInitializingObjectClass(const ClassInfo*);
#endif

    bool currentThreadIsHoldingAPILock() const { return m_apiLock->currentThreadIsHoldingLock(); }

    JSLock& apiLock() { return *m_apiLock; }
    CodeCache* codeCache() { return m_codeCache.get(); }

    JS_EXPORT_PRIVATE void whenIdle(std::function<void()>);

    JS_EXPORT_PRIVATE void deleteAllCode(DeleteAllCodeEffort);
    JS_EXPORT_PRIVATE void deleteAllLinkedCode(DeleteAllCodeEffort);

    WatchpointSet* ensureWatchpointSetForImpureProperty(const Identifier&);
    void registerWatchpointForImpureProperty(const Identifier&, Watchpoint*);
    
    // FIXME: Use AtomicString once it got merged with Identifier.
    JS_EXPORT_PRIVATE void addImpureProperty(const String&);

    BuiltinExecutables* builtinExecutables() { return m_builtinExecutables.get(); }

    bool enableTypeProfiler();
    bool disableTypeProfiler();
    TypeProfilerLog* typeProfilerLog() { return m_typeProfilerLog.get(); }
    TypeProfiler* typeProfiler() { return m_typeProfiler.get(); }
    JS_EXPORT_PRIVATE void dumpTypeProfilerData();

    FunctionHasExecutedCache* functionHasExecutedCache() { return &m_functionHasExecutedCache; }

    ControlFlowProfiler* controlFlowProfiler() { return m_controlFlowProfiler.get(); }
    bool enableControlFlowProfiler();
    bool disableControlFlowProfiler();

    JS_EXPORT_PRIVATE void queueMicrotask(JSGlobalObject*, Ref<Microtask>&&);
    JS_EXPORT_PRIVATE void drainMicrotasks();
    void setGlobalConstRedeclarationShouldThrow(bool globalConstRedeclarationThrow) { m_globalConstRedeclarationShouldThrow = globalConstRedeclarationThrow; }
    ALWAYS_INLINE bool globalConstRedeclarationShouldThrow() const { return m_globalConstRedeclarationShouldThrow; }

    void setShouldBuildPCToCodeOriginMapping() { m_shouldBuildPCToCodeOriginMapping = true; }
    bool shouldBuilderPCToCodeOriginMapping() const { return m_shouldBuildPCToCodeOriginMapping; }

    BytecodeIntrinsicRegistry& bytecodeIntrinsicRegistry() { return *m_bytecodeIntrinsicRegistry; }
    
    ShadowChicken& shadowChicken() { return *m_shadowChicken; }
    
    template<typename Func>
    void logEvent(CodeBlock*, const char* summary, const Func& func);

    std::optional<PlatformThread> ownerThread() const { return m_apiLock->ownerThread(); }

    VMTraps& traps() { return m_traps; }

    void handleTraps(ExecState* exec, VMTraps::Mask mask = VMTraps::Mask::allEventTypes()) { m_traps.handleTraps(exec, mask); }

    bool needTrapHandling() { return m_traps.needTrapHandling(); }
    bool needTrapHandling(VMTraps::Mask mask) { return m_traps.needTrapHandling(mask); }
    void* needTrapHandlingAddress() { return m_traps.needTrapHandlingAddress(); }

    void notifyNeedDebuggerBreak() { m_traps.fireTrap(VMTraps::NeedDebuggerBreak); }
    void notifyNeedTermination() { m_traps.fireTrap(VMTraps::NeedTermination); }
    void notifyNeedWatchdogCheck() { m_traps.fireTrap(VMTraps::NeedWatchdogCheck); }

private:
    friend class LLIntOffsetsExtractor;

    VM(VMType, HeapType);
    static VM*& sharedInstanceInternal();
    void createNativeThunk();

    void updateStackLimits();

    bool isSafeToRecurse(void* stackLimit) const
    {
        ASSERT(wtfThreadData().stack().isGrowingDownward());
        void* curr = reinterpret_cast<void*>(&curr);
        return curr >= stackLimit;
    }

    void setException(Exception* exception)
    {
        m_exception = exception;
        m_lastException = exception;
    }
    Exception* exception() const
    {
#if ENABLE(EXCEPTION_SCOPE_VERIFICATION)
        m_needExceptionCheck = false;
#endif
        return m_exception;
    }
    void clearException()
    {
#if ENABLE(EXCEPTION_SCOPE_VERIFICATION)
        m_needExceptionCheck = false;
#endif
        m_exception = nullptr;
    }

#if !ENABLE(JIT)    
    bool ensureStackCapacityForCLoop(Register* newTopOfStack);
    bool isSafeToRecurseSoftCLoop() const;
#endif // !ENABLE(JIT)

    JS_EXPORT_PRIVATE void throwException(ExecState*, Exception*);
    JS_EXPORT_PRIVATE JSValue throwException(ExecState*, JSValue);
    JS_EXPORT_PRIVATE JSObject* throwException(ExecState*, JSObject*);

#if ENABLE(EXCEPTION_SCOPE_VERIFICATION)
    void verifyExceptionCheckNeedIsSatisfied(unsigned depth, ExceptionEventLocation&);
#endif

#if ENABLE(ASSEMBLER)
    bool m_canUseAssembler;
#endif
#if ENABLE(JIT)
    bool m_canUseJIT;
#endif
#if ENABLE(YARR_JIT)
    bool m_canUseRegExpJIT;
#endif
#if ENABLE(GC_VALIDATION)
    const ClassInfo* m_initializingObjectClass;
#endif

    void* m_stackPointerAtVMEntry;
    size_t m_currentSoftReservedZoneSize;
    void* m_stackLimit { nullptr };
    void* m_softStackLimit { nullptr };
#if !ENABLE(JIT)
    void* m_cloopStackLimit { nullptr };
#endif
    void* m_lastStackTop;

    Exception* m_exception { nullptr };
    Exception* m_lastException { nullptr };
#if ENABLE(EXCEPTION_SCOPE_VERIFICATION)
    ExceptionScope* m_topExceptionScope { nullptr };
    ExceptionEventLocation m_simulatedThrowPointLocation;
    unsigned m_simulatedThrowPointRecursionDepth { 0 };
    mutable bool m_needExceptionCheck { false };
#endif

    bool m_failNextNewCodeBlock { false };
    DeletePropertyMode m_deletePropertyMode { DeletePropertyMode::Default };
    bool m_globalConstRedeclarationShouldThrow { true };
    bool m_shouldBuildPCToCodeOriginMapping { false };
    std::unique_ptr<CodeCache> m_codeCache;
    std::unique_ptr<BuiltinExecutables> m_builtinExecutables;
    HashMap<String, RefPtr<WatchpointSet>> m_impurePropertyWatchpointSets;
    std::unique_ptr<TypeProfiler> m_typeProfiler;
    std::unique_ptr<TypeProfilerLog> m_typeProfilerLog;
    unsigned m_typeProfilerEnabledCount;
    FunctionHasExecutedCache m_functionHasExecutedCache;
    std::unique_ptr<ControlFlowProfiler> m_controlFlowProfiler;
    unsigned m_controlFlowProfilerEnabledCount;
    Deque<std::unique_ptr<QueuedTask>> m_microtaskQueue;
    MallocPtr<EncodedJSValue> m_exceptionFuzzBuffer;
    VMTraps m_traps;
    RefPtr<Watchdog> m_watchdog;
    std::unique_ptr<HeapProfiler> m_heapProfiler;
#if ENABLE(SAMPLING_PROFILER)
    RefPtr<SamplingProfiler> m_samplingProfiler;
#endif
    std::unique_ptr<ShadowChicken> m_shadowChicken;
    std::unique_ptr<BytecodeIntrinsicRegistry> m_bytecodeIntrinsicRegistry;

    VM* m_prev; // Required by DoublyLinkedListNode.
    VM* m_next; // Required by DoublyLinkedListNode.

    // Friends for exception checking purpose only.
    friend class Heap;
    friend class CatchScope;
    friend class ExceptionScope;
    friend class ThrowScope;
    friend class VMTraps;
    friend class WTF::DoublyLinkedListNode<VM>;
};

#if ENABLE(GC_VALIDATION)
inline bool VM::isInitializingObject() const
{
    return !!m_initializingObjectClass;
}

inline void VM::setInitializingObjectClass(const ClassInfo* initializingObjectClass)
{
    m_initializingObjectClass = initializingObjectClass;
}
#endif

inline Heap* WeakSet::heap() const
{
    return &m_vm->heap;
}

#if ENABLE(JIT)
extern "C" void sanitizeStackForVMImpl(VM*);
#endif

void sanitizeStackForVM(VM*);
void logSanitizeStack(VM*);

} // namespace JSC
