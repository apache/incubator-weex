/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <Block.h>
#include <wtf/Assertions.h>

namespace WTF {

extern "C" void* _NSConcreteMallocBlock[32];

template<typename> class BlockPtr;

template<typename R, typename... Args>
class BlockPtr<R (Args...)> {
public:
    using BlockType = R (^)(Args...);

    template<typename F>
    static BlockPtr fromCallable(F function)
    {
        struct Descriptor {
            uintptr_t reserved;
            uintptr_t size;
            void (*copy)(void *dst, const void *src);
            void (*dispose)(const void *);
        };

        struct Block {
            void* isa;
            int32_t flags;
            int32_t reserved;
            R (*invoke)(void *, Args...);
            const struct Descriptor* descriptor;
            F f;
        };

        static const Descriptor descriptor {
            0,
            sizeof(Block),

            // We keep the copy function null - the block is already on the heap
            // so it should never be copied.
            nullptr,

            [](const void* ptr) {
                static_cast<Block*>(const_cast<void*>(ptr))->f.~F();
            }
        };

        Block* block = static_cast<Block*>(malloc(sizeof(Block)));
        block->isa = _NSConcreteMallocBlock;

        enum {
            BLOCK_NEEDS_FREE = (1 << 24),
            BLOCK_HAS_COPY_DISPOSE = (1 << 25),
        };
        const unsigned retainCount = 1;

        block->flags = BLOCK_HAS_COPY_DISPOSE | BLOCK_NEEDS_FREE | (retainCount << 1);
        block->reserved = 0;
        block->invoke = [](void *ptr, Args... args) -> R {
            return static_cast<Block*>(ptr)->f(std::forward<Args>(args)...);
        };
        block->descriptor = &descriptor;

        new (&block->f) F { std::move(function) };

        BlockPtr blockPtr;
        blockPtr.m_block = reinterpret_cast<BlockType>(block);

        return blockPtr;
    }

    BlockPtr()
        : m_block(nullptr)
    {
    }

    BlockPtr(BlockType block)
        : m_block(Block_copy(block))
    {
    }

    BlockPtr(const BlockPtr& other)
        : m_block(Block_copy(other.m_block))
    {
    }
    
    BlockPtr(BlockPtr&& other)
        : m_block(std::exchange(other.m_block, nullptr))
    {
    }
    
    ~BlockPtr()
    {
        Block_release(m_block);
    }

    BlockPtr& operator=(const BlockPtr& other)
    {
        if (this != &other) {
            Block_release(m_block);
            m_block = Block_copy(other.m_block);
        }
        
        return *this;
    }

    BlockPtr& operator=(BlockPtr&& other)
    {
        ASSERT(this != &other);

        Block_release(m_block);
        m_block = std::exchange(other.m_block, nullptr);

        return *this;
    }

    BlockType get() const { return m_block; }

    explicit operator bool() const { return m_block; }
    bool operator!() const { return !m_block; }

    R operator()(Args... arguments) const
    {
        ASSERT(m_block);
        
        return m_block(std::forward<Args>(arguments)...);
    }

private:
    BlockType m_block;
};

template<typename R, typename... Args>
inline BlockPtr<R (Args...)> makeBlockPtr(R (^block)(Args...))
{
    return BlockPtr<R (Args...)>(block);
}

}

using WTF::BlockPtr;
using WTF::makeBlockPtr;

