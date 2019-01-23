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
#pragma once

#include "Identifier.h"
#include <wtf/Noncopyable.h>

// MarkedArgumentBuffer of property names, passed to a macro so we can do set them up various
// ways without repeating the list.
#define JSC_COMMON_IDENTIFIERS_EACH_PROPERTY_NAME(macro) \
    macro(Animation) \
    macro(AnimationEffectReadOnly) \
    macro(AnimationTimeline) \
    macro(Array) \
    macro(ArrayBuffer) \
    macro(ArrayIterator) \
    macro(Audio) \
    macro(BYTES_PER_ELEMENT) \
    macro(Boolean) \
    macro(Collator) \
    macro(Credential) \
    macro(CustomElementRegistry) \
    macro(Date) \
    macro(DateTimeFormat) \
    macro(DocumentTimeline) \
    macro(Error) \
    macro(EvalError) \
    macro(Function) \
    macro(Gamepad) \
    macro(GamepadButton) \
    macro(GamepadEvent) \
    macro(GeneratorFunction) \
    macro(Headers) \
    macro(HTMLAudioElement) \
    macro(HTMLSlotElement) \
    macro(IDBCursor) \
    macro(IDBCursorWithValue) \
    macro(IDBDatabase) \
    macro(IDBFactory) \
    macro(IDBIndex) \
    macro(IDBKeyRange) \
    macro(IDBObjectStore) \
    macro(IDBOpenDBRequest) \
    macro(IDBRequest) \
    macro(IDBTransaction) \
    macro(IDBVersionChangeEvent) \
    macro(Infinity) \
    macro(InputEvent) \
    macro(IntersectionObserver) \
    macro(IntersectionObserverEntry) \
    macro(Intl) \
    macro(JSON) \
    macro(Loader) \
    macro(Map)\
    macro(MapIterator)\
    macro(Math) \
    macro(MediaEncryptedEvent) \
    macro(MediaKeyMessageEvent) \
    macro(MediaKeySession) \
    macro(MediaKeyStatusMap) \
    macro(MediaKeySystemAccess) \
    macro(MediaKeys) \
    macro(MediaStreamEvent) \
    macro(ModernMediaControls) \
    macro(NaN) \
    macro(NavigatorMediaDevices) \
    macro(NavigatorUserMedia) \
    macro(Number) \
    macro(NumberFormat) \
    macro(Object) \
    macro(PasswordCredential) \
    macro(PerformanceEntry) \
    macro(PerformanceEntryList) \
    macro(PerformanceMark) \
    macro(PerformanceMeasure) \
    macro(PerformanceObserver) \
    macro(PerformanceObserverEntryList) \
    macro(PerformanceResourceTiming) \
    macro(Promise) \
    macro(Proxy) \
    macro(RangeError) \
    macro(ReferenceError) \
    macro(Reflect) \
    macro(RegExp) \
    macro(Response) \
    macro(Request) \
    macro(RTCDTMFToneChangeEvent) \
    macro(RTCIceCandidate) \
    macro(RTCPeerConnection) \
    macro(RTCRtpReceiver) \
    macro(RTCRtpSender) \
    macro(RTCRtpTransceiver) \
    macro(RTCSessionDescription) \
    macro(RTCTrackEvent) \
    macro(Set)\
    macro(SetIterator)\
    macro(ShadowRoot) \
    macro(SharedArrayBuffer) \
    macro(SiteBoundCredential) \
    macro(StaticRange) \
    macro(String) \
    macro(Symbol) \
    macro(SyntaxError) \
    macro(TypeError) \
    macro(URIError) \
    macro(UTC) \
    macro(WeakMap)\
    macro(WeakSet)\
    macro(WebGL2RenderingContext) \
    macro(WebGLVertexArrayObject) \
    macro(WebGPUBuffer) \
    macro(WebGPUCommandBuffer) \
    macro(WebGPUCommandQueue) \
    macro(WebGPUDepthStencilDescriptor) \
    macro(WebGPUDepthStencilState) \
    macro(WebGPUDrawable) \
    macro(WebGPUFunction) \
    macro(WebGPULibrary) \
    macro(WebGPURenderCommandEncoder) \
    macro(WebGPURenderPassAttachmentDescriptor) \
    macro(WebGPURenderPassColorAttachmentDescriptor) \
    macro(WebGPURenderPassDepthAttachmentDescriptor) \
    macro(WebGPURenderPassDescriptor) \
    macro(WebGPURenderPipelineColorAttachmentDescriptor) \
    macro(WebGPURenderPipelineDescriptor) \
    macro(WebGPURenderPipelineState) \
    macro(WebGPURenderingContext) \
    macro(WebGPUTexture) \
    macro(WebGPUTextureDescriptor) \
    macro(WebSocket) \
    macro(__defineGetter__) \
    macro(__defineSetter__) \
    macro(__lookupGetter__) \
    macro(__lookupSetter__) \
    macro(add) \
    macro(additionalJettisonReason) \
    macro(anonymous) \
    macro(arguments) \
    macro(as) \
    macro(assign) \
    macro(back) \
    macro(bind) \
    macro(blur) \
    macro(buffer) \
    macro(byteLength) \
    macro(byteOffset) \
    macro(bytecode) \
    macro(bytecodeIndex) \
    macro(bytecodes) \
    macro(bytecodesID) \
    macro(calendar) \
    macro(callee) \
    macro(caller) \
    macro(caseFirst) \
    macro(clear) \
    macro(close) \
    macro(closed) \
    macro(collation) \
    macro(column) \
    macro(compilationKind) \
    macro(compilationUID) \
    macro(compilations) \
    macro(compile) \
    macro(configurable) \
    macro(constructor) \
    macro(count) \
    macro(counters) \
    macro(customElements) \
    macro(day) \
    macro(defineProperty) \
    macro(description) \
    macro(descriptions) \
    macro(detail) \
    macro(displayName) \
    macro(document) \
    macro(done) \
    macro(enumerable) \
    macro(era) \
    macro(eval) \
    macro(events) \
    macro(exec) \
    macro(executionCount) \
    macro(exitKind) \
    macro(fetch) \
    macro(flags) \
    macro(focus) \
    macro(forEach) \
    macro(formatMatcher) \
    macro(forward) \
    macro(frames) \
    macro(from) \
    macro(fromCharCode) \
    macro(get) \
    macro(global) \
    macro(go) \
    macro(has) \
    macro(hasOwnProperty) \
    macro(hash) \
    macro(header) \
    macro(hour) \
    macro(hour12) \
    macro(href) \
    macro(id) \
    macro(ignoreCase) \
    macro(ignorePunctuation) \
    macro(index) \
    macro(indexedDB) \
    macro(inferredName) \
    macro(input) \
    macro(instructionCount) \
    macro(isArray) \
    macro(isEnabled) \
    macro(isPrototypeOf) \
    macro(isView) \
    macro(isWatchpoint) \
    macro(jettisonReason) \
    macro(join) \
    macro(KeyframeEffect) \
    macro(lastIndex) \
    macro(length) \
    macro(line) \
    macro(locale) \
    macro(localeMatcher) \
    macro(location) \
    macro(message) \
    macro(minute) \
    macro(month) \
    macro(multiline) \
    macro(name) \
    macro(next) \
    macro(now) \
    macro(numInlinedCalls) \
    macro(numInlinedGetByIds) \
    macro(numInlinedPutByIds) \
    macro(numberingSystem) \
    macro(numeric) \
    macro(of) \
    macro(opcode) \
    macro(opener) \
    macro(origin) \
    macro(osrExitSites) \
    macro(osrExits) \
    macro(parent) \
    macro(parse) \
    macro(parseInt) \
    macro(postMessage) \
    macro(profiledBytecodes) \
    macro(propertyIsEnumerable) \
    macro(prototype) \
    macro(raw) \
    macro(reload) \
    macro(replace) \
    macro(resolve) \
    macro(second) \
    macro(self) \
    macro(sensitivity) \
    macro(set) \
    macro(showModalDialog) \
    macro(size) \
    macro(slice) \
    macro(source) \
    macro(sourceCode) \
    macro(sourceURL) \
    macro(stack) \
    macro(sticky) \
    macro(subarray) \
    macro(summary) \
    macro(target) \
    macro(test) \
    macro(then) \
    macro(time) \
    macro(timeZone) \
    macro(timeZoneName) \
    macro(toExponential) \
    macro(toFixed) \
    macro(toISOString) \
    macro(toJSON) \
    macro(toLocaleString) \
    macro(toPrecision) \
    macro(toString) \
    macro(top) \
    macro(uid) \
    macro(unicode) \
    macro(usage) \
    macro(value) \
    macro(valueOf) \
    macro(webkit) \
    macro(collectMatchingElementsInFlatTree) \
    macro(matchingElementInFlatTree) \
    macro(webkitIDBCursor) \
    macro(webkitIDBDatabase) \
    macro(webkitIDBFactory) \
    macro(webkitIDBIndex) \
    macro(webkitIDBKeyRange) \
    macro(webkitIDBObjectStore) \
    macro(webkitIDBRequest) \
    macro(webkitIDBTransaction) \
    macro(webkitIndexedDB) \
    macro(weekday) \
    macro(window) \
    macro(writable) \
    macro(year)

#define JSC_COMMON_IDENTIFIERS_EACH_KEYWORD(macro) \
    macro(async) \
    macro(await) \
    macro(break) \
    macro(case) \
    macro(catch) \
    macro(class) \
    macro(const) \
    macro(continue) \
    macro(debugger) \
    macro(default) \
    macro(delete) \
    macro(do) \
    macro(else) \
    macro(enum) \
    macro(export) \
    macro(extends) \
    macro(false) \
    macro(finally) \
    macro(for) \
    macro(function) \
    macro(if) \
    macro(implements) \
    macro(import) \
    macro(in) \
    macro(instanceof) \
    macro(interface) \
    macro(let) \
    macro(new) \
    macro(null) \
    macro(package) \
    macro(private) \
    macro(protected) \
    macro(public) \
    macro(return) \
    macro(static) \
    macro(super) \
    macro(switch) \
    macro(this) \
    macro(throw) \
    macro(true) \
    macro(try) \
    macro(typeof) \
    macro(undefined) \
    macro(var) \
    macro(void) \
    macro(while) \
    macro(with) \
    macro(yield)

#define JSC_COMMON_PRIVATE_IDENTIFIERS_EACH_WELL_KNOWN_SYMBOL(macro) \
    macro(hasInstance) \
    macro(isConcatSpreadable) \
    macro(iterator) \
    macro(match) \
    macro(replace) \
    macro(search) \
    macro(species) \
    macro(split) \
    macro(toPrimitive) \
    macro(toStringTag) \
    macro(unscopables)

namespace JSC {
    
    class BuiltinNames;
    
    class CommonIdentifiers {
        WTF_MAKE_NONCOPYABLE(CommonIdentifiers); WTF_MAKE_FAST_ALLOCATED;
    private:
        CommonIdentifiers(VM*);
        ~CommonIdentifiers();
        friend class VM;
        
    public:
        const BuiltinNames& builtinNames() const { return *m_builtinNames; }
        const Identifier nullIdentifier;
        const Identifier emptyIdentifier;
        const Identifier underscoreProto;
        const Identifier thisIdentifier;
        const Identifier useStrictIdentifier;
        const Identifier timesIdentifier;
    private:
        std::unique_ptr<BuiltinNames> m_builtinNames;

    public:
        
#define JSC_IDENTIFIER_DECLARE_KEYWORD_NAME_GLOBAL(name) const Identifier name##Keyword;
        JSC_COMMON_IDENTIFIERS_EACH_KEYWORD(JSC_IDENTIFIER_DECLARE_KEYWORD_NAME_GLOBAL)
#undef JSC_IDENTIFIER_DECLARE_KEYWORD_NAME_GLOBAL
        
#define JSC_IDENTIFIER_DECLARE_PROPERTY_NAME_GLOBAL(name) const Identifier name;
        JSC_COMMON_IDENTIFIERS_EACH_PROPERTY_NAME(JSC_IDENTIFIER_DECLARE_PROPERTY_NAME_GLOBAL)
#undef JSC_IDENTIFIER_DECLARE_PROPERTY_NAME_GLOBAL

#define JSC_IDENTIFIER_DECLARE_PRIVATE_WELL_KNOWN_SYMBOL_GLOBAL(name) const Identifier name##Symbol;
        JSC_COMMON_PRIVATE_IDENTIFIERS_EACH_WELL_KNOWN_SYMBOL(JSC_IDENTIFIER_DECLARE_PRIVATE_WELL_KNOWN_SYMBOL_GLOBAL)
#undef JSC_IDENTIFIER_DECLARE_PRIVATE_WELL_KNOWN_SYMBOL_GLOBAL

        bool isPrivateName(SymbolImpl& uid) const;
        bool isPrivateName(UniquedStringImpl& uid) const;
        bool isPrivateName(const Identifier&) const;

        const Identifier* lookUpPrivateName(const Identifier&) const;
        Identifier lookUpPublicName(const Identifier&) const;

        // Callers of this method should make sure that identifiers given to this method 
        // survive the lifetime of CommonIdentifiers and related VM.
        JS_EXPORT_PRIVATE void appendExternalName(const Identifier& publicName, const Identifier& privateName);
    };

} // namespace JSC
