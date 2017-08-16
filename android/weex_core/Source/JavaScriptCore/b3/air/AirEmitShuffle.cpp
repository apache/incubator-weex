/*
 * Copyright (C) 2016-2017 Apple Inc. All rights reserved.
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
#include "AirEmitShuffle.h"

#if ENABLE(B3_JIT)

#include "AirCode.h"
#include "AirInstInlines.h"
#include <wtf/GraphNodeWorklist.h>
#include <wtf/ListDump.h>

namespace JSC { namespace B3 { namespace Air {

namespace {

bool verbose = false;

template<typename Functor>
Tmp findPossibleScratch(Code& code, Bank bank, const Functor& functor) {
    for (Reg reg : code.regsInPriorityOrder(bank)) {
        Tmp tmp(reg);
        if (functor(tmp))
            return tmp;
    }
    return Tmp();
}

Tmp findPossibleScratch(Code& code, Bank bank, const Arg& arg1, const Arg& arg2) {
    return findPossibleScratch(
        code, bank,
        [&] (Tmp tmp) -> bool {
            return !arg1.usesTmp(tmp) && !arg2.usesTmp(tmp);
        });
}

// Example: (a => b, b => a, a => c, b => d)
struct Rotate {
    Vector<ShufflePair> loop; // in the example, this is the loop: (a => b, b => a)
    Vector<ShufflePair> fringe; // in the example, these are the associated shifts: (a => c, b => d)
};

} // anonymous namespace

void ShufflePair::dump(PrintStream& out) const
{
    out.print(width(), ":", src(), "=>", dst());
}

Inst createShuffle(Value* origin, const Vector<ShufflePair>& pairs)
{
    Inst result(Shuffle, origin);
    for (const ShufflePair& pair : pairs)
        result.append(pair.src(), pair.dst(), Arg::widthArg(pair.width()));
    return result;
}

Vector<Inst> emitShuffle(
    Code& code, Vector<ShufflePair> pairs, std::array<Arg, 2> scratches, Bank bank,
    Value* origin)
{
    if (verbose) {
        dataLog(
            "Dealing with pairs: ", listDump(pairs), " and scratches ", scratches[0], ", ",
            scratches[1], "\n");
    }
    
    pairs.removeAllMatching(
        [&] (const ShufflePair& pair) -> bool {
            return pair.src() == pair.dst();
        });
    
    // First validate that this is the kind of shuffle that we know how to deal with.
#if !ASSERT_DISABLED
    for (const ShufflePair& pair : pairs) {
        ASSERT(pair.src().isBank(bank));
        ASSERT(pair.dst().isBank(bank));
        ASSERT(pair.dst().isTmp() || pair.dst().isMemory());
    }
#endif // !ASSERT_DISABLED

    // There are two possible kinds of operations that we will do:
    //
    // - Shift. Example: (a => b, b => c). We emit this as "Move b, c; Move a, b". This only requires
    //   scratch registers if there are memory->memory moves. We want to find as many of these as
    //   possible because they are cheaper. Note that shifts can involve the same source mentioned
    //   multiple times. Example: (a => b, a => c, b => d, b => e).
    //
    // - Rotate. Example: (a => b, b => a). We want to emit this as "Swap a, b", but that instruction
    //   may not be available, in which case we may need a scratch register or a scratch memory
    //   location. A gnarlier example is (a => b, b => c, c => a). We can emit this as "Swap b, c;
    //   Swap a, b". Note that swapping has to be careful about differing widths.
    //
    // Note that a rotate can have "fringe". For example, we might have (a => b, b => a, a =>c,
    // b => d). This has a rotate loop (a => b, b => a) and some fringe (a => c, b => d). We treat
    // the whole thing as a single rotate.
    //
    // We will find multiple disjoint such operations. We can execute them in any order.

    // We interpret these as Moves that should be executed backwards. All shifts are keyed by their
    // starting source.
    HashMap<Arg, Vector<ShufflePair>> shifts;

    // We interpret these as Swaps over src()'s that should be executed backwards, i.e. for a list
    // of size 3 we would do "Swap list[1].src(), list[2].src(); Swap list[0].src(), list[1].src()".
    // Note that we actually can't do that if the widths don't match or other bad things happen.
    // But, prior to executing all of that, we need to execute the fringe: the shifts comming off the
    // rotate.
    Vector<Rotate> rotates;

    {
        HashMap<Arg, Vector<ShufflePair>> mapping;
        for (const ShufflePair& pair : pairs)
            mapping.add(pair.src(), Vector<ShufflePair>()).iterator->value.append(pair);

        Vector<ShufflePair> currentPairs;

        while (!mapping.isEmpty()) {
            ASSERT(currentPairs.isEmpty());
            Arg originalSrc = mapping.begin()->key;
            ASSERT(!shifts.contains(originalSrc));
            if (verbose)
                dataLog("Processing from ", originalSrc, "\n");
            
            GraphNodeWorklist<Arg> worklist;
            worklist.push(originalSrc);
            while (Arg src = worklist.pop()) {
                HashMap<Arg, Vector<ShufflePair>>::iterator iter = mapping.find(src);
                if (iter == mapping.end()) {
                    // With a shift it's possible that we previously built the tail of this shift.
                    // See if that's the case now.
                    if (verbose)
                        dataLog("Trying to append shift at ", src, "\n");
                    currentPairs.appendVector(shifts.take(src));
                    continue;
                }
                Vector<ShufflePair> pairs = WTFMove(iter->value);
                mapping.remove(iter);

                for (const ShufflePair& pair : pairs) {
                    currentPairs.append(pair);
                    ASSERT(pair.src() == src);
                    worklist.push(pair.dst());
                }
            }

            ASSERT(currentPairs.size());
            ASSERT(currentPairs[0].src() == originalSrc);

            if (verbose)
                dataLog("currentPairs = ", listDump(currentPairs), "\n");

            bool isRotate = false;
            for (const ShufflePair& pair : currentPairs) {
                if (pair.dst() == originalSrc) {
                    isRotate = true;
                    break;
                }
            }

            if (isRotate) {
                if (verbose)
                    dataLog("It's a rotate.\n");
                Rotate rotate;

                // The common case is that the rotate does not have fringe. The only way to
                // check for this is to examine the whole rotate.
                bool ok;
                if (currentPairs.last().dst() == originalSrc) {
                    ok = true;
                    for (unsigned i = currentPairs.size() - 1; i--;)
                        ok &= currentPairs[i].dst() == currentPairs[i + 1].src();
                } else
                    ok = false;
                
                if (ok)
                    rotate.loop = WTFMove(currentPairs);
                else {
                    // This is the slow path. The rotate has fringe.
                    
                    HashMap<Arg, ShufflePair> dstMapping;
                    for (const ShufflePair& pair : currentPairs)
                        dstMapping.add(pair.dst(), pair);

                    ShufflePair pair = dstMapping.take(originalSrc);
                    for (;;) {
                        rotate.loop.append(pair);

                        auto iter = dstMapping.find(pair.src());
                        if (iter == dstMapping.end())
                            break;
                        pair = iter->value;
                        dstMapping.remove(iter);
                    }

                    rotate.loop.reverse();

                    // Make sure that the fringe appears in the same order as how it appeared in the
                    // currentPairs, since that's the DFS order.
                    for (const ShufflePair& pair : currentPairs) {
                        // But of course we only include it if it's not in the loop.
                        if (dstMapping.contains(pair.dst()))
                            rotate.fringe.append(pair);
                    }
                }
                
                // If the graph search terminates because we returned to the first source, then the
                // pair list has to have a very particular shape.
                for (unsigned i = rotate.loop.size() - 1; i--;)
                    ASSERT(rotate.loop[i].dst() == rotate.loop[i + 1].src());
                rotates.append(WTFMove(rotate));
                currentPairs.resize(0);
            } else {
                if (verbose)
                    dataLog("It's a shift.\n");
                shifts.add(originalSrc, WTFMove(currentPairs));
            }
        }
    }

    if (verbose) {
        dataLog("Shifts:\n");
        for (auto& entry : shifts)
            dataLog("    ", entry.key, ": ", listDump(entry.value), "\n");
        dataLog("Rotates:\n");
        for (auto& rotate : rotates)
            dataLog("    loop = ", listDump(rotate.loop), ", fringe = ", listDump(rotate.fringe), "\n");
    }

    // In the worst case, we need two scratch registers. The way we do this is that the client passes
    // us what scratch registers he happens to have laying around. We will need scratch registers in
    // the following cases:
    //
    // - Shuffle pairs where both src and dst refer to memory.
    // - Rotate when no Swap instruction is available.
    //
    // Lucky for us, we are guaranteed to have extra scratch registers anytime we have a Shift that
    // ends with a register. We search for such a register right now.

    auto moveForWidth = [&] (Width width) -> Opcode {
        switch (width) {
        case Width32:
            return bank == GP ? Move32 : MoveFloat;
        case Width64:
            return bank == GP ? Move : MoveDouble;
        default:
            RELEASE_ASSERT_NOT_REACHED();
        }
    };

    Opcode conservativeMove = moveForWidth(conservativeWidth(bank));

    // We will emit things in reverse. We maintain a list of packs of instructions, and then we emit
    // append them together in reverse (for example the thing at the end of resultPacks is placed
    // first). This is useful because the last thing we emit frees up its destination registers, so
    // it affects how we emit things before it.
    Vector<Vector<Inst>> resultPacks;
    Vector<Inst> result;

    auto commitResult = [&] () {
        resultPacks.append(WTFMove(result));
    };

    auto getScratch = [&] (unsigned index, Tmp possibleScratch) -> Tmp {
        if (scratches[index].isTmp())
            return scratches[index].tmp();

        if (!possibleScratch)
            return Tmp();
        result.append(Inst(conservativeMove, origin, possibleScratch, scratches[index]));
        return possibleScratch;
    };

    auto returnScratch = [&] (unsigned index, Tmp tmp) {
        if (Arg(tmp) != scratches[index])
            result.append(Inst(conservativeMove, origin, scratches[index], tmp));
    };

    auto handleShiftPair = [&] (const ShufflePair& pair, unsigned scratchIndex) {
        Opcode move = moveForWidth(pair.width());
        
        if (!isValidForm(move, pair.src().kind(), pair.dst().kind())) {
            Tmp scratch =
                getScratch(scratchIndex, findPossibleScratch(code, bank, pair.src(), pair.dst()));
            RELEASE_ASSERT(scratch);
            if (isValidForm(move, pair.src().kind(), Arg::Tmp))
                result.append(Inst(moveForWidth(pair.width()), origin, pair.src(), scratch));
            else {
                ASSERT(pair.src().isSomeImm());
                ASSERT(move == Move32);
                result.append(Inst(Move, origin, Arg::bigImm(pair.src().value()), scratch));
            }
            result.append(Inst(moveForWidth(pair.width()), origin, scratch, pair.dst()));
            returnScratch(scratchIndex, scratch);
            return;
        }
        
        result.append(Inst(move, origin, pair.src(), pair.dst()));
    };

    auto handleShift = [&] (Vector<ShufflePair>& shift) {
        // FIXME: We could optimize the spill behavior of the shifter by checking if any of the
        // shifts need spills. If they do, then we could try to get a register out here. Note that
        // this may fail where the current strategy succeeds: out here we need a register that does
        // not interfere with any of the shifts, while the current strategy only needs to find a
        // scratch register that does not interfer with a particular shift. So, this optimization
        // will be opportunistic: if it succeeds, then the individual shifts can use that scratch,
        // otherwise they will do what they do now.
        
        for (unsigned i = shift.size(); i--;)
            handleShiftPair(shift[i], 0);

        Arg lastDst = shift.last().dst();
        if (lastDst.isTmp()) {
            for (Arg& scratch : scratches) {
                ASSERT(scratch != lastDst);
                if (!scratch.isTmp()) {
                    scratch = lastDst;
                    break;
                }
            }
        }
    };

    // First handle shifts whose last destination is a tmp because these free up scratch registers.
    // These end up last in the final sequence, so the final destination of these shifts will be
    // available as a scratch location for anything emitted prior (so, after, since we're emitting in
    // reverse).
    for (auto& entry : shifts) {
        Vector<ShufflePair>& shift = entry.value;
        if (shift.last().dst().isTmp())
            handleShift(shift);
        commitResult();
    }

    // Now handle the rest of the shifts.
    for (auto& entry : shifts) {
        Vector<ShufflePair>& shift = entry.value;
        if (!shift.last().dst().isTmp())
            handleShift(shift);
        commitResult();
    }

    for (Rotate& rotate : rotates) {
        if (!rotate.fringe.isEmpty()) {
            // Make sure we do the fringe first! This won't clobber any of the registers that are
            // part of the rotation.
            handleShift(rotate.fringe);
        }
        
        bool canSwap = false;
        Opcode swap = Oops;
        Width swapWidth = Width8; // bogus value

        // Currently, the swap instruction is not available for floating point on any architecture we
        // support.
        if (bank == GP) {
            // Figure out whether we will be doing 64-bit swaps or 32-bit swaps. If we have a mix of
            // widths we handle that by fixing up the relevant register with zero-extends.
            swap = Swap32;
            swapWidth = Width32;
            bool hasMemory = false;
            bool hasIndex = false;
            for (ShufflePair& pair : rotate.loop) {
                switch (pair.width()) {
                case Width32:
                    break;
                case Width64:
                    swap = Swap64;
                    swapWidth = Width64;
                    break;
                default:
                    RELEASE_ASSERT_NOT_REACHED();
                    break;
                }

                hasMemory |= pair.src().isMemory() || pair.dst().isMemory();
                hasIndex |= pair.src().isIndex() || pair.dst().isIndex();
            }
            
            canSwap = isValidForm(swap, Arg::Tmp, Arg::Tmp);

            // We can totally use swaps even if there are shuffles involving memory. But, we play it
            // safe in that case. There are corner cases we don't handle, and our ability to do it is
            // contingent upon swap form availability.
            
            if (hasMemory) {
                canSwap &= isValidForm(swap, Arg::Tmp, Arg::Addr);
                
                // We don't take the swapping path if there is a mix of widths and some of the
                // shuffles involve memory. That gets too confusing. We might be able to relax this
                // to only bail if there are subwidth pairs involving memory, but I haven't thought
                // about it very hard. Anyway, this case is not common: rotates involving memory
                // don't arise for function calls, and they will only happen for rotates in user code
                // if some of the variables get spilled. It's hard to imagine a program that rotates
                // data around in variables while also doing a combination of uint32->uint64 and
                // int64->int32 casts.
                for (ShufflePair& pair : rotate.loop)
                    canSwap &= pair.width() == swapWidth;
            }

            if (hasIndex)
                canSwap &= isValidForm(swap, Arg::Tmp, Arg::Index);
        }

        if (canSwap) {
            for (unsigned i = rotate.loop.size() - 1; i--;) {
                Arg left = rotate.loop[i].src();
                Arg right = rotate.loop[i + 1].src();

                if (left.isMemory() && right.isMemory()) {
                    // Note that this is a super rare outcome. Rotates are rare. Spills are rare.
                    // Moving data between two spills is rare. To get here a lot of rare stuff has to
                    // all happen at once.
                    
                    Tmp scratch = getScratch(0, findPossibleScratch(code, bank, left, right));
                    RELEASE_ASSERT(scratch);
                    result.append(Inst(moveForWidth(swapWidth), origin, left, scratch));
                    result.append(Inst(swap, origin, scratch, right));
                    result.append(Inst(moveForWidth(swapWidth), origin, scratch, left));
                    returnScratch(0, scratch);
                    continue;
                }

                if (left.isMemory())
                    std::swap(left, right);
                
                result.append(Inst(swap, origin, left, right));
            }

            for (ShufflePair pair : rotate.loop) {
                if (pair.width() == swapWidth)
                    continue;

                RELEASE_ASSERT(pair.width() == Width32);
                RELEASE_ASSERT(swapWidth == Width64);
                RELEASE_ASSERT(pair.dst().isTmp());

                // Need to do an extra zero extension.
                result.append(Inst(Move32, origin, pair.dst(), pair.dst()));
            }
        } else {
            // We can treat this as a shift so long as we take the last destination (i.e. first
            // source) and save it first. Then we handle the first entry in the pair in the rotate
            // specially, after we restore the last destination. This requires some special care to
            // find a scratch register. It's possible that we have a rotate that uses the entire
            // available register file.

            Tmp scratch = findPossibleScratch(
                code, bank,
                [&] (Tmp tmp) -> bool {
                    for (ShufflePair pair : rotate.loop) {
                        if (pair.src().usesTmp(tmp))
                            return false;
                        if (pair.dst().usesTmp(tmp))
                            return false;
                    }
                    return true;
                });

            // NOTE: This is the most likely use of scratch registers.
            scratch = getScratch(0, scratch);

            // We may not have found a scratch register. When this happens, we can just use the spill
            // slot directly.
            Arg rotateSave = scratch ? Arg(scratch) : scratches[0];
            
            handleShiftPair(
                ShufflePair(rotate.loop.last().dst(), rotateSave, rotate.loop[0].width()), 1);

            for (unsigned i = rotate.loop.size(); i-- > 1;)
                handleShiftPair(rotate.loop[i], 1);

            handleShiftPair(
                ShufflePair(rotateSave, rotate.loop[0].dst(), rotate.loop[0].width()), 1);

            if (scratch)
                returnScratch(0, scratch);
        }

        commitResult();
    }

    ASSERT(result.isEmpty());

    for (unsigned i = resultPacks.size(); i--;)
        result.appendVector(resultPacks[i]);

    return result;
}

Vector<Inst> emitShuffle(
    Code& code, const Vector<ShufflePair>& pairs,
    const std::array<Arg, 2>& gpScratch, const std::array<Arg, 2>& fpScratch,
    Value* origin)
{
    Vector<ShufflePair> gpPairs;
    Vector<ShufflePair> fpPairs;
    for (const ShufflePair& pair : pairs) {
        if (pair.src().isMemory() && pair.dst().isMemory() && pair.width() > pointerWidth()) {
            // 8-byte memory-to-memory moves on a 32-bit platform are best handled as float moves.
            fpPairs.append(pair);
        } else if (pair.src().isGP() && pair.dst().isGP()) {
            // This means that gpPairs gets memory-to-memory shuffles. The assumption is that we
            // can do that more efficiently using GPRs, except in the special case above.
            gpPairs.append(pair);
        } else
            fpPairs.append(pair);
    }

    Vector<Inst> result;
    result.appendVector(emitShuffle(code, gpPairs, gpScratch, GP, origin));
    result.appendVector(emitShuffle(code, fpPairs, fpScratch, FP, origin));
    return result;
}

} } } // namespace JSC::B3::Air

#endif // ENABLE(B3_JIT)

