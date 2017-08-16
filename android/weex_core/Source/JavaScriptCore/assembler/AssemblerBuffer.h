/*
 * Copyright (C) 2008, 2012, 2014 Apple Inc. All rights reserved.
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

#if ENABLE(ASSEMBLER)

#include "ExecutableAllocator.h"
#include "JITCompilationEffort.h"
#include "stdint.h"
#include <string.h>
#include <wtf/Assertions.h>
#include <wtf/FastMalloc.h>
#include <wtf/StdLibExtras.h>

namespace JSC {

    struct AssemblerLabel {
        AssemblerLabel()
            : m_offset(std::numeric_limits<uint32_t>::max())
        {
        }

        explicit AssemblerLabel(uint32_t offset)
            : m_offset(offset)
        {
        }

        bool isSet() const { return (m_offset != std::numeric_limits<uint32_t>::max()); }

        AssemblerLabel labelAtOffset(int offset) const
        {
            return AssemblerLabel(m_offset + offset);
        }

        bool operator==(const AssemblerLabel& other) const { return m_offset == other.m_offset; }

        uint32_t m_offset;
    };

    class AssemblerData {
        WTF_MAKE_NONCOPYABLE(AssemblerData);
        static const size_t InlineCapacity = 128;
    public:
        AssemblerData()
            : m_buffer(m_inlineBuffer)
            , m_capacity(InlineCapacity)
        {
        }

        AssemblerData(size_t initialCapacity)
        {
            if (initialCapacity <= InlineCapacity) {
                m_capacity = InlineCapacity;
                m_buffer = m_inlineBuffer;
            } else {
                m_capacity = initialCapacity;
                m_buffer = static_cast<char*>(fastMalloc(m_capacity));
            }
        }

        AssemblerData(AssemblerData&& other)
        {
            if (other.isInlineBuffer()) {
                ASSERT(other.m_capacity == InlineCapacity);
                memcpy(m_inlineBuffer, other.m_inlineBuffer, InlineCapacity);
                m_buffer = m_inlineBuffer;
            } else
                m_buffer = other.m_buffer;
            m_capacity = other.m_capacity;

            other.m_buffer = nullptr;
            other.m_capacity = 0;
        }

        AssemblerData& operator=(AssemblerData&& other)
        {
            if (m_buffer && !isInlineBuffer())
                fastFree(m_buffer);

            if (other.isInlineBuffer()) {
                ASSERT(other.m_capacity == InlineCapacity);
                memcpy(m_inlineBuffer, other.m_inlineBuffer, InlineCapacity);
                m_buffer = m_inlineBuffer;
            } else
                m_buffer = other.m_buffer;
            m_capacity = other.m_capacity;

            other.m_buffer = nullptr;
            other.m_capacity = 0;
            return *this;
        }

        ~AssemblerData()
        {
            if (m_buffer && !isInlineBuffer())
                fastFree(m_buffer);
        }

        char* buffer() const { return m_buffer; }

        unsigned capacity() const { return m_capacity; }

        void grow(unsigned extraCapacity = 0)
        {
            m_capacity = m_capacity + m_capacity / 2 + extraCapacity;
            if (isInlineBuffer()) {
                m_buffer = static_cast<char*>(fastMalloc(m_capacity));
                memcpy(m_buffer, m_inlineBuffer, InlineCapacity);
            } else
                m_buffer = static_cast<char*>(fastRealloc(m_buffer, m_capacity));
        }

    private:
        bool isInlineBuffer() const { return m_buffer == m_inlineBuffer; }
        char* m_buffer;
        char m_inlineBuffer[InlineCapacity];
        unsigned m_capacity;
    };

    class AssemblerBuffer {
    public:
        AssemblerBuffer()
            : m_storage()
            , m_index(0)
        {
        }

        bool isAvailable(unsigned space)
        {
            return m_index + space <= m_storage.capacity();
        }

        void ensureSpace(unsigned space)
        {
            while (!isAvailable(space))
                outOfLineGrow();
        }

        bool isAligned(int alignment) const
        {
            return !(m_index & (alignment - 1));
        }

        void putByteUnchecked(int8_t value) { putIntegralUnchecked(value); }
        void putByte(int8_t value) { putIntegral(value); }
        void putShortUnchecked(int16_t value) { putIntegralUnchecked(value); }
        void putShort(int16_t value) { putIntegral(value); }
        void putIntUnchecked(int32_t value) { putIntegralUnchecked(value); }
        void putInt(int32_t value) { putIntegral(value); }
        void putInt64Unchecked(int64_t value) { putIntegralUnchecked(value); }
        void putInt64(int64_t value) { putIntegral(value); }

        void* data() const
        {
            return m_storage.buffer();
        }

        size_t codeSize() const
        {
            return m_index;
        }

        void setCodeSize(size_t index)
        {
            // Warning: Only use this if you know exactly what you are doing.
            // For example, say you want 40 bytes of nops, it's ok to grow
            // and then fill 40 bytes of nops using bigger instructions.
            m_index = index;
            ASSERT(m_index <= m_storage.capacity());
        }

        AssemblerLabel label() const
        {
            return AssemblerLabel(m_index);
        }

        unsigned debugOffset() { return m_index; }

        AssemblerData&& releaseAssemblerData() { return WTFMove(m_storage); }

        // LocalWriter is a trick to keep the storage buffer and the index
        // in memory while issuing multiple Stores.
        // It is created in a block scope and its attribute can stay live
        // between writes.
        //
        // LocalWriter *CANNOT* be mixed with other types of access to AssemblerBuffer.
        // AssemblerBuffer cannot be used until its LocalWriter goes out of scope.
        class LocalWriter {
        public:
            LocalWriter(AssemblerBuffer& buffer, unsigned requiredSpace)
                : m_buffer(buffer)
            {
                buffer.ensureSpace(requiredSpace);
                m_storageBuffer = buffer.m_storage.buffer();
                m_index = buffer.m_index;
#if !defined(NDEBUG)
                m_initialIndex = m_index;
                m_requiredSpace = requiredSpace;
#endif
            }

            ~LocalWriter()
            {
                ASSERT(m_index - m_initialIndex <= m_requiredSpace);
                ASSERT(m_buffer.m_index == m_initialIndex);
                ASSERT(m_storageBuffer == m_buffer.m_storage.buffer());
                m_buffer.m_index = m_index;
            }

            void putByteUnchecked(int8_t value) { putIntegralUnchecked(value); }
            void putShortUnchecked(int16_t value) { putIntegralUnchecked(value); }
            void putIntUnchecked(int32_t value) { putIntegralUnchecked(value); }
            void putInt64Unchecked(int64_t value) { putIntegralUnchecked(value); }
        private:
            template<typename IntegralType>
            void putIntegralUnchecked(IntegralType value)
            {
                ASSERT(m_index + sizeof(IntegralType) <= m_buffer.m_storage.capacity());
                *reinterpret_cast_ptr<IntegralType*>(m_storageBuffer + m_index) = value;
                m_index += sizeof(IntegralType);
            }
            AssemblerBuffer& m_buffer;
            char* m_storageBuffer;
            unsigned m_index;
#if !defined(NDEBUG)
            unsigned m_initialIndex;
            unsigned m_requiredSpace;
#endif
        };

    protected:
        template<typename IntegralType>
        void putIntegral(IntegralType value)
        {
            unsigned nextIndex = m_index + sizeof(IntegralType);
            if (UNLIKELY(nextIndex > m_storage.capacity()))
                outOfLineGrow();
            ASSERT(isAvailable(sizeof(IntegralType)));
            *reinterpret_cast_ptr<IntegralType*>(m_storage.buffer() + m_index) = value;
            m_index = nextIndex;
        }

        template<typename IntegralType>
        void putIntegralUnchecked(IntegralType value)
        {
            ASSERT(isAvailable(sizeof(IntegralType)));
            *reinterpret_cast_ptr<IntegralType*>(m_storage.buffer() + m_index) = value;
            m_index += sizeof(IntegralType);
        }

        void append(const char* data, int size)
        {
            if (!isAvailable(size))
                grow(size);

            memcpy(m_storage.buffer() + m_index, data, size);
            m_index += size;
        }

        void grow(int extraCapacity = 0)
        {
            m_storage.grow(extraCapacity);
        }

    private:
        NEVER_INLINE void outOfLineGrow()
        {
            m_storage.grow();
        }

        friend LocalWriter;

        AssemblerData m_storage;
        unsigned m_index;
    };

} // namespace JSC

#endif // ENABLE(ASSEMBLER)
