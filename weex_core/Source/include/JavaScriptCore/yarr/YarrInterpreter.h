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

#include "ConcurrentJSLock.h"
#include "YarrPattern.h"

namespace WTF {
class BumpPointerAllocator;
}
using WTF::BumpPointerAllocator;

namespace JSC { namespace Yarr {

class ByteDisjunction;

struct ByteTerm {
    enum Type {
        TypeBodyAlternativeBegin,
        TypeBodyAlternativeDisjunction,
        TypeBodyAlternativeEnd,
        TypeAlternativeBegin,
        TypeAlternativeDisjunction,
        TypeAlternativeEnd,
        TypeSubpatternBegin,
        TypeSubpatternEnd,
        TypeAssertionBOL,
        TypeAssertionEOL,
        TypeAssertionWordBoundary,
        TypePatternCharacterOnce,
        TypePatternCharacterFixed,
        TypePatternCharacterGreedy,
        TypePatternCharacterNonGreedy,
        TypePatternCasedCharacterOnce,
        TypePatternCasedCharacterFixed,
        TypePatternCasedCharacterGreedy,
        TypePatternCasedCharacterNonGreedy,
        TypeCharacterClass,
        TypeBackReference,
        TypeParenthesesSubpattern,
        TypeParenthesesSubpatternOnceBegin,
        TypeParenthesesSubpatternOnceEnd,
        TypeParenthesesSubpatternTerminalBegin,
        TypeParenthesesSubpatternTerminalEnd,
        TypeParentheticalAssertionBegin,
        TypeParentheticalAssertionEnd,
        TypeCheckInput,
        TypeUncheckInput,
        TypeDotStarEnclosure,
    } type;
    union {
        struct {
            union {
                UChar32 patternCharacter;
                struct {
                    UChar32 lo;
                    UChar32 hi;
                } casedCharacter;
                CharacterClass* characterClass;
                unsigned subpatternId;
            };
            union {
                ByteDisjunction* parenthesesDisjunction;
                unsigned parenthesesWidth;
            };
            QuantifierType quantityType;
            unsigned quantityMinCount;
            unsigned quantityMaxCount;
        } atom;
        struct {
            int next;
            int end;
            bool onceThrough;
        } alternative;
        struct {
            bool m_bol : 1;
            bool m_eol : 1;
        } anchors;
        unsigned checkInputCount;
    };
    unsigned frameLocation;
    bool m_capture : 1;
    bool m_invert : 1;
    unsigned inputPosition;

    ByteTerm(UChar32 ch, unsigned inputPos, unsigned frameLocation, Checked<unsigned> quantityCount, QuantifierType quantityType)
        : frameLocation(frameLocation)
        , m_capture(false)
        , m_invert(false)
    {
        atom.patternCharacter = ch;
        atom.quantityType = quantityType;
        atom.quantityMinCount = quantityCount.unsafeGet();
        atom.quantityMaxCount = quantityCount.unsafeGet();
        inputPosition = inputPos;

        switch (quantityType) {
        case QuantifierFixedCount:
            type = (quantityCount == 1) ? ByteTerm::TypePatternCharacterOnce : ByteTerm::TypePatternCharacterFixed;
            break;
        case QuantifierGreedy:
            type = ByteTerm::TypePatternCharacterGreedy;
            break;
        case QuantifierNonGreedy:
            type = ByteTerm::TypePatternCharacterNonGreedy;
            break;
        }
    }

    ByteTerm(UChar32 lo, UChar32 hi, unsigned inputPos, unsigned frameLocation, Checked<unsigned> quantityCount, QuantifierType quantityType)
        : frameLocation(frameLocation)
        , m_capture(false)
        , m_invert(false)
    {
        switch (quantityType) {
        case QuantifierFixedCount:
            type = (quantityCount == 1) ? ByteTerm::TypePatternCasedCharacterOnce : ByteTerm::TypePatternCasedCharacterFixed;
            break;
        case QuantifierGreedy:
            type = ByteTerm::TypePatternCasedCharacterGreedy;
            break;
        case QuantifierNonGreedy:
            type = ByteTerm::TypePatternCasedCharacterNonGreedy;
            break;
        }

        atom.casedCharacter.lo = lo;
        atom.casedCharacter.hi = hi;
        atom.quantityType = quantityType;
        atom.quantityMinCount = quantityCount.unsafeGet();
        atom.quantityMaxCount = quantityCount.unsafeGet();
        inputPosition = inputPos;
    }

    ByteTerm(CharacterClass* characterClass, bool invert, unsigned inputPos)
        : type(ByteTerm::TypeCharacterClass)
        , m_capture(false)
        , m_invert(invert)
    {
        atom.characterClass = characterClass;
        atom.quantityType = QuantifierFixedCount;
        atom.quantityMinCount = 1;
        atom.quantityMaxCount = 1;
        inputPosition = inputPos;
    }

    ByteTerm(Type type, unsigned subpatternId, ByteDisjunction* parenthesesInfo, bool capture, unsigned inputPos)
        : type(type)
        , m_capture(capture)
        , m_invert(false)
    {
        atom.subpatternId = subpatternId;
        atom.parenthesesDisjunction = parenthesesInfo;
        atom.quantityType = QuantifierFixedCount;
        atom.quantityMinCount = 1;
        atom.quantityMaxCount = 1;
        inputPosition = inputPos;
    }
    
    ByteTerm(Type type, bool invert = false)
        : type(type)
        , m_capture(false)
        , m_invert(invert)
    {
        atom.quantityType = QuantifierFixedCount;
        atom.quantityMinCount = 1;
        atom.quantityMaxCount = 1;
    }

    ByteTerm(Type type, unsigned subpatternId, bool capture, bool invert, unsigned inputPos)
        : type(type)
        , m_capture(capture)
        , m_invert(invert)
    {
        atom.subpatternId = subpatternId;
        atom.quantityType = QuantifierFixedCount;
        atom.quantityMinCount = 1;
        atom.quantityMaxCount = 1;
        inputPosition = inputPos;
    }

    static ByteTerm BOL(unsigned inputPos)
    {
        ByteTerm term(TypeAssertionBOL);
        term.inputPosition = inputPos;
        return term;
    }

    static ByteTerm CheckInput(Checked<unsigned> count)
    {
        ByteTerm term(TypeCheckInput);
        term.checkInputCount = count.unsafeGet();
        return term;
    }

    static ByteTerm UncheckInput(Checked<unsigned> count)
    {
        ByteTerm term(TypeUncheckInput);
        term.checkInputCount = count.unsafeGet();
        return term;
    }
    
    static ByteTerm EOL(unsigned inputPos)
    {
        ByteTerm term(TypeAssertionEOL);
        term.inputPosition = inputPos;
        return term;
    }

    static ByteTerm WordBoundary(bool invert, unsigned inputPos)
    {
        ByteTerm term(TypeAssertionWordBoundary, invert);
        term.inputPosition = inputPos;
        return term;
    }
    
    static ByteTerm BackReference(unsigned subpatternId, unsigned inputPos)
    {
        return ByteTerm(TypeBackReference, subpatternId, false, false, inputPos);
    }

    static ByteTerm BodyAlternativeBegin(bool onceThrough)
    {
        ByteTerm term(TypeBodyAlternativeBegin);
        term.alternative.next = 0;
        term.alternative.end = 0;
        term.alternative.onceThrough = onceThrough;
        return term;
    }

    static ByteTerm BodyAlternativeDisjunction(bool onceThrough)
    {
        ByteTerm term(TypeBodyAlternativeDisjunction);
        term.alternative.next = 0;
        term.alternative.end = 0;
        term.alternative.onceThrough = onceThrough;
        return term;
    }

    static ByteTerm BodyAlternativeEnd()
    {
        ByteTerm term(TypeBodyAlternativeEnd);
        term.alternative.next = 0;
        term.alternative.end = 0;
        term.alternative.onceThrough = false;
        return term;
    }

    static ByteTerm AlternativeBegin()
    {
        ByteTerm term(TypeAlternativeBegin);
        term.alternative.next = 0;
        term.alternative.end = 0;
        term.alternative.onceThrough = false;
        return term;
    }

    static ByteTerm AlternativeDisjunction()
    {
        ByteTerm term(TypeAlternativeDisjunction);
        term.alternative.next = 0;
        term.alternative.end = 0;
        term.alternative.onceThrough = false;
        return term;
    }

    static ByteTerm AlternativeEnd()
    {
        ByteTerm term(TypeAlternativeEnd);
        term.alternative.next = 0;
        term.alternative.end = 0;
        term.alternative.onceThrough = false;
        return term;
    }

    static ByteTerm SubpatternBegin()
    {
        return ByteTerm(TypeSubpatternBegin);
    }

    static ByteTerm SubpatternEnd()
    {
        return ByteTerm(TypeSubpatternEnd);
    }
    
    static ByteTerm DotStarEnclosure(bool bolAnchor, bool eolAnchor)
    {
        ByteTerm term(TypeDotStarEnclosure);
        term.anchors.m_bol = bolAnchor;
        term.anchors.m_eol = eolAnchor;
        return term;
    }

    bool invert()
    {
        return m_invert;
    }

    bool capture()
    {
        return m_capture;
    }
};

class ByteDisjunction {
    WTF_MAKE_FAST_ALLOCATED;
public:
    ByteDisjunction(unsigned numSubpatterns, unsigned frameSize)
        : m_numSubpatterns(numSubpatterns)
        , m_frameSize(frameSize)
    {
    }

    size_t estimatedSizeInBytes() const { return terms.capacity() * sizeof(ByteTerm); }

    Vector<ByteTerm> terms;
    unsigned m_numSubpatterns;
    unsigned m_frameSize;
};

struct BytecodePattern {
    WTF_MAKE_FAST_ALLOCATED;
public:
    BytecodePattern(std::unique_ptr<ByteDisjunction> body, Vector<std::unique_ptr<ByteDisjunction>>& parenthesesInfoToAdopt, YarrPattern& pattern, BumpPointerAllocator* allocator, ConcurrentJSLock* lock)
        : m_body(WTFMove(body))
        , m_flags(pattern.m_flags)
        , m_allocator(allocator)
        , m_lock(lock)
    {
        m_body->terms.shrinkToFit();

        newlineCharacterClass = pattern.newlineCharacterClass();
        if (unicode() && ignoreCase())
            wordcharCharacterClass = pattern.wordUnicodeIgnoreCaseCharCharacterClass();
        else
            wordcharCharacterClass = pattern.wordcharCharacterClass();

        m_allParenthesesInfo.swap(parenthesesInfoToAdopt);
        m_allParenthesesInfo.shrinkToFit();

        m_userCharacterClasses.swap(pattern.m_userCharacterClasses);
        m_userCharacterClasses.shrinkToFit();
    }

    size_t estimatedSizeInBytes() const { return m_body->estimatedSizeInBytes(); }
    
    bool ignoreCase() const { return m_flags & FlagIgnoreCase; }
    bool multiline() const { return m_flags & FlagMultiline; }
    bool sticky() const { return m_flags & FlagSticky; }
    bool unicode() const { return m_flags & FlagUnicode; }

    std::unique_ptr<ByteDisjunction> m_body;
    RegExpFlags m_flags;
    // Each BytecodePattern is associated with a RegExp, each RegExp is associated
    // with a VM.  Cache a pointer to out VM's m_regExpAllocator.
    BumpPointerAllocator* m_allocator;
    ConcurrentJSLock* m_lock;

    CharacterClass* newlineCharacterClass;
    CharacterClass* wordcharCharacterClass;

private:
    Vector<std::unique_ptr<ByteDisjunction>> m_allParenthesesInfo;
    Vector<std::unique_ptr<CharacterClass>> m_userCharacterClasses;
};

JS_EXPORT_PRIVATE std::unique_ptr<BytecodePattern> byteCompile(YarrPattern&, BumpPointerAllocator*, ConcurrentJSLock* = nullptr);
JS_EXPORT_PRIVATE unsigned interpret(BytecodePattern*, const String& input, unsigned start, unsigned* output);
unsigned interpret(BytecodePattern*, const LChar* input, unsigned length, unsigned start, unsigned* output);
unsigned interpret(BytecodePattern*, const UChar* input, unsigned length, unsigned start, unsigned* output);

} } // namespace JSC::Yarr
