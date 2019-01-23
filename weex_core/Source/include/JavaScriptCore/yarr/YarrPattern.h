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

#include "RegExpKey.h"
#include <wtf/CheckedArithmetic.h>
#include <wtf/Vector.h>
#include <wtf/text/WTFString.h>

namespace JSC { namespace Yarr {

struct PatternDisjunction;

struct CharacterRange {
    UChar32 begin;
    UChar32 end;

    CharacterRange(UChar32 begin, UChar32 end)
        : begin(begin)
        , end(end)
    {
    }
};

struct CharacterClass {
    WTF_MAKE_FAST_ALLOCATED;
public:
    // All CharacterClass instances have to have the full set of matches and ranges,
    // they may have an optional m_table for faster lookups (which must match the
    // specified matches and ranges)
    CharacterClass()
        : m_table(0)
    {
    }
    CharacterClass(const char* table, bool inverted)
        : m_table(table)
        , m_tableInverted(inverted)
    {
    }
    Vector<UChar32> m_matches;
    Vector<CharacterRange> m_ranges;
    Vector<UChar32> m_matchesUnicode;
    Vector<CharacterRange> m_rangesUnicode;

    const char* m_table;
    bool m_tableInverted;
};

enum QuantifierType {
    QuantifierFixedCount,
    QuantifierGreedy,
    QuantifierNonGreedy,
};

struct PatternTerm {
    enum Type {
        TypeAssertionBOL,
        TypeAssertionEOL,
        TypeAssertionWordBoundary,
        TypePatternCharacter,
        TypeCharacterClass,
        TypeBackReference,
        TypeForwardReference,
        TypeParenthesesSubpattern,
        TypeParentheticalAssertion,
        TypeDotStarEnclosure,
    } type;
    bool m_capture :1;
    bool m_invert :1;
    union {
        UChar32 patternCharacter;
        CharacterClass* characterClass;
        unsigned backReferenceSubpatternId;
        struct {
            PatternDisjunction* disjunction;
            unsigned subpatternId;
            unsigned lastSubpatternId;
            bool isCopy;
            bool isTerminal;
        } parentheses;
        struct {
            bool bolAnchor : 1;
            bool eolAnchor : 1;
        } anchors;
    };
    QuantifierType quantityType;
    Checked<unsigned> quantityMinCount;
    Checked<unsigned> quantityMaxCount;
    unsigned inputPosition;
    unsigned frameLocation;

    PatternTerm(UChar32 ch)
        : type(PatternTerm::TypePatternCharacter)
        , m_capture(false)
        , m_invert(false)
    {
        patternCharacter = ch;
        quantityType = QuantifierFixedCount;
        quantityMinCount = quantityMaxCount = 1;
    }

    PatternTerm(CharacterClass* charClass, bool invert)
        : type(PatternTerm::TypeCharacterClass)
        , m_capture(false)
        , m_invert(invert)
    {
        characterClass = charClass;
        quantityType = QuantifierFixedCount;
        quantityMinCount = quantityMaxCount = 1;
    }

    PatternTerm(Type type, unsigned subpatternId, PatternDisjunction* disjunction, bool capture = false, bool invert = false)
        : type(type)
        , m_capture(capture)
        , m_invert(invert)
    {
        parentheses.disjunction = disjunction;
        parentheses.subpatternId = subpatternId;
        parentheses.isCopy = false;
        parentheses.isTerminal = false;
        quantityType = QuantifierFixedCount;
        quantityMinCount = quantityMaxCount = 1;
    }
    
    PatternTerm(Type type, bool invert = false)
        : type(type)
        , m_capture(false)
        , m_invert(invert)
    {
        quantityType = QuantifierFixedCount;
        quantityMinCount = quantityMaxCount = 1;
    }

    PatternTerm(unsigned spatternId)
        : type(TypeBackReference)
        , m_capture(false)
        , m_invert(false)
    {
        backReferenceSubpatternId = spatternId;
        quantityType = QuantifierFixedCount;
        quantityMinCount = quantityMaxCount = 1;
    }

    PatternTerm(bool bolAnchor, bool eolAnchor)
        : type(TypeDotStarEnclosure)
        , m_capture(false)
        , m_invert(false)
    {
        anchors.bolAnchor = bolAnchor;
        anchors.eolAnchor = eolAnchor;
        quantityType = QuantifierFixedCount;
        quantityMinCount = quantityMaxCount = 1;
    }
    
    static PatternTerm ForwardReference()
    {
        return PatternTerm(TypeForwardReference);
    }

    static PatternTerm BOL()
    {
        return PatternTerm(TypeAssertionBOL);
    }

    static PatternTerm EOL()
    {
        return PatternTerm(TypeAssertionEOL);
    }

    static PatternTerm WordBoundary(bool invert)
    {
        return PatternTerm(TypeAssertionWordBoundary, invert);
    }
    
    bool invert()
    {
        return m_invert;
    }

    bool capture()
    {
        return m_capture;
    }
    
    void quantify(unsigned count, QuantifierType type)
    {
        quantityMinCount = 0;
        quantityMaxCount = count;
        quantityType = type;
    }

    void quantify(unsigned minCount, unsigned maxCount, QuantifierType type)
    {
        // Currently only Parentheses can specify a non-zero min with a different max.
        ASSERT(this->type == TypeParenthesesSubpattern || !minCount || minCount == maxCount);
        ASSERT(minCount <= maxCount);
        quantityMinCount = minCount;
        quantityMaxCount = maxCount;
        quantityType = type;
    }
};

struct PatternAlternative {
    WTF_MAKE_FAST_ALLOCATED;
public:
    PatternAlternative(PatternDisjunction* disjunction)
        : m_parent(disjunction)
        , m_onceThrough(false)
        , m_hasFixedSize(false)
        , m_startsWithBOL(false)
        , m_containsBOL(false)
    {
    }

    PatternTerm& lastTerm()
    {
        ASSERT(m_terms.size());
        return m_terms[m_terms.size() - 1];
    }
    
    void removeLastTerm()
    {
        ASSERT(m_terms.size());
        m_terms.shrink(m_terms.size() - 1);
    }
    
    void setOnceThrough()
    {
        m_onceThrough = true;
    }
    
    bool onceThrough()
    {
        return m_onceThrough;
    }

    Vector<PatternTerm> m_terms;
    PatternDisjunction* m_parent;
    unsigned m_minimumSize;
    bool m_onceThrough : 1;
    bool m_hasFixedSize : 1;
    bool m_startsWithBOL : 1;
    bool m_containsBOL : 1;
};

struct PatternDisjunction {
    WTF_MAKE_FAST_ALLOCATED;
public:
    PatternDisjunction(PatternAlternative* parent = 0)
        : m_parent(parent)
        , m_hasFixedSize(false)
    {
    }
    
    PatternAlternative* addNewAlternative()
    {
        m_alternatives.append(std::make_unique<PatternAlternative>(this));
        return static_cast<PatternAlternative*>(m_alternatives.last().get());
    }

    Vector<std::unique_ptr<PatternAlternative>> m_alternatives;
    PatternAlternative* m_parent;
    unsigned m_minimumSize;
    unsigned m_callFrameSize;
    bool m_hasFixedSize;
};

// You probably don't want to be calling these functions directly
// (please to be calling newlineCharacterClass() et al on your
// friendly neighborhood YarrPattern instance to get nicely
// cached copies).
std::unique_ptr<CharacterClass> newlineCreate();
std::unique_ptr<CharacterClass> digitsCreate();
std::unique_ptr<CharacterClass> spacesCreate();
std::unique_ptr<CharacterClass> wordcharCreate();
std::unique_ptr<CharacterClass> wordUnicodeIgnoreCaseCharCreate();
std::unique_ptr<CharacterClass> nondigitsCreate();
std::unique_ptr<CharacterClass> nonspacesCreate();
std::unique_ptr<CharacterClass> nonwordcharCreate();
std::unique_ptr<CharacterClass> nonwordUnicodeIgnoreCaseCharCreate();

struct TermChain {
    TermChain(PatternTerm term)
        : term(term)
    {}

    PatternTerm term;
    Vector<TermChain> hotTerms;
};


struct YarrPattern {
    JS_EXPORT_PRIVATE YarrPattern(const String& pattern, RegExpFlags, const char** error, void* stackLimit = nullptr);

    enum ErrorCode {
        NoError,
        PatternTooLarge,
        QuantifierOutOfOrder,
        QuantifierWithoutAtom,
        QuantifierTooLarge,
        MissingParentheses,
        ParenthesesUnmatched,
        ParenthesesTypeInvalid,
        CharacterClassUnmatched,
        CharacterClassOutOfOrder,
        EscapeUnterminated,
        InvalidUnicodeEscape,
        InvalidIdentityEscape,
        TooManyDisjunctions,
        OffsetTooLarge,
        InvalidRegularExpressionFlags,
        NumberOfErrorCodes
    };
    
    JS_EXPORT_PRIVATE static const char* errorMessage(ErrorCode);

    void reset()
    {
        m_numSubpatterns = 0;
        m_maxBackReference = 0;

        m_containsBackreferences = false;
        m_containsBOL = false;
        m_containsUnsignedLengthPattern = false;
        m_hasCopiedParenSubexpressions = false;

        newlineCached = 0;
        digitsCached = 0;
        spacesCached = 0;
        wordcharCached = 0;
        wordUnicodeIgnoreCaseCharCached = 0;
        nondigitsCached = 0;
        nonspacesCached = 0;
        nonwordcharCached = 0;
        nonwordUnicodeIgnoreCasecharCached = 0;

        m_disjunctions.clear();
        m_userCharacterClasses.clear();
    }

    bool containsIllegalBackReference()
    {
        return m_maxBackReference > m_numSubpatterns;
    }

    bool containsUnsignedLengthPattern()
    {
        return m_containsUnsignedLengthPattern;
    }

    CharacterClass* newlineCharacterClass()
    {
        if (!newlineCached) {
            m_userCharacterClasses.append(newlineCreate());
            newlineCached = m_userCharacterClasses.last().get();
        }
        return newlineCached;
    }
    CharacterClass* digitsCharacterClass()
    {
        if (!digitsCached) {
            m_userCharacterClasses.append(digitsCreate());
            digitsCached = m_userCharacterClasses.last().get();
        }
        return digitsCached;
    }
    CharacterClass* spacesCharacterClass()
    {
        if (!spacesCached) {
            m_userCharacterClasses.append(spacesCreate());
            spacesCached = m_userCharacterClasses.last().get();
        }
        return spacesCached;
    }
    CharacterClass* wordcharCharacterClass()
    {
        if (!wordcharCached) {
            m_userCharacterClasses.append(wordcharCreate());
            wordcharCached = m_userCharacterClasses.last().get();
        }
        return wordcharCached;
    }
    CharacterClass* wordUnicodeIgnoreCaseCharCharacterClass()
    {
        if (!wordUnicodeIgnoreCaseCharCached) {
            m_userCharacterClasses.append(wordUnicodeIgnoreCaseCharCreate());
            wordUnicodeIgnoreCaseCharCached = m_userCharacterClasses.last().get();
        }
        return wordUnicodeIgnoreCaseCharCached;
    }
    CharacterClass* nondigitsCharacterClass()
    {
        if (!nondigitsCached) {
            m_userCharacterClasses.append(nondigitsCreate());
            nondigitsCached = m_userCharacterClasses.last().get();
        }
        return nondigitsCached;
    }
    CharacterClass* nonspacesCharacterClass()
    {
        if (!nonspacesCached) {
            m_userCharacterClasses.append(nonspacesCreate());
            nonspacesCached = m_userCharacterClasses.last().get();
        }
        return nonspacesCached;
    }
    CharacterClass* nonwordcharCharacterClass()
    {
        if (!nonwordcharCached) {
            m_userCharacterClasses.append(nonwordcharCreate());
            nonwordcharCached = m_userCharacterClasses.last().get();
        }
        return nonwordcharCached;
    }
    CharacterClass* nonwordUnicodeIgnoreCaseCharCharacterClass()
    {
        if (!nonwordUnicodeIgnoreCasecharCached) {
            m_userCharacterClasses.append(nonwordUnicodeIgnoreCaseCharCreate());
            nonwordUnicodeIgnoreCasecharCached = m_userCharacterClasses.last().get();
        }
        return nonwordUnicodeIgnoreCasecharCached;
    }

    bool ignoreCase() const { return m_flags & FlagIgnoreCase; }
    bool multiline() const { return m_flags & FlagMultiline; }
    bool sticky() const { return m_flags & FlagSticky; }
    bool unicode() const { return m_flags & FlagUnicode; }

    bool m_containsBackreferences : 1;
    bool m_containsBOL : 1;
    bool m_containsUnsignedLengthPattern : 1;
    bool m_hasCopiedParenSubexpressions : 1;
    RegExpFlags m_flags;
    unsigned m_numSubpatterns;
    unsigned m_maxBackReference;
    PatternDisjunction* m_body;
    Vector<std::unique_ptr<PatternDisjunction>, 4> m_disjunctions;
    Vector<std::unique_ptr<CharacterClass>> m_userCharacterClasses;

private:
    const char* compile(const String& patternString, void* stackLimit);

    CharacterClass* newlineCached;
    CharacterClass* digitsCached;
    CharacterClass* spacesCached;
    CharacterClass* wordcharCached;
    CharacterClass* wordUnicodeIgnoreCaseCharCached;
    CharacterClass* nondigitsCached;
    CharacterClass* nonspacesCached;
    CharacterClass* nonwordcharCached;
    CharacterClass* nonwordUnicodeIgnoreCasecharCached;
};

} } // namespace JSC::Yarr
