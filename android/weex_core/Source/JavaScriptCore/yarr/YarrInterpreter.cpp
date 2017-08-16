/*
 * Copyright (C) 2009, 2013, 2016 Apple Inc. All rights reserved.
 * Copyright (C) 2010 Peter Varga (pvarga@inf.u-szeged.hu), University of Szeged
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
#include "YarrInterpreter.h"

#include "SuperSampler.h"
#include "Yarr.h"
#include "YarrCanonicalize.h"
#include <wtf/BumpPointerAllocator.h>
#include <wtf/DataLog.h>
#include <wtf/text/CString.h>
#include <wtf/text/WTFString.h>

using namespace WTF;

namespace JSC { namespace Yarr {

template<typename CharType>
class Interpreter {
public:
    struct ParenthesesDisjunctionContext;

    struct BackTrackInfoPatternCharacter {
        uintptr_t begin; // Only needed for unicode patterns
        uintptr_t matchAmount;
    };
    struct BackTrackInfoCharacterClass {
        uintptr_t begin; // Only needed for unicode patterns
        uintptr_t matchAmount;
    };
    struct BackTrackInfoBackReference {
        uintptr_t begin; // Not really needed for greedy quantifiers.
        uintptr_t matchAmount; // Not really needed for fixed quantifiers.
    };
    struct BackTrackInfoAlternative {
        uintptr_t offset;
    };
    struct BackTrackInfoParentheticalAssertion {
        uintptr_t begin;
    };
    struct BackTrackInfoParenthesesOnce {
        uintptr_t begin;
    };
    struct BackTrackInfoParenthesesTerminal {
        uintptr_t begin;
    };
    struct BackTrackInfoParentheses {
        uintptr_t matchAmount;
        ParenthesesDisjunctionContext* lastContext;
    };

    static inline void appendParenthesesDisjunctionContext(BackTrackInfoParentheses* backTrack, ParenthesesDisjunctionContext* context)
    {
        context->next = backTrack->lastContext;
        backTrack->lastContext = context;
        ++backTrack->matchAmount;
    }

    static inline void popParenthesesDisjunctionContext(BackTrackInfoParentheses* backTrack)
    {
        RELEASE_ASSERT(backTrack->matchAmount);
        RELEASE_ASSERT(backTrack->lastContext);
        backTrack->lastContext = backTrack->lastContext->next;
        --backTrack->matchAmount;
    }

    struct DisjunctionContext
    {
        DisjunctionContext()
            : term(0)
        {
        }

        void* operator new(size_t, void* where)
        {
            return where;
        }

        int term;
        unsigned matchBegin;
        unsigned matchEnd;
        uintptr_t frame[1];
    };

    DisjunctionContext* allocDisjunctionContext(ByteDisjunction* disjunction)
    {
        size_t size = sizeof(DisjunctionContext) - sizeof(uintptr_t) + disjunction->m_frameSize * sizeof(uintptr_t);
        allocatorPool = allocatorPool->ensureCapacity(size);
        RELEASE_ASSERT(allocatorPool);
        return new (allocatorPool->alloc(size)) DisjunctionContext();
    }

    void freeDisjunctionContext(DisjunctionContext* context)
    {
        allocatorPool = allocatorPool->dealloc(context);
    }

    struct ParenthesesDisjunctionContext
    {
        ParenthesesDisjunctionContext(unsigned* output, ByteTerm& term)
            : next(0)
        {
            unsigned firstSubpatternId = term.atom.subpatternId;
            unsigned numNestedSubpatterns = term.atom.parenthesesDisjunction->m_numSubpatterns;

            for (unsigned i = 0; i < (numNestedSubpatterns << 1); ++i) {
                subpatternBackup[i] = output[(firstSubpatternId << 1) + i];
                output[(firstSubpatternId << 1) + i] = offsetNoMatch;
            }

            new (getDisjunctionContext(term)) DisjunctionContext();
        }

        void* operator new(size_t, void* where)
        {
            return where;
        }

        void restoreOutput(unsigned* output, unsigned firstSubpatternId, unsigned numNestedSubpatterns)
        {
            for (unsigned i = 0; i < (numNestedSubpatterns << 1); ++i)
                output[(firstSubpatternId << 1) + i] = subpatternBackup[i];
        }

        DisjunctionContext* getDisjunctionContext(ByteTerm& term)
        {
            return reinterpret_cast<DisjunctionContext*>(&(subpatternBackup[term.atom.parenthesesDisjunction->m_numSubpatterns << 1]));
        }

        ParenthesesDisjunctionContext* next;
        unsigned subpatternBackup[1];
    };

    ParenthesesDisjunctionContext* allocParenthesesDisjunctionContext(ByteDisjunction* disjunction, unsigned* output, ByteTerm& term)
    {
        size_t size = sizeof(ParenthesesDisjunctionContext) - sizeof(unsigned) + (term.atom.parenthesesDisjunction->m_numSubpatterns << 1) * sizeof(unsigned) + sizeof(DisjunctionContext) - sizeof(uintptr_t) + static_cast<size_t>(disjunction->m_frameSize) * sizeof(uintptr_t);
        allocatorPool = allocatorPool->ensureCapacity(size);
        RELEASE_ASSERT(allocatorPool);
        return new (allocatorPool->alloc(size)) ParenthesesDisjunctionContext(output, term);
    }

    void freeParenthesesDisjunctionContext(ParenthesesDisjunctionContext* context)
    {
        allocatorPool = allocatorPool->dealloc(context);
    }

    class InputStream {
    public:
        InputStream(const CharType* input, unsigned start, unsigned length, bool decodeSurrogatePairs)
            : input(input)
            , pos(start)
            , length(length)
            , decodeSurrogatePairs(decodeSurrogatePairs)
        {
        }

        void next()
        {
            ++pos;
        }

        void rewind(unsigned amount)
        {
            ASSERT(pos >= amount);
            pos -= amount;
        }

        int read()
        {
            ASSERT(pos < length);
            if (pos < length)
                return input[pos];
            return -1;
        }

        int readPair()
        {
            ASSERT(pos + 1 < length);
            return input[pos] | input[pos + 1] << 16;
        }

        int readChecked(unsigned negativePositionOffest)
        {
            RELEASE_ASSERT(pos >= negativePositionOffest);
            unsigned p = pos - negativePositionOffest;
            ASSERT(p < length);
            int result = input[p];
            if (U16_IS_LEAD(result) && decodeSurrogatePairs && p + 1 < length && U16_IS_TRAIL(input[p + 1])) {
                if (atEnd())
                    return -1;
                
                result = U16_GET_SUPPLEMENTARY(result, input[p + 1]);
                next();
            }
            return result;
        }
        
        int readSurrogatePairChecked(unsigned negativePositionOffset)
        {
            RELEASE_ASSERT(pos >= negativePositionOffset);
            unsigned p = pos - negativePositionOffset;
            ASSERT(p < length);
            if (p + 1 >= length)
                return -1;

            int first = input[p];
            int second = input[p + 1];
            if (U16_IS_LEAD(first) && U16_IS_TRAIL(second))
                return U16_GET_SUPPLEMENTARY(first, second);

            return -1;
        }

        int reread(unsigned from)
        {
            ASSERT(from < length);
            int result = input[from];
            if (U16_IS_LEAD(result) && decodeSurrogatePairs && from + 1 < length && U16_IS_TRAIL(input[from + 1]))
                result = U16_GET_SUPPLEMENTARY(result, input[from + 1]);
            return result;
        }

        int prev()
        {
            ASSERT(!(pos > length));
            if (pos && length)
                return input[pos - 1];
            return -1;
        }

        unsigned getPos()
        {
            return pos;
        }

        void setPos(unsigned p)
        {
            pos = p;
        }

        bool atStart()
        {
            return pos == 0;
        }

        bool atEnd()
        {
            return pos == length;
        }

        unsigned end()
        {
            return length;
        }

        bool checkInput(unsigned count)
        {
            if (((pos + count) <= length) && ((pos + count) >= pos)) {
                pos += count;
                return true;
            }
            return false;
        }

        void uncheckInput(unsigned count)
        {
            RELEASE_ASSERT(pos >= count);
            pos -= count;
        }

        bool atStart(unsigned negativePositionOffset)
        {
            return pos == negativePositionOffset;
        }

        bool atEnd(unsigned negativePositionOffest)
        {
            RELEASE_ASSERT(pos >= negativePositionOffest);
            return (pos - negativePositionOffest) == length;
        }

        bool isAvailableInput(unsigned offset)
        {
            return (((pos + offset) <= length) && ((pos + offset) >= pos));
        }

    private:
        const CharType* input;
        unsigned pos;
        unsigned length;
        bool decodeSurrogatePairs;
    };

    bool testCharacterClass(CharacterClass* characterClass, int ch)
    {
        if (!isASCII(ch)) {
            for (unsigned i = 0; i < characterClass->m_matchesUnicode.size(); ++i)
                if (ch == characterClass->m_matchesUnicode[i])
                    return true;
            for (unsigned i = 0; i < characterClass->m_rangesUnicode.size(); ++i)
                if ((ch >= characterClass->m_rangesUnicode[i].begin) && (ch <= characterClass->m_rangesUnicode[i].end))
                    return true;
        } else {
            for (unsigned i = 0; i < characterClass->m_matches.size(); ++i)
                if (ch == characterClass->m_matches[i])
                    return true;
            for (unsigned i = 0; i < characterClass->m_ranges.size(); ++i)
                if ((ch >= characterClass->m_ranges[i].begin) && (ch <= characterClass->m_ranges[i].end))
                    return true;
        }

        return false;
    }

    bool checkCharacter(int testChar, unsigned negativeInputOffset)
    {
        return testChar == input.readChecked(negativeInputOffset);
    }

    bool checkSurrogatePair(int testUnicodeChar, unsigned negativeInputOffset)
    {
        return testUnicodeChar == input.readSurrogatePairChecked(negativeInputOffset);
    }

    bool checkCasedCharacter(int loChar, int hiChar, unsigned negativeInputOffset)
    {
        int ch = input.readChecked(negativeInputOffset);
        return (loChar == ch) || (hiChar == ch);
    }

    bool checkCharacterClass(CharacterClass* characterClass, bool invert, unsigned negativeInputOffset)
    {
        bool match = testCharacterClass(characterClass, input.readChecked(negativeInputOffset));
        return invert ? !match : match;
    }

    bool tryConsumeBackReference(int matchBegin, int matchEnd, unsigned negativeInputOffset)
    {
        unsigned matchSize = (unsigned)(matchEnd - matchBegin);

        if (!input.checkInput(matchSize))
            return false;

        for (unsigned i = 0; i < matchSize; ++i) {
            int oldCh = input.reread(matchBegin + i);
            int ch;
            if (!U_IS_BMP(oldCh)) {
                ch = input.readSurrogatePairChecked(negativeInputOffset + matchSize - i);
                ++i;
            } else
                ch = input.readChecked(negativeInputOffset + matchSize - i);

            if (oldCh == ch)
                continue;

            if (pattern->ignoreCase()) {
                // See ES 6.0, 21.2.2.8.2 for the definition of Canonicalize(). For non-Unicode
                // patterns, Unicode values are never allowed to match against ASCII ones.
                // For Unicode, we need to check all canonical equivalents of a character.
                if (!unicode && (isASCII(oldCh) || isASCII(ch))) {
                    if (toASCIIUpper(oldCh) == toASCIIUpper(ch))
                        continue;
                } else if (areCanonicallyEquivalent(oldCh, ch, unicode ? CanonicalMode::Unicode : CanonicalMode::UCS2))
                    continue;
            }

            input.uncheckInput(matchSize);
            return false;
        }

        return true;
    }

    bool matchAssertionBOL(ByteTerm& term)
    {
        return (input.atStart(term.inputPosition)) || (pattern->multiline() && testCharacterClass(pattern->newlineCharacterClass, input.readChecked(term.inputPosition + 1)));
    }

    bool matchAssertionEOL(ByteTerm& term)
    {
        if (term.inputPosition)
            return (input.atEnd(term.inputPosition)) || (pattern->multiline() && testCharacterClass(pattern->newlineCharacterClass, input.readChecked(term.inputPosition)));

        return (input.atEnd()) || (pattern->multiline() && testCharacterClass(pattern->newlineCharacterClass, input.read()));
    }

    bool matchAssertionWordBoundary(ByteTerm& term)
    {
        bool prevIsWordchar = !input.atStart(term.inputPosition) && testCharacterClass(pattern->wordcharCharacterClass, input.readChecked(term.inputPosition + 1));
        bool readIsWordchar;
        if (term.inputPosition)
            readIsWordchar = !input.atEnd(term.inputPosition) && testCharacterClass(pattern->wordcharCharacterClass, input.readChecked(term.inputPosition));
        else
            readIsWordchar = !input.atEnd() && testCharacterClass(pattern->wordcharCharacterClass, input.read());

        bool wordBoundary = prevIsWordchar != readIsWordchar;
        return term.invert() ? !wordBoundary : wordBoundary;
    }

    bool backtrackPatternCharacter(ByteTerm& term, DisjunctionContext* context)
    {
        BackTrackInfoPatternCharacter* backTrack = reinterpret_cast<BackTrackInfoPatternCharacter*>(context->frame + term.frameLocation);

        switch (term.atom.quantityType) {
        case QuantifierFixedCount:
            break;

        case QuantifierGreedy:
            if (backTrack->matchAmount) {
                --backTrack->matchAmount;
                input.uncheckInput(U16_LENGTH(term.atom.patternCharacter));
                return true;
            }
            break;

        case QuantifierNonGreedy:
            if ((backTrack->matchAmount < term.atom.quantityMaxCount) && input.checkInput(1)) {
                ++backTrack->matchAmount;
                if (checkCharacter(term.atom.patternCharacter, term.inputPosition + 1))
                    return true;
            }
            input.setPos(backTrack->begin);
            break;
        }

        return false;
    }

    bool backtrackPatternCasedCharacter(ByteTerm& term, DisjunctionContext* context)
    {
        BackTrackInfoPatternCharacter* backTrack = reinterpret_cast<BackTrackInfoPatternCharacter*>(context->frame + term.frameLocation);

        switch (term.atom.quantityType) {
        case QuantifierFixedCount:
            break;

        case QuantifierGreedy:
            if (backTrack->matchAmount) {
                --backTrack->matchAmount;
                input.uncheckInput(1);
                return true;
            }
            break;

        case QuantifierNonGreedy:
            if ((backTrack->matchAmount < term.atom.quantityMaxCount) && input.checkInput(1)) {
                ++backTrack->matchAmount;
                if (checkCasedCharacter(term.atom.casedCharacter.lo, term.atom.casedCharacter.hi, term.inputPosition + 1))
                    return true;
            }
            input.uncheckInput(backTrack->matchAmount);
            break;
        }

        return false;
    }

    bool matchCharacterClass(ByteTerm& term, DisjunctionContext* context)
    {
        ASSERT(term.type == ByteTerm::TypeCharacterClass);
        BackTrackInfoCharacterClass* backTrack = reinterpret_cast<BackTrackInfoCharacterClass*>(context->frame + term.frameLocation);

        switch (term.atom.quantityType) {
        case QuantifierFixedCount: {
            if (unicode) {
                backTrack->begin = input.getPos();
                unsigned matchAmount = 0;
                for (matchAmount = 0; matchAmount < term.atom.quantityMaxCount; ++matchAmount) {
                    if (!checkCharacterClass(term.atom.characterClass, term.invert(), term.inputPosition - matchAmount)) {
                        input.setPos(backTrack->begin);
                        return false;
                    }
                }

                return true;
            }

            for (unsigned matchAmount = 0; matchAmount < term.atom.quantityMaxCount; ++matchAmount) {
                if (!checkCharacterClass(term.atom.characterClass, term.invert(), term.inputPosition - matchAmount))
                    return false;
            }
            return true;
        }

        case QuantifierGreedy: {
            unsigned position = input.getPos();
            backTrack->begin = position;
            unsigned matchAmount = 0;
            while ((matchAmount < term.atom.quantityMaxCount) && input.checkInput(1)) {
                if (!checkCharacterClass(term.atom.characterClass, term.invert(), term.inputPosition + 1)) {
                    input.setPos(position);
                    break;
                }
                ++matchAmount;
                position = input.getPos();
            }
            backTrack->matchAmount = matchAmount;

            return true;
        }

        case QuantifierNonGreedy:
            backTrack->begin = input.getPos();
            backTrack->matchAmount = 0;
            return true;
        }

        RELEASE_ASSERT_NOT_REACHED();
        return false;
    }

    bool backtrackCharacterClass(ByteTerm& term, DisjunctionContext* context)
    {
        ASSERT(term.type == ByteTerm::TypeCharacterClass);
        BackTrackInfoCharacterClass* backTrack = reinterpret_cast<BackTrackInfoCharacterClass*>(context->frame + term.frameLocation);

        switch (term.atom.quantityType) {
        case QuantifierFixedCount:
            if (unicode)
                input.setPos(backTrack->begin);
            break;

        case QuantifierGreedy:
            if (backTrack->matchAmount) {
                if (unicode) {
                    // Rematch one less match
                    input.setPos(backTrack->begin);
                    --backTrack->matchAmount;
                    for (unsigned matchAmount = 0; (matchAmount < backTrack->matchAmount) && input.checkInput(1); ++matchAmount) {
                        if (!checkCharacterClass(term.atom.characterClass, term.invert(), term.inputPosition + 1)) {
                            input.uncheckInput(1);
                            break;
                        }
                    }
                    return true;
                }
                --backTrack->matchAmount;
                input.uncheckInput(1);
                return true;
            }
            break;

        case QuantifierNonGreedy:
            if ((backTrack->matchAmount < term.atom.quantityMaxCount) && input.checkInput(1)) {
                ++backTrack->matchAmount;
                if (checkCharacterClass(term.atom.characterClass, term.invert(), term.inputPosition + 1))
                    return true;
            }
            input.setPos(backTrack->begin);
            break;
        }

        return false;
    }

    bool matchBackReference(ByteTerm& term, DisjunctionContext* context)
    {
        ASSERT(term.type == ByteTerm::TypeBackReference);
        BackTrackInfoBackReference* backTrack = reinterpret_cast<BackTrackInfoBackReference*>(context->frame + term.frameLocation);

        unsigned matchBegin = output[(term.atom.subpatternId << 1)];
        unsigned matchEnd = output[(term.atom.subpatternId << 1) + 1];

        // If the end position of the referenced match hasn't set yet then the backreference in the same parentheses where it references to that.
        // In this case the result of match is empty string like when it references to a parentheses with zero-width match.
        // Eg.: /(a\1)/
        if (matchEnd == offsetNoMatch)
            return true;

        if (matchBegin == offsetNoMatch)
            return true;

        ASSERT(matchBegin <= matchEnd);

        if (matchBegin == matchEnd)
            return true;

        switch (term.atom.quantityType) {
        case QuantifierFixedCount: {
            backTrack->begin = input.getPos();
            for (unsigned matchAmount = 0; matchAmount < term.atom.quantityMaxCount; ++matchAmount) {
                if (!tryConsumeBackReference(matchBegin, matchEnd, term.inputPosition)) {
                    input.setPos(backTrack->begin);
                    return false;
                }
            }
            return true;
        }

        case QuantifierGreedy: {
            unsigned matchAmount = 0;
            while ((matchAmount < term.atom.quantityMaxCount) && tryConsumeBackReference(matchBegin, matchEnd, term.inputPosition))
                ++matchAmount;
            backTrack->matchAmount = matchAmount;
            return true;
        }

        case QuantifierNonGreedy:
            backTrack->begin = input.getPos();
            backTrack->matchAmount = 0;
            return true;
        }

        RELEASE_ASSERT_NOT_REACHED();
        return false;
    }

    bool backtrackBackReference(ByteTerm& term, DisjunctionContext* context)
    {
        ASSERT(term.type == ByteTerm::TypeBackReference);
        BackTrackInfoBackReference* backTrack = reinterpret_cast<BackTrackInfoBackReference*>(context->frame + term.frameLocation);

        unsigned matchBegin = output[(term.atom.subpatternId << 1)];
        unsigned matchEnd = output[(term.atom.subpatternId << 1) + 1];

        if (matchBegin == offsetNoMatch)
            return false;

        ASSERT(matchBegin <= matchEnd);

        if (matchBegin == matchEnd)
            return false;

        switch (term.atom.quantityType) {
        case QuantifierFixedCount:
            // for quantityMaxCount == 1, could rewind.
            input.setPos(backTrack->begin);
            break;

        case QuantifierGreedy:
            if (backTrack->matchAmount) {
                --backTrack->matchAmount;
                input.rewind(matchEnd - matchBegin);
                return true;
            }
            break;

        case QuantifierNonGreedy:
            if ((backTrack->matchAmount < term.atom.quantityMaxCount) && tryConsumeBackReference(matchBegin, matchEnd, term.inputPosition)) {
                ++backTrack->matchAmount;
                return true;
            }
            input.setPos(backTrack->begin);
            break;
        }

        return false;
    }

    void recordParenthesesMatch(ByteTerm& term, ParenthesesDisjunctionContext* context)
    {
        if (term.capture()) {
            unsigned subpatternId = term.atom.subpatternId;
            output[(subpatternId << 1)] = context->getDisjunctionContext(term)->matchBegin - term.inputPosition;
            output[(subpatternId << 1) + 1] = context->getDisjunctionContext(term)->matchEnd - term.inputPosition;
        }
    }
    void resetMatches(ByteTerm& term, ParenthesesDisjunctionContext* context)
    {
        unsigned firstSubpatternId = term.atom.subpatternId;
        unsigned count = term.atom.parenthesesDisjunction->m_numSubpatterns;
        context->restoreOutput(output, firstSubpatternId, count);
    }
    JSRegExpResult parenthesesDoBacktrack(ByteTerm& term, BackTrackInfoParentheses* backTrack)
    {
        while (backTrack->matchAmount) {
            ParenthesesDisjunctionContext* context = backTrack->lastContext;

            JSRegExpResult result = matchDisjunction(term.atom.parenthesesDisjunction, context->getDisjunctionContext(term), true);
            if (result == JSRegExpMatch)
                return JSRegExpMatch;

            resetMatches(term, context);
            popParenthesesDisjunctionContext(backTrack);
            freeParenthesesDisjunctionContext(context);

            if (result != JSRegExpNoMatch)
                return result;
        }

        return JSRegExpNoMatch;
    }

    bool matchParenthesesOnceBegin(ByteTerm& term, DisjunctionContext* context)
    {
        ASSERT(term.type == ByteTerm::TypeParenthesesSubpatternOnceBegin);
        ASSERT(term.atom.quantityMaxCount == 1);

        BackTrackInfoParenthesesOnce* backTrack = reinterpret_cast<BackTrackInfoParenthesesOnce*>(context->frame + term.frameLocation);

        switch (term.atom.quantityType) {
        case QuantifierGreedy: {
            // set this speculatively; if we get to the parens end this will be true.
            backTrack->begin = input.getPos();
            break;
        }
        case QuantifierNonGreedy: {
            backTrack->begin = notFound;
            context->term += term.atom.parenthesesWidth;
            return true;
        }
        case QuantifierFixedCount:
            break;
        }

        if (term.capture()) {
            unsigned subpatternId = term.atom.subpatternId;
            output[(subpatternId << 1)] = input.getPos() - term.inputPosition;
        }

        return true;
    }

    bool matchParenthesesOnceEnd(ByteTerm& term, DisjunctionContext* context)
    {
        ASSERT(term.type == ByteTerm::TypeParenthesesSubpatternOnceEnd);
        ASSERT(term.atom.quantityMaxCount == 1);

        if (term.capture()) {
            unsigned subpatternId = term.atom.subpatternId;
            output[(subpatternId << 1) + 1] = input.getPos() - term.inputPosition;
        }

        if (term.atom.quantityType == QuantifierFixedCount)
            return true;

        BackTrackInfoParenthesesOnce* backTrack = reinterpret_cast<BackTrackInfoParenthesesOnce*>(context->frame + term.frameLocation);
        return backTrack->begin != input.getPos();
    }

    bool backtrackParenthesesOnceBegin(ByteTerm& term, DisjunctionContext* context)
    {
        ASSERT(term.type == ByteTerm::TypeParenthesesSubpatternOnceBegin);
        ASSERT(term.atom.quantityMaxCount == 1);

        BackTrackInfoParenthesesOnce* backTrack = reinterpret_cast<BackTrackInfoParenthesesOnce*>(context->frame + term.frameLocation);

        if (term.capture()) {
            unsigned subpatternId = term.atom.subpatternId;
            output[(subpatternId << 1)] = offsetNoMatch;
            output[(subpatternId << 1) + 1] = offsetNoMatch;
        }

        switch (term.atom.quantityType) {
        case QuantifierGreedy:
            // if we backtrack to this point, there is another chance - try matching nothing.
            ASSERT(backTrack->begin != notFound);
            backTrack->begin = notFound;
            context->term += term.atom.parenthesesWidth;
            return true;
        case QuantifierNonGreedy:
            ASSERT(backTrack->begin != notFound);
            FALLTHROUGH;
        case QuantifierFixedCount:
            break;
        }

        return false;
    }

    bool backtrackParenthesesOnceEnd(ByteTerm& term, DisjunctionContext* context)
    {
        ASSERT(term.type == ByteTerm::TypeParenthesesSubpatternOnceEnd);
        ASSERT(term.atom.quantityMaxCount == 1);

        BackTrackInfoParenthesesOnce* backTrack = reinterpret_cast<BackTrackInfoParenthesesOnce*>(context->frame + term.frameLocation);

        switch (term.atom.quantityType) {
        case QuantifierGreedy:
            if (backTrack->begin == notFound) {
                context->term -= term.atom.parenthesesWidth;
                return false;
            }
            FALLTHROUGH;
        case QuantifierNonGreedy:
            if (backTrack->begin == notFound) {
                backTrack->begin = input.getPos();
                if (term.capture()) {
                    // Technically this access to inputPosition should be accessing the begin term's
                    // inputPosition, but for repeats other than fixed these values should be
                    // the same anyway! (We don't pre-check for greedy or non-greedy matches.)
                    ASSERT((&term - term.atom.parenthesesWidth)->type == ByteTerm::TypeParenthesesSubpatternOnceBegin);
                    ASSERT((&term - term.atom.parenthesesWidth)->inputPosition == term.inputPosition);
                    unsigned subpatternId = term.atom.subpatternId;
                    output[subpatternId << 1] = input.getPos() - term.inputPosition;
                }
                context->term -= term.atom.parenthesesWidth;
                return true;
            }
            FALLTHROUGH;
        case QuantifierFixedCount:
            break;
        }

        return false;
    }

    bool matchParenthesesTerminalBegin(ByteTerm& term, DisjunctionContext* context)
    {
        ASSERT(term.type == ByteTerm::TypeParenthesesSubpatternTerminalBegin);
        ASSERT(term.atom.quantityType == QuantifierGreedy);
        ASSERT(term.atom.quantityMaxCount == quantifyInfinite);
        ASSERT(!term.capture());

        BackTrackInfoParenthesesTerminal* backTrack = reinterpret_cast<BackTrackInfoParenthesesTerminal*>(context->frame + term.frameLocation);
        backTrack->begin = input.getPos();
        return true;
    }

    bool matchParenthesesTerminalEnd(ByteTerm& term, DisjunctionContext* context)
    {
        ASSERT(term.type == ByteTerm::TypeParenthesesSubpatternTerminalEnd);

        BackTrackInfoParenthesesTerminal* backTrack = reinterpret_cast<BackTrackInfoParenthesesTerminal*>(context->frame + term.frameLocation);
        // Empty match is a failed match.
        if (backTrack->begin == input.getPos())
            return false;

        // Successful match! Okay, what's next? - loop around and try to match more!
        context->term -= (term.atom.parenthesesWidth + 1);
        return true;
    }

    bool backtrackParenthesesTerminalBegin(ByteTerm& term, DisjunctionContext* context)
    {
        ASSERT(term.type == ByteTerm::TypeParenthesesSubpatternTerminalBegin);
        ASSERT(term.atom.quantityType == QuantifierGreedy);
        ASSERT(term.atom.quantityMaxCount == quantifyInfinite);
        ASSERT(!term.capture());

        // If we backtrack to this point, we have failed to match this iteration of the parens.
        // Since this is greedy / zero minimum a failed is also accepted as a match!
        context->term += term.atom.parenthesesWidth;
        return true;
    }

    bool backtrackParenthesesTerminalEnd(ByteTerm&, DisjunctionContext*)
    {
        // 'Terminal' parentheses are at the end of the regex, and as such a match past end
        // should always be returned as a successful match - we should never backtrack to here.
        RELEASE_ASSERT_NOT_REACHED();
        return false;
    }

    bool matchParentheticalAssertionBegin(ByteTerm& term, DisjunctionContext* context)
    {
        ASSERT(term.type == ByteTerm::TypeParentheticalAssertionBegin);
        ASSERT(term.atom.quantityMaxCount == 1);

        BackTrackInfoParentheticalAssertion* backTrack = reinterpret_cast<BackTrackInfoParentheticalAssertion*>(context->frame + term.frameLocation);

        backTrack->begin = input.getPos();
        return true;
    }

    bool matchParentheticalAssertionEnd(ByteTerm& term, DisjunctionContext* context)
    {
        ASSERT(term.type == ByteTerm::TypeParentheticalAssertionEnd);
        ASSERT(term.atom.quantityMaxCount == 1);

        BackTrackInfoParentheticalAssertion* backTrack = reinterpret_cast<BackTrackInfoParentheticalAssertion*>(context->frame + term.frameLocation);

        input.setPos(backTrack->begin);

        // We've reached the end of the parens; if they are inverted, this is failure.
        if (term.invert()) {
            context->term -= term.atom.parenthesesWidth;
            return false;
        }

        return true;
    }

    bool backtrackParentheticalAssertionBegin(ByteTerm& term, DisjunctionContext* context)
    {
        ASSERT(term.type == ByteTerm::TypeParentheticalAssertionBegin);
        ASSERT(term.atom.quantityMaxCount == 1);

        // We've failed to match parens; if they are inverted, this is win!
        if (term.invert()) {
            context->term += term.atom.parenthesesWidth;
            return true;
        }

        return false;
    }

    bool backtrackParentheticalAssertionEnd(ByteTerm& term, DisjunctionContext* context)
    {
        ASSERT(term.type == ByteTerm::TypeParentheticalAssertionEnd);
        ASSERT(term.atom.quantityMaxCount == 1);

        BackTrackInfoParentheticalAssertion* backTrack = reinterpret_cast<BackTrackInfoParentheticalAssertion*>(context->frame + term.frameLocation);

        input.setPos(backTrack->begin);

        context->term -= term.atom.parenthesesWidth;
        return false;
    }

    JSRegExpResult matchParentheses(ByteTerm& term, DisjunctionContext* context)
    {
        ASSERT(term.type == ByteTerm::TypeParenthesesSubpattern);

        BackTrackInfoParentheses* backTrack = reinterpret_cast<BackTrackInfoParentheses*>(context->frame + term.frameLocation);
        ByteDisjunction* disjunctionBody = term.atom.parenthesesDisjunction;

        backTrack->matchAmount = 0;
        backTrack->lastContext = 0;

        ASSERT(term.atom.quantityType != QuantifierFixedCount || term.atom.quantityMinCount == term.atom.quantityMaxCount);

        unsigned minimumMatchCount = term.atom.quantityMinCount;
        JSRegExpResult fixedMatchResult;

        // Handle fixed matches and the minimum part of a variable length match.
        if (minimumMatchCount) {
            // While we haven't yet reached our fixed limit,
            while (backTrack->matchAmount < minimumMatchCount) {
                // Try to do a match, and it it succeeds, add it to the list.
                ParenthesesDisjunctionContext* context = allocParenthesesDisjunctionContext(disjunctionBody, output, term);
                fixedMatchResult = matchDisjunction(disjunctionBody, context->getDisjunctionContext(term));
                if (fixedMatchResult == JSRegExpMatch)
                    appendParenthesesDisjunctionContext(backTrack, context);
                else {
                    // The match failed; try to find an alternate point to carry on from.
                    resetMatches(term, context);
                    freeParenthesesDisjunctionContext(context);
                    
                    if (fixedMatchResult != JSRegExpNoMatch)
                        return fixedMatchResult;
                    JSRegExpResult backtrackResult = parenthesesDoBacktrack(term, backTrack);
                    if (backtrackResult != JSRegExpMatch)
                        return backtrackResult;
                }
            }

            ParenthesesDisjunctionContext* context = backTrack->lastContext;
            recordParenthesesMatch(term, context);
        }

        switch (term.atom.quantityType) {
        case QuantifierFixedCount: {
            ASSERT(backTrack->matchAmount == term.atom.quantityMaxCount);
            return JSRegExpMatch;
        }

        case QuantifierGreedy: {
            while (backTrack->matchAmount < term.atom.quantityMaxCount) {
                ParenthesesDisjunctionContext* context = allocParenthesesDisjunctionContext(disjunctionBody, output, term);
                JSRegExpResult result = matchNonZeroDisjunction(disjunctionBody, context->getDisjunctionContext(term));
                if (result == JSRegExpMatch)
                    appendParenthesesDisjunctionContext(backTrack, context);
                else {
                    resetMatches(term, context);
                    freeParenthesesDisjunctionContext(context);

                    if (result != JSRegExpNoMatch)
                        return result;

                    break;
                }
            }

            if (backTrack->matchAmount) {
                ParenthesesDisjunctionContext* context = backTrack->lastContext;
                recordParenthesesMatch(term, context);
            }
            return JSRegExpMatch;
        }

        case QuantifierNonGreedy:
            return JSRegExpMatch;
        }

        RELEASE_ASSERT_NOT_REACHED();
        return JSRegExpErrorNoMatch;
    }

    // Rules for backtracking differ depending on whether this is greedy or non-greedy.
    //
    // Greedy matches never should try just adding more - you should already have done
    // the 'more' cases.  Always backtrack, at least a leetle bit.  However cases where
    // you backtrack an item off the list needs checking, since we'll never have matched
    // the one less case.  Tracking forwards, still add as much as possible.
    //
    // Non-greedy, we've already done the one less case, so don't match on popping.
    // We haven't done the one more case, so always try to add that.
    //
    JSRegExpResult backtrackParentheses(ByteTerm& term, DisjunctionContext* context)
    {
        ASSERT(term.type == ByteTerm::TypeParenthesesSubpattern);

        BackTrackInfoParentheses* backTrack = reinterpret_cast<BackTrackInfoParentheses*>(context->frame + term.frameLocation);
        ByteDisjunction* disjunctionBody = term.atom.parenthesesDisjunction;

        switch (term.atom.quantityType) {
        case QuantifierFixedCount: {
            ASSERT(backTrack->matchAmount == term.atom.quantityMaxCount);

            ParenthesesDisjunctionContext* context = 0;
            JSRegExpResult result = parenthesesDoBacktrack(term, backTrack);

            if (result != JSRegExpMatch)
                return result;

            // While we haven't yet reached our fixed limit,
            while (backTrack->matchAmount < term.atom.quantityMaxCount) {
                // Try to do a match, and it it succeeds, add it to the list.
                context = allocParenthesesDisjunctionContext(disjunctionBody, output, term);
                result = matchDisjunction(disjunctionBody, context->getDisjunctionContext(term));

                if (result == JSRegExpMatch)
                    appendParenthesesDisjunctionContext(backTrack, context);
                else {
                    // The match failed; try to find an alternate point to carry on from.
                    resetMatches(term, context);
                    freeParenthesesDisjunctionContext(context);

                    if (result != JSRegExpNoMatch)
                        return result;
                    JSRegExpResult backtrackResult = parenthesesDoBacktrack(term, backTrack);
                    if (backtrackResult != JSRegExpMatch)
                        return backtrackResult;
                }
            }

            ASSERT(backTrack->matchAmount == term.atom.quantityMaxCount);
            context = backTrack->lastContext;
            recordParenthesesMatch(term, context);
            return JSRegExpMatch;
        }

        case QuantifierGreedy: {
            if (!backTrack->matchAmount)
                return JSRegExpNoMatch;

            ParenthesesDisjunctionContext* context = backTrack->lastContext;
            JSRegExpResult result = matchNonZeroDisjunction(disjunctionBody, context->getDisjunctionContext(term), true);
            if (result == JSRegExpMatch) {
                while (backTrack->matchAmount < term.atom.quantityMaxCount) {
                    ParenthesesDisjunctionContext* context = allocParenthesesDisjunctionContext(disjunctionBody, output, term);
                    JSRegExpResult parenthesesResult = matchNonZeroDisjunction(disjunctionBody, context->getDisjunctionContext(term));
                    if (parenthesesResult == JSRegExpMatch)
                        appendParenthesesDisjunctionContext(backTrack, context);
                    else {
                        resetMatches(term, context);
                        freeParenthesesDisjunctionContext(context);

                        if (parenthesesResult != JSRegExpNoMatch)
                            return parenthesesResult;

                        break;
                    }
                }
            } else {
                resetMatches(term, context);
                popParenthesesDisjunctionContext(backTrack);
                freeParenthesesDisjunctionContext(context);

                if (result != JSRegExpNoMatch)
                    return result;
            }

            if (backTrack->matchAmount) {
                ParenthesesDisjunctionContext* context = backTrack->lastContext;
                recordParenthesesMatch(term, context);
            }
            return JSRegExpMatch;
        }

        case QuantifierNonGreedy: {
            // If we've not reached the limit, try to add one more match.
            if (backTrack->matchAmount < term.atom.quantityMaxCount) {
                ParenthesesDisjunctionContext* context = allocParenthesesDisjunctionContext(disjunctionBody, output, term);
                JSRegExpResult result = matchNonZeroDisjunction(disjunctionBody, context->getDisjunctionContext(term));
                if (result == JSRegExpMatch) {
                    appendParenthesesDisjunctionContext(backTrack, context);
                    recordParenthesesMatch(term, context);
                    return JSRegExpMatch;
                }

                resetMatches(term, context);
                freeParenthesesDisjunctionContext(context);

                if (result != JSRegExpNoMatch)
                    return result;
            }

            // Nope - okay backtrack looking for an alternative.
            while (backTrack->matchAmount) {
                ParenthesesDisjunctionContext* context = backTrack->lastContext;
                JSRegExpResult result = matchNonZeroDisjunction(disjunctionBody, context->getDisjunctionContext(term), true);
                if (result == JSRegExpMatch) {
                    // successful backtrack! we're back in the game!
                    if (backTrack->matchAmount) {
                        context = backTrack->lastContext;
                        recordParenthesesMatch(term, context);
                    }
                    return JSRegExpMatch;
                }

                // pop a match off the stack
                resetMatches(term, context);
                popParenthesesDisjunctionContext(backTrack);
                freeParenthesesDisjunctionContext(context);

                if (result != JSRegExpNoMatch)
                    return result;
            }

            return JSRegExpNoMatch;
        }
        }

        RELEASE_ASSERT_NOT_REACHED();
        return JSRegExpErrorNoMatch;
    }

    bool matchDotStarEnclosure(ByteTerm& term, DisjunctionContext* context)
    {
        UNUSED_PARAM(term);
        unsigned matchBegin = context->matchBegin;

        if (matchBegin) {
            for (matchBegin--; true; matchBegin--) {
                if (testCharacterClass(pattern->newlineCharacterClass, input.reread(matchBegin))) {
                    ++matchBegin;
                    break;
                }

                if (!matchBegin)
                    break;
            }
        }

        unsigned matchEnd = input.getPos();

        for (; (matchEnd != input.end())
             && (!testCharacterClass(pattern->newlineCharacterClass, input.reread(matchEnd))); matchEnd++) { }

        if (((matchBegin && term.anchors.m_bol)
             || ((matchEnd != input.end()) && term.anchors.m_eol))
            && !pattern->multiline())
            return false;

        context->matchBegin = matchBegin;
        context->matchEnd = matchEnd;
        return true;
    }

#define MATCH_NEXT() { ++context->term; goto matchAgain; }
#define BACKTRACK() { --context->term; goto backtrack; }
#define currentTerm() (disjunction->terms[context->term])
    JSRegExpResult matchDisjunction(ByteDisjunction* disjunction, DisjunctionContext* context, bool btrack = false)
    {
        if (!--remainingMatchCount)
            return JSRegExpErrorHitLimit;

        if (btrack)
            BACKTRACK();

        context->matchBegin = input.getPos();
        context->term = 0;

    matchAgain:
        ASSERT(context->term < static_cast<int>(disjunction->terms.size()));

        switch (currentTerm().type) {
        case ByteTerm::TypeSubpatternBegin:
            MATCH_NEXT();
        case ByteTerm::TypeSubpatternEnd:
            context->matchEnd = input.getPos();
            return JSRegExpMatch;

        case ByteTerm::TypeBodyAlternativeBegin:
            MATCH_NEXT();
        case ByteTerm::TypeBodyAlternativeDisjunction:
        case ByteTerm::TypeBodyAlternativeEnd:
            context->matchEnd = input.getPos();
            return JSRegExpMatch;

        case ByteTerm::TypeAlternativeBegin:
            MATCH_NEXT();
        case ByteTerm::TypeAlternativeDisjunction:
        case ByteTerm::TypeAlternativeEnd: {
            int offset = currentTerm().alternative.end;
            BackTrackInfoAlternative* backTrack = reinterpret_cast<BackTrackInfoAlternative*>(context->frame + currentTerm().frameLocation);
            backTrack->offset = offset;
            context->term += offset;
            MATCH_NEXT();
        }

        case ByteTerm::TypeAssertionBOL:
            if (matchAssertionBOL(currentTerm()))
                MATCH_NEXT();
            BACKTRACK();
        case ByteTerm::TypeAssertionEOL:
            if (matchAssertionEOL(currentTerm()))
                MATCH_NEXT();
            BACKTRACK();
        case ByteTerm::TypeAssertionWordBoundary:
            if (matchAssertionWordBoundary(currentTerm()))
                MATCH_NEXT();
            BACKTRACK();

        case ByteTerm::TypePatternCharacterOnce:
        case ByteTerm::TypePatternCharacterFixed: {
            if (unicode) {
                if (!U_IS_BMP(currentTerm().atom.patternCharacter)) {
                    for (unsigned matchAmount = 0; matchAmount < currentTerm().atom.quantityMaxCount; ++matchAmount) {
                        if (!checkSurrogatePair(currentTerm().atom.patternCharacter, currentTerm().inputPosition - 2 * matchAmount)) {
                            BACKTRACK();
                        }
                    }
                    MATCH_NEXT();
                }
            }
            unsigned position = input.getPos(); // May need to back out reading a surrogate pair.

            for (unsigned matchAmount = 0; matchAmount < currentTerm().atom.quantityMaxCount; ++matchAmount) {
                if (!checkCharacter(currentTerm().atom.patternCharacter, currentTerm().inputPosition - matchAmount)) {
                    input.setPos(position);
                    BACKTRACK();
                }
            }
            MATCH_NEXT();
        }
        case ByteTerm::TypePatternCharacterGreedy: {
            BackTrackInfoPatternCharacter* backTrack = reinterpret_cast<BackTrackInfoPatternCharacter*>(context->frame + currentTerm().frameLocation);
            unsigned matchAmount = 0;
            unsigned position = input.getPos(); // May need to back out reading a surrogate pair.
            while ((matchAmount < currentTerm().atom.quantityMaxCount) && input.checkInput(1)) {
                if (!checkCharacter(currentTerm().atom.patternCharacter, currentTerm().inputPosition + 1)) {
                    input.setPos(position);
                    break;
                }
                ++matchAmount;
                position = input.getPos();
            }
            backTrack->matchAmount = matchAmount;

            MATCH_NEXT();
        }
        case ByteTerm::TypePatternCharacterNonGreedy: {
            BackTrackInfoPatternCharacter* backTrack = reinterpret_cast<BackTrackInfoPatternCharacter*>(context->frame + currentTerm().frameLocation);
            backTrack->begin = input.getPos();
            backTrack->matchAmount = 0;
            MATCH_NEXT();
        }

        case ByteTerm::TypePatternCasedCharacterOnce:
        case ByteTerm::TypePatternCasedCharacterFixed: {
            if (unicode) {
                // Case insensitive matching of unicode characters is handled as TypeCharacterClass.
                ASSERT(U_IS_BMP(currentTerm().atom.patternCharacter));

                unsigned position = input.getPos(); // May need to back out reading a surrogate pair.
                
                for (unsigned matchAmount = 0; matchAmount < currentTerm().atom.quantityMaxCount; ++matchAmount) {
                    if (!checkCasedCharacter(currentTerm().atom.casedCharacter.lo, currentTerm().atom.casedCharacter.hi, currentTerm().inputPosition - matchAmount)) {
                        input.setPos(position);
                        BACKTRACK();
                    }
                }
                MATCH_NEXT();
            }

            for (unsigned matchAmount = 0; matchAmount < currentTerm().atom.quantityMaxCount; ++matchAmount) {
                if (!checkCasedCharacter(currentTerm().atom.casedCharacter.lo, currentTerm().atom.casedCharacter.hi, currentTerm().inputPosition - matchAmount))
                    BACKTRACK();
            }
            MATCH_NEXT();
        }
        case ByteTerm::TypePatternCasedCharacterGreedy: {
            BackTrackInfoPatternCharacter* backTrack = reinterpret_cast<BackTrackInfoPatternCharacter*>(context->frame + currentTerm().frameLocation);

            // Case insensitive matching of unicode characters is handled as TypeCharacterClass.
            ASSERT(!unicode || U_IS_BMP(currentTerm().atom.patternCharacter));

            unsigned matchAmount = 0;
            while ((matchAmount < currentTerm().atom.quantityMaxCount) && input.checkInput(1)) {
                if (!checkCasedCharacter(currentTerm().atom.casedCharacter.lo, currentTerm().atom.casedCharacter.hi, currentTerm().inputPosition + 1)) {
                    input.uncheckInput(1);
                    break;
                }
                ++matchAmount;
            }
            backTrack->matchAmount = matchAmount;

            MATCH_NEXT();
        }
        case ByteTerm::TypePatternCasedCharacterNonGreedy: {
            BackTrackInfoPatternCharacter* backTrack = reinterpret_cast<BackTrackInfoPatternCharacter*>(context->frame + currentTerm().frameLocation);

            // Case insensitive matching of unicode characters is handled as TypeCharacterClass.
            ASSERT(!unicode || U_IS_BMP(currentTerm().atom.patternCharacter));
            
            backTrack->matchAmount = 0;
            MATCH_NEXT();
        }

        case ByteTerm::TypeCharacterClass:
            if (matchCharacterClass(currentTerm(), context))
                MATCH_NEXT();
            BACKTRACK();
        case ByteTerm::TypeBackReference:
            if (matchBackReference(currentTerm(), context))
                MATCH_NEXT();
            BACKTRACK();
        case ByteTerm::TypeParenthesesSubpattern: {
            JSRegExpResult result = matchParentheses(currentTerm(), context);

            if (result == JSRegExpMatch) {
                MATCH_NEXT();
            }  else if (result != JSRegExpNoMatch)
                return result;

            BACKTRACK();
        }
        case ByteTerm::TypeParenthesesSubpatternOnceBegin:
            if (matchParenthesesOnceBegin(currentTerm(), context))
                MATCH_NEXT();
            BACKTRACK();
        case ByteTerm::TypeParenthesesSubpatternOnceEnd:
            if (matchParenthesesOnceEnd(currentTerm(), context))
                MATCH_NEXT();
            BACKTRACK();
        case ByteTerm::TypeParenthesesSubpatternTerminalBegin:
            if (matchParenthesesTerminalBegin(currentTerm(), context))
                MATCH_NEXT();
            BACKTRACK();
        case ByteTerm::TypeParenthesesSubpatternTerminalEnd:
            if (matchParenthesesTerminalEnd(currentTerm(), context))
                MATCH_NEXT();
            BACKTRACK();
        case ByteTerm::TypeParentheticalAssertionBegin:
            if (matchParentheticalAssertionBegin(currentTerm(), context))
                MATCH_NEXT();
            BACKTRACK();
        case ByteTerm::TypeParentheticalAssertionEnd:
            if (matchParentheticalAssertionEnd(currentTerm(), context))
                MATCH_NEXT();
            BACKTRACK();

        case ByteTerm::TypeCheckInput:
            if (input.checkInput(currentTerm().checkInputCount))
                MATCH_NEXT();
            BACKTRACK();

        case ByteTerm::TypeUncheckInput:
            input.uncheckInput(currentTerm().checkInputCount);
            MATCH_NEXT();
                
        case ByteTerm::TypeDotStarEnclosure:
            if (matchDotStarEnclosure(currentTerm(), context))
                return JSRegExpMatch;
            BACKTRACK();
        }

        // We should never fall-through to here.
        RELEASE_ASSERT_NOT_REACHED();

    backtrack:
        ASSERT(context->term < static_cast<int>(disjunction->terms.size()));

        switch (currentTerm().type) {
        case ByteTerm::TypeSubpatternBegin:
            return JSRegExpNoMatch;
        case ByteTerm::TypeSubpatternEnd:
            RELEASE_ASSERT_NOT_REACHED();

        case ByteTerm::TypeBodyAlternativeBegin:
        case ByteTerm::TypeBodyAlternativeDisjunction: {
            int offset = currentTerm().alternative.next;
            context->term += offset;
            if (offset > 0)
                MATCH_NEXT();

            if (input.atEnd() || pattern->sticky())
                return JSRegExpNoMatch;

            input.next();

            context->matchBegin = input.getPos();

            if (currentTerm().alternative.onceThrough)
                context->term += currentTerm().alternative.next;

            MATCH_NEXT();
        }
        case ByteTerm::TypeBodyAlternativeEnd:
            RELEASE_ASSERT_NOT_REACHED();

        case ByteTerm::TypeAlternativeBegin:
        case ByteTerm::TypeAlternativeDisjunction: {
            int offset = currentTerm().alternative.next;
            context->term += offset;
            if (offset > 0)
                MATCH_NEXT();
            BACKTRACK();
        }
        case ByteTerm::TypeAlternativeEnd: {
            // We should never backtrack back into an alternative of the main body of the regex.
            BackTrackInfoAlternative* backTrack = reinterpret_cast<BackTrackInfoAlternative*>(context->frame + currentTerm().frameLocation);
            unsigned offset = backTrack->offset;
            context->term -= offset;
            BACKTRACK();
        }

        case ByteTerm::TypeAssertionBOL:
        case ByteTerm::TypeAssertionEOL:
        case ByteTerm::TypeAssertionWordBoundary:
            BACKTRACK();

        case ByteTerm::TypePatternCharacterOnce:
        case ByteTerm::TypePatternCharacterFixed:
        case ByteTerm::TypePatternCharacterGreedy:
        case ByteTerm::TypePatternCharacterNonGreedy:
            if (backtrackPatternCharacter(currentTerm(), context))
                MATCH_NEXT();
            BACKTRACK();
        case ByteTerm::TypePatternCasedCharacterOnce:
        case ByteTerm::TypePatternCasedCharacterFixed:
        case ByteTerm::TypePatternCasedCharacterGreedy:
        case ByteTerm::TypePatternCasedCharacterNonGreedy:
            if (backtrackPatternCasedCharacter(currentTerm(), context))
                MATCH_NEXT();
            BACKTRACK();
        case ByteTerm::TypeCharacterClass:
            if (backtrackCharacterClass(currentTerm(), context))
                MATCH_NEXT();
            BACKTRACK();
        case ByteTerm::TypeBackReference:
            if (backtrackBackReference(currentTerm(), context))
                MATCH_NEXT();
            BACKTRACK();
        case ByteTerm::TypeParenthesesSubpattern: {
            JSRegExpResult result = backtrackParentheses(currentTerm(), context);

            if (result == JSRegExpMatch) {
                MATCH_NEXT();
            } else if (result != JSRegExpNoMatch)
                return result;

            BACKTRACK();
        }
        case ByteTerm::TypeParenthesesSubpatternOnceBegin:
            if (backtrackParenthesesOnceBegin(currentTerm(), context))
                MATCH_NEXT();
            BACKTRACK();
        case ByteTerm::TypeParenthesesSubpatternOnceEnd:
            if (backtrackParenthesesOnceEnd(currentTerm(), context))
                MATCH_NEXT();
            BACKTRACK();
        case ByteTerm::TypeParenthesesSubpatternTerminalBegin:
            if (backtrackParenthesesTerminalBegin(currentTerm(), context))
                MATCH_NEXT();
            BACKTRACK();
        case ByteTerm::TypeParenthesesSubpatternTerminalEnd:
            if (backtrackParenthesesTerminalEnd(currentTerm(), context))
                MATCH_NEXT();
            BACKTRACK();
        case ByteTerm::TypeParentheticalAssertionBegin:
            if (backtrackParentheticalAssertionBegin(currentTerm(), context))
                MATCH_NEXT();
            BACKTRACK();
        case ByteTerm::TypeParentheticalAssertionEnd:
            if (backtrackParentheticalAssertionEnd(currentTerm(), context))
                MATCH_NEXT();
            BACKTRACK();

        case ByteTerm::TypeCheckInput:
            input.uncheckInput(currentTerm().checkInputCount);
            BACKTRACK();

        case ByteTerm::TypeUncheckInput:
            input.checkInput(currentTerm().checkInputCount);
            BACKTRACK();

        case ByteTerm::TypeDotStarEnclosure:
            RELEASE_ASSERT_NOT_REACHED();
        }

        RELEASE_ASSERT_NOT_REACHED();
        return JSRegExpErrorNoMatch;
    }

    JSRegExpResult matchNonZeroDisjunction(ByteDisjunction* disjunction, DisjunctionContext* context, bool btrack = false)
    {
        JSRegExpResult result = matchDisjunction(disjunction, context, btrack);

        if (result == JSRegExpMatch) {
            while (context->matchBegin == context->matchEnd) {
                result = matchDisjunction(disjunction, context, true);
                if (result != JSRegExpMatch)
                    return result;
            }
            return JSRegExpMatch;
        }

        return result;
    }

    unsigned interpret()
    {
        if (!input.isAvailableInput(0))
            return offsetNoMatch;

        if (pattern->m_lock)
            pattern->m_lock->lock();
        
        for (unsigned i = 0; i < pattern->m_body->m_numSubpatterns + 1; ++i)
            output[i << 1] = offsetNoMatch;

        allocatorPool = pattern->m_allocator->startAllocator();
        RELEASE_ASSERT(allocatorPool);

        DisjunctionContext* context = allocDisjunctionContext(pattern->m_body.get());

        JSRegExpResult result = matchDisjunction(pattern->m_body.get(), context, false);
        if (result == JSRegExpMatch) {
            output[0] = context->matchBegin;
            output[1] = context->matchEnd;
        }

        freeDisjunctionContext(context);

        pattern->m_allocator->stopAllocator();

        ASSERT((result == JSRegExpMatch) == (output[0] != offsetNoMatch));

        if (pattern->m_lock)
            pattern->m_lock->unlock();
        
        return output[0];
    }

    Interpreter(BytecodePattern* pattern, unsigned* output, const CharType* input, unsigned length, unsigned start)
        : pattern(pattern)
        , unicode(pattern->unicode())
        , output(output)
        , input(input, start, length, pattern->unicode())
        , allocatorPool(0)
        , remainingMatchCount(matchLimit)
    {
    }

private:
    BytecodePattern* pattern;
    bool unicode;
    unsigned* output;
    InputStream input;
    BumpPointerPool* allocatorPool;
    unsigned remainingMatchCount;
};

class ByteCompiler {
    struct ParenthesesStackEntry {
        unsigned beginTerm;
        unsigned savedAlternativeIndex;
        ParenthesesStackEntry(unsigned beginTerm, unsigned savedAlternativeIndex/*, unsigned subpatternId, bool capture = false*/)
            : beginTerm(beginTerm)
            , savedAlternativeIndex(savedAlternativeIndex)
        {
        }
    };

public:
    ByteCompiler(YarrPattern& pattern)
        : m_pattern(pattern)
    {
        m_currentAlternativeIndex = 0;
    }

    std::unique_ptr<BytecodePattern> compile(BumpPointerAllocator* allocator, ConcurrentJSLock* lock)
    {
        regexBegin(m_pattern.m_numSubpatterns, m_pattern.m_body->m_callFrameSize, m_pattern.m_body->m_alternatives[0]->onceThrough());
        emitDisjunction(m_pattern.m_body);
        regexEnd();

        return std::make_unique<BytecodePattern>(WTFMove(m_bodyDisjunction), m_allParenthesesInfo, m_pattern, allocator, lock);
    }

    void checkInput(unsigned count)
    {
        m_bodyDisjunction->terms.append(ByteTerm::CheckInput(count));
    }

    void uncheckInput(unsigned count)
    {
        m_bodyDisjunction->terms.append(ByteTerm::UncheckInput(count));
    }
    
    void assertionBOL(unsigned inputPosition)
    {
        m_bodyDisjunction->terms.append(ByteTerm::BOL(inputPosition));
    }

    void assertionEOL(unsigned inputPosition)
    {
        m_bodyDisjunction->terms.append(ByteTerm::EOL(inputPosition));
    }

    void assertionWordBoundary(bool invert, unsigned inputPosition)
    {
        m_bodyDisjunction->terms.append(ByteTerm::WordBoundary(invert, inputPosition));
    }

    void atomPatternCharacter(UChar32 ch, unsigned inputPosition, unsigned frameLocation, Checked<unsigned> quantityMaxCount, QuantifierType quantityType)
    {
        if (m_pattern.ignoreCase()) {
            UChar32 lo = u_tolower(ch);
            UChar32 hi = u_toupper(ch);

            if (lo != hi) {
                m_bodyDisjunction->terms.append(ByteTerm(lo, hi, inputPosition, frameLocation, quantityMaxCount, quantityType));
                return;
            }
        }

        m_bodyDisjunction->terms.append(ByteTerm(ch, inputPosition, frameLocation, quantityMaxCount, quantityType));
    }

    void atomCharacterClass(CharacterClass* characterClass, bool invert, unsigned inputPosition, unsigned frameLocation, Checked<unsigned> quantityMaxCount, QuantifierType quantityType)
    {
        m_bodyDisjunction->terms.append(ByteTerm(characterClass, invert, inputPosition));

        m_bodyDisjunction->terms[m_bodyDisjunction->terms.size() - 1].atom.quantityMaxCount = quantityMaxCount.unsafeGet();
        m_bodyDisjunction->terms[m_bodyDisjunction->terms.size() - 1].atom.quantityType = quantityType;
        m_bodyDisjunction->terms[m_bodyDisjunction->terms.size() - 1].frameLocation = frameLocation;
    }

    void atomBackReference(unsigned subpatternId, unsigned inputPosition, unsigned frameLocation, Checked<unsigned> quantityMaxCount, QuantifierType quantityType)
    {
        ASSERT(subpatternId);

        m_bodyDisjunction->terms.append(ByteTerm::BackReference(subpatternId, inputPosition));

        m_bodyDisjunction->terms[m_bodyDisjunction->terms.size() - 1].atom.quantityMaxCount = quantityMaxCount.unsafeGet();
        m_bodyDisjunction->terms[m_bodyDisjunction->terms.size() - 1].atom.quantityType = quantityType;
        m_bodyDisjunction->terms[m_bodyDisjunction->terms.size() - 1].frameLocation = frameLocation;
    }

    void atomParenthesesOnceBegin(unsigned subpatternId, bool capture, unsigned inputPosition, unsigned frameLocation, unsigned alternativeFrameLocation)
    {
        int beginTerm = m_bodyDisjunction->terms.size();

        m_bodyDisjunction->terms.append(ByteTerm(ByteTerm::TypeParenthesesSubpatternOnceBegin, subpatternId, capture, false, inputPosition));
        m_bodyDisjunction->terms[m_bodyDisjunction->terms.size() - 1].frameLocation = frameLocation;
        m_bodyDisjunction->terms.append(ByteTerm::AlternativeBegin());
        m_bodyDisjunction->terms[m_bodyDisjunction->terms.size() - 1].frameLocation = alternativeFrameLocation;

        m_parenthesesStack.append(ParenthesesStackEntry(beginTerm, m_currentAlternativeIndex));
        m_currentAlternativeIndex = beginTerm + 1;
    }

    void atomParenthesesTerminalBegin(unsigned subpatternId, bool capture, unsigned inputPosition, unsigned frameLocation, unsigned alternativeFrameLocation)
    {
        int beginTerm = m_bodyDisjunction->terms.size();

        m_bodyDisjunction->terms.append(ByteTerm(ByteTerm::TypeParenthesesSubpatternTerminalBegin, subpatternId, capture, false, inputPosition));
        m_bodyDisjunction->terms[m_bodyDisjunction->terms.size() - 1].frameLocation = frameLocation;
        m_bodyDisjunction->terms.append(ByteTerm::AlternativeBegin());
        m_bodyDisjunction->terms[m_bodyDisjunction->terms.size() - 1].frameLocation = alternativeFrameLocation;

        m_parenthesesStack.append(ParenthesesStackEntry(beginTerm, m_currentAlternativeIndex));
        m_currentAlternativeIndex = beginTerm + 1;
    }

    void atomParenthesesSubpatternBegin(unsigned subpatternId, bool capture, unsigned inputPosition, unsigned frameLocation, unsigned alternativeFrameLocation)
    {
        // Errrk! - this is a little crazy, we initially generate as a TypeParenthesesSubpatternOnceBegin,
        // then fix this up at the end! - simplifying this should make it much clearer.
        // https://bugs.webkit.org/show_bug.cgi?id=50136

        int beginTerm = m_bodyDisjunction->terms.size();

        m_bodyDisjunction->terms.append(ByteTerm(ByteTerm::TypeParenthesesSubpatternOnceBegin, subpatternId, capture, false, inputPosition));
        m_bodyDisjunction->terms[m_bodyDisjunction->terms.size() - 1].frameLocation = frameLocation;
        m_bodyDisjunction->terms.append(ByteTerm::AlternativeBegin());
        m_bodyDisjunction->terms[m_bodyDisjunction->terms.size() - 1].frameLocation = alternativeFrameLocation;

        m_parenthesesStack.append(ParenthesesStackEntry(beginTerm, m_currentAlternativeIndex));
        m_currentAlternativeIndex = beginTerm + 1;
    }

    void atomParentheticalAssertionBegin(unsigned subpatternId, bool invert, unsigned frameLocation, unsigned alternativeFrameLocation)
    {
        int beginTerm = m_bodyDisjunction->terms.size();

        m_bodyDisjunction->terms.append(ByteTerm(ByteTerm::TypeParentheticalAssertionBegin, subpatternId, false, invert, 0));
        m_bodyDisjunction->terms[m_bodyDisjunction->terms.size() - 1].frameLocation = frameLocation;
        m_bodyDisjunction->terms.append(ByteTerm::AlternativeBegin());
        m_bodyDisjunction->terms[m_bodyDisjunction->terms.size() - 1].frameLocation = alternativeFrameLocation;

        m_parenthesesStack.append(ParenthesesStackEntry(beginTerm, m_currentAlternativeIndex));
        m_currentAlternativeIndex = beginTerm + 1;
    }

    void atomParentheticalAssertionEnd(unsigned inputPosition, unsigned frameLocation, Checked<unsigned> quantityMaxCount, QuantifierType quantityType)
    {
        unsigned beginTerm = popParenthesesStack();
        closeAlternative(beginTerm + 1);
        unsigned endTerm = m_bodyDisjunction->terms.size();

        ASSERT(m_bodyDisjunction->terms[beginTerm].type == ByteTerm::TypeParentheticalAssertionBegin);

        bool invert = m_bodyDisjunction->terms[beginTerm].invert();
        unsigned subpatternId = m_bodyDisjunction->terms[beginTerm].atom.subpatternId;

        m_bodyDisjunction->terms.append(ByteTerm(ByteTerm::TypeParentheticalAssertionEnd, subpatternId, false, invert, inputPosition));
        m_bodyDisjunction->terms[beginTerm].atom.parenthesesWidth = endTerm - beginTerm;
        m_bodyDisjunction->terms[endTerm].atom.parenthesesWidth = endTerm - beginTerm;
        m_bodyDisjunction->terms[endTerm].frameLocation = frameLocation;

        m_bodyDisjunction->terms[beginTerm].atom.quantityMaxCount = quantityMaxCount.unsafeGet();
        m_bodyDisjunction->terms[beginTerm].atom.quantityType = quantityType;
        m_bodyDisjunction->terms[endTerm].atom.quantityMaxCount = quantityMaxCount.unsafeGet();
        m_bodyDisjunction->terms[endTerm].atom.quantityType = quantityType;
    }

    void assertionDotStarEnclosure(bool bolAnchored, bool eolAnchored)
    {
        m_bodyDisjunction->terms.append(ByteTerm::DotStarEnclosure(bolAnchored, eolAnchored));
    }

    unsigned popParenthesesStack()
    {
        ASSERT(m_parenthesesStack.size());
        int stackEnd = m_parenthesesStack.size() - 1;
        unsigned beginTerm = m_parenthesesStack[stackEnd].beginTerm;
        m_currentAlternativeIndex = m_parenthesesStack[stackEnd].savedAlternativeIndex;
        m_parenthesesStack.shrink(stackEnd);

        ASSERT(beginTerm < m_bodyDisjunction->terms.size());
        ASSERT(m_currentAlternativeIndex < m_bodyDisjunction->terms.size());

        return beginTerm;
    }

#ifndef NDEBUG
    void dumpDisjunction(ByteDisjunction* disjunction)
    {
        dataLogF("ByteDisjunction(%p):\n\t", disjunction);
        for (unsigned i = 0; i < disjunction->terms.size(); ++i)
            dataLogF("{ %d } ", disjunction->terms[i].type);
        dataLogF("\n");
    }
#endif

    void closeAlternative(int beginTerm)
    {
        int origBeginTerm = beginTerm;
        ASSERT(m_bodyDisjunction->terms[beginTerm].type == ByteTerm::TypeAlternativeBegin);
        int endIndex = m_bodyDisjunction->terms.size();

        unsigned frameLocation = m_bodyDisjunction->terms[beginTerm].frameLocation;

        if (!m_bodyDisjunction->terms[beginTerm].alternative.next)
            m_bodyDisjunction->terms.remove(beginTerm);
        else {
            while (m_bodyDisjunction->terms[beginTerm].alternative.next) {
                beginTerm += m_bodyDisjunction->terms[beginTerm].alternative.next;
                ASSERT(m_bodyDisjunction->terms[beginTerm].type == ByteTerm::TypeAlternativeDisjunction);
                m_bodyDisjunction->terms[beginTerm].alternative.end = endIndex - beginTerm;
                m_bodyDisjunction->terms[beginTerm].frameLocation = frameLocation;
            }

            m_bodyDisjunction->terms[beginTerm].alternative.next = origBeginTerm - beginTerm;

            m_bodyDisjunction->terms.append(ByteTerm::AlternativeEnd());
            m_bodyDisjunction->terms[endIndex].frameLocation = frameLocation;
        }
    }

    void closeBodyAlternative()
    {
        int beginTerm = 0;
        int origBeginTerm = 0;
        ASSERT(m_bodyDisjunction->terms[beginTerm].type == ByteTerm::TypeBodyAlternativeBegin);
        int endIndex = m_bodyDisjunction->terms.size();

        unsigned frameLocation = m_bodyDisjunction->terms[beginTerm].frameLocation;

        while (m_bodyDisjunction->terms[beginTerm].alternative.next) {
            beginTerm += m_bodyDisjunction->terms[beginTerm].alternative.next;
            ASSERT(m_bodyDisjunction->terms[beginTerm].type == ByteTerm::TypeBodyAlternativeDisjunction);
            m_bodyDisjunction->terms[beginTerm].alternative.end = endIndex - beginTerm;
            m_bodyDisjunction->terms[beginTerm].frameLocation = frameLocation;
        }

        m_bodyDisjunction->terms[beginTerm].alternative.next = origBeginTerm - beginTerm;

        m_bodyDisjunction->terms.append(ByteTerm::BodyAlternativeEnd());
        m_bodyDisjunction->terms[endIndex].frameLocation = frameLocation;
    }

    void atomParenthesesSubpatternEnd(unsigned lastSubpatternId, unsigned inputPosition, unsigned frameLocation, Checked<unsigned> quantityMinCount, Checked<unsigned> quantityMaxCount, QuantifierType quantityType, unsigned callFrameSize = 0)
    {
        unsigned beginTerm = popParenthesesStack();
        closeAlternative(beginTerm + 1);
        unsigned endTerm = m_bodyDisjunction->terms.size();

        ASSERT(m_bodyDisjunction->terms[beginTerm].type == ByteTerm::TypeParenthesesSubpatternOnceBegin);

        ByteTerm& parenthesesBegin = m_bodyDisjunction->terms[beginTerm];

        bool capture = parenthesesBegin.capture();
        unsigned subpatternId = parenthesesBegin.atom.subpatternId;

        unsigned numSubpatterns = lastSubpatternId - subpatternId + 1;
        auto parenthesesDisjunction = std::make_unique<ByteDisjunction>(numSubpatterns, callFrameSize);

        unsigned firstTermInParentheses = beginTerm + 1;
        parenthesesDisjunction->terms.reserveInitialCapacity(endTerm - firstTermInParentheses + 2);

        parenthesesDisjunction->terms.append(ByteTerm::SubpatternBegin());
        for (unsigned termInParentheses = firstTermInParentheses; termInParentheses < endTerm; ++termInParentheses)
            parenthesesDisjunction->terms.append(m_bodyDisjunction->terms[termInParentheses]);
        parenthesesDisjunction->terms.append(ByteTerm::SubpatternEnd());

        m_bodyDisjunction->terms.shrink(beginTerm);

        m_bodyDisjunction->terms.append(ByteTerm(ByteTerm::TypeParenthesesSubpattern, subpatternId, parenthesesDisjunction.get(), capture, inputPosition));
        m_allParenthesesInfo.append(WTFMove(parenthesesDisjunction));

        m_bodyDisjunction->terms[beginTerm].atom.quantityMinCount = quantityMinCount.unsafeGet();
        m_bodyDisjunction->terms[beginTerm].atom.quantityMaxCount = quantityMaxCount.unsafeGet();
        m_bodyDisjunction->terms[beginTerm].atom.quantityType = quantityType;
        m_bodyDisjunction->terms[beginTerm].frameLocation = frameLocation;
    }

    void atomParenthesesOnceEnd(unsigned inputPosition, unsigned frameLocation, Checked<unsigned> quantityMinCount, Checked<unsigned> quantityMaxCount, QuantifierType quantityType)
    {
        unsigned beginTerm = popParenthesesStack();
        closeAlternative(beginTerm + 1);
        unsigned endTerm = m_bodyDisjunction->terms.size();

        ASSERT(m_bodyDisjunction->terms[beginTerm].type == ByteTerm::TypeParenthesesSubpatternOnceBegin);

        bool capture = m_bodyDisjunction->terms[beginTerm].capture();
        unsigned subpatternId = m_bodyDisjunction->terms[beginTerm].atom.subpatternId;

        m_bodyDisjunction->terms.append(ByteTerm(ByteTerm::TypeParenthesesSubpatternOnceEnd, subpatternId, capture, false, inputPosition));
        m_bodyDisjunction->terms[beginTerm].atom.parenthesesWidth = endTerm - beginTerm;
        m_bodyDisjunction->terms[endTerm].atom.parenthesesWidth = endTerm - beginTerm;
        m_bodyDisjunction->terms[endTerm].frameLocation = frameLocation;

        m_bodyDisjunction->terms[beginTerm].atom.quantityMinCount = quantityMinCount.unsafeGet();
        m_bodyDisjunction->terms[beginTerm].atom.quantityMaxCount = quantityMaxCount.unsafeGet();
        m_bodyDisjunction->terms[beginTerm].atom.quantityType = quantityType;
        m_bodyDisjunction->terms[endTerm].atom.quantityMinCount = quantityMinCount.unsafeGet();
        m_bodyDisjunction->terms[endTerm].atom.quantityMaxCount = quantityMaxCount.unsafeGet();
        m_bodyDisjunction->terms[endTerm].atom.quantityType = quantityType;
    }

    void atomParenthesesTerminalEnd(unsigned inputPosition, unsigned frameLocation, Checked<unsigned> quantityMinCount, Checked<unsigned> quantityMaxCount, QuantifierType quantityType)
    {
        unsigned beginTerm = popParenthesesStack();
        closeAlternative(beginTerm + 1);
        unsigned endTerm = m_bodyDisjunction->terms.size();

        ASSERT(m_bodyDisjunction->terms[beginTerm].type == ByteTerm::TypeParenthesesSubpatternTerminalBegin);

        bool capture = m_bodyDisjunction->terms[beginTerm].capture();
        unsigned subpatternId = m_bodyDisjunction->terms[beginTerm].atom.subpatternId;

        m_bodyDisjunction->terms.append(ByteTerm(ByteTerm::TypeParenthesesSubpatternTerminalEnd, subpatternId, capture, false, inputPosition));
        m_bodyDisjunction->terms[beginTerm].atom.parenthesesWidth = endTerm - beginTerm;
        m_bodyDisjunction->terms[endTerm].atom.parenthesesWidth = endTerm - beginTerm;
        m_bodyDisjunction->terms[endTerm].frameLocation = frameLocation;

        m_bodyDisjunction->terms[beginTerm].atom.quantityMinCount = quantityMinCount.unsafeGet();
        m_bodyDisjunction->terms[beginTerm].atom.quantityMaxCount = quantityMaxCount.unsafeGet();
        m_bodyDisjunction->terms[beginTerm].atom.quantityType = quantityType;
        m_bodyDisjunction->terms[endTerm].atom.quantityMinCount = quantityMinCount.unsafeGet();
        m_bodyDisjunction->terms[endTerm].atom.quantityMaxCount = quantityMaxCount.unsafeGet();
        m_bodyDisjunction->terms[endTerm].atom.quantityType = quantityType;
    }

    void regexBegin(unsigned numSubpatterns, unsigned callFrameSize, bool onceThrough)
    {
        m_bodyDisjunction = std::make_unique<ByteDisjunction>(numSubpatterns, callFrameSize);
        m_bodyDisjunction->terms.append(ByteTerm::BodyAlternativeBegin(onceThrough));
        m_bodyDisjunction->terms[0].frameLocation = 0;
        m_currentAlternativeIndex = 0;
    }

    void regexEnd()
    {
        closeBodyAlternative();
    }

    void alternativeBodyDisjunction(bool onceThrough)
    {
        int newAlternativeIndex = m_bodyDisjunction->terms.size();
        m_bodyDisjunction->terms[m_currentAlternativeIndex].alternative.next = newAlternativeIndex - m_currentAlternativeIndex;
        m_bodyDisjunction->terms.append(ByteTerm::BodyAlternativeDisjunction(onceThrough));

        m_currentAlternativeIndex = newAlternativeIndex;
    }

    void alternativeDisjunction()
    {
        int newAlternativeIndex = m_bodyDisjunction->terms.size();
        m_bodyDisjunction->terms[m_currentAlternativeIndex].alternative.next = newAlternativeIndex - m_currentAlternativeIndex;
        m_bodyDisjunction->terms.append(ByteTerm::AlternativeDisjunction());

        m_currentAlternativeIndex = newAlternativeIndex;
    }

    void emitDisjunction(PatternDisjunction* disjunction, unsigned inputCountAlreadyChecked = 0, unsigned parenthesesInputCountAlreadyChecked = 0)
    {
        for (unsigned alt = 0; alt < disjunction->m_alternatives.size(); ++alt) {
            unsigned currentCountAlreadyChecked = inputCountAlreadyChecked;

            PatternAlternative* alternative = disjunction->m_alternatives[alt].get();

            if (alt) {
                if (disjunction == m_pattern.m_body)
                    alternativeBodyDisjunction(alternative->onceThrough());
                else
                    alternativeDisjunction();
            }

            unsigned minimumSize = alternative->m_minimumSize;
            ASSERT(minimumSize >= parenthesesInputCountAlreadyChecked);
            unsigned countToCheck = minimumSize - parenthesesInputCountAlreadyChecked;

            if (countToCheck) {
                checkInput(countToCheck);
                currentCountAlreadyChecked += countToCheck;
            }

            for (auto& term : alternative->m_terms) {
                switch (term.type) {
                case PatternTerm::TypeAssertionBOL:
                    assertionBOL(currentCountAlreadyChecked - term.inputPosition);
                    break;

                case PatternTerm::TypeAssertionEOL:
                    assertionEOL(currentCountAlreadyChecked - term.inputPosition);
                    break;

                case PatternTerm::TypeAssertionWordBoundary:
                    assertionWordBoundary(term.invert(), currentCountAlreadyChecked - term.inputPosition);
                    break;

                case PatternTerm::TypePatternCharacter:
                    atomPatternCharacter(term.patternCharacter, currentCountAlreadyChecked - term.inputPosition, term.frameLocation, term.quantityMaxCount, term.quantityType);
                    break;

                case PatternTerm::TypeCharacterClass:
                    atomCharacterClass(term.characterClass, term.invert(), currentCountAlreadyChecked- term.inputPosition, term.frameLocation, term.quantityMaxCount, term.quantityType);
                    break;

                case PatternTerm::TypeBackReference:
                    atomBackReference(term.backReferenceSubpatternId, currentCountAlreadyChecked - term.inputPosition, term.frameLocation, term.quantityMaxCount, term.quantityType);
                        break;

                case PatternTerm::TypeForwardReference:
                    break;

                case PatternTerm::TypeParenthesesSubpattern: {
                    unsigned disjunctionAlreadyCheckedCount = 0;
                    if (term.quantityMaxCount == 1 && !term.parentheses.isCopy) {
                        unsigned alternativeFrameLocation = term.frameLocation;
                        // For QuantifierFixedCount we pre-check the minimum size; for greedy/non-greedy we reserve a slot in the frame.
                        if (term.quantityType == QuantifierFixedCount)
                            disjunctionAlreadyCheckedCount = term.parentheses.disjunction->m_minimumSize;
                        else
                            alternativeFrameLocation += YarrStackSpaceForBackTrackInfoParenthesesOnce;
                        ASSERT(currentCountAlreadyChecked >= term.inputPosition);
                        unsigned delegateEndInputOffset = currentCountAlreadyChecked - term.inputPosition;
                        atomParenthesesOnceBegin(term.parentheses.subpatternId, term.capture(), disjunctionAlreadyCheckedCount + delegateEndInputOffset, term.frameLocation, alternativeFrameLocation);
                        emitDisjunction(term.parentheses.disjunction, currentCountAlreadyChecked, disjunctionAlreadyCheckedCount);
                        atomParenthesesOnceEnd(delegateEndInputOffset, term.frameLocation, term.quantityMinCount, term.quantityMaxCount, term.quantityType);
                    } else if (term.parentheses.isTerminal) {
                        ASSERT(currentCountAlreadyChecked >= term.inputPosition);
                        unsigned delegateEndInputOffset = currentCountAlreadyChecked - term.inputPosition;
                        atomParenthesesTerminalBegin(term.parentheses.subpatternId, term.capture(), disjunctionAlreadyCheckedCount + delegateEndInputOffset, term.frameLocation, term.frameLocation + YarrStackSpaceForBackTrackInfoParenthesesOnce);
                        emitDisjunction(term.parentheses.disjunction, currentCountAlreadyChecked, disjunctionAlreadyCheckedCount);
                        atomParenthesesTerminalEnd(delegateEndInputOffset, term.frameLocation, term.quantityMinCount, term.quantityMaxCount, term.quantityType);
                    } else {
                        ASSERT(currentCountAlreadyChecked >= term.inputPosition);
                        unsigned delegateEndInputOffset = currentCountAlreadyChecked - term.inputPosition;
                        atomParenthesesSubpatternBegin(term.parentheses.subpatternId, term.capture(), disjunctionAlreadyCheckedCount + delegateEndInputOffset, term.frameLocation, 0);
                        emitDisjunction(term.parentheses.disjunction, currentCountAlreadyChecked, 0);
                        atomParenthesesSubpatternEnd(term.parentheses.lastSubpatternId, delegateEndInputOffset, term.frameLocation, term.quantityMinCount, term.quantityMaxCount, term.quantityType, term.parentheses.disjunction->m_callFrameSize);
                    }
                    break;
                }

                case PatternTerm::TypeParentheticalAssertion: {
                    unsigned alternativeFrameLocation = term.frameLocation + YarrStackSpaceForBackTrackInfoParentheticalAssertion;

                    ASSERT(currentCountAlreadyChecked >= term.inputPosition);
                    unsigned positiveInputOffset = currentCountAlreadyChecked - term.inputPosition;
                    unsigned uncheckAmount = 0;
                    if (positiveInputOffset > term.parentheses.disjunction->m_minimumSize) {
                        uncheckAmount = positiveInputOffset - term.parentheses.disjunction->m_minimumSize;
                        uncheckInput(uncheckAmount);
                        currentCountAlreadyChecked -= uncheckAmount;
                    }

                    atomParentheticalAssertionBegin(term.parentheses.subpatternId, term.invert(), term.frameLocation, alternativeFrameLocation);
                    emitDisjunction(term.parentheses.disjunction, currentCountAlreadyChecked, positiveInputOffset - uncheckAmount);
                    atomParentheticalAssertionEnd(0, term.frameLocation, term.quantityMaxCount, term.quantityType);
                    if (uncheckAmount) {
                        checkInput(uncheckAmount);
                        currentCountAlreadyChecked += uncheckAmount;
                    }
                    break;
                }

                case PatternTerm::TypeDotStarEnclosure:
                    assertionDotStarEnclosure(term.anchors.bolAnchor, term.anchors.eolAnchor);
                    break;
                }
            }
        }
    }

private:
    YarrPattern& m_pattern;
    std::unique_ptr<ByteDisjunction> m_bodyDisjunction;
    unsigned m_currentAlternativeIndex;
    Vector<ParenthesesStackEntry> m_parenthesesStack;
    Vector<std::unique_ptr<ByteDisjunction>> m_allParenthesesInfo;
};

std::unique_ptr<BytecodePattern> byteCompile(YarrPattern& pattern, BumpPointerAllocator* allocator, ConcurrentJSLock* lock)
{
    return ByteCompiler(pattern).compile(allocator, lock);
}

unsigned interpret(BytecodePattern* bytecode, const String& input, unsigned start, unsigned* output)
{
    SuperSamplerScope superSamplerScope(false);
    if (input.is8Bit())
        return Interpreter<LChar>(bytecode, output, input.characters8(), input.length(), start).interpret();
    return Interpreter<UChar>(bytecode, output, input.characters16(), input.length(), start).interpret();
}

unsigned interpret(BytecodePattern* bytecode, const LChar* input, unsigned length, unsigned start, unsigned* output)
{
    SuperSamplerScope superSamplerScope(false);
    return Interpreter<LChar>(bytecode, output, input, length, start).interpret();
}

unsigned interpret(BytecodePattern* bytecode, const UChar* input, unsigned length, unsigned start, unsigned* output)
{
    SuperSamplerScope superSamplerScope(false);
    return Interpreter<UChar>(bytecode, output, input, length, start).interpret();
}

// These should be the same for both UChar & LChar.
COMPILE_ASSERT(sizeof(Interpreter<UChar>::BackTrackInfoPatternCharacter) == (YarrStackSpaceForBackTrackInfoPatternCharacter * sizeof(uintptr_t)), CheckYarrStackSpaceForBackTrackInfoPatternCharacter);
COMPILE_ASSERT(sizeof(Interpreter<UChar>::BackTrackInfoCharacterClass) == (YarrStackSpaceForBackTrackInfoCharacterClass * sizeof(uintptr_t)), CheckYarrStackSpaceForBackTrackInfoCharacterClass);
COMPILE_ASSERT(sizeof(Interpreter<UChar>::BackTrackInfoBackReference) == (YarrStackSpaceForBackTrackInfoBackReference * sizeof(uintptr_t)), CheckYarrStackSpaceForBackTrackInfoBackReference);
COMPILE_ASSERT(sizeof(Interpreter<UChar>::BackTrackInfoAlternative) == (YarrStackSpaceForBackTrackInfoAlternative * sizeof(uintptr_t)), CheckYarrStackSpaceForBackTrackInfoAlternative);
COMPILE_ASSERT(sizeof(Interpreter<UChar>::BackTrackInfoParentheticalAssertion) == (YarrStackSpaceForBackTrackInfoParentheticalAssertion * sizeof(uintptr_t)), CheckYarrStackSpaceForBackTrackInfoParentheticalAssertion);
COMPILE_ASSERT(sizeof(Interpreter<UChar>::BackTrackInfoParenthesesOnce) == (YarrStackSpaceForBackTrackInfoParenthesesOnce * sizeof(uintptr_t)), CheckYarrStackSpaceForBackTrackInfoParenthesesOnce);
COMPILE_ASSERT(sizeof(Interpreter<UChar>::BackTrackInfoParentheses) == (YarrStackSpaceForBackTrackInfoParentheses * sizeof(uintptr_t)), CheckYarrStackSpaceForBackTrackInfoParentheses);


} }
