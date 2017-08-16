/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
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

#if ENABLE(B3_JIT)

#include "B3ConstrainedValue.h"
#include "B3Value.h"
#include "B3ValueRep.h"
#include "CCallHelpers.h"
#include "RegisterSet.h"
#include <wtf/SharedTask.h>

namespace JSC { namespace B3 {

class StackmapGenerationParams;

typedef void StackmapGeneratorFunction(CCallHelpers&, const StackmapGenerationParams&);
typedef SharedTask<StackmapGeneratorFunction> StackmapGenerator;

class JS_EXPORT_PRIVATE StackmapValue : public Value {
public:
    static bool accepts(Kind kind)
    {
        // This needs to include opcodes of all subclasses.
        switch (kind.opcode()) {
        case CheckAdd:
        case CheckSub:
        case CheckMul:
        case Check:
        case Patchpoint:
            return true;
        default:
            return false;
        }
    }

    ~StackmapValue();

    // Use this to add children. Note that you could also add children by doing
    // children().append(). That will work fine, but it's not recommended.
    void append(const ConstrainedValue& value)
    {
        append(value.value(), value.rep());
    }

    void append(Value*, const ValueRep&);

    template<typename VectorType>
    void appendVector(const VectorType& vector)
    {
        for (const auto& value : vector)
            append(value);
    }

    // Helper for appending a bunch of values with some ValueRep.
    template<typename VectorType>
    void appendVectorWithRep(const VectorType& vector, const ValueRep& rep)
    {
        for (Value* value : vector)
            append(value, rep);
    }

    // Helper for appending cold any's. This often used by clients to implement OSR.
    template<typename VectorType>
    void appendColdAnys(const VectorType& vector)
    {
        appendVectorWithRep(vector, ValueRep::ColdAny);
    }
    template<typename VectorType>
    void appendLateColdAnys(const VectorType& vector)
    {
        appendVectorWithRep(vector, ValueRep::LateColdAny);
    }

    // This is a helper for something you might do a lot of: append a value that should be constrained
    // to SomeRegister.
    void appendSomeRegister(Value*);

    const Vector<ValueRep>& reps() const { return m_reps; }

    // Stackmaps allow you to specify that the operation may clobber some registers. Clobbering a register
    // means that the operation appears to store a value into the register, but the compiler doesn't
    // assume to know anything about what kind of value might have been stored. In B3's model of
    // execution, registers are read or written at instruction boundaries rather than inside the
    // instructions themselves. A register could be read or written immediately before the instruction
    // executes, or immediately after. Note that at a boundary between instruction A and instruction B we
    // simultaneously look at what A does after it executes and what B does before it executes. This is
    // because when the compiler considers what happens to registers, it views the boundary between two
    // instructions as a kind of atomic point where the late effects of A happen at the same time as the
    // early effects of B.
    //
    // The compiler views a stackmap as a single instruction, even though of course the stackmap may be
    // composed of any number of instructions (if it's a Patchpoint). You can claim that a stackmap value
    // clobbers a set of registers before the stackmap's instruction or after. Clobbering before is called
    // early clobber, while clobbering after is called late clobber.
    //
    // This is quite flexible but it has its limitations. Any register listed as an early clobber will
    // interfere with all uses of the stackmap. Any register listed as a late clobber will interfere with
    // all defs of the stackmap (i.e. the result). This means that it's currently not possible to claim
    // to clobber a register while still allowing that register to be used for both an input and an output
    // of the instruction. It just so happens that B3's sole client (the FTL) currently never wants to
    // convey such a constraint, but it will want it eventually (FIXME:
    // https://bugs.webkit.org/show_bug.cgi?id=151823).
    //
    // Note that a common use case of early clobber sets is to indicate that this is the set of registers
    // that shall not be used for inputs to the value. But B3 supports two different ways of specifying
    // this, the other being LateUse in combination with late clobber (not yet available to stackmaps
    // directly, FIXME: https://bugs.webkit.org/show_bug.cgi?id=151335). A late use makes the use of that
    // value appear to happen after the instruction. This means that a late use cannot use the same
    // register as the result and it cannot use the same register as either early or late clobbered
    // registers. Late uses are usually a better way of saying that a clobbered register cannot be used
    // for an input. Early clobber means that some register(s) interfere with *all* inputs, while LateUse
    // means that some value interferes with whatever is live after the instruction. Below is a list of
    // examples of how the FTL can handle its various kinds of scenarios using a combination of early
    // clobber, late clobber, and late use. These examples are for X86_64, w.l.o.g.
    //
    // Basic ById patchpoint: Early and late clobber of r11. Early clobber prevents any inputs from using
    // r11 since that would mess with the MacroAssembler's assumptions when we
    // AllowMacroScratchRegisterUsage. Late clobber tells B3 that the patchpoint may overwrite r11.
    //
    // ById patchpoint in a try block with some live state: This might throw an exception after already
    // assigning to the result. So, this should LateUse all stackmap values to ensure that the stackmap
    // values don't interfere with the result. Note that we do not LateUse the non-OSR inputs of the ById
    // since LateUse implies that the use is cold: the register allocator will assume that the use is not
    // important for the critical path. Also, early and late clobber of r11.
    //
    // Basic ByIdFlush patchpoint: We could do Flush the same way we did it with LLVM: ignore it and let
    // PolymorphicAccess figure it out. Or, we could add internal clobber support (FIXME:
    // https://bugs.webkit.org/show_bug.cgi?id=151823). Or, we could do it by early clobbering r11, late
    // clobbering all volatile registers, and constraining the result to some register. Or, we could do
    // that but leave the result constrained to SomeRegister, which will cause it to use a callee-save
    // register. Internal clobber support would allow us to use SomeRegister while getting the result into
    // a volatile register.
    //
    // ByIdFlush patchpoint in a try block with some live state: LateUse all for-OSR stackmap values,
    // early clobber of r11 to prevent the other inputs from using r11, and late clobber of all volatile
    // registers to make way for the call. To handle the result, we could do any of what is listed in the
    // previous paragraph.
    //
    // Basic JS call: Force all non-OSR inputs into specific locations (register, stack, whatever).
    // All volatile registers are late-clobbered. The output is constrained to a register as well.
    //
    // JS call in a try block with some live state: LateUse all for-OSR stackmap values, fully constrain
    // all non-OSR inputs and the result, and late clobber all volatile registers.
    //
    // JS tail call: Pass all inputs as a warm variant of Any (FIXME:
    // https://bugs.webkit.org/show_bug.cgi?id=151811).
    //
    // Note that we cannot yet do all of these things because although Air already supports all of these
    // various forms of uses (LateUse and warm unconstrained use), B3 doesn't yet expose all of it. The
    // bugs are:
    // https://bugs.webkit.org/show_bug.cgi?id=151335 (LateUse)
    // https://bugs.webkit.org/show_bug.cgi?id=151811 (warm Any)
    void clobberEarly(const RegisterSet& set)
    {
        m_earlyClobbered.merge(set);
    }

    void clobberLate(const RegisterSet& set)
    {
        m_lateClobbered.merge(set);
    }

    void clobber(const RegisterSet& set)
    {
        clobberEarly(set);
        clobberLate(set);
    }

    RegisterSet& earlyClobbered() { return m_earlyClobbered; }
    RegisterSet& lateClobbered() { return m_lateClobbered; }
    const RegisterSet& earlyClobbered() const { return m_earlyClobbered; }
    const RegisterSet& lateClobbered() const { return m_lateClobbered; }

    void setGenerator(RefPtr<StackmapGenerator> generator)
    {
        m_generator = generator;
    }

    template<typename Functor>
    void setGenerator(const Functor& functor)
    {
        m_generator = createSharedTask<StackmapGeneratorFunction>(functor);
    }

    RefPtr<StackmapGenerator> generator() const { return m_generator; }

    ConstrainedValue constrainedChild(unsigned index) const
    {
        return ConstrainedValue(child(index), index < m_reps.size() ? m_reps[index] : ValueRep::ColdAny);
    }

    void setConstrainedChild(unsigned index, const ConstrainedValue&);
    
    void setConstraint(unsigned index, const ValueRep&);

    class ConstrainedValueCollection {
    public:
        ConstrainedValueCollection(const StackmapValue& value)
            : m_value(value)
        {
        }

        unsigned size() const { return m_value.numChildren(); }
        
        ConstrainedValue at(unsigned index) const { return m_value.constrainedChild(index); }

        ConstrainedValue operator[](unsigned index) const { return at(index); }

        class iterator {
        public:
            iterator()
                : m_collection(nullptr)
                , m_index(0)
            {
            }

            iterator(const ConstrainedValueCollection& collection, unsigned index)
                : m_collection(&collection)
                , m_index(index)
            {
            }

            ConstrainedValue operator*() const
            {
                return m_collection->at(m_index);
            }

            iterator& operator++()
            {
                m_index++;
                return *this;
            }

            bool operator==(const iterator& other) const
            {
                ASSERT(m_collection == other.m_collection);
                return m_index == other.m_index;
            }

            bool operator!=(const iterator& other) const
            {
                return !(*this == other);
            }
            
        private:
            const ConstrainedValueCollection* m_collection;
            unsigned m_index;
        };

        iterator begin() const { return iterator(*this, 0); }
        iterator end() const { return iterator(*this, size()); }

    private:
        const StackmapValue& m_value;
    };

    ConstrainedValueCollection constrainedChildren() const
    {
        return ConstrainedValueCollection(*this);
    }

protected:
    void dumpChildren(CommaPrinter&, PrintStream&) const override;
    void dumpMeta(CommaPrinter&, PrintStream&) const override;

    StackmapValue(CheckedOpcodeTag, Kind, Type, Origin);

private:
    friend class CheckSpecial;
    friend class PatchpointSpecial;
    friend class StackmapGenerationParams;
    friend class StackmapSpecial;
    
    Vector<ValueRep> m_reps;
    RefPtr<StackmapGenerator> m_generator;
    RegisterSet m_earlyClobbered;
    RegisterSet m_lateClobbered;
    RegisterSet m_usedRegisters; // Stackmaps could be further duplicated by Air, but that's unlikely, so we just merge the used registers sets if that were to happen.
};

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)
