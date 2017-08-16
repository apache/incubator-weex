/*
 * Copyright (C) 2013, 2015 Apple Inc. All rights reserved.
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
#include "BinarySwitch.h"

#if ENABLE(JIT)

#include "JSCInlines.h"
#include <wtf/ListDump.h>

namespace JSC {

static const bool verbose = false;

static unsigned globalCounter; // We use a different seed every time we are invoked.

BinarySwitch::BinarySwitch(GPRReg value, const Vector<int64_t>& cases, Type type)
    : m_value(value)
    , m_weakRandom(globalCounter++)
    , m_index(0)
    , m_caseIndex(UINT_MAX)
    , m_type(type)
{
    if (cases.isEmpty())
        return;

    if (verbose)
        dataLog("Original cases: ", listDump(cases), "\n");
    
    for (unsigned i = 0; i < cases.size(); ++i)
        m_cases.append(Case(cases[i], i));
    
    std::sort(m_cases.begin(), m_cases.end());

    if (verbose)
        dataLog("Sorted cases: ", listDump(m_cases), "\n");
    
    for (unsigned i = 1; i < m_cases.size(); ++i)
        RELEASE_ASSERT(m_cases[i - 1] < m_cases[i]);
    
    build(0, false, m_cases.size());
}

BinarySwitch::~BinarySwitch()
{
}

bool BinarySwitch::advance(MacroAssembler& jit)
{
    if (m_cases.isEmpty()) {
        m_fallThrough.append(jit.jump());
        return false;
    }
    
    if (m_index == m_branches.size()) {
        RELEASE_ASSERT(m_jumpStack.isEmpty());
        return false;
    }
    
    for (;;) {
        const BranchCode& code = m_branches[m_index++];
        switch (code.kind) {
        case NotEqualToFallThrough:
            switch (m_type) {
            case Int32:
                m_fallThrough.append(jit.branch32(
                    MacroAssembler::NotEqual, m_value,
                    MacroAssembler::Imm32(static_cast<int32_t>(m_cases[code.index].value))));
                break;
            case IntPtr:
                m_fallThrough.append(jit.branchPtr(
                    MacroAssembler::NotEqual, m_value,
                    MacroAssembler::ImmPtr(bitwise_cast<const void*>(static_cast<intptr_t>(m_cases[code.index].value)))));
                break;
            }
            break;
        case NotEqualToPush:
            switch (m_type) {
            case Int32:
                m_jumpStack.append(jit.branch32(
                    MacroAssembler::NotEqual, m_value,
                    MacroAssembler::Imm32(static_cast<int32_t>(m_cases[code.index].value))));
                break;
            case IntPtr:
                m_jumpStack.append(jit.branchPtr(
                    MacroAssembler::NotEqual, m_value,
                    MacroAssembler::ImmPtr(bitwise_cast<const void*>(static_cast<intptr_t>(m_cases[code.index].value)))));
                break;
            }
            break;
        case LessThanToPush:
            switch (m_type) {
            case Int32:
                m_jumpStack.append(jit.branch32(
                    MacroAssembler::LessThan, m_value,
                    MacroAssembler::Imm32(static_cast<int32_t>(m_cases[code.index].value))));
                break;
            case IntPtr:
                m_jumpStack.append(jit.branchPtr(
                    MacroAssembler::LessThan, m_value,
                    MacroAssembler::ImmPtr(bitwise_cast<const void*>(static_cast<intptr_t>(m_cases[code.index].value)))));
                break;
            }
            break;
        case Pop:
            m_jumpStack.takeLast().link(&jit);
            break;
        case ExecuteCase:
            m_caseIndex = code.index;
            return true;
        }
    }
}

void BinarySwitch::build(unsigned start, bool hardStart, unsigned end)
{
    if (verbose)
        dataLog("Building with start = ", start, ", hardStart = ", hardStart, ", end = ", end, "\n");

    auto append = [&] (const BranchCode& code) {
        if (verbose)
            dataLog("==> ", code, "\n");
        m_branches.append(code);
    };
    
    unsigned size = end - start;
    
    RELEASE_ASSERT(size);
    
    // This code uses some random numbers to keep things balanced. It's important to keep in mind
    // that this does not improve average-case throughput under the assumption that all cases fire
    // with equal probability. It just ensures that there will not be some switch structure that
    // when combined with some input will always produce pathologically good or pathologically bad
    // performance.
    
    const unsigned leafThreshold = 3;
    
    if (size <= leafThreshold) {
        if (verbose)
            dataLog("It's a leaf.\n");
        
        // It turns out that for exactly three cases or less, it's better to just compare each
        // case individually. This saves 1/6 of a branch on average, and up to 1/3 of a branch in
        // extreme cases where the divide-and-conquer bottoms out in a lot of 3-case subswitches.
        //
        // This assumes that we care about the cost of hitting some case more than we care about
        // bottoming out in a default case. I believe that in most places where we use switch
        // statements, we are more likely to hit one of the cases than we are to fall through to
        // default. Intuitively, if we wanted to improve the performance of default, we would
        // reduce the value of leafThreshold to 2 or even to 1. See below for a deeper discussion.
        
        bool allConsecutive = false;
        
        if ((hardStart || (start && m_cases[start - 1].value == m_cases[start].value - 1))
            && start + size < m_cases.size()
            && m_cases[start + size - 1].value == m_cases[start + size].value - 1) {
            allConsecutive = true;
            for (unsigned i = 0; i < size - 1; ++i) {
                if (m_cases[start + i].value + 1 != m_cases[start + i + 1].value) {
                    allConsecutive = false;
                    break;
                }
            }
        }

        if (verbose)
            dataLog("allConsecutive = ", allConsecutive, "\n");
        
        Vector<unsigned, 3> localCaseIndices;
        for (unsigned i = 0; i < size; ++i)
            localCaseIndices.append(start + i);
        
        std::random_shuffle(
            localCaseIndices.begin(), localCaseIndices.end(),
            [this] (unsigned n) {
                // We use modulo to get a random number in the range we want fully knowing that
                // this introduces a tiny amount of bias, but we're fine with such tiny bias.
                return m_weakRandom.getUint32() % n;
            });
        
        for (unsigned i = 0; i < size - 1; ++i) {
            append(BranchCode(NotEqualToPush, localCaseIndices[i]));
            append(BranchCode(ExecuteCase, localCaseIndices[i]));
            append(BranchCode(Pop));
        }
        
        if (!allConsecutive)
            append(BranchCode(NotEqualToFallThrough, localCaseIndices.last()));
        
        append(BranchCode(ExecuteCase, localCaseIndices.last()));
        return;
    }

    if (verbose)
        dataLog("It's not a leaf.\n");
        
    // There are two different strategies we could consider here:
    //
    // Isolate median and split: pick a median and check if the comparison value is equal to it;
    // if so, execute the median case. Otherwise check if the value is less than the median, and
    // recurse left or right based on this. This has two subvariants: we could either first test
    // equality for the median and then do the less-than, or we could first do the less-than and
    // then check equality on the not-less-than path.
    //
    // Ignore median and split: do a less-than comparison on a value that splits the cases in two
    // equal-sized halves. Recurse left or right based on the comparison. Do not test for equality
    // against the median (or anything else); let the recursion handle those equality comparisons
    // once we bottom out in a list that case 3 cases or less (see above).
    //
    // I'll refer to these strategies as Isolate and Ignore. I initially believed that Isolate
    // would be faster since it leads to less branching for some lucky cases. It turns out that
    // Isolate is almost a total fail in the average, assuming all cases are equally likely. How
    // bad Isolate is depends on whether you believe that doing two consecutive branches based on
    // the same comparison is cheaper than doing the compare/branches separately. This is
    // difficult to evaluate. For small immediates that aren't blinded, we just care about
    // avoiding a second compare instruction. For large immediates or when blinding is in play, we
    // also care about the instructions used to materialize the immediate a second time. Isolate
    // can help with both costs since it involves first doing a < compare+branch on some value,
    // followed by a == compare+branch on the same exact value (or vice-versa). Ignore will do a <
    // compare+branch on some value, and then the == compare+branch on that same value will happen
    // much later.
    //
    // To evaluate these costs, I wrote the recurrence relation for Isolate and Ignore, assuming
    // that ComparisonCost is the cost of a compare+branch and ChainedComparisonCost is the cost
    // of a compare+branch on some value that you've just done another compare+branch for. These
    // recurrence relations compute the total cost incurred if you executed the switch statement
    // on each matching value. So the average cost of hitting some case can be computed as
    // Isolate[n]/n or Ignore[n]/n, respectively for the two relations.
    //
    // Isolate[1] = ComparisonCost
    // Isolate[2] = (2 + 1) * ComparisonCost
    // Isolate[3] = (3 + 2 + 1) * ComparisonCost
    // Isolate[n_] := With[
    //     {medianIndex = Floor[n/2] + If[EvenQ[n], RandomInteger[], 1]},
    //     ComparisonCost + ChainedComparisonCost +
    //     (ComparisonCost * (medianIndex - 1) + Isolate[medianIndex - 1]) +
    //     (2 * ComparisonCost * (n - medianIndex) + Isolate[n - medianIndex])]
    //
    // Ignore[1] = ComparisonCost
    // Ignore[2] = (2 + 1) * ComparisonCost
    // Ignore[3] = (3 + 2 + 1) * ComparisonCost
    // Ignore[n_] := With[
    //     {medianIndex = If[EvenQ[n], n/2, Floor[n/2] + RandomInteger[]]},
    //     (medianIndex * ComparisonCost + Ignore[medianIndex]) +
    //     ((n - medianIndex) * ComparisonCost + Ignore[n - medianIndex])]
    //
    // This does not account for the average cost of hitting the default case. See further below
    // for a discussion of that.
    //
    // It turns out that for ComparisonCost = 1 and ChainedComparisonCost = 1, Ignore is always
    // better than Isolate. If we assume that ChainedComparisonCost = 0, then Isolate wins for
    // switch statements that have 20 cases or fewer, though the margin of victory is never large
    // - it might sometimes save an average of 0.3 ComparisonCost. For larger switch statements,
    // we see divergence between the two with Ignore winning. This is of course rather
    // unrealistic since the chained comparison is never free. For ChainedComparisonCost = 0.5, we
    // see Isolate winning for 10 cases or fewer, by maybe 0.2 ComparisonCost. Again we see
    // divergence for large switches with Ignore winning, for example if a switch statement has
    // 100 cases then Ignore saves one branch on average.
    //
    // Our current JIT backends don't provide for optimization for chained comparisons, except for
    // reducing the code for materializing the immediate if the immediates are large or blinding
    // comes into play. Probably our JIT backends live somewhere north of
    // ChainedComparisonCost = 0.5.
    //
    // This implies that using the Ignore strategy is likely better. If we wanted to incorporate
    // the Isolate strategy, we'd want to determine the switch size threshold at which the two
    // cross over and then use Isolate for switches that are smaller than that size.
    //
    // The average cost of hitting the default case is similar, but involves a different cost for
    // the base cases: you have to assume that you will always fail each branch. For the Ignore
    // strategy we would get this recurrence relation; the same kind of thing happens to the
    // Isolate strategy:
    //
    // Ignore[1] = ComparisonCost
    // Ignore[2] = (2 + 2) * ComparisonCost
    // Ignore[3] = (3 + 3 + 3) * ComparisonCost
    // Ignore[n_] := With[
    //     {medianIndex = If[EvenQ[n], n/2, Floor[n/2] + RandomInteger[]]},
    //     (medianIndex * ComparisonCost + Ignore[medianIndex]) +
    //     ((n - medianIndex) * ComparisonCost + Ignore[n - medianIndex])]
    //
    // This means that if we cared about the default case more, we would likely reduce
    // leafThreshold. Reducing it to 2 would reduce the average cost of the default case by 1/3
    // in the most extreme cases (num switch cases = 3, 6, 12, 24, ...). But it would also
    // increase the average cost of taking one of the non-default cases by 1/3. Typically the
    // difference is 1/6 in either direction. This makes it a very simple trade-off: if we believe
    // that the default case is more important then we would want leafThreshold to be 2, and the
    // default case would become 1/6 faster on average. But we believe that most switch statements
    // are more likely to take one of the cases than the default, so we use leafThreshold = 3
    // and get a 1/6 speed-up on average for taking an explicit case.
        
    unsigned medianIndex = (start + end) / 2;

    if (verbose)
        dataLog("medianIndex = ", medianIndex, "\n");

    // We want medianIndex to point to the thing we will do a less-than compare against. We want
    // this less-than compare to split the current sublist into equal-sized sublists, or
    // nearly-equal-sized with some randomness if we're in the odd case. With the above
    // calculation, in the odd case we will have medianIndex pointing at either the element we
    // want or the element to the left of the one we want. Consider the case of five elements:
    //
    //     0 1 2 3 4
    //
    // start will be 0, end will be 5. The average is 2.5, which rounds down to 2. If we do
    // value < 2, then we will split the list into 2 elements on the left and three on the right.
    // That's pretty good, but in this odd case we'd like to at random choose 3 instead to ensure
    // that we don't become unbalanced on the right. This does not improve throughput since one
    // side will always get shafted, and that side might still be odd, in which case it will also
    // have two sides and one of them will get shafted - and so on. We just want to avoid
    // deterministic pathologies.
    //
    // In the even case, we will always end up pointing at the element we want:
    //
    //     0 1 2 3
    //
    // start will be 0, end will be 4. So, the average is 2, which is what we'd like.
    if (size & 1) {
        RELEASE_ASSERT(medianIndex - start + 1 == end - medianIndex);
        medianIndex += m_weakRandom.getUint32() & 1;
    } else
        RELEASE_ASSERT(medianIndex - start == end - medianIndex);
        
    RELEASE_ASSERT(medianIndex > start);
    RELEASE_ASSERT(medianIndex + 1 < end);
        
    if (verbose)
        dataLog("fixed medianIndex = ", medianIndex, "\n");

    append(BranchCode(LessThanToPush, medianIndex));
    build(medianIndex, true, end);
    append(BranchCode(Pop));
    build(start, hardStart, medianIndex);
}

void BinarySwitch::Case::dump(PrintStream& out) const
{
    out.print("<value: " , value, ", index: ", index, ">");
}

void BinarySwitch::BranchCode::dump(PrintStream& out) const
{
    switch (kind) {
    case NotEqualToFallThrough:
        out.print("NotEqualToFallThrough");
        break;
    case NotEqualToPush:
        out.print("NotEqualToPush");
        break;
    case LessThanToPush:
        out.print("LessThanToPush");
        break;
    case Pop:
        out.print("Pop");
        break;
    case ExecuteCase:
        out.print("ExecuteCase");
        break;
    }

    if (index != UINT_MAX)
        out.print("(", index, ")");
}

} // namespace JSC

#endif // ENABLE(JIT)

