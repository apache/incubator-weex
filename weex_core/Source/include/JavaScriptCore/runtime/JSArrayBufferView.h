/*
 * Copyright (C) 2013, 2016 Apple Inc. All rights reserved.
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

#pragma once

#include "AuxiliaryBarrier.h"
#include "JSObject.h"

namespace JSC {

class LLIntOffsetsExtractor;

// This class serves two purposes:
//
// 1) It provides those parts of JSGenericTypedArrayView that don't depend
//    on template parameters.
//
// 2) It represents the DOM/WebCore-visible "JSArrayBufferView" type, which
//    C++ code uses when it wants to pass around a view of an array buffer
//    without concern for the actual type of the view.
//
// These two roles are quite different. (1) is just a matter of optimizing
// compile and link times by having as much code and data as possible not
// be subject to template specialization. (2) is *almost* a matter of
// semantics; indeed at the very least it is a matter of obeying a contract
// that we have with WebCore right now.
//
// One convenient thing that saves us from too much crazy is that
// ArrayBufferView is not instantiable.

// Typed array views have different modes depending on how big they are and
// whether the user has done anything that requires a separate backing
// buffer or the DOM-specified neutering capabilities.
enum TypedArrayMode : uint32_t {
    // Small and fast typed array. B is unused, V points to a vector
    // allocated in copied space, and M = FastTypedArray. V's liveness is
    // determined entirely by the view's liveness.
    FastTypedArray,
    
    // A large typed array that still attempts not to waste too much
    // memory. B is initialized to point to a slot that could hold a
    // buffer pointer, V points to a vector allocated using fastCalloc(),
    // and M = OversizeTypedArray. V's liveness is determined entirely by
    // the view's liveness, and the view will add a finalizer to delete V.
    OversizeTypedArray,
    
    // A typed array that was used in some crazy way. B's IndexingHeader
    // is hijacked to contain a reference to the native array buffer. The
    // native typed array view points back to the JS view. V points to a
    // vector allocated using who-knows-what, and M = WastefulTypedArray.
    // The view does not own the vector.
    WastefulTypedArray,
    
    // A data view. B is unused, V points to a vector allocated using who-
    // knows-what, and M = DataViewMode. The view does not own the vector.
    // There is an extra field (in JSDataView) that points to the
    // ArrayBuffer.
    DataViewMode
};

inline bool hasArrayBuffer(TypedArrayMode mode)
{
    return mode >= WastefulTypedArray;
}

// When WebCore uses a JSArrayBufferView, it expects to be able to get the native
// ArrayBuffer and little else. This requires slowing down and wasting memory,
// and then accessing things via the Butterfly. When JS uses a JSArrayBufferView
// it is always via the usual methods in the MethodTable, so this class's
// implementation of those has no need to even exist - we could at any time sink
// code into JSGenericTypedArrayView if it was convenient.

class JSArrayBufferView : public JSNonFinalObject {
public:
    typedef JSNonFinalObject Base;
    static const unsigned fastSizeLimit = 1000;
    
    static size_t sizeOf(uint32_t length, uint32_t elementSize)
    {
        return (length * elementSize + sizeof(EncodedJSValue) - 1)
            & ~(sizeof(EncodedJSValue) - 1);
    }

    static size_t allocationSize(size_t inlineCapacity)
    {
        ASSERT_UNUSED(inlineCapacity, !inlineCapacity);
        return sizeof(JSArrayBufferView);
    }
        
protected:
    class ConstructionContext {
        WTF_MAKE_NONCOPYABLE(ConstructionContext);
        
    public:
        enum InitializationMode { ZeroFill, DontInitialize };
        
        JS_EXPORT_PRIVATE ConstructionContext(VM&, Structure*, uint32_t length, uint32_t elementSize, InitializationMode = ZeroFill);
        
        // This is only for constructing fast typed arrays. It's used by the JIT's slow path.
        ConstructionContext(Structure*, uint32_t length, void* vector);
        
        JS_EXPORT_PRIVATE ConstructionContext(
            VM&, Structure*, RefPtr<ArrayBuffer>&&,
            unsigned byteOffset, unsigned length);
        
        enum DataViewTag { DataView };
        ConstructionContext(
            Structure*, RefPtr<ArrayBuffer>&&,
            unsigned byteOffset, unsigned length, DataViewTag);
        
        bool operator!() const { return !m_structure; }
        
        Structure* structure() const { return m_structure; }
        void* vector() const { return m_vector; }
        uint32_t length() const { return m_length; }
        TypedArrayMode mode() const { return m_mode; }
        Butterfly* butterfly() const { return m_butterfly; }
        
    private:
        Structure* m_structure;
        void* m_vector;
        uint32_t m_length;
        TypedArrayMode m_mode;
        Butterfly* m_butterfly;
    };
    
    JS_EXPORT_PRIVATE JSArrayBufferView(VM&, ConstructionContext&);
    JS_EXPORT_PRIVATE void finishCreation(VM&);
    
    static bool put(JSCell*, ExecState*, PropertyName, JSValue, PutPropertySlot&);

    static void visitChildren(JSCell*, SlotVisitor&);
    
public:
    TypedArrayMode mode() const { return m_mode; }
    bool hasArrayBuffer() const { return JSC::hasArrayBuffer(mode()); }
    
    bool isShared();
    JS_EXPORT_PRIVATE ArrayBuffer* unsharedBuffer();
    ArrayBuffer* possiblySharedBuffer();
    JSArrayBuffer* unsharedJSBuffer(ExecState* exec);
    JSArrayBuffer* possiblySharedJSBuffer(ExecState* exec);
    RefPtr<ArrayBufferView> unsharedImpl();
    RefPtr<ArrayBufferView> possiblySharedImpl();
    bool isNeutered() { return hasArrayBuffer() && !vector(); }
    void neuter();
    
    void* vector() const { return m_vector.get(); }
    
    unsigned byteOffset();
    unsigned length() const { return m_length; }

    DECLARE_EXPORT_INFO;
    
    static ptrdiff_t offsetOfVector() { return OBJECT_OFFSETOF(JSArrayBufferView, m_vector); }
    static ptrdiff_t offsetOfLength() { return OBJECT_OFFSETOF(JSArrayBufferView, m_length); }
    static ptrdiff_t offsetOfMode() { return OBJECT_OFFSETOF(JSArrayBufferView, m_mode); }
    
    static RefPtr<ArrayBufferView> toWrapped(VM&, JSValue);

private:
    static void finalize(JSCell*);

protected:
    friend class LLIntOffsetsExtractor;

    ArrayBuffer* existingBufferInButterfly();

    static String toStringName(const JSObject*, ExecState*);

    AuxiliaryBarrier<void*> m_vector;
    uint32_t m_length;
    TypedArrayMode m_mode;
};

} // namespace JSC

namespace WTF {

JS_EXPORT_PRIVATE void printInternal(PrintStream&, JSC::TypedArrayMode);

} // namespace WTF
