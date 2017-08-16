cmake_minimum_required(VERSION 2.8.12)
include(WebKitCommon)
set_property(DIRECTORY . PROPERTY FOLDER "JavaScriptCore")

set(JavaScriptCore_INCLUDE_DIRECTORIES
    "${CMAKE_BINARY_DIR}"
    "${JAVASCRIPTCORE_DIR}"
    "${JAVASCRIPTCORE_DIR}/.."
    "${JAVASCRIPTCORE_DIR}/API"
    "${JAVASCRIPTCORE_DIR}/ForwardingHeaders"
    "${JAVASCRIPTCORE_DIR}/assembler"
    "${JAVASCRIPTCORE_DIR}/b3"
    "${JAVASCRIPTCORE_DIR}/b3/air"
    "${JAVASCRIPTCORE_DIR}/bindings"
    "${JAVASCRIPTCORE_DIR}/builtins"
    "${JAVASCRIPTCORE_DIR}/bytecode"
    "${JAVASCRIPTCORE_DIR}/bytecompiler"
    "${JAVASCRIPTCORE_DIR}/dfg"
    "${JAVASCRIPTCORE_DIR}/disassembler"
    "${JAVASCRIPTCORE_DIR}/disassembler/udis86"
    "${JAVASCRIPTCORE_DIR}/disassembler/ARM64"
    "${JAVASCRIPTCORE_DIR}/domjit"
    "${JAVASCRIPTCORE_DIR}/ftl"
    "${JAVASCRIPTCORE_DIR}/heap"
    "${JAVASCRIPTCORE_DIR}/debugger"
    "${JAVASCRIPTCORE_DIR}/inspector"
    "${JAVASCRIPTCORE_DIR}/inspector/agents"
    "${JAVASCRIPTCORE_DIR}/inspector/augmentable"
    "${JAVASCRIPTCORE_DIR}/inspector/remote"
    "${JAVASCRIPTCORE_DIR}/interpreter"
    "${JAVASCRIPTCORE_DIR}/jit"
    "${JAVASCRIPTCORE_DIR}/llint"
    "${JAVASCRIPTCORE_DIR}/parser"
    "${JAVASCRIPTCORE_DIR}/profiler"
    "${JAVASCRIPTCORE_DIR}/replay"
    "${JAVASCRIPTCORE_DIR}/runtime"
    "${JAVASCRIPTCORE_DIR}/tools"
    "${JAVASCRIPTCORE_DIR}/wasm"
    "${JAVASCRIPTCORE_DIR}/wasm/js"
    "${JAVASCRIPTCORE_DIR}/yarr"
    "${DERIVED_SOURCES_DIR}/ForwardingHeaders"
    "${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}"
    "${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/inspector"
)

set(JavaScriptCore_SYSTEM_INCLUDE_DIRECTORIES
    "${ICU_INCLUDE_DIRS}"
)

set(JavaScriptCore_SOURCES
    API/JSBase.cpp
    API/JSCTestRunnerUtils.cpp
    API/JSCallbackConstructor.cpp
    API/JSCallbackFunction.cpp
    API/JSCallbackObject.cpp
    API/JSClassRef.cpp
    API/JSContextRef.cpp
    API/JSObjectRef.cpp
    API/JSTypedArray.cpp
    API/JSScriptRef.cpp
    API/JSStringRef.cpp
    API/JSValueRef.cpp
    API/JSWeakObjectMapRefPrivate.cpp
    API/OpaqueJSString.cpp

    assembler/ARMAssembler.cpp
    assembler/AbstractMacroAssembler.cpp
    assembler/LinkBuffer.cpp
    assembler/MacroAssembler.cpp
    assembler/MacroAssemblerARM.cpp
    assembler/MacroAssemblerARMv7.cpp
    assembler/MacroAssemblerCodeRef.cpp
    assembler/MacroAssemblerPrinter.cpp
    assembler/MacroAssemblerX86Common.cpp

    b3/air/AirAllocateRegistersByGraphColoring.cpp
    b3/air/AirAllocateStack.cpp
    b3/air/AirArg.cpp
    b3/air/AirBasicBlock.cpp
    b3/air/AirBlockInsertionSet.cpp
    b3/air/AirCCallSpecial.cpp
    b3/air/AirCCallingConvention.cpp
    b3/air/AirCode.cpp
    b3/air/AirCustom.cpp
    b3/air/AirDisassembler.cpp
    b3/air/AirEliminateDeadCode.cpp
    b3/air/AirEmitShuffle.cpp
    b3/air/AirFixObviousSpills.cpp
    b3/air/AirFixPartialRegisterStalls.cpp
    b3/air/AirGenerate.cpp
    b3/air/AirGenerated.cpp
    b3/air/AirHandleCalleeSaves.cpp
    b3/air/AirInsertionSet.cpp
    b3/air/AirInst.cpp
    b3/air/AirKind.cpp
    b3/air/AirLogRegisterPressure.cpp
    b3/air/AirLowerAfterRegAlloc.cpp
    b3/air/AirLowerEntrySwitch.cpp
    b3/air/AirLowerMacros.cpp
    b3/air/AirOptimizeBlockOrder.cpp
    b3/air/AirPadInterference.cpp
    b3/air/AirPhaseScope.cpp
    b3/air/AirReportUsedRegisters.cpp
    b3/air/AirSimplifyCFG.cpp
    b3/air/AirSpecial.cpp
    b3/air/AirSpillEverything.cpp
    b3/air/AirStackSlot.cpp
    b3/air/AirStackSlotKind.cpp
    b3/air/AirTmp.cpp
    b3/air/AirTmpWidth.cpp
    b3/air/AirValidate.cpp

    b3/B3ArgumentRegValue.cpp
    b3/B3AtomicValue.cpp
    b3/B3Bank.cpp
    b3/B3BasicBlock.cpp
    b3/B3BlockInsertionSet.cpp
    b3/B3BreakCriticalEdges.cpp
    b3/B3CCallValue.cpp
    b3/B3CaseCollection.cpp
    b3/B3CheckSpecial.cpp
    b3/B3CheckValue.cpp
    b3/B3Common.cpp
    b3/B3Commutativity.cpp
    b3/B3Compile.cpp
    b3/B3Compilation.cpp
    b3/B3Const32Value.cpp
    b3/B3Const64Value.cpp
    b3/B3ConstDoubleValue.cpp
    b3/B3ConstFloatValue.cpp
    b3/B3ConstrainedValue.cpp
    b3/B3DataSection.cpp
    b3/B3DuplicateTails.cpp
    b3/B3Effects.cpp
    b3/B3EliminateCommonSubexpressions.cpp
    b3/B3FenceValue.cpp
    b3/B3FixSSA.cpp
    b3/B3FoldPathConstants.cpp
    b3/B3FrequencyClass.cpp
    b3/B3Generate.cpp
    b3/B3HeapRange.cpp
    b3/B3InferSwitches.cpp
    b3/B3InsertionSet.cpp
    b3/B3Kind.cpp
    b3/B3LegalizeMemoryOffsets.cpp
    b3/B3LowerMacros.cpp
    b3/B3LowerMacrosAfterOptimizations.cpp
    b3/B3LowerToAir.cpp
    b3/B3MathExtras.cpp
    b3/B3MemoryValue.cpp
    b3/B3MoveConstants.cpp
    b3/B3OpaqueByproducts.cpp
    b3/B3Opcode.cpp
    b3/B3Origin.cpp
    b3/B3OriginDump.cpp
    b3/B3PatchpointSpecial.cpp
    b3/B3PatchpointValue.cpp
    b3/B3PhaseScope.cpp
    b3/B3PhiChildren.cpp
    b3/B3Procedure.cpp
    b3/B3PureCSE.cpp
    b3/B3ReduceDoubleToFloat.cpp
    b3/B3ReduceStrength.cpp
    b3/B3SSACalculator.cpp
    b3/B3SlotBaseValue.cpp
    b3/B3StackmapGenerationParams.cpp
    b3/B3StackmapSpecial.cpp
    b3/B3StackmapValue.cpp
    b3/B3StackSlot.cpp
    b3/B3SwitchCase.cpp
    b3/B3SwitchValue.cpp
    b3/B3TimingScope.cpp
    b3/B3Type.cpp
    b3/B3UpsilonValue.cpp
    b3/B3UseCounts.cpp
    b3/B3Validate.cpp
    b3/B3Value.cpp
    b3/B3ValueKey.cpp
    b3/B3ValueRep.cpp
    b3/B3Variable.cpp
    b3/B3VariableValue.cpp
    b3/B3WasmAddressValue.cpp
    b3/B3WasmBoundsCheckValue.cpp
    b3/B3Width.cpp

    bindings/ScriptFunctionCall.cpp
    bindings/ScriptObject.cpp
    bindings/ScriptValue.cpp

    builtins/BuiltinExecutables.cpp
    builtins/BuiltinExecutableCreator.cpp

    bytecode/AccessCase.cpp
    bytecode/AdaptiveInferredPropertyValueWatchpointBase.cpp
    bytecode/ArithProfile.cpp
    bytecode/ArrayAllocationProfile.cpp
    bytecode/ArrayProfile.cpp
    bytecode/BytecodeBasicBlock.cpp
    bytecode/BytecodeDumper.cpp
    bytecode/BytecodeGeneratorification.cpp
    bytecode/BytecodeIntrinsicRegistry.cpp
    bytecode/BytecodeLivenessAnalysis.cpp
    bytecode/BytecodeRewriter.cpp
    bytecode/CallEdge.cpp
    bytecode/CallLinkInfo.cpp
    bytecode/CallLinkStatus.cpp
    bytecode/CallMode.cpp
    bytecode/CallVariant.cpp
    bytecode/CodeBlock.cpp
    bytecode/CodeBlockHash.cpp
    bytecode/CodeBlockJettisoningWatchpoint.cpp
    bytecode/CodeOrigin.cpp
    bytecode/CodeType.cpp
    bytecode/ComplexGetStatus.cpp
    bytecode/DFGExitProfile.cpp
    bytecode/DOMJITAccessCasePatchpointParams.cpp
    bytecode/DataFormat.cpp
    bytecode/DeferredCompilationCallback.cpp
    bytecode/DeferredSourceDump.cpp
    bytecode/DirectEvalCodeCache.cpp
    bytecode/EvalCodeBlock.cpp
    bytecode/ExecutionCounter.cpp
    bytecode/ExitKind.cpp
    bytecode/ExitingJITType.cpp
    bytecode/FullCodeOrigin.cpp
    bytecode/FunctionCodeBlock.cpp
    bytecode/GetByIdStatus.cpp
    bytecode/GetByIdVariant.cpp
    bytecode/GetterSetterAccessCase.cpp
    bytecode/InlineAccess.cpp
    bytecode/InlineCallFrame.cpp
    bytecode/InlineCallFrameSet.cpp
    bytecode/IntrinsicGetterAccessCase.cpp
    bytecode/JumpTable.cpp
    bytecode/LLIntPrototypeLoadAdaptiveStructureWatchpoint.cpp
    bytecode/LazyOperandValueProfile.cpp
    bytecode/MethodOfGettingAValueProfile.cpp
    bytecode/ModuleNamespaceAccessCase.cpp
    bytecode/ModuleProgramCodeBlock.cpp
    bytecode/ObjectPropertyCondition.cpp
    bytecode/ObjectPropertyConditionSet.cpp
    bytecode/Opcode.cpp
    bytecode/PolymorphicAccess.cpp
    bytecode/PreciseJumpTargets.cpp
    bytecode/ProgramCodeBlock.cpp
    bytecode/PropertyCondition.cpp
    bytecode/ProxyableAccessCase.cpp
    bytecode/PutByIdFlags.cpp
    bytecode/PutByIdStatus.cpp
    bytecode/PutByIdVariant.cpp
    bytecode/ReduceWhitespace.cpp
    bytecode/SpecialPointer.cpp
    bytecode/SpeculatedType.cpp
    bytecode/StructureSet.cpp
    bytecode/StructureStubClearingWatchpoint.cpp
    bytecode/StructureStubInfo.cpp
    bytecode/SuperSampler.cpp
    bytecode/ToThisStatus.cpp
    bytecode/TrackedReferences.cpp
    bytecode/UnlinkedCodeBlock.cpp
    bytecode/UnlinkedEvalCodeBlock.cpp
    bytecode/UnlinkedFunctionCodeBlock.cpp
    bytecode/UnlinkedFunctionExecutable.cpp
    bytecode/UnlinkedInstructionStream.cpp
    bytecode/UnlinkedModuleProgramCodeBlock.cpp
    bytecode/UnlinkedProgramCodeBlock.cpp
    bytecode/ValueRecovery.cpp
    bytecode/VariableWriteFireDetail.cpp
    bytecode/VirtualRegister.cpp
    bytecode/Watchpoint.cpp

    bytecompiler/BytecodeGenerator.cpp
    bytecompiler/NodesCodegen.cpp

    debugger/Debugger.cpp
    debugger/DebuggerCallFrame.cpp
    debugger/DebuggerLocation.cpp
    debugger/DebuggerParseData.cpp
    debugger/DebuggerScope.cpp

    dfg/DFGAbstractHeap.cpp
    dfg/DFGAbstractValue.cpp
    dfg/DFGAdaptiveInferredPropertyValueWatchpoint.cpp
    dfg/DFGAdaptiveStructureWatchpoint.cpp
    dfg/DFGArgumentsEliminationPhase.cpp
    dfg/DFGArgumentsUtilities.cpp
    dfg/DFGArithMode.cpp
    dfg/DFGArrayMode.cpp
    dfg/DFGAtTailAbstractState.cpp
    dfg/DFGAvailability.cpp
    dfg/DFGAvailabilityMap.cpp
    dfg/DFGBackwardsPropagationPhase.cpp
    dfg/DFGBasicBlock.cpp
    dfg/DFGBlockInsertionSet.cpp
    dfg/DFGBlockSet.cpp
    dfg/DFGByteCodeParser.cpp
    dfg/DFGCFAPhase.cpp
    dfg/DFGCFGSimplificationPhase.cpp
    dfg/DFGCPSRethreadingPhase.cpp
    dfg/DFGCSEPhase.cpp
    dfg/DFGCapabilities.cpp
    dfg/DFGCleanUpPhase.cpp
    dfg/DFGClobberSet.cpp
    dfg/DFGClobberize.cpp
    dfg/DFGClobbersExitState.cpp
    dfg/DFGCombinedLiveness.cpp
    dfg/DFGCommon.cpp
    dfg/DFGCommonData.cpp
    dfg/DFGCompilationKey.cpp
    dfg/DFGCompilationMode.cpp
    dfg/DFGConstantFoldingPhase.cpp
    dfg/DFGConstantHoistingPhase.cpp
    dfg/DFGCriticalEdgeBreakingPhase.cpp
    dfg/DFGDCEPhase.cpp
    dfg/DFGDOMJITPatchpointParams.cpp
    dfg/DFGDesiredIdentifiers.cpp
    dfg/DFGDesiredTransitions.cpp
    dfg/DFGDesiredWatchpoints.cpp
    dfg/DFGDesiredWeakReferences.cpp
    dfg/DFGDisassembler.cpp
    dfg/DFGDoesGC.cpp
    dfg/DFGDriver.cpp
    dfg/DFGEdge.cpp
    dfg/DFGEpoch.cpp
    dfg/DFGFailedFinalizer.cpp
    dfg/DFGFinalizer.cpp
    dfg/DFGFixupPhase.cpp
    dfg/DFGFlowIndexing.cpp
    dfg/DFGFlushFormat.cpp
    dfg/DFGFlushedAt.cpp
    dfg/DFGLiveCatchVariablePreservationPhase.cpp
    dfg/DFGFrozenValue.cpp
    dfg/DFGGraph.cpp
    dfg/DFGGraphSafepoint.cpp
    dfg/DFGHeapLocation.cpp
    dfg/DFGInPlaceAbstractState.cpp
    dfg/DFGInferredTypeCheck.cpp
    dfg/DFGInsertionSet.cpp
    dfg/DFGIntegerCheckCombiningPhase.cpp
    dfg/DFGIntegerRangeOptimizationPhase.cpp
    dfg/DFGInvalidationPointInjectionPhase.cpp
    dfg/DFGJITCode.cpp
    dfg/DFGJITCompiler.cpp
    dfg/DFGJITFinalizer.cpp
    dfg/DFGJumpReplacement.cpp
    dfg/DFGLICMPhase.cpp
    dfg/DFGLazyJSValue.cpp
    dfg/DFGLazyNode.cpp
    dfg/DFGLivenessAnalysisPhase.cpp
    dfg/DFGLongLivedState.cpp
    dfg/DFGLoopPreHeaderCreationPhase.cpp
    dfg/DFGMaximalFlushInsertionPhase.cpp
    dfg/DFGMayExit.cpp
    dfg/DFGMinifiedGraph.cpp
    dfg/DFGMinifiedNode.cpp
    dfg/DFGMovHintRemovalPhase.cpp
    dfg/DFGMultiGetByOffsetData.cpp
    dfg/DFGNaturalLoops.cpp
    dfg/DFGNode.cpp
    dfg/DFGNodeAbstractValuePair.cpp
    dfg/DFGNodeFlags.cpp
    dfg/DFGNodeFlowProjection.cpp
    dfg/DFGNodeOrigin.cpp
    dfg/DFGOSRAvailabilityAnalysisPhase.cpp
    dfg/DFGOSREntry.cpp
    dfg/DFGOSREntrypointCreationPhase.cpp
    dfg/DFGOSRExit.cpp
    dfg/DFGOSRExitBase.cpp
    dfg/DFGOSRExitCompiler.cpp
    dfg/DFGOSRExitCompiler32_64.cpp
    dfg/DFGOSRExitCompiler64.cpp
    dfg/DFGOSRExitCompilerCommon.cpp
    dfg/DFGOSRExitFuzz.cpp
    dfg/DFGOSRExitJumpPlaceholder.cpp
    dfg/DFGOSRExitPreparation.cpp
    dfg/DFGObjectAllocationSinkingPhase.cpp
    dfg/DFGObjectMaterializationData.cpp
    dfg/DFGOperations.cpp
    dfg/DFGPhantomInsertionPhase.cpp
    dfg/DFGPhase.cpp
    dfg/DFGPhiChildren.cpp
    dfg/DFGPlan.cpp
    dfg/DFGPrePostNumbering.cpp
    dfg/DFGPredictionInjectionPhase.cpp
    dfg/DFGPredictionPropagationPhase.cpp
    dfg/DFGPromotedHeapLocation.cpp
    dfg/DFGPureValue.cpp
    dfg/DFGPutStackSinkingPhase.cpp
    dfg/DFGRegisteredStructureSet.cpp
    dfg/DFGSSACalculator.cpp
    dfg/DFGSSAConversionPhase.cpp
    dfg/DFGSSALoweringPhase.cpp
    dfg/DFGSafepoint.cpp
    dfg/DFGSpeculativeJIT.cpp
    dfg/DFGSpeculativeJIT32_64.cpp
    dfg/DFGSpeculativeJIT64.cpp
    dfg/DFGStackLayoutPhase.cpp
    dfg/DFGStaticExecutionCountEstimationPhase.cpp
    dfg/DFGStoreBarrierClusteringPhase.cpp
    dfg/DFGStoreBarrierInsertionPhase.cpp
    dfg/DFGStrengthReductionPhase.cpp
    dfg/DFGStructureAbstractValue.cpp
    dfg/DFGThreadData.cpp
    dfg/DFGThunks.cpp
    dfg/DFGTierUpCheckInjectionPhase.cpp
    dfg/DFGToFTLDeferredCompilationCallback.cpp
    dfg/DFGToFTLForOSREntryDeferredCompilationCallback.cpp
    dfg/DFGTransition.cpp
    dfg/DFGTypeCheckHoistingPhase.cpp
    dfg/DFGUnificationPhase.cpp
    dfg/DFGUseKind.cpp
    dfg/DFGValidate.cpp
    dfg/DFGValueSource.cpp
    dfg/DFGValueStrength.cpp
    dfg/DFGVarargsForwardingPhase.cpp
    dfg/DFGVariableAccessData.cpp
    dfg/DFGVariableAccessDataDump.cpp
    dfg/DFGVariableEvent.cpp
    dfg/DFGVariableEventStream.cpp
    dfg/DFGVirtualRegisterAllocationPhase.cpp
    dfg/DFGWatchpointCollectionPhase.cpp
    dfg/DFGWorklist.cpp

    disassembler/ARM64Disassembler.cpp
    disassembler/ARMLLVMDisassembler.cpp
    disassembler/ARMv7Disassembler.cpp
    disassembler/Disassembler.cpp
    disassembler/UDis86Disassembler.cpp
    disassembler/X86Disassembler.cpp

    disassembler/ARM64/A64DOpcode.cpp

    disassembler/ARMv7/ARMv7DOpcode.cpp

    disassembler/udis86/udis86.c
    disassembler/udis86/udis86_decode.c
    disassembler/udis86/udis86_itab_holder.c
    disassembler/udis86/udis86_syn-att.c
    disassembler/udis86/udis86_syn-intel.c
    disassembler/udis86/udis86_syn.c

    domjit/DOMJITAbstractHeap.cpp
    domjit/DOMJITHeapRange.cpp

    ftl/FTLAbstractHeap.cpp
    ftl/FTLAbstractHeapRepository.cpp
    ftl/FTLAvailableRecovery.cpp
    ftl/FTLCapabilities.cpp
    ftl/FTLCommonValues.cpp
    ftl/FTLCompile.cpp
    ftl/FTLDOMJITPatchpointParams.cpp
    ftl/FTLExceptionTarget.cpp
    ftl/FTLExitArgument.cpp
    ftl/FTLExitArgumentForOperand.cpp
    ftl/FTLExitPropertyValue.cpp
    ftl/FTLExitTimeObjectMaterialization.cpp
    ftl/FTLExitValue.cpp
    ftl/FTLFail.cpp
    ftl/FTLForOSREntryJITCode.cpp
    ftl/FTLJITCode.cpp
    ftl/FTLJITFinalizer.cpp
    ftl/FTLLazySlowPath.cpp
    ftl/FTLLink.cpp
    ftl/FTLLocation.cpp
    ftl/FTLLowerDFGToB3.cpp
    ftl/FTLOSREntry.cpp
    ftl/FTLOSRExit.cpp
    ftl/FTLOSRExitCompiler.cpp
    ftl/FTLOSRExitHandle.cpp
    ftl/FTLOperations.cpp
    ftl/FTLOutput.cpp
    ftl/FTLPatchpointExceptionHandle.cpp
    ftl/FTLRecoveryOpcode.cpp
    ftl/FTLSaveRestore.cpp
    ftl/FTLSlowPathCall.cpp
    ftl/FTLSlowPathCallKey.cpp
    ftl/FTLState.cpp
    ftl/FTLThunks.cpp
    ftl/FTLValueRange.cpp

    heap/AllocatorAttributes.cpp
    heap/CellContainer.cpp
    heap/CodeBlockSet.cpp
    heap/CollectionScope.cpp
    heap/CollectorPhase.cpp
    heap/ConservativeRoots.cpp
    heap/DeferGC.cpp
    heap/DestructionMode.cpp
    heap/EdenGCActivityCallback.cpp
    heap/FullGCActivityCallback.cpp
    heap/FreeList.cpp
    heap/GCActivityCallback.cpp
    heap/GCConductor.cpp
    heap/GCLogging.cpp
    heap/HandleSet.cpp
    heap/HandleStack.cpp
    heap/Heap.cpp
    heap/HeapCell.cpp
    heap/HeapHelperPool.cpp
    heap/HeapProfiler.cpp
    heap/HeapSnapshot.cpp
    heap/HeapSnapshotBuilder.cpp
    heap/HeapTimer.cpp
    heap/IncrementalSweeper.cpp
    heap/JITStubRoutineSet.cpp
    heap/LargeAllocation.cpp
    heap/MachineStackMarker.cpp
    heap/MarkStack.cpp
    heap/MarkedAllocator.cpp
    heap/MarkedBlock.cpp
    heap/MarkedSpace.cpp
    heap/MarkingConstraint.cpp
    heap/MarkingConstraintSet.cpp
    heap/MutatorScheduler.cpp
    heap/MutatorState.cpp
    heap/SlotVisitor.cpp
    heap/SpaceTimeMutatorScheduler.cpp
    heap/StochasticSpaceTimeMutatorScheduler.cpp
    heap/StopIfNecessaryTimer.cpp
    heap/Subspace.cpp
    heap/SynchronousStopTheWorldMutatorScheduler.cpp
    heap/VisitRaceKey.cpp
    heap/Weak.cpp
    heap/WeakBlock.cpp
    heap/WeakHandleOwner.cpp
    heap/WeakSet.cpp
    heap/WriteBarrierSupport.cpp

    inspector/AsyncStackTrace.cpp
    inspector/ConsoleMessage.cpp
    inspector/ContentSearchUtilities.cpp
    inspector/EventLoop.cpp
    inspector/IdentifiersFactory.cpp
    inspector/InjectedScript.cpp
    inspector/InjectedScriptBase.cpp
    inspector/InjectedScriptHost.cpp
    inspector/InjectedScriptManager.cpp
    inspector/InjectedScriptModule.cpp
    inspector/InspectorAgentRegistry.cpp
    inspector/InspectorFrontendRouter.cpp
    inspector/InspectorBackendDispatcher.cpp
    inspector/InspectorValues.cpp
    inspector/JSGlobalObjectConsoleClient.cpp
    inspector/JSGlobalObjectInspectorController.cpp
    inspector/JSGlobalObjectScriptDebugServer.cpp
    inspector/JSInjectedScriptHost.cpp
    inspector/JSInjectedScriptHostPrototype.cpp
    inspector/JSJavaScriptCallFrame.cpp
    inspector/JSJavaScriptCallFramePrototype.cpp
    inspector/JavaScriptCallFrame.cpp
    inspector/PerGlobalObjectWrapperWorld.cpp
    inspector/ScriptArguments.cpp
    inspector/ScriptCallFrame.cpp
    inspector/ScriptCallStack.cpp
    inspector/ScriptCallStackFactory.cpp
    inspector/ScriptDebugServer.cpp

    inspector/agents/InspectorAgent.cpp
    inspector/agents/InspectorConsoleAgent.cpp
    inspector/agents/InspectorDebuggerAgent.cpp
    inspector/agents/InspectorHeapAgent.cpp
    inspector/agents/InspectorRuntimeAgent.cpp
    inspector/agents/InspectorScriptProfilerAgent.cpp
    inspector/agents/JSGlobalObjectConsoleAgent.cpp
    inspector/agents/JSGlobalObjectDebuggerAgent.cpp
    inspector/agents/JSGlobalObjectRuntimeAgent.cpp

    interpreter/AbstractPC.cpp
    interpreter/CLoopStack.cpp
    interpreter/CallFrame.cpp
    interpreter/Interpreter.cpp
    interpreter/ProtoCallFrame.cpp
    interpreter/ShadowChicken.cpp
    interpreter/StackVisitor.cpp

    jit/AssemblyHelpers.cpp
    jit/BinarySwitch.cpp
    jit/CCallHelpers.cpp
    jit/CachedRecovery.cpp
    jit/CallFrameShuffleData.cpp
    jit/CallFrameShuffler.cpp
    jit/CallFrameShuffler32_64.cpp
    jit/CallFrameShuffler64.cpp
    jit/ExecutableAllocationFuzz.cpp
    jit/ExecutableAllocator.cpp
    jit/GCAwareJITStubRoutine.cpp
    jit/GPRInfo.cpp
    jit/HostCallReturnValue.cpp
    jit/ICStats.cpp
    jit/IntrinsicEmitter.cpp
    jit/JIT.cpp
    jit/JITAddGenerator.cpp
    jit/JITArithmetic.cpp
    jit/JITArithmetic32_64.cpp
    jit/JITBitAndGenerator.cpp
    jit/JITBitOrGenerator.cpp
    jit/JITBitXorGenerator.cpp
    jit/JITCall.cpp
    jit/JITCall32_64.cpp
    jit/JITCode.cpp
    jit/JITDisassembler.cpp
    jit/JITDivGenerator.cpp
    jit/JITExceptions.cpp
    jit/JITInlineCacheGenerator.cpp
    jit/JITLeftShiftGenerator.cpp
    jit/JITMulGenerator.cpp
    jit/JITNegGenerator.cpp
    jit/JITOpcodes.cpp
    jit/JITOpcodes32_64.cpp
    jit/JITOperations.cpp
    jit/JITPropertyAccess.cpp
    jit/JITPropertyAccess32_64.cpp
    jit/JITRightShiftGenerator.cpp
    jit/JITStubRoutine.cpp
    jit/JITSubGenerator.cpp
    jit/JITThunks.cpp
    jit/JITToDFGDeferredCompilationCallback.cpp
    jit/JITWorklist.cpp
    jit/PCToCodeOriginMap.cpp
    jit/PolymorphicCallStubRoutine.cpp
    jit/Reg.cpp
    jit/RegisterAtOffset.cpp
    jit/RegisterAtOffsetList.cpp
    jit/RegisterSet.cpp
    jit/Repatch.cpp
    jit/ScratchRegisterAllocator.cpp
    jit/SetupVarargsFrame.cpp
    jit/TagRegistersMode.cpp
    jit/TempRegisterSet.cpp
    jit/ThunkGenerators.cpp

    llint/LLIntCLoop.cpp
    llint/LLIntData.cpp
    llint/LLIntEntrypoint.cpp
    llint/LLIntExceptions.cpp
    llint/LLIntSlowPaths.cpp
    llint/LLIntThunks.cpp
    llint/LowLevelInterpreter.cpp

    parser/Lexer.cpp
    parser/ModuleAnalyzer.cpp
    parser/Nodes.cpp
    parser/NodesAnalyzeModule.cpp
    parser/Parser.cpp
    parser/ParserArena.cpp
    parser/SourceProvider.cpp
    parser/SourceProviderCache.cpp
    parser/UnlinkedSourceCode.cpp
    parser/VariableEnvironment.cpp

    profiler/ProfilerBytecode.cpp
    profiler/ProfilerBytecodeSequence.cpp
    profiler/ProfilerBytecodes.cpp
    profiler/ProfilerCompilation.cpp
    profiler/ProfilerCompilationKind.cpp
    profiler/ProfilerCompiledBytecode.cpp
    profiler/ProfilerDatabase.cpp
    profiler/ProfilerEvent.cpp
    profiler/ProfilerJettisonReason.cpp
    profiler/ProfilerOSRExit.cpp
    profiler/ProfilerOSRExitSite.cpp
    profiler/ProfilerOrigin.cpp
    profiler/ProfilerOriginStack.cpp
    profiler/ProfilerProfiledBytecodes.cpp
    profiler/ProfilerUID.cpp

    runtime/AbstractModuleRecord.cpp
    runtime/ArgList.cpp
    runtime/ArrayBuffer.cpp
    runtime/ArrayBufferNeuteringWatchpoint.cpp
    runtime/ArrayBufferView.cpp
    runtime/ArrayConstructor.cpp
    runtime/ArrayConventions.cpp
    runtime/ArrayIteratorAdaptiveWatchpoint.cpp
    runtime/ArrayIteratorPrototype.cpp
    runtime/ArrayPrototype.cpp
    runtime/AtomicsObject.cpp
    runtime/AsyncFunctionConstructor.cpp
    runtime/AsyncFunctionPrototype.cpp
    runtime/BasicBlockLocation.cpp
    runtime/BooleanConstructor.cpp
    runtime/BooleanObject.cpp
    runtime/BooleanPrototype.cpp
    runtime/CallData.cpp
    runtime/CatchScope.cpp
    runtime/ClonedArguments.cpp
    runtime/CodeCache.cpp
    runtime/CodeSpecializationKind.cpp
    runtime/CommonIdentifiers.cpp
    runtime/CommonSlowPaths.cpp
    runtime/CommonSlowPathsExceptions.cpp
    runtime/CompilationResult.cpp
    runtime/Completion.cpp
    runtime/ConfigFile.cpp
    runtime/ConsoleClient.cpp
    runtime/ConsoleObject.cpp
    runtime/ConstantMode.cpp
    runtime/ConstructData.cpp
    runtime/ControlFlowProfiler.cpp
    runtime/CustomGetterSetter.cpp
    runtime/DataView.cpp
    runtime/DateConstructor.cpp
    runtime/DateConversion.cpp
    runtime/DateInstance.cpp
    runtime/DatePrototype.cpp
    runtime/DirectArguments.cpp
    runtime/DirectArgumentsOffset.cpp
    runtime/DirectEvalExecutable.cpp
    runtime/DumpContext.cpp
    runtime/ECMAScriptSpecInternalFunctions.cpp
    runtime/Error.cpp
    runtime/ErrorConstructor.cpp
    runtime/ErrorHandlingScope.cpp
    runtime/ErrorInstance.cpp
    runtime/ErrorPrototype.cpp
    runtime/EvalExecutable.cpp
    runtime/Exception.cpp
    runtime/ExceptionEventLocation.cpp
    runtime/ExceptionFuzz.cpp
    runtime/ExceptionHelpers.cpp
    runtime/ExceptionScope.cpp
    runtime/ExecutableBase.cpp
    runtime/FunctionConstructor.cpp
    runtime/FunctionExecutable.cpp
    runtime/FunctionExecutableDump.cpp
    runtime/FunctionHasExecutedCache.cpp
    runtime/FunctionPrototype.cpp
    runtime/FunctionRareData.cpp
    runtime/GeneratorFunctionConstructor.cpp
    runtime/GeneratorFunctionPrototype.cpp
    runtime/GeneratorPrototype.cpp
    runtime/GetterSetter.cpp
    runtime/HashMapImpl.cpp
    runtime/Identifier.cpp
    runtime/IndexingType.cpp
    runtime/IndirectEvalExecutable.cpp
    runtime/InferredType.cpp
    runtime/InferredTypeTable.cpp
    runtime/InferredValue.cpp
    runtime/InitializeThreading.cpp
    runtime/InspectorInstrumentationObject.cpp
    runtime/InternalFunction.cpp
    runtime/IntlCollator.cpp
    runtime/IntlCollatorConstructor.cpp
    runtime/IntlCollatorPrototype.cpp
    runtime/IntlDateTimeFormat.cpp
    runtime/IntlDateTimeFormatConstructor.cpp
    runtime/IntlDateTimeFormatPrototype.cpp
    runtime/IntlNumberFormat.cpp
    runtime/IntlNumberFormatConstructor.cpp
    runtime/IntlNumberFormatPrototype.cpp
    runtime/IntlObject.cpp
    runtime/IteratorOperations.cpp
    runtime/IteratorPrototype.cpp
    runtime/JSAPIValueWrapper.cpp
    runtime/JSArray.cpp
    runtime/JSArrayBuffer.cpp
    runtime/JSArrayBufferConstructor.cpp
    runtime/JSArrayBufferPrototype.cpp
    runtime/JSArrayBufferView.cpp
    runtime/JSAsyncFunction.cpp
    runtime/JSBoundFunction.cpp
    runtime/JSCJSValue.cpp
    runtime/JSCallee.cpp
    runtime/JSCell.cpp
    runtime/JSCustomGetterSetterFunction.cpp
    runtime/JSDataView.cpp
    runtime/JSDataViewPrototype.cpp
    runtime/JSDateMath.cpp
    runtime/JSDestructibleObjectSubspace.cpp
    runtime/JSEnvironmentRecord.cpp
    runtime/JSFixedArray.cpp
    runtime/JSFunction.cpp
    runtime/JSGeneratorFunction.cpp
    runtime/JSGlobalLexicalEnvironment.cpp
    runtime/JSGlobalObject.cpp
    runtime/JSGlobalObjectDebuggable.cpp
    runtime/JSGlobalObjectFunctions.cpp
    runtime/JSInternalPromise.cpp
    runtime/JSInternalPromiseConstructor.cpp
    runtime/JSInternalPromiseDeferred.cpp
    runtime/JSInternalPromisePrototype.cpp
    runtime/JSJob.cpp
    runtime/JSLexicalEnvironment.cpp
    runtime/JSLock.cpp
    runtime/JSMap.cpp
    runtime/JSMapIterator.cpp
    runtime/JSModuleEnvironment.cpp
    runtime/JSModuleLoader.cpp
    runtime/JSModuleNamespaceObject.cpp
    runtime/JSModuleRecord.cpp
    runtime/JSNativeStdFunction.cpp
    runtime/JSONObject.cpp
    runtime/JSObject.cpp
    runtime/JSPromise.cpp
    runtime/JSPromiseConstructor.cpp
    runtime/JSPromiseDeferred.cpp
    runtime/JSPromisePrototype.cpp
    runtime/JSPropertyNameEnumerator.cpp
    runtime/JSPropertyNameIterator.cpp
    runtime/JSProxy.cpp
    runtime/JSScope.cpp
    runtime/JSScriptFetcher.cpp
    runtime/JSSegmentedVariableObject.cpp
    runtime/JSSegmentedVariableObjectSubspace.cpp
    runtime/JSSet.cpp
    runtime/JSSetIterator.cpp
    runtime/JSSourceCode.cpp
    runtime/JSString.cpp
    runtime/JSStringIterator.cpp
    runtime/JSStringJoiner.cpp
    runtime/JSStringSubspace.cpp
    runtime/JSSymbolTableObject.cpp
    runtime/JSTemplateRegistryKey.cpp
    runtime/JSTypedArrayConstructors.cpp
    runtime/JSTypedArrayPrototypes.cpp
    runtime/JSTypedArrayViewConstructor.cpp
    runtime/JSTypedArrayViewPrototype.cpp
    runtime/JSTypedArrays.cpp
    runtime/JSWeakMap.cpp
    runtime/JSWeakSet.cpp
    runtime/JSWithScope.cpp
    runtime/JSWrapperObject.cpp
    runtime/LazyClassStructure.cpp
    runtime/LiteralParser.cpp
    runtime/Lookup.cpp
    runtime/MapBase.cpp
    runtime/MapConstructor.cpp
    runtime/MapIteratorPrototype.cpp
    runtime/MapPrototype.cpp
    runtime/MatchResult.cpp
    runtime/MathCommon.cpp
    runtime/MathObject.cpp
    runtime/MemoryStatistics.cpp
    runtime/ModuleLoaderPrototype.cpp
    runtime/ModuleProgramExecutable.cpp
    runtime/NativeErrorConstructor.cpp
    runtime/NativeErrorPrototype.cpp
    runtime/NativeExecutable.cpp
    runtime/NativeStdFunctionCell.cpp
    runtime/NullGetterFunction.cpp
    runtime/NullSetterFunction.cpp
    runtime/NumberConstructor.cpp
    runtime/NumberObject.cpp
    runtime/NumberPrototype.cpp
    runtime/ObjectConstructor.cpp
    runtime/ObjectPrototype.cpp
    runtime/Operations.cpp
    runtime/Options.cpp
    runtime/ProgramExecutable.cpp
    runtime/PropertyDescriptor.cpp
    runtime/PropertySlot.cpp
    runtime/PropertyTable.cpp
    runtime/PrototypeMap.cpp
    runtime/ProxyConstructor.cpp
    runtime/ProxyObject.cpp
    runtime/ProxyRevoke.cpp
    runtime/ReflectObject.cpp
    runtime/RegExp.cpp
    runtime/RegExpCache.cpp
    runtime/RegExpCachedResult.cpp
    runtime/RegExpConstructor.cpp
    runtime/RegExpMatchesArray.cpp
    runtime/RegExpObject.cpp
    runtime/RegExpPrototype.cpp
    runtime/RuntimeType.cpp
    runtime/SamplingCounter.cpp
    runtime/SamplingProfiler.cpp
    runtime/ScopeOffset.cpp
    runtime/ScopedArguments.cpp
    runtime/ScopedArgumentsTable.cpp
    runtime/ScriptExecutable.cpp
    runtime/SetConstructor.cpp
    runtime/SetIteratorPrototype.cpp
    runtime/SetPrototype.cpp
    runtime/SimpleTypedArrayController.cpp
    runtime/SmallStrings.cpp
    runtime/SparseArrayValueMap.cpp
    runtime/StackFrame.cpp
    runtime/StrictEvalActivation.cpp
    runtime/StringConstructor.cpp
    runtime/StringIteratorPrototype.cpp
    runtime/StringObject.cpp
    runtime/StringPrototype.cpp
    runtime/StringRecursionChecker.cpp
    runtime/Structure.cpp
    runtime/StructureChain.cpp
    runtime/StructureIDTable.cpp
    runtime/StructureRareData.cpp
    runtime/Symbol.cpp
    runtime/SymbolConstructor.cpp
    runtime/SymbolObject.cpp
    runtime/SymbolPrototype.cpp
    runtime/SymbolTable.cpp
    runtime/TemplateRegistry.cpp
    runtime/TemplateRegistryKey.cpp
    runtime/TemplateRegistryKeyTable.cpp
    runtime/TestRunnerUtils.cpp
    runtime/ThrowScope.cpp
    runtime/TypeLocationCache.cpp
    runtime/TypeProfiler.cpp
    runtime/TypeProfilerLog.cpp
    runtime/TypeSet.cpp
    runtime/TypedArrayController.cpp
    runtime/TypedArrayType.cpp
    runtime/TypeofType.cpp
    runtime/VM.cpp
    runtime/VMEntryScope.cpp
    runtime/VMTraps.cpp
    runtime/VarOffset.cpp
    runtime/Watchdog.cpp
    runtime/WeakMapConstructor.cpp
    runtime/WeakMapData.cpp
    runtime/WeakMapPrototype.cpp
    runtime/WeakSetConstructor.cpp
    runtime/WeakSetPrototype.cpp

    tools/CellList.cpp
    tools/CodeProfile.cpp
    tools/CodeProfiling.cpp
    tools/FunctionOverrides.cpp
    tools/FunctionWhitelist.cpp
    tools/HeapVerifier.cpp
    tools/JSDollarVM.cpp
    tools/JSDollarVMPrototype.cpp
    tools/SigillCrashAnalyzer.cpp
    tools/StackTrace.cpp
    tools/VMInspector.cpp

    wasm/JSWebAssembly.cpp
    wasm/WasmB3IRGenerator.cpp
    wasm/WasmBinding.cpp
    wasm/WasmCallingConvention.cpp
    wasm/WasmFaultSignalHandler.cpp
    wasm/WasmFormat.cpp
    wasm/WasmMemory.cpp
    wasm/WasmMemoryInformation.cpp
    wasm/WasmModuleParser.cpp
    wasm/WasmPageCount.cpp
    wasm/WasmPlan.cpp
    wasm/WasmSignature.cpp
    wasm/WasmValidate.cpp

    wasm/js/JSWebAssemblyCallee.cpp
    wasm/js/JSWebAssemblyCodeBlock.cpp
    wasm/js/JSWebAssemblyCompileError.cpp
    wasm/js/JSWebAssemblyInstance.cpp
    wasm/js/JSWebAssemblyLinkError.cpp
    wasm/js/JSWebAssemblyMemory.cpp
    wasm/js/JSWebAssemblyModule.cpp
    wasm/js/JSWebAssemblyRuntimeError.cpp
    wasm/js/JSWebAssemblyTable.cpp
    wasm/js/WebAssemblyCompileErrorConstructor.cpp
    wasm/js/WebAssemblyCompileErrorPrototype.cpp
    wasm/js/WebAssemblyFunction.cpp
    wasm/js/WebAssemblyInstanceConstructor.cpp
    wasm/js/WebAssemblyInstancePrototype.cpp
    wasm/js/WebAssemblyLinkErrorConstructor.cpp
    wasm/js/WebAssemblyLinkErrorPrototype.cpp
    wasm/js/WebAssemblyMemoryConstructor.cpp
    wasm/js/WebAssemblyMemoryPrototype.cpp
    wasm/js/WebAssemblyModuleConstructor.cpp
    wasm/js/WebAssemblyModulePrototype.cpp
    wasm/js/WebAssemblyModuleRecord.cpp
    wasm/js/WebAssemblyPrototype.cpp
    wasm/js/WebAssemblyRuntimeErrorConstructor.cpp
    wasm/js/WebAssemblyRuntimeErrorPrototype.cpp
    wasm/js/WebAssemblyTableConstructor.cpp
    wasm/js/WebAssemblyTablePrototype.cpp
    wasm/js/WebAssemblyToJSCallee.cpp
    wasm/js/WebAssemblyWrapperFunction.cpp

    yarr/RegularExpression.cpp
    yarr/YarrCanonicalizeUCS2.cpp
    yarr/YarrInterpreter.cpp
    yarr/YarrJIT.cpp
    yarr/YarrPattern.cpp
    yarr/YarrSyntaxChecker.cpp
)

# Extra flags for compile sources can go here.
if (NOT MSVC)
    set_source_files_properties(runtime/ProxyObject.cpp PROPERTIES COMPILE_FLAGS -fno-optimize-sibling-calls)
else ()
    # FIXME: Investigate if we need to set a similar flag on Windows.
endif ()

set(JavaScriptCore_OBJECT_LUT_SOURCES
    runtime/ArrayConstructor.cpp
    runtime/ArrayIteratorPrototype.cpp
    runtime/BooleanPrototype.cpp
    runtime/DateConstructor.cpp
    runtime/DatePrototype.cpp
    runtime/ErrorPrototype.cpp
    runtime/GeneratorPrototype.cpp
    runtime/InspectorInstrumentationObject.cpp
    runtime/IntlCollatorConstructor.cpp
    runtime/IntlCollatorPrototype.cpp
    runtime/IntlDateTimeFormatConstructor.cpp
    runtime/IntlDateTimeFormatPrototype.cpp
    runtime/IntlNumberFormatConstructor.cpp
    runtime/IntlNumberFormatPrototype.cpp
    runtime/JSDataViewPrototype.cpp
    runtime/JSGlobalObject.cpp
    runtime/JSInternalPromiseConstructor.cpp
    runtime/JSONObject.cpp
    runtime/JSPromiseConstructor.cpp
    runtime/JSPromisePrototype.cpp
    runtime/MapPrototype.cpp
    runtime/ModuleLoaderPrototype.cpp
    runtime/NumberConstructor.cpp
    runtime/NumberPrototype.cpp
    runtime/ObjectConstructor.cpp
    runtime/ReflectObject.cpp
    runtime/RegExpConstructor.cpp
    runtime/RegExpPrototype.cpp
    runtime/SetPrototype.cpp
    runtime/StringConstructor.cpp
    runtime/StringIteratorPrototype.cpp
    runtime/StringPrototype.cpp
    runtime/SymbolConstructor.cpp
    runtime/SymbolPrototype.cpp

    wasm/js/WebAssemblyCompileErrorConstructor.cpp
    wasm/js/WebAssemblyCompileErrorPrototype.cpp
    wasm/js/WebAssemblyInstanceConstructor.cpp
    wasm/js/WebAssemblyInstancePrototype.cpp
    wasm/js/WebAssemblyLinkErrorConstructor.cpp
    wasm/js/WebAssemblyLinkErrorPrototype.cpp
    wasm/js/WebAssemblyMemoryConstructor.cpp
    wasm/js/WebAssemblyMemoryPrototype.cpp
    wasm/js/WebAssemblyModuleConstructor.cpp
    wasm/js/WebAssemblyModulePrototype.cpp
    wasm/js/WebAssemblyPrototype.cpp
    wasm/js/WebAssemblyRuntimeErrorConstructor.cpp
    wasm/js/WebAssemblyRuntimeErrorPrototype.cpp
    wasm/js/WebAssemblyTableConstructor.cpp
    wasm/js/WebAssemblyTablePrototype.cpp
)

set(JavaScriptCore_LIBRARIES
    WTF${DEBUG_SUFFIX}
    ${ICU_I18N_LIBRARIES}
    ${LLVM_LIBRARIES}
)

set(JavaScriptCore_SCRIPTS_SOURCES_DIR "${JAVASCRIPTCORE_DIR}/Scripts")

# Globbing relies on the fact that generator-specific file names are prefixed with their directory.
# Top-level scripts should have a file extension, since they are invoked during the build.

set(JavaScriptCore_SCRIPTS_SOURCES_PATHS
    ${JavaScriptCore_SCRIPTS_SOURCES_DIR}/*.pl
    ${JavaScriptCore_SCRIPTS_SOURCES_DIR}/*.py
    ${JavaScriptCore_SCRIPTS_SOURCES_DIR}/builtins/builtins*.py
)

# Force JavaScriptCore to run scripts from the same staging path as WebCore.
set(JavaScriptCore_SCRIPTS_DIR "${DERIVED_SOURCES_DIR}/ForwardingHeaders/JavaScriptCore/Scripts")

file(MAKE_DIRECTORY ${JavaScriptCore_SCRIPTS_DIR})

# The directory flattening performed below mirrors what the Mac port does with private headers.

file(GLOB JavaScriptCore_SCRIPTS_SOURCES ${JavaScriptCore_SCRIPTS_SOURCES_PATHS})

foreach (_file ${JavaScriptCore_SCRIPTS_SOURCES})
    get_filename_component(_script "${_file}" NAME)
    add_custom_command(
        OUTPUT ${JavaScriptCore_SCRIPTS_DIR}/${_script}
        MAIN_DEPENDENCY ${_file}
        WORKING_DIRECTORY ${DERIVED_SOURCES_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_file} ${JavaScriptCore_SCRIPTS_DIR}/${_script}
        VERBATIM)
    list(APPEND JavaScriptCore_SCRIPTS ${JavaScriptCore_SCRIPTS_DIR}/${_script})
endforeach ()

set(UDIS_GEN_DEP
    disassembler/udis86/ud_opcode.py
)

add_custom_command(
    OUTPUT ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/udis86_itab.c ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/udis86_itab.h
    DEPENDS ${UDIS_GEN_DEP}
    WORKING_DIRECTORY ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}
    COMMAND ${PYTHON_EXECUTABLE} ${JAVASCRIPTCORE_DIR}/disassembler/udis86/ud_itab.py ${JAVASCRIPTCORE_DIR}/disassembler/udis86/optable.xml ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}
    VERBATIM)

list(APPEND JavaScriptCore_HEADERS
    ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/udis86_itab.h
)

set(LLINT_ASM
    llint/LowLevelInterpreter.asm
    llint/LowLevelInterpreter32_64.asm
    llint/LowLevelInterpreter64.asm
)

set(OFFLINE_ASM
    offlineasm/arm.rb
    offlineasm/arm64.rb
    offlineasm/ast.rb
    offlineasm/backends.rb
    offlineasm/cloop.rb
    offlineasm/config.rb
    offlineasm/instructions.rb
    offlineasm/mips.rb
    offlineasm/offsets.rb
    offlineasm/opt.rb
    offlineasm/parser.rb
    offlineasm/registers.rb
    offlineasm/risc.rb
    offlineasm/self_hash.rb
    offlineasm/settings.rb
    offlineasm/transform.rb
    offlineasm/x86.rb
)

add_custom_command(
    OUTPUT ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/Bytecodes.h ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/InitBytecodes.asm
    MAIN_DEPENDENCY ${JAVASCRIPTCORE_DIR}/generate-bytecode-files
    DEPENDS ${JAVASCRIPTCORE_DIR}/generate-bytecode-files bytecode/BytecodeList.json
    COMMAND ${PYTHON_EXECUTABLE} ${JAVASCRIPTCORE_DIR}/generate-bytecode-files --bytecodes_h ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/Bytecodes.h --init_bytecodes_asm ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/InitBytecodes.asm ${JAVASCRIPTCORE_DIR}/bytecode/BytecodeList.json
    VERBATIM)

list(APPEND JavaScriptCore_HEADERS
    ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/Bytecodes.h
)

add_custom_command(
    OUTPUT ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/LLIntDesiredOffsets.h
    MAIN_DEPENDENCY ${JAVASCRIPTCORE_DIR}/offlineasm/generate_offset_extractor.rb
    DEPENDS ${LLINT_ASM} ${OFFLINE_ASM} ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/InitBytecodes.asm
    COMMAND ${RUBY_EXECUTABLE} ${JAVASCRIPTCORE_DIR}/offlineasm/generate_offset_extractor.rb -I${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/ ${JAVASCRIPTCORE_DIR}/llint/LowLevelInterpreter.asm ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/LLIntDesiredOffsets.h
    VERBATIM)

# We add the header files directly to the ADD_EXECUTABLE call instead of setting the
# OBJECT_DEPENDS property in LLIntOffsetsExtractor.cpp because generate_offset_extractor.rb and
# generate-bytecode-files may not regenerate the .h files in case the hash it calculates does not change.
# In this case, if some of the dependencies specified in the ADD_CUSTOM_COMMANDs above have
# changed the command will always be called because the mtime of the .h files will
# always be older than that of their dependencies.
# Additionally, setting the OBJECT_DEPENDS property will make the .h files a Makefile
# dependency of both LLIntOffsetsExtractor and LLIntOffsetsExtractor.cpp, so the command will
# actually be run multiple times!
add_executable(LLIntOffsetsExtractor
    ${JAVASCRIPTCORE_DIR}/llint/LLIntOffsetsExtractor.cpp
    ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/LLIntDesiredOffsets.h ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/Bytecodes.h
)
target_link_libraries(LLIntOffsetsExtractor WTF)

# The build system will execute asm.rb every time LLIntOffsetsExtractor's mtime is newer than
# LLIntAssembly.h's mtime. The problem we have here is: asm.rb has some built-in optimization
# that generates a checksum of the LLIntOffsetsExtractor binary, if the checksum of the new
# LLIntOffsetsExtractor matches, no output is generated. To make this target consistent and avoid
# running this command for every build, we artificially update LLIntAssembly.h's mtime (using touch)
# after every asm.rb run.
if (MSVC)
    set(LLIntOutput LowLevelInterpreterWin.asm)
    set(OFFLINE_ASM_ARGS --assembler=MASM)
else ()
    set(LLIntOutput LLIntAssembly.h)
endif ()

add_custom_command(
    OUTPUT ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/${LLIntOutput}
    MAIN_DEPENDENCY ${JAVASCRIPTCORE_DIR}/offlineasm/asm.rb
    DEPENDS LLIntOffsetsExtractor ${LLINT_ASM} ${OFFLINE_ASM} ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/InitBytecodes.asm
    COMMAND ${RUBY_EXECUTABLE} ${JAVASCRIPTCORE_DIR}/offlineasm/asm.rb -I${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/ ${JAVASCRIPTCORE_DIR}/llint/LowLevelInterpreter.asm $<TARGET_FILE:LLIntOffsetsExtractor> ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/${LLIntOutput} ${OFFLINE_ASM_ARGS}
    COMMAND ${CMAKE_COMMAND} -E touch_nocreate ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/${LLIntOutput}
    WORKING_DIRECTORY ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}
    VERBATIM)

# The explanation for not making LLIntAssembly.h part of the OBJECT_DEPENDS property of some of
# the .cpp files below is similar to the one in the previous comment. However, since these .cpp
# files are used to build JavaScriptCore itself, we can just add LLIntAssembly.h to JSC_HEADERS
# since it is used in the add_library() call at the end of this file.
if (MSVC)
    enable_language(ASM_MASM)
    list(APPEND JavaScriptCore_SOURCES
        ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/LowLevelInterpreterWin.asm
    )
    # Win32 needs /safeseh with assembly, but Win64 does not.
    if (CMAKE_SIZEOF_VOID_P EQUAL 4)
        set_source_files_properties(${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/LowLevelInterpreterWin.asm
            PROPERTIES COMPILE_FLAGS  "/safeseh"
        )
    endif ()
else ()
    list(APPEND JavaScriptCore_HEADERS
        ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/LLIntAssembly.h
    )
endif ()

# WebAssembly generator

macro(GENERATE_PYTHON _generator _input _output)
    add_custom_command(
        OUTPUT ${_output}
        MAIN_DEPENDENCY ${_generator}
        DEPENDS ${_input}
        COMMAND ${PYTHON_EXECUTABLE} ${_generator} ${_input} ${_output}
        VERBATIM)
    list(APPEND JavaScriptCore_HEADERS ${_output})
    ADD_SOURCE_DEPENDENCIES(${_input} ${_output})
endmacro()
GENERATE_PYTHON(${CMAKE_CURRENT_SOURCE_DIR}/wasm/generateWasmOpsHeader.py ${CMAKE_CURRENT_SOURCE_DIR}/wasm/wasm.json ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/WasmOps.h)
GENERATE_PYTHON(${CMAKE_CURRENT_SOURCE_DIR}/wasm/generateWasmValidateInlinesHeader.py ${CMAKE_CURRENT_SOURCE_DIR}/wasm/wasm.json ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/WasmValidateInlines.h)
GENERATE_PYTHON(${CMAKE_CURRENT_SOURCE_DIR}/wasm/generateWasmB3IRGeneratorInlinesHeader.py ${CMAKE_CURRENT_SOURCE_DIR}/wasm/wasm.json ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/WasmB3IRGeneratorInlines.h)

# LUT generator

set(HASH_LUT_GENERATOR ${CMAKE_CURRENT_SOURCE_DIR}/create_hash_table)
macro(GENERATE_HASH_LUT _input _output)
    add_custom_command(
        OUTPUT ${_output}
        MAIN_DEPENDENCY ${HASH_LUT_GENERATOR}
        DEPENDS ${_input}
        COMMAND ${PERL_EXECUTABLE} ${HASH_LUT_GENERATOR} ${_input} > ${_output}
        VERBATIM)
    list(APPEND JavaScriptCore_HEADERS ${_output})
    ADD_SOURCE_DEPENDENCIES(${_input} ${_output})
endmacro()

# GENERATOR 1-A: LUT creator

foreach (_file ${JavaScriptCore_OBJECT_LUT_SOURCES})
    get_filename_component(_name ${_file} NAME_WE)
    GENERATE_HASH_LUT(${CMAKE_CURRENT_SOURCE_DIR}/${_file} ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/${_name}.lut.h)
endforeach ()

set(JavaScriptCore_FORWARDING_HEADERS_DIRECTORIES
    API
    assembler
    bindings
    builtins
    bytecode
    debugger
    dfg
    disassembler
    domjit
    heap
    inspector
    interpreter
    jit
    llint
    parser
    profiler
    replay
    runtime
    wasm
    yarr

    collector/handles

    inspector/agents
    inspector/augmentable
    inspector/remote
)

# GENERATOR 1-B: particular LUT creator (for 1 file only)
GENERATE_HASH_LUT(${CMAKE_CURRENT_SOURCE_DIR}/parser/Keywords.table ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/Lexer.lut.h)

#GENERATOR: "RegExpJitTables.h": tables used by Yarr
add_custom_command(
    OUTPUT ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/RegExpJitTables.h
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/create_regex_tables
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/create_regex_tables > ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/RegExpJitTables.h
    VERBATIM)
ADD_SOURCE_DEPENDENCIES(${CMAKE_CURRENT_SOURCE_DIR}/yarr/YarrPattern.cpp ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/RegExpJitTables.h)

add_custom_command(
    OUTPUT ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/YarrCanonicalizeUnicode.cpp
    MAIN_DEPENDENCY ${JAVASCRIPTCORE_DIR}/generateYarrCanonicalizeUnicode
    DEPENDS ${JAVASCRIPTCORE_DIR}/ucd/CaseFolding.txt
    COMMAND ${PYTHON_EXECUTABLE} ${JAVASCRIPTCORE_DIR}/generateYarrCanonicalizeUnicode ${JAVASCRIPTCORE_DIR}/ucd/CaseFolding.txt ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/YarrCanonicalizeUnicode.cpp
    VERBATIM)

list(APPEND JavaScriptCore_SOURCES
    ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/YarrCanonicalizeUnicode.cpp
)

#GENERATOR: "KeywordLookup.h": keyword decision tree used by the lexer
add_custom_command(
    OUTPUT ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/KeywordLookup.h
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/KeywordLookupGenerator.py
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/parser/Keywords.table
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/KeywordLookupGenerator.py ${CMAKE_CURRENT_SOURCE_DIR}/parser/Keywords.table > ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/KeywordLookup.h
    VERBATIM)
ADD_SOURCE_DEPENDENCIES(${CMAKE_CURRENT_SOURCE_DIR}/parser/Lexer.cpp ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/KeywordLookup.h)


# Inspector Interfaces

set(JavaScriptCore_INSPECTOR_SCRIPTS_DIR "${JAVASCRIPTCORE_DIR}/inspector/scripts")

set(JavaScriptCore_INSPECTOR_PROTOCOL_SCRIPTS
    ${JavaScriptCore_INSPECTOR_SCRIPTS_DIR}/generate-inspector-protocol-bindings.py
    ${JavaScriptCore_INSPECTOR_SCRIPTS_DIR}/codegen/cpp_generator.py
    ${JavaScriptCore_INSPECTOR_SCRIPTS_DIR}/codegen/cpp_generator_templates.py
    ${JavaScriptCore_INSPECTOR_SCRIPTS_DIR}/codegen/generate_js_backend_commands.py
    ${JavaScriptCore_INSPECTOR_SCRIPTS_DIR}/codegen/generate_cpp_backend_dispatcher_header.py
    ${JavaScriptCore_INSPECTOR_SCRIPTS_DIR}/codegen/generate_cpp_backend_dispatcher_implementation.py
    ${JavaScriptCore_INSPECTOR_SCRIPTS_DIR}/codegen/generate_cpp_frontend_dispatcher_header.py
    ${JavaScriptCore_INSPECTOR_SCRIPTS_DIR}/codegen/generate_cpp_frontend_dispatcher_implementation.py
    ${JavaScriptCore_INSPECTOR_SCRIPTS_DIR}/codegen/generate_cpp_protocol_types_header.py
    ${JavaScriptCore_INSPECTOR_SCRIPTS_DIR}/codegen/generate_cpp_protocol_types_implementation.py
    ${JavaScriptCore_INSPECTOR_SCRIPTS_DIR}/codegen/generator.py
    ${JavaScriptCore_INSPECTOR_SCRIPTS_DIR}/codegen/generator_templates.py
    ${JavaScriptCore_INSPECTOR_SCRIPTS_DIR}/codegen/__init__.py
    ${JavaScriptCore_INSPECTOR_SCRIPTS_DIR}/codegen/models.py
)

set(JavaScriptCore_INSPECTOR_DOMAINS
    ${JAVASCRIPTCORE_DIR}/inspector/protocol/ApplicationCache.json
    ${JAVASCRIPTCORE_DIR}/inspector/protocol/CSS.json
    ${JAVASCRIPTCORE_DIR}/inspector/protocol/Console.json
    ${JAVASCRIPTCORE_DIR}/inspector/protocol/DOM.json
    ${JAVASCRIPTCORE_DIR}/inspector/protocol/DOMDebugger.json
    ${JAVASCRIPTCORE_DIR}/inspector/protocol/DOMStorage.json
    ${JAVASCRIPTCORE_DIR}/inspector/protocol/Database.json
    ${JAVASCRIPTCORE_DIR}/inspector/protocol/Debugger.json
    ${JAVASCRIPTCORE_DIR}/inspector/protocol/GenericTypes.json
    ${JAVASCRIPTCORE_DIR}/inspector/protocol/Heap.json
    ${JAVASCRIPTCORE_DIR}/inspector/protocol/Inspector.json
    ${JAVASCRIPTCORE_DIR}/inspector/protocol/LayerTree.json
    ${JAVASCRIPTCORE_DIR}/inspector/protocol/Network.json
    ${JAVASCRIPTCORE_DIR}/inspector/protocol/OverlayTypes.json
    ${JAVASCRIPTCORE_DIR}/inspector/protocol/Page.json
    ${JAVASCRIPTCORE_DIR}/inspector/protocol/Runtime.json
    ${JAVASCRIPTCORE_DIR}/inspector/protocol/ScriptProfiler.json
    ${JAVASCRIPTCORE_DIR}/inspector/protocol/Timeline.json
    ${JAVASCRIPTCORE_DIR}/inspector/protocol/Worker.json
)

if (ENABLE_INDEXED_DATABASE)
    list(APPEND JavaScriptCore_INSPECTOR_DOMAINS
        ${JAVASCRIPTCORE_DIR}/inspector/protocol/IndexedDB.json
    )
endif ()

if (ENABLE_RESOURCE_USAGE)
    list(APPEND JavaScriptCore_INSPECTOR_DOMAINS
        ${JAVASCRIPTCORE_DIR}/inspector/protocol/Memory.json
    )
endif ()

if (ENABLE_WEB_REPLAY)
    list(APPEND JavaScriptCore_INSPECTOR_DOMAINS
        ${JAVASCRIPTCORE_DIR}/inspector/protocol/Replay.json
    )
endif ()

add_custom_command(
    OUTPUT ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/CombinedDomains.json
    MAIN_DEPENDENCY ${JavaScriptCore_SCRIPTS_DIR}/generate-combined-inspector-json.py
    DEPENDS ${JavaScriptCore_INSPECTOR_DOMAINS}
    COMMAND ${PYTHON_EXECUTABLE} ${JavaScriptCore_SCRIPTS_DIR}/generate-combined-inspector-json.py ${JavaScriptCore_INSPECTOR_DOMAINS} > ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/CombinedDomains.json
    VERBATIM)

# Inspector Backend Dispatchers, Frontend Dispatchers, Type Builders
file(MAKE_DIRECTORY ${DERIVED_SOURCES_WEBINSPECTORUI_DIR}/UserInterface/Protocol)
file(MAKE_DIRECTORY ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/inspector)
add_custom_command(
    OUTPUT ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/inspector/InspectorBackendDispatchers.cpp
           ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/inspector/InspectorBackendDispatchers.h
           ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/inspector/InspectorFrontendDispatchers.cpp
           ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/inspector/InspectorFrontendDispatchers.h
           ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/inspector/InspectorProtocolObjects.cpp
           ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/inspector/InspectorProtocolObjects.h
           ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/inspector/InspectorBackendCommands.js
    MAIN_DEPENDENCY ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/CombinedDomains.json
    DEPENDS ${JavaScriptCore_INSPECTOR_PROTOCOL_SCRIPTS}
    COMMAND ${PYTHON_EXECUTABLE} ${JavaScriptCore_INSPECTOR_SCRIPTS_DIR}/generate-inspector-protocol-bindings.py --outputDir "${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/inspector" --framework JavaScriptCore ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/CombinedDomains.json
    VERBATIM)

# JSCBuiltins

set(BUILTINS_GENERATOR_SCRIPTS
    ${JavaScriptCore_SCRIPTS_DIR}/builtins.py
    ${JavaScriptCore_SCRIPTS_DIR}/builtins_generator.py
    ${JavaScriptCore_SCRIPTS_DIR}/builtins_model.py
    ${JavaScriptCore_SCRIPTS_DIR}/builtins_templates.py
    ${JavaScriptCore_SCRIPTS_DIR}/builtins_generate_combined_header.py
    ${JavaScriptCore_SCRIPTS_DIR}/builtins_generate_combined_implementation.py
    ${JavaScriptCore_SCRIPTS_DIR}/builtins_generate_separate_header.py
    ${JavaScriptCore_SCRIPTS_DIR}/builtins_generate_separate_implementation.py
    ${JavaScriptCore_SCRIPTS_DIR}/builtins_generate_wrapper_header.py
    ${JavaScriptCore_SCRIPTS_DIR}/builtins_generate_wrapper_implementation.py
    ${JavaScriptCore_SCRIPTS_DIR}/builtins_generate_internals_wrapper_header.py
    ${JavaScriptCore_SCRIPTS_DIR}/builtins_generate_internals_wrapper_implementation.py
    ${JavaScriptCore_SCRIPTS_DIR}/generate-js-builtins.py
    ${JavaScriptCore_SCRIPTS_DIR}/lazywriter.py
)

set(JavaScriptCore_BUILTINS_SOURCES
    ${JAVASCRIPTCORE_DIR}/builtins/ArrayConstructor.js
    ${JAVASCRIPTCORE_DIR}/builtins/ArrayIteratorPrototype.js
    ${JAVASCRIPTCORE_DIR}/builtins/ArrayPrototype.js
    ${JAVASCRIPTCORE_DIR}/builtins/AsyncFunctionPrototype.js
    ${JAVASCRIPTCORE_DIR}/builtins/DatePrototype.js
    ${JAVASCRIPTCORE_DIR}/builtins/FunctionPrototype.js
    ${JAVASCRIPTCORE_DIR}/builtins/GeneratorPrototype.js
    ${JAVASCRIPTCORE_DIR}/builtins/GlobalObject.js
    ${JAVASCRIPTCORE_DIR}/builtins/GlobalOperations.js
    ${JAVASCRIPTCORE_DIR}/builtins/InspectorInstrumentationObject.js
    ${JAVASCRIPTCORE_DIR}/builtins/InternalPromiseConstructor.js
    ${JAVASCRIPTCORE_DIR}/builtins/IteratorHelpers.js
    ${JAVASCRIPTCORE_DIR}/builtins/IteratorPrototype.js
    ${JAVASCRIPTCORE_DIR}/builtins/MapPrototype.js
    ${JAVASCRIPTCORE_DIR}/builtins/ModuleLoaderPrototype.js
    ${JAVASCRIPTCORE_DIR}/builtins/NumberConstructor.js
    ${JAVASCRIPTCORE_DIR}/builtins/NumberPrototype.js
    ${JAVASCRIPTCORE_DIR}/builtins/ObjectConstructor.js
    ${JAVASCRIPTCORE_DIR}/builtins/PromiseConstructor.js
    ${JAVASCRIPTCORE_DIR}/builtins/PromiseOperations.js
    ${JAVASCRIPTCORE_DIR}/builtins/PromisePrototype.js
    ${JAVASCRIPTCORE_DIR}/builtins/ReflectObject.js
    ${JAVASCRIPTCORE_DIR}/builtins/RegExpPrototype.js
    ${JAVASCRIPTCORE_DIR}/builtins/SetPrototype.js
    ${JAVASCRIPTCORE_DIR}/builtins/StringConstructor.js
    ${JAVASCRIPTCORE_DIR}/builtins/StringIteratorPrototype.js
    ${JAVASCRIPTCORE_DIR}/builtins/StringPrototype.js
    ${JAVASCRIPTCORE_DIR}/builtins/TypedArrayConstructor.js
    ${JAVASCRIPTCORE_DIR}/builtins/TypedArrayPrototype.js
)

add_custom_command(
    OUTPUT ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/AirOpcode.h ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/AirOpcodeGenerated.h
    MAIN_DEPENDENCY ${JAVASCRIPTCORE_DIR}/b3/air/AirOpcode.opcodes
    DEPENDS ${JAVASCRIPTCORE_DIR}/b3/air/opcode_generator.rb
    COMMAND ${RUBY_EXECUTABLE} ${JAVASCRIPTCORE_DIR}/b3/air/opcode_generator.rb ${JAVASCRIPTCORE_DIR}/b3/air/AirOpcode.opcodes VERBATIM
    WORKING_DIRECTORY ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}
)

list(APPEND JavaScriptCore_SOURCES
    ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/AirOpcode.h
    ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/AirOpcodeGenerated.h
)

add_custom_command(
    OUTPUT ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/JSCBuiltins.cpp ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/JSCBuiltins.h
    MAIN_DEPENDENCY ${JavaScriptCore_SCRIPTS_DIR}/generate-js-builtins.py
    DEPENDS ${JavaScriptCore_BUILTINS_SOURCES} ${BUILTINS_GENERATOR_SCRIPTS}
    COMMAND ${PYTHON_EXECUTABLE} ${JavaScriptCore_SCRIPTS_DIR}/generate-js-builtins.py --framework JavaScriptCore --output-directory ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR} --combined ${JavaScriptCore_BUILTINS_SOURCES}
    VERBATIM)

list(APPEND JavaScriptCore_SOURCES
    ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/inspector/InspectorBackendDispatchers.cpp
    ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/inspector/InspectorFrontendDispatchers.cpp
    ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/inspector/InspectorProtocolObjects.cpp
    ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/JSCBuiltins.cpp
)

list(APPEND JavaScriptCore_HEADERS
    ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/inspector/InspectorBackendDispatchers.h
    ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/inspector/InspectorFrontendDispatchers.h
    ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/inspector/InspectorProtocolObjects.h
    ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/JSCBuiltins.h
)

add_custom_command(
    OUTPUT ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/InjectedScriptSource.h ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/InjectedScriptSource.min.js
    MAIN_DEPENDENCY ${JAVASCRIPTCORE_DIR}/inspector/InjectedScriptSource.js
    DEPENDS ${JavaScriptCore_SCRIPTS_DIR}/xxd.pl ${JavaScriptCore_SCRIPTS_DIR}/jsmin.py
    COMMAND ${CMAKE_COMMAND} -E echo "//# sourceURL=__InjectedScript_InjectedScriptSource.js" > ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/InjectedScriptSource.min.js
    COMMAND ${PYTHON_EXECUTABLE} ${JavaScriptCore_SCRIPTS_DIR}/jsmin.py < ${JAVASCRIPTCORE_DIR}/inspector/InjectedScriptSource.js >> ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/InjectedScriptSource.min.js
    COMMAND ${PERL_EXECUTABLE} ${JavaScriptCore_SCRIPTS_DIR}/xxd.pl InjectedScriptSource_js ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/InjectedScriptSource.min.js ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/InjectedScriptSource.h
    VERBATIM)

list(APPEND JavaScriptCore_HEADERS ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/InjectedScriptSource.h)

# Web Replay inputs generator
if (ENABLE_WEB_REPLAY)
    set(JavaScript_WEB_REPLAY_INPUTS ${CMAKE_CURRENT_SOURCE_DIR}/replay/JSInputs.json)
    add_custom_command(
        OUTPUT ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/JSReplayInputs.h ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/JSReplayInputs.cpp
        MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/replay/scripts/CodeGeneratorReplayInputs.py
        DEPENDS ${JavaScript_WEB_REPLAY_INPUTS}
        COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/replay/scripts/CodeGeneratorReplayInputs.py --outputDir ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/ --framework JavaScriptCore ${JavaScript_WEB_REPLAY_INPUTS}
        VERBATIM)

    list(APPEND JavaScriptCore_SOURCES
        replay/EncodedValue.cpp
        ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/JSReplayInputs.cpp
    )
    list(APPEND JavaScriptCore_HEADERS ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/JSReplayInputs.h)
endif ()

if (WTF_CPU_ARM)
elseif (WTF_CPU_ARM64)
elseif (WTF_CPU_HPPA)
elseif (WTF_CPU_PPC)
elseif (WTF_CPU_PPC64)
elseif (WTF_CPU_PPC64LE)
elseif (WTF_CPU_S390)
elseif (WTF_CPU_S390X)
elseif (WTF_CPU_MIPS)
elseif (WTF_CPU_SH4)
elseif (WTF_CPU_X86)
elseif (WTF_CPU_X86_64)
    if (MSVC AND ENABLE_JIT)
        add_custom_command(
            OUTPUT ${DERIVED_SOURCES_DIR}/JITStubsMSVC64.obj
            MAIN_DEPENDENCY ${JAVASCRIPTCORE_DIR}/jit/JITStubsMSVC64.asm
            COMMAND ml64 -nologo -c -Fo ${DERIVED_SOURCES_DIR}/JITStubsMSVC64.obj ${JAVASCRIPTCORE_DIR}/jit/JITStubsMSVC64.asm
            VERBATIM)

        list(APPEND JavaScriptCore_SOURCES ${DERIVED_SOURCES_DIR}/JITStubsMSVC64.obj)
    endif ()
else ()
    message(FATAL_ERROR "Unknown CPU")
endif ()

if (WTF_CPU_X86)
    enable_language(ASM)
    list(APPEND JavaScriptCore_SOURCES
        runtime/X86Atomics.S
        )
endif ()

WEBKIT_INCLUDE_CONFIG_FILES_IF_EXISTS()

WEBKIT_CREATE_FORWARDING_HEADERS(JavaScriptCore DIRECTORIES ${JavaScriptCore_FORWARDING_HEADERS_DIRECTORIES} FILES ${JavaScriptCore_FORWARDING_HEADERS_FILES})

target_include_directories(LLIntOffsetsExtractor PRIVATE ${JavaScriptCore_INCLUDE_DIRECTORIES})

add_subdirectory(shell)

WEBKIT_WRAP_SOURCELIST(${JavaScriptCore_SOURCES})
WEBKIT_FRAMEWORK(JavaScriptCore)

MESSAGE("JavaScriptCore_LIBRARY_TYPE: ${JavaScriptCore_LIBRARY_TYPE}")
if (NOT "${PORT}" STREQUAL "Mac")
    if (${JavaScriptCore_LIBRARY_TYPE} STREQUAL "SHARED")
        POPULATE_LIBRARY_VERSION(JAVASCRIPTCORE)
        set_target_properties(JavaScriptCore PROPERTIES VERSION ${JAVASCRIPTCORE_VERSION} SOVERSION ${JAVASCRIPTCORE_VERSION_MAJOR})
        install(TARGETS JavaScriptCore DESTINATION "${LIB_INSTALL_DIR}")
    endif ()
endif ()

# Force staging of shared scripts, even if they aren't directly used to build JavaScriptCore.

add_custom_target(stageSharedScripts DEPENDS ${JavaScriptCore_SCRIPTS})
add_dependencies(JavaScriptCore stageSharedScripts)

if (MSVC)
    add_custom_command(
        TARGET JavaScriptCore
        PRE_BUILD
        COMMAND ${PERL_EXECUTABLE} ${WEBKIT_LIBRARIES_DIR}/tools/scripts/auto-version.pl ${DERIVED_SOURCES_DIR}
        VERBATIM)

    add_custom_command(
        TARGET JavaScriptCore
        POST_BUILD
        COMMAND ${PERL_EXECUTABLE} ${WEBKIT_LIBRARIES_DIR}/tools/scripts/version-stamp.pl ${DERIVED_SOURCES_DIR} $<TARGET_FILE:JavaScriptCore>
        VERBATIM)
endif ()
