/*
 * Copyright (C) 2009, 2013-2016 Apple Inc. All rights reserved.
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
#include "YarrPattern.h"

#include "Yarr.h"
#include "YarrCanonicalize.h"
#include "YarrParser.h"
#include <wtf/Vector.h>
#include <wtf/WTFThreadData.h>

using namespace WTF;

namespace JSC { namespace Yarr {

#include "RegExpJitTables.h"

class CharacterClassConstructor {
public:
    CharacterClassConstructor(bool isCaseInsensitive, CanonicalMode canonicalMode)
        : m_isCaseInsensitive(isCaseInsensitive)
        , m_canonicalMode(canonicalMode)
    {
    }
    
    void reset()
    {
        m_matches.clear();
        m_ranges.clear();
        m_matchesUnicode.clear();
        m_rangesUnicode.clear();
    }

    void append(const CharacterClass* other)
    {
        for (size_t i = 0; i < other->m_matches.size(); ++i)
            addSorted(m_matches, other->m_matches[i]);
        for (size_t i = 0; i < other->m_ranges.size(); ++i)
            addSortedRange(m_ranges, other->m_ranges[i].begin, other->m_ranges[i].end);
        for (size_t i = 0; i < other->m_matchesUnicode.size(); ++i)
            addSorted(m_matchesUnicode, other->m_matchesUnicode[i]);
        for (size_t i = 0; i < other->m_rangesUnicode.size(); ++i)
            addSortedRange(m_rangesUnicode, other->m_rangesUnicode[i].begin, other->m_rangesUnicode[i].end);
    }

    void putChar(UChar32 ch)
    {
        if (!m_isCaseInsensitive) {
            addSorted(ch);
            return;
        }

        if (m_canonicalMode == CanonicalMode::UCS2 && isASCII(ch)) {
            // Handle ASCII cases.
            if (isASCIIAlpha(ch)) {
                addSorted(m_matches, toASCIIUpper(ch));
                addSorted(m_matches, toASCIILower(ch));
            } else
                addSorted(m_matches, ch);
            return;
        }

        // Add multiple matches, if necessary.
        const CanonicalizationRange* info = canonicalRangeInfoFor(ch, m_canonicalMode);
        if (info->type == CanonicalizeUnique)
            addSorted(ch);
        else
            putUnicodeIgnoreCase(ch, info);
    }

    void putUnicodeIgnoreCase(UChar32 ch, const CanonicalizationRange* info)
    {
        ASSERT(m_isCaseInsensitive);
        ASSERT(ch >= info->begin && ch <= info->end);
        ASSERT(info->type != CanonicalizeUnique);
        if (info->type == CanonicalizeSet) {
            for (const UChar32* set = canonicalCharacterSetInfo(info->value, m_canonicalMode); (ch = *set); ++set)
                addSorted(ch);
        } else {
            addSorted(ch);
            addSorted(getCanonicalPair(info, ch));
        }
    }

    void putRange(UChar32 lo, UChar32 hi)
    {
        if (isASCII(lo)) {
            char asciiLo = lo;
            char asciiHi = std::min(hi, (UChar32)0x7f);
            addSortedRange(m_ranges, lo, asciiHi);
            
            if (m_isCaseInsensitive) {
                if ((asciiLo <= 'Z') && (asciiHi >= 'A'))
                    addSortedRange(m_ranges, std::max(asciiLo, 'A')+('a'-'A'), std::min(asciiHi, 'Z')+('a'-'A'));
                if ((asciiLo <= 'z') && (asciiHi >= 'a'))
                    addSortedRange(m_ranges, std::max(asciiLo, 'a')+('A'-'a'), std::min(asciiHi, 'z')+('A'-'a'));
            }
        }
        if (isASCII(hi))
            return;

        lo = std::max(lo, (UChar32)0x80);
        addSortedRange(m_rangesUnicode, lo, hi);
        
        if (!m_isCaseInsensitive)
            return;

        const CanonicalizationRange* info = canonicalRangeInfoFor(lo, m_canonicalMode);
        while (true) {
            // Handle the range [lo .. end]
            UChar32 end = std::min<UChar32>(info->end, hi);

            switch (info->type) {
            case CanonicalizeUnique:
                // Nothing to do - no canonical equivalents.
                break;
            case CanonicalizeSet: {
                UChar ch;
                for (const UChar32* set = canonicalCharacterSetInfo(info->value, m_canonicalMode); (ch = *set); ++set)
                    addSorted(m_matchesUnicode, ch);
                break;
            }
            case CanonicalizeRangeLo:
                addSortedRange(m_rangesUnicode, lo + info->value, end + info->value);
                break;
            case CanonicalizeRangeHi:
                addSortedRange(m_rangesUnicode, lo - info->value, end - info->value);
                break;
            case CanonicalizeAlternatingAligned:
                // Use addSortedRange since there is likely an abutting range to combine with.
                if (lo & 1)
                    addSortedRange(m_rangesUnicode, lo - 1, lo - 1);
                if (!(end & 1))
                    addSortedRange(m_rangesUnicode, end + 1, end + 1);
                break;
            case CanonicalizeAlternatingUnaligned:
                // Use addSortedRange since there is likely an abutting range to combine with.
                if (!(lo & 1))
                    addSortedRange(m_rangesUnicode, lo - 1, lo - 1);
                if (end & 1)
                    addSortedRange(m_rangesUnicode, end + 1, end + 1);
                break;
            }

            if (hi == end)
                return;

            ++info;
            lo = info->begin;
        };

    }

    std::unique_ptr<CharacterClass> charClass()
    {
        auto characterClass = std::make_unique<CharacterClass>();

        characterClass->m_matches.swap(m_matches);
        characterClass->m_ranges.swap(m_ranges);
        characterClass->m_matchesUnicode.swap(m_matchesUnicode);
        characterClass->m_rangesUnicode.swap(m_rangesUnicode);

        return characterClass;
    }

private:
    void addSorted(UChar32 ch)
    {
        addSorted(isASCII(ch) ? m_matches : m_matchesUnicode, ch);
    }

    void addSorted(Vector<UChar32>& matches, UChar32 ch)
    {
        unsigned pos = 0;
        unsigned range = matches.size();

        // binary chop, find position to insert char.
        while (range) {
            unsigned index = range >> 1;

            int val = matches[pos+index] - ch;
            if (!val)
                return;
            else if (val > 0)
                range = index;
            else {
                pos += (index+1);
                range -= (index+1);
            }
        }
        
        if (pos == matches.size())
            matches.append(ch);
        else
            matches.insert(pos, ch);
    }

    void addSortedRange(Vector<CharacterRange>& ranges, UChar32 lo, UChar32 hi)
    {
        unsigned end = ranges.size();
        
        // Simple linear scan - I doubt there are that many ranges anyway...
        // feel free to fix this with something faster (eg binary chop).
        for (unsigned i = 0; i < end; ++i) {
            // does the new range fall before the current position in the array
            if (hi < ranges[i].begin) {
                // optional optimization: concatenate appending ranges? - may not be worthwhile.
                if (hi == (ranges[i].begin - 1)) {
                    ranges[i].begin = lo;
                    return;
                }
                ranges.insert(i, CharacterRange(lo, hi));
                return;
            }
            // Okay, since we didn't hit the last case, the end of the new range is definitely at or after the begining
            // If the new range start at or before the end of the last range, then the overlap (if it starts one after the
            // end of the last range they concatenate, which is just as good.
            if (lo <= (ranges[i].end + 1)) {
                // found an intersect! we'll replace this entry in the array.
                ranges[i].begin = std::min(ranges[i].begin, lo);
                ranges[i].end = std::max(ranges[i].end, hi);

                // now check if the new range can subsume any subsequent ranges.
                unsigned next = i+1;
                // each iteration of the loop we will either remove something from the list, or break the loop.
                while (next < ranges.size()) {
                    if (ranges[next].begin <= (ranges[i].end + 1)) {
                        // the next entry now overlaps / concatenates this one.
                        ranges[i].end = std::max(ranges[i].end, ranges[next].end);
                        ranges.remove(next);
                    } else
                        break;
                }
                
                return;
            }
        }

        // CharacterRange comes after all existing ranges.
        ranges.append(CharacterRange(lo, hi));
    }

    bool m_isCaseInsensitive;
    CanonicalMode m_canonicalMode;

    Vector<UChar32> m_matches;
    Vector<CharacterRange> m_ranges;
    Vector<UChar32> m_matchesUnicode;
    Vector<CharacterRange> m_rangesUnicode;
};

class YarrPatternConstructor {
public:
    YarrPatternConstructor(YarrPattern& pattern, void* stackLimit)
        : m_pattern(pattern)
        , m_characterClassConstructor(pattern.ignoreCase(), pattern.unicode() ? CanonicalMode::Unicode : CanonicalMode::UCS2)
        , m_stackLimit(stackLimit)
        , m_invertParentheticalAssertion(false)
    {
        auto body = std::make_unique<PatternDisjunction>();
        m_pattern.m_body = body.get();
        m_alternative = body->addNewAlternative();
        m_pattern.m_disjunctions.append(WTFMove(body));
    }

    ~YarrPatternConstructor()
    {
    }

    void reset()
    {
        m_pattern.reset();
        m_characterClassConstructor.reset();

        auto body = std::make_unique<PatternDisjunction>();
        m_pattern.m_body = body.get();
        m_alternative = body->addNewAlternative();
        m_pattern.m_disjunctions.append(WTFMove(body));
    }
    
    void assertionBOL()
    {
        if (!m_alternative->m_terms.size() && !m_invertParentheticalAssertion) {
            m_alternative->m_startsWithBOL = true;
            m_alternative->m_containsBOL = true;
            m_pattern.m_containsBOL = true;
        }
        m_alternative->m_terms.append(PatternTerm::BOL());
    }
    void assertionEOL()
    {
        m_alternative->m_terms.append(PatternTerm::EOL());
    }
    void assertionWordBoundary(bool invert)
    {
        m_alternative->m_terms.append(PatternTerm::WordBoundary(invert));
    }

    void atomPatternCharacter(UChar32 ch)
    {
        // We handle case-insensitive checking of unicode characters which do have both
        // cases by handling them as if they were defined using a CharacterClass.
        if (!m_pattern.ignoreCase() || (isASCII(ch) && !m_pattern.unicode())) {
            m_alternative->m_terms.append(PatternTerm(ch));
            return;
        }

        const CanonicalizationRange* info = canonicalRangeInfoFor(ch, m_pattern.unicode() ? CanonicalMode::Unicode : CanonicalMode::UCS2);
        if (info->type == CanonicalizeUnique) {
            m_alternative->m_terms.append(PatternTerm(ch));
            return;
        }

        m_characterClassConstructor.putUnicodeIgnoreCase(ch, info);
        auto newCharacterClass = m_characterClassConstructor.charClass();
        m_alternative->m_terms.append(PatternTerm(newCharacterClass.get(), false));
        m_pattern.m_userCharacterClasses.append(WTFMove(newCharacterClass));
    }

    void atomBuiltInCharacterClass(BuiltInCharacterClassID classID, bool invert)
    {
        switch (classID) {
        case DigitClassID:
            m_alternative->m_terms.append(PatternTerm(m_pattern.digitsCharacterClass(), invert));
            break;
        case SpaceClassID:
            m_alternative->m_terms.append(PatternTerm(m_pattern.spacesCharacterClass(), invert));
            break;
        case WordClassID:
            if (m_pattern.unicode() && m_pattern.ignoreCase())
                m_alternative->m_terms.append(PatternTerm(m_pattern.wordUnicodeIgnoreCaseCharCharacterClass(), invert));
            else
                m_alternative->m_terms.append(PatternTerm(m_pattern.wordcharCharacterClass(), invert));
            break;
        case NewlineClassID:
            m_alternative->m_terms.append(PatternTerm(m_pattern.newlineCharacterClass(), invert));
            break;
        }
    }

    void atomCharacterClassBegin(bool invert = false)
    {
        m_invertCharacterClass = invert;
    }

    void atomCharacterClassAtom(UChar32 ch)
    {
        m_characterClassConstructor.putChar(ch);
    }

    void atomCharacterClassRange(UChar32 begin, UChar32 end)
    {
        m_characterClassConstructor.putRange(begin, end);
    }

    void atomCharacterClassBuiltIn(BuiltInCharacterClassID classID, bool invert)
    {
        ASSERT(classID != NewlineClassID);

        switch (classID) {
        case DigitClassID:
            m_characterClassConstructor.append(invert ? m_pattern.nondigitsCharacterClass() : m_pattern.digitsCharacterClass());
            break;
        
        case SpaceClassID:
            m_characterClassConstructor.append(invert ? m_pattern.nonspacesCharacterClass() : m_pattern.spacesCharacterClass());
            break;
        
        case WordClassID:
            if (m_pattern.unicode() && m_pattern.ignoreCase())
                m_characterClassConstructor.append(invert ? m_pattern.nonwordUnicodeIgnoreCaseCharCharacterClass() : m_pattern.wordUnicodeIgnoreCaseCharCharacterClass());
            else
                m_characterClassConstructor.append(invert ? m_pattern.nonwordcharCharacterClass() : m_pattern.wordcharCharacterClass());
            break;
        
        default:
            RELEASE_ASSERT_NOT_REACHED();
        }
    }

    void atomCharacterClassEnd()
    {
        auto newCharacterClass = m_characterClassConstructor.charClass();
        m_alternative->m_terms.append(PatternTerm(newCharacterClass.get(), m_invertCharacterClass));
        m_pattern.m_userCharacterClasses.append(WTFMove(newCharacterClass));
    }

    void atomParenthesesSubpatternBegin(bool capture = true)
    {
        unsigned subpatternId = m_pattern.m_numSubpatterns + 1;
        if (capture)
            m_pattern.m_numSubpatterns++;

        auto parenthesesDisjunction = std::make_unique<PatternDisjunction>(m_alternative);
        m_alternative->m_terms.append(PatternTerm(PatternTerm::TypeParenthesesSubpattern, subpatternId, parenthesesDisjunction.get(), capture, false));
        m_alternative = parenthesesDisjunction->addNewAlternative();
        m_pattern.m_disjunctions.append(WTFMove(parenthesesDisjunction));
    }

    void atomParentheticalAssertionBegin(bool invert = false)
    {
        auto parenthesesDisjunction = std::make_unique<PatternDisjunction>(m_alternative);
        m_alternative->m_terms.append(PatternTerm(PatternTerm::TypeParentheticalAssertion, m_pattern.m_numSubpatterns + 1, parenthesesDisjunction.get(), false, invert));
        m_alternative = parenthesesDisjunction->addNewAlternative();
        m_invertParentheticalAssertion = invert;
        m_pattern.m_disjunctions.append(WTFMove(parenthesesDisjunction));
    }

    void atomParenthesesEnd()
    {
        ASSERT(m_alternative->m_parent);
        ASSERT(m_alternative->m_parent->m_parent);

        PatternDisjunction* parenthesesDisjunction = m_alternative->m_parent;
        m_alternative = m_alternative->m_parent->m_parent;

        PatternTerm& lastTerm = m_alternative->lastTerm();

        unsigned numParenAlternatives = parenthesesDisjunction->m_alternatives.size();
        unsigned numBOLAnchoredAlts = 0;

        for (unsigned i = 0; i < numParenAlternatives; i++) {
            // Bubble up BOL flags
            if (parenthesesDisjunction->m_alternatives[i]->m_startsWithBOL)
                numBOLAnchoredAlts++;
        }

        if (numBOLAnchoredAlts) {
            m_alternative->m_containsBOL = true;
            // If all the alternatives in parens start with BOL, then so does this one
            if (numBOLAnchoredAlts == numParenAlternatives)
                m_alternative->m_startsWithBOL = true;
        }

        lastTerm.parentheses.lastSubpatternId = m_pattern.m_numSubpatterns;
        m_invertParentheticalAssertion = false;
    }

    void atomBackReference(unsigned subpatternId)
    {
        ASSERT(subpatternId);
        m_pattern.m_containsBackreferences = true;
        m_pattern.m_maxBackReference = std::max(m_pattern.m_maxBackReference, subpatternId);

        if (subpatternId > m_pattern.m_numSubpatterns) {
            m_alternative->m_terms.append(PatternTerm::ForwardReference());
            return;
        }

        PatternAlternative* currentAlternative = m_alternative;
        ASSERT(currentAlternative);

        // Note to self: if we waited until the AST was baked, we could also remove forwards refs 
        while ((currentAlternative = currentAlternative->m_parent->m_parent)) {
            PatternTerm& term = currentAlternative->lastTerm();
            ASSERT((term.type == PatternTerm::TypeParenthesesSubpattern) || (term.type == PatternTerm::TypeParentheticalAssertion));

            if ((term.type == PatternTerm::TypeParenthesesSubpattern) && term.capture() && (subpatternId == term.parentheses.subpatternId)) {
                m_alternative->m_terms.append(PatternTerm::ForwardReference());
                return;
            }
        }

        m_alternative->m_terms.append(PatternTerm(subpatternId));
    }

    // deep copy the argument disjunction.  If filterStartsWithBOL is true, 
    // skip alternatives with m_startsWithBOL set true.
    PatternDisjunction* copyDisjunction(PatternDisjunction* disjunction, bool filterStartsWithBOL = false)
    {
        std::unique_ptr<PatternDisjunction> newDisjunction;
        for (unsigned alt = 0; alt < disjunction->m_alternatives.size(); ++alt) {
            PatternAlternative* alternative = disjunction->m_alternatives[alt].get();
            if (!filterStartsWithBOL || !alternative->m_startsWithBOL) {
                if (!newDisjunction) {
                    newDisjunction = std::make_unique<PatternDisjunction>();
                    newDisjunction->m_parent = disjunction->m_parent;
                }
                PatternAlternative* newAlternative = newDisjunction->addNewAlternative();
                newAlternative->m_terms.reserveInitialCapacity(alternative->m_terms.size());
                for (unsigned i = 0; i < alternative->m_terms.size(); ++i)
                    newAlternative->m_terms.append(copyTerm(alternative->m_terms[i], filterStartsWithBOL));
            }
        }
        
        if (!newDisjunction)
            return 0;

        PatternDisjunction* copiedDisjunction = newDisjunction.get();
        m_pattern.m_disjunctions.append(WTFMove(newDisjunction));
        return copiedDisjunction;
    }
    
    PatternTerm copyTerm(PatternTerm& term, bool filterStartsWithBOL = false)
    {
        if ((term.type != PatternTerm::TypeParenthesesSubpattern) && (term.type != PatternTerm::TypeParentheticalAssertion))
            return PatternTerm(term);
        
        PatternTerm termCopy = term;
        termCopy.parentheses.disjunction = copyDisjunction(termCopy.parentheses.disjunction, filterStartsWithBOL);
        m_pattern.m_hasCopiedParenSubexpressions = true;
        return termCopy;
    }
    
    void quantifyAtom(unsigned min, unsigned max, bool greedy)
    {
        ASSERT(min <= max);
        ASSERT(m_alternative->m_terms.size());

        if (!max) {
            m_alternative->removeLastTerm();
            return;
        }

        PatternTerm& term = m_alternative->lastTerm();
        ASSERT(term.type > PatternTerm::TypeAssertionWordBoundary);
        ASSERT(term.quantityMinCount == 1 && term.quantityMaxCount == 1 && term.quantityType == QuantifierFixedCount);

        if (term.type == PatternTerm::TypeParentheticalAssertion) {
            // If an assertion is quantified with a minimum count of zero, it can simply be removed.
            // This arises from the RepeatMatcher behaviour in the spec. Matching an assertion never
            // results in any input being consumed, however the continuation passed to the assertion
            // (called in steps, 8c and 9 of the RepeatMatcher definition, ES5.1 15.10.2.5) will
            // reject all zero length matches (see step 2.1). A match from the continuation of the
            // expression will still be accepted regardless (via steps 8a and 11) - the upshot of all
            // this is that matches from the assertion are not required, and won't be accepted anyway,
            // so no need to ever run it.
            if (!min)
                m_alternative->removeLastTerm();
            // We never need to run an assertion more than once. Subsequent interations will be run
            // with the same start index (since assertions are non-capturing) and the same captures
            // (per step 4 of RepeatMatcher in ES5.1 15.10.2.5), and as such will always produce the
            // same result and captures. If the first match succeeds then the subsequent (min - 1)
            // matches will too. Any additional optional matches will fail (on the same basis as the
            // minimum zero quantified assertions, above), but this will still result in a match.
            return;
        }

        if (min == max)
            term.quantify(min, max, QuantifierFixedCount);
        else if (!min || (term.type == PatternTerm::TypeParenthesesSubpattern && m_pattern.m_hasCopiedParenSubexpressions))
            term.quantify(min, max, greedy ? QuantifierGreedy : QuantifierNonGreedy);
        else {
            term.quantify(min, min, QuantifierFixedCount);
            m_alternative->m_terms.append(copyTerm(term));
            // NOTE: this term is interesting from an analysis perspective, in that it can be ignored.....
            m_alternative->lastTerm().quantify((max == quantifyInfinite) ? max : max - min, greedy ? QuantifierGreedy : QuantifierNonGreedy);
            if (m_alternative->lastTerm().type == PatternTerm::TypeParenthesesSubpattern)
                m_alternative->lastTerm().parentheses.isCopy = true;
        }
    }

    void disjunction()
    {
        m_alternative = m_alternative->m_parent->addNewAlternative();
    }

    YarrPattern::ErrorCode setupAlternativeOffsets(PatternAlternative* alternative, unsigned currentCallFrameSize, unsigned initialInputPosition, unsigned& newCallFrameSize) WARN_UNUSED_RETURN
    {
        if (UNLIKELY(!isSafeToRecurse()))
            return YarrPattern::TooManyDisjunctions;

        YarrPattern::ErrorCode error = YarrPattern::NoError;
        alternative->m_hasFixedSize = true;
        Checked<unsigned, RecordOverflow> currentInputPosition = initialInputPosition;

        for (unsigned i = 0; i < alternative->m_terms.size(); ++i) {
            PatternTerm& term = alternative->m_terms[i];

            switch (term.type) {
            case PatternTerm::TypeAssertionBOL:
            case PatternTerm::TypeAssertionEOL:
            case PatternTerm::TypeAssertionWordBoundary:
                term.inputPosition = currentInputPosition.unsafeGet();
                break;

            case PatternTerm::TypeBackReference:
                term.inputPosition = currentInputPosition.unsafeGet();
                term.frameLocation = currentCallFrameSize;
                currentCallFrameSize += YarrStackSpaceForBackTrackInfoBackReference;
                alternative->m_hasFixedSize = false;
                break;

            case PatternTerm::TypeForwardReference:
                break;

            case PatternTerm::TypePatternCharacter:
                term.inputPosition = currentInputPosition.unsafeGet();
                if (term.quantityType != QuantifierFixedCount) {
                    term.frameLocation = currentCallFrameSize;
                    currentCallFrameSize += YarrStackSpaceForBackTrackInfoPatternCharacter;
                    alternative->m_hasFixedSize = false;
                } else if (m_pattern.unicode()) {
                    currentInputPosition += U16_LENGTH(term.patternCharacter) * term.quantityMaxCount;
                } else
                    currentInputPosition += term.quantityMaxCount;
                break;

            case PatternTerm::TypeCharacterClass:
                term.inputPosition = currentInputPosition.unsafeGet();
                if (term.quantityType != QuantifierFixedCount) {
                    term.frameLocation = currentCallFrameSize;
                    currentCallFrameSize += YarrStackSpaceForBackTrackInfoCharacterClass;
                    alternative->m_hasFixedSize = false;
                } else if (m_pattern.unicode()) {
                    term.frameLocation = currentCallFrameSize;
                    currentCallFrameSize += YarrStackSpaceForBackTrackInfoCharacterClass;
                    currentInputPosition += term.quantityMaxCount;
                    alternative->m_hasFixedSize = false;
                } else
                    currentInputPosition += term.quantityMaxCount;
                break;

            case PatternTerm::TypeParenthesesSubpattern:
                // Note: for fixed once parentheses we will ensure at least the minimum is available; others are on their own.
                term.frameLocation = currentCallFrameSize;
                if (term.quantityMaxCount == 1 && !term.parentheses.isCopy) {
                    if (term.quantityType != QuantifierFixedCount)
                        currentCallFrameSize += YarrStackSpaceForBackTrackInfoParenthesesOnce;
                    error = setupDisjunctionOffsets(term.parentheses.disjunction, currentCallFrameSize, currentInputPosition.unsafeGet(), currentCallFrameSize);
                    if (error)
                        return error;
                    // If quantity is fixed, then pre-check its minimum size.
                    if (term.quantityType == QuantifierFixedCount)
                        currentInputPosition += term.parentheses.disjunction->m_minimumSize;
                    term.inputPosition = currentInputPosition.unsafeGet();
                } else if (term.parentheses.isTerminal) {
                    currentCallFrameSize += YarrStackSpaceForBackTrackInfoParenthesesTerminal;
                    error = setupDisjunctionOffsets(term.parentheses.disjunction, currentCallFrameSize, currentInputPosition.unsafeGet(), currentCallFrameSize);
                    if (error)
                        return error;
                    term.inputPosition = currentInputPosition.unsafeGet();
                } else {
                    term.inputPosition = currentInputPosition.unsafeGet();
                    unsigned ignoredCallFrameSize;
                    error = setupDisjunctionOffsets(term.parentheses.disjunction, 0, currentInputPosition.unsafeGet(), ignoredCallFrameSize);
                    if (error)
                        return error;
                    currentCallFrameSize += YarrStackSpaceForBackTrackInfoParentheses;
                }
                // Fixed count of 1 could be accepted, if they have a fixed size *AND* if all alternatives are of the same length.
                alternative->m_hasFixedSize = false;
                break;

            case PatternTerm::TypeParentheticalAssertion:
                term.inputPosition = currentInputPosition.unsafeGet();
                term.frameLocation = currentCallFrameSize;
                error = setupDisjunctionOffsets(term.parentheses.disjunction, currentCallFrameSize + YarrStackSpaceForBackTrackInfoParentheticalAssertion, currentInputPosition.unsafeGet(), currentCallFrameSize);
                if (error)
                    return error;
                break;

            case PatternTerm::TypeDotStarEnclosure:
                alternative->m_hasFixedSize = false;
                term.inputPosition = initialInputPosition;
                break;
            }
            if (currentInputPosition.hasOverflowed())
                return YarrPattern::OffsetTooLarge;
        }

        alternative->m_minimumSize = (currentInputPosition - initialInputPosition).unsafeGet();
        newCallFrameSize = currentCallFrameSize;
        return error;
    }

    YarrPattern::ErrorCode setupDisjunctionOffsets(PatternDisjunction* disjunction, unsigned initialCallFrameSize, unsigned initialInputPosition, unsigned& callFrameSize)
    {
        if (UNLIKELY(!isSafeToRecurse()))
            return YarrPattern::TooManyDisjunctions;

        if ((disjunction != m_pattern.m_body) && (disjunction->m_alternatives.size() > 1))
            initialCallFrameSize += YarrStackSpaceForBackTrackInfoAlternative;

        unsigned minimumInputSize = UINT_MAX;
        unsigned maximumCallFrameSize = 0;
        bool hasFixedSize = true;
        YarrPattern::ErrorCode error = YarrPattern::NoError;

        for (unsigned alt = 0; alt < disjunction->m_alternatives.size(); ++alt) {
            PatternAlternative* alternative = disjunction->m_alternatives[alt].get();
            unsigned currentAlternativeCallFrameSize;
            error = setupAlternativeOffsets(alternative, initialCallFrameSize, initialInputPosition, currentAlternativeCallFrameSize);
            if (error)
                return error;
            minimumInputSize = std::min(minimumInputSize, alternative->m_minimumSize);
            maximumCallFrameSize = std::max(maximumCallFrameSize, currentAlternativeCallFrameSize);
            hasFixedSize &= alternative->m_hasFixedSize;
            if (alternative->m_minimumSize > INT_MAX)
                m_pattern.m_containsUnsignedLengthPattern = true;
        }
        
        ASSERT(minimumInputSize != UINT_MAX);
        ASSERT(maximumCallFrameSize >= initialCallFrameSize);

        disjunction->m_hasFixedSize = hasFixedSize;
        disjunction->m_minimumSize = minimumInputSize;
        disjunction->m_callFrameSize = maximumCallFrameSize;
        callFrameSize = maximumCallFrameSize;
        return error;
    }

    const char* setupOffsets()
    {
        // FIXME: Yarr should not use the stack to handle subpatterns (rdar://problem/26436314).
        unsigned ignoredCallFrameSize;
        YarrPattern::ErrorCode error = setupDisjunctionOffsets(m_pattern.m_body, 0, 0, ignoredCallFrameSize);
        if (error)
            return YarrPattern::errorMessage(error);
        return nullptr;
    }

    // This optimization identifies sets of parentheses that we will never need to backtrack.
    // In these cases we do not need to store state from prior iterations.
    // We can presently avoid backtracking for:
    //   * where the parens are at the end of the regular expression (last term in any of the
    //     alternatives of the main body disjunction).
    //   * where the parens are non-capturing, and quantified unbounded greedy (*).
    //   * where the parens do not contain any capturing subpatterns.
    void checkForTerminalParentheses()
    {
        // This check is much too crude; should be just checking whether the candidate
        // node contains nested capturing subpatterns, not the whole expression!
        if (m_pattern.m_numSubpatterns)
            return;

        Vector<std::unique_ptr<PatternAlternative>>& alternatives = m_pattern.m_body->m_alternatives;
        for (size_t i = 0; i < alternatives.size(); ++i) {
            Vector<PatternTerm>& terms = alternatives[i]->m_terms;
            if (terms.size()) {
                PatternTerm& term = terms.last();
                if (term.type == PatternTerm::TypeParenthesesSubpattern
                    && term.quantityType == QuantifierGreedy
                    && term.quantityMinCount == 0
                    && term.quantityMaxCount == quantifyInfinite
                    && !term.capture())
                    term.parentheses.isTerminal = true;
            }
        }
    }

    void optimizeBOL()
    {
        // Look for expressions containing beginning of line (^) anchoring and unroll them.
        // e.g. /^a|^b|c/ becomes /^a|^b|c/ which is executed once followed by /c/ which loops
        // This code relies on the parsing code tagging alternatives with m_containsBOL and
        // m_startsWithBOL and rolling those up to containing alternatives.
        // At this point, this is only valid for non-multiline expressions.
        PatternDisjunction* disjunction = m_pattern.m_body;
        
        if (!m_pattern.m_containsBOL || m_pattern.multiline())
            return;
        
        PatternDisjunction* loopDisjunction = copyDisjunction(disjunction, true);

        // Set alternatives in disjunction to "onceThrough"
        for (unsigned alt = 0; alt < disjunction->m_alternatives.size(); ++alt)
            disjunction->m_alternatives[alt]->setOnceThrough();

        if (loopDisjunction) {
            // Move alternatives from loopDisjunction to disjunction
            for (unsigned alt = 0; alt < loopDisjunction->m_alternatives.size(); ++alt)
                disjunction->m_alternatives.append(loopDisjunction->m_alternatives[alt].release());
                
            loopDisjunction->m_alternatives.clear();
        }
    }

    bool containsCapturingTerms(PatternAlternative* alternative, size_t firstTermIndex, size_t endIndex)
    {
        Vector<PatternTerm>& terms = alternative->m_terms;

        ASSERT(endIndex <= terms.size());
        for (size_t termIndex = firstTermIndex; termIndex < endIndex; ++termIndex) {
            PatternTerm& term = terms[termIndex];

            if (term.m_capture)
                return true;

            if (term.type == PatternTerm::TypeParenthesesSubpattern) {
                PatternDisjunction* nestedDisjunction = term.parentheses.disjunction;
                for (unsigned alt = 0; alt < nestedDisjunction->m_alternatives.size(); ++alt) {
                    if (containsCapturingTerms(nestedDisjunction->m_alternatives[alt].get(), 0, nestedDisjunction->m_alternatives[alt]->m_terms.size()))
                        return true;
                }
            }
        }

        return false;
    }

    // This optimization identifies alternatives in the form of 
    // [^].*[?]<expression>.*[$] for expressions that don't have any 
    // capturing terms. The alternative is changed to <expression> 
    // followed by processing of the dot stars to find and adjust the 
    // beginning and the end of the match.
    void optimizeDotStarWrappedExpressions()
    {
        Vector<std::unique_ptr<PatternAlternative>>& alternatives = m_pattern.m_body->m_alternatives;
        if (alternatives.size() != 1)
            return;

        PatternAlternative* alternative = alternatives[0].get();
        Vector<PatternTerm>& terms = alternative->m_terms;
        if (terms.size() >= 3) {
            bool startsWithBOL = false;
            bool endsWithEOL = false;
            size_t termIndex, firstExpressionTerm;

            termIndex = 0;
            if (terms[termIndex].type == PatternTerm::TypeAssertionBOL) {
                startsWithBOL = true;
                ++termIndex;
            }
            
            PatternTerm& firstNonAnchorTerm = terms[termIndex];
            if ((firstNonAnchorTerm.type != PatternTerm::TypeCharacterClass) || (firstNonAnchorTerm.characterClass != m_pattern.newlineCharacterClass()) || !((firstNonAnchorTerm.quantityType == QuantifierGreedy) || (firstNonAnchorTerm.quantityType == QuantifierNonGreedy)))
                return;
            
            firstExpressionTerm = termIndex + 1;
            
            termIndex = terms.size() - 1;
            if (terms[termIndex].type == PatternTerm::TypeAssertionEOL) {
                endsWithEOL = true;
                --termIndex;
            }
            
            PatternTerm& lastNonAnchorTerm = terms[termIndex];
            if ((lastNonAnchorTerm.type != PatternTerm::TypeCharacterClass) || (lastNonAnchorTerm.characterClass != m_pattern.newlineCharacterClass()) || (lastNonAnchorTerm.quantityType != QuantifierGreedy))
                return;

            size_t endIndex = termIndex;
            if (firstExpressionTerm >= endIndex)
                return;

            if (!containsCapturingTerms(alternative, firstExpressionTerm, endIndex)) {
                for (termIndex = terms.size() - 1; termIndex >= endIndex; --termIndex)
                    terms.remove(termIndex);

                for (termIndex = firstExpressionTerm; termIndex > 0; --termIndex)
                    terms.remove(termIndex - 1);

                terms.append(PatternTerm(startsWithBOL, endsWithEOL));
                
                m_pattern.m_containsBOL = false;
            }
        }
    }

private:
    bool isSafeToRecurse() const
    {
        if (!m_stackLimit)
            return true;
        ASSERT(wtfThreadData().stack().isGrowingDownward());
        int8_t* curr = reinterpret_cast<int8_t*>(&curr);
        int8_t* limit = reinterpret_cast<int8_t*>(m_stackLimit);
        return curr >= limit;
    }

    YarrPattern& m_pattern;
    PatternAlternative* m_alternative;
    CharacterClassConstructor m_characterClassConstructor;
    void* m_stackLimit;
    bool m_invertCharacterClass;
    bool m_invertParentheticalAssertion;
};

const char* YarrPattern::errorMessage(YarrPattern::ErrorCode error)
{
#define REGEXP_ERROR_PREFIX "Invalid regular expression: "
    // The order of this array must match the ErrorCode enum.
    static const char* errorMessages[NumberOfErrorCodes] = {
        nullptr, // NoError
        REGEXP_ERROR_PREFIX "regular expression too large",
        REGEXP_ERROR_PREFIX "numbers out of order in {} quantifier",
        REGEXP_ERROR_PREFIX "nothing to repeat",
        REGEXP_ERROR_PREFIX "number too large in {} quantifier",
        REGEXP_ERROR_PREFIX "missing )",
        REGEXP_ERROR_PREFIX "unmatched parentheses",
        REGEXP_ERROR_PREFIX "unrecognized character after (?",
        REGEXP_ERROR_PREFIX "missing terminating ] for character class",
        REGEXP_ERROR_PREFIX "range out of order in character class",
        REGEXP_ERROR_PREFIX "\\ at end of pattern",
        REGEXP_ERROR_PREFIX "invalid unicode {} escape",
        REGEXP_ERROR_PREFIX "invalid escaped character for unicode pattern",
        REGEXP_ERROR_PREFIX "too many nested disjunctions",
        REGEXP_ERROR_PREFIX "pattern exceeds string length limits",
        REGEXP_ERROR_PREFIX "invalid flags"
    };

    return errorMessages[error];
}

const char* YarrPattern::compile(const String& patternString, void* stackLimit)
{
    YarrPatternConstructor constructor(*this, stackLimit);

    if (m_flags == InvalidFlags)
        return errorMessage(InvalidRegularExpressionFlags);

    if (const char* error = parse(constructor, patternString, unicode()))
        return error;
    
    // If the pattern contains illegal backreferences reset & reparse.
    // Quoting Netscape's "What's new in JavaScript 1.2",
    //      "Note: if the number of left parentheses is less than the number specified
    //       in \#, the \# is taken as an octal escape as described in the next row."
    if (containsIllegalBackReference()) {
        unsigned numSubpatterns = m_numSubpatterns;

        constructor.reset();
#if !ASSERT_DISABLED
        const char* error =
#endif
            parse(constructor, patternString, unicode(), numSubpatterns);

        ASSERT(!error);
        ASSERT(numSubpatterns == m_numSubpatterns);
    }

    constructor.checkForTerminalParentheses();
    constructor.optimizeDotStarWrappedExpressions();
    constructor.optimizeBOL();
        
    if (const char* error = constructor.setupOffsets())
        return error;

    return nullptr;
}

YarrPattern::YarrPattern(const String& pattern, RegExpFlags flags, const char** error, void* stackLimit)
    : m_containsBackreferences(false)
    , m_containsBOL(false)
    , m_containsUnsignedLengthPattern(false)
    , m_hasCopiedParenSubexpressions(false)
    , m_flags(flags)
    , m_numSubpatterns(0)
    , m_maxBackReference(0)
    , newlineCached(0)
    , digitsCached(0)
    , spacesCached(0)
    , wordcharCached(0)
    , wordUnicodeIgnoreCaseCharCached(0)
    , nondigitsCached(0)
    , nonspacesCached(0)
    , nonwordcharCached(0)
    , nonwordUnicodeIgnoreCasecharCached(0)
{
    *error = compile(pattern, stackLimit);
}

} }
