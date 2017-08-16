/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#pragma once

#include <unicode/ubrk.h>
#include <wtf/Optional.h>
#include <wtf/text/icu/UTextProviderLatin1.h>

#define USE_ICU_CARET_ITERATOR (PLATFORM(MAC) && __MAC_OS_X_VERSION_MIN_REQUIRED < 101200)

namespace WTF {

#if USE_ICU_CARET_ITERATOR
static String caretRules()
{
    return ASCIILiteral(
        // This rule set is based on character-break iterator rules of ICU 57
        // <http://source.icu-project.org/repos/icu/icu/tags/release-57-1/source/data/brkitr/>.
        // The major differences from the original ones are listed below:
        // * Replaced '[\p{Grapheme_Cluster_Break = SpacingMark}]' with '[\p{General_Category = Spacing Mark} - $Extend]' for ICU 3.8 or earlier;
        // * Removed rules that prevent a caret from moving after prepend characters (Bug 24342);
        // * Added rules that prevent a caret from moving after virama signs of Indic languages except Tamil (Bug 15790), and;
        // * Added rules that prevent a caret from moving before Japanese half-width katakara voiced marks.
        // * Added rules for regional indicator symbols.
        "$CR      = [\\p{Grapheme_Cluster_Break = CR}];"
        "$LF      = [\\p{Grapheme_Cluster_Break = LF}];"
        "$Control = [\\p{Grapheme_Cluster_Break = Control}];"
        "$VoiceMarks = [\\uFF9E\\uFF9F];" // Japanese half-width katakana voiced marks
        "$Extend  = [\\p{Grapheme_Cluster_Break = Extend} $VoiceMarks - [\\u0E30 \\u0E32 \\u0E45 \\u0EB0 \\u0EB2]];"
        "$SpacingMark = [[\\p{General_Category = Spacing Mark}] - $Extend];"
        "$L       = [\\p{Grapheme_Cluster_Break = L}];"
        "$V       = [\\p{Grapheme_Cluster_Break = V}];"
        "$T       = [\\p{Grapheme_Cluster_Break = T}];"
        "$LV      = [\\p{Grapheme_Cluster_Break = LV}];"
        "$LVT     = [\\p{Grapheme_Cluster_Break = LVT}];"
        "$Hin0    = [\\u0905-\\u0939];" // Devanagari Letter A,...,Ha
        "$HinV    = \\u094D;" // Devanagari Sign Virama
        "$Hin1    = [\\u0915-\\u0939];" // Devanagari Letter Ka,...,Ha
        "$Ben0    = [\\u0985-\\u09B9];" // Bengali Letter A,...,Ha
        "$BenV    = \\u09CD;" // Bengali Sign Virama
        "$Ben1    = [\\u0995-\\u09B9];" // Bengali Letter Ka,...,Ha
        "$Pan0    = [\\u0A05-\\u0A39];" // Gurmukhi Letter A,...,Ha
        "$PanV    = \\u0A4D;" // Gurmukhi Sign Virama
        "$Pan1    = [\\u0A15-\\u0A39];" // Gurmukhi Letter Ka,...,Ha
        "$Guj0    = [\\u0A85-\\u0AB9];" // Gujarati Letter A,...,Ha
        "$GujV    = \\u0ACD;" // Gujarati Sign Virama
        "$Guj1    = [\\u0A95-\\u0AB9];" // Gujarati Letter Ka,...,Ha
        "$Ori0    = [\\u0B05-\\u0B39];" // Oriya Letter A,...,Ha
        "$OriV    = \\u0B4D;" // Oriya Sign Virama
        "$Ori1    = [\\u0B15-\\u0B39];" // Oriya Letter Ka,...,Ha
        "$Tel0    = [\\u0C05-\\u0C39];" // Telugu Letter A,...,Ha
        "$TelV    = \\u0C4D;" // Telugu Sign Virama
        "$Tel1    = [\\u0C14-\\u0C39];" // Telugu Letter Ka,...,Ha
        "$Kan0    = [\\u0C85-\\u0CB9];" // Kannada Letter A,...,Ha
        "$KanV    = \\u0CCD;" // Kannada Sign Virama
        "$Kan1    = [\\u0C95-\\u0CB9];" // Kannada Letter A,...,Ha
        "$Mal0    = [\\u0D05-\\u0D39];" // Malayalam Letter A,...,Ha
        "$MalV    = \\u0D4D;" // Malayalam Sign Virama
        "$Mal1    = [\\u0D15-\\u0D39];" // Malayalam Letter A,...,Ha
        "$RI      = [\\U0001F1E6-\\U0001F1FF];" // Emoji regional indicators
        "$ZWJ     = \\u200D;" // Zero width joiner
        "$EmojiVar = [\\uFE0F];" // Emoji-style variation selector
        "$EmojiForSeqs = [\\u2640 \\u2642 \\u26F9 \\u2764 \\U0001F308 \\U0001F3C3-\\U0001F3C4 \\U0001F3CA-\\U0001F3CC \\U0001F3F3 \\U0001F441 \\U0001F466-\\U0001F469 \\U0001F46E-\\U0001F46F \\U0001F471 \\U0001F473 \\U0001F477 \\U0001F481-\\U0001F482 \\U0001F486-\\U0001F487 \\U0001F48B \\U0001F575 \\U0001F5E8 \\U0001F645-\\U0001F647 \\U0001F64B \\U0001F64D-\\U0001F64E \\U0001F6A3 \\U0001F6B4-\\U0001F6B6 \\u2695-\\u2696 \\u2708 \\U0001F33E \\U0001F373 \\U0001F393 \\U0001F3A4 \\U0001F3A8 \\U0001F3EB \\U0001F3ED \\U0001F4BB-\\U0001F4BC \\U0001F527 \\U0001F52C \\U0001F680 \\U0001F692 \\U0001F926 \\U0001F937-\\U0001F939 \\U0001F93C-\\U0001F93E];" // Emoji that participate in ZWJ sequences
        "$EmojiForMods = [\\u261D \\u26F9 \\u270A-\\u270D \\U0001F385 \\U0001F3C3-\\U0001F3C4 \\U0001F3CA \\U0001F3CB \\U0001F442-\\U0001F443 \\U0001F446-\\U0001F450 \\U0001F466-\\U0001F478 \\U0001F47C \\U0001F481-\\U0001F483 \\U0001F485-\\U0001F487 \\U0001F4AA \\U0001F575 \\U0001F590 \\U0001F595 \\U0001F596 \\U0001F645-\\U0001F647 \\U0001F64B-\\U0001F64F \\U0001F6A3 \\U0001F6B4-\\U0001F6B6 \\U0001F6C0 \\U0001F918 \\U0001F3C2 \\U0001F3C7 \\U0001F3CC \\U0001F574 \\U0001F57A \\U0001F6CC \\U0001F919-\\U0001F91E \\U0001F926 \\U0001F930 \\U0001F933-\\U0001F939 \\U0001F93C-\\U0001F93E] ;" // Emoji that take Fitzpatrick modifiers
        "$EmojiMods = [\\U0001F3FB-\\U0001F3FF];" // Fitzpatrick modifiers
        "!!chain;"
        "!!RINoChain;"
        "!!forward;"
        "$CR $LF;"
        "$L ($L | $V | $LV | $LVT);"
        "($LV | $V) ($V | $T);"
        "($LVT | $T) $T;"
        "$RI $RI $Extend* / $RI;"
        "$RI $RI $Extend*;"
        "[^$Control $CR $LF] $Extend;"
        "[^$Control $CR $LF] $SpacingMark;"
        "$Hin0 $HinV $Hin1;" // Devanagari Virama (forward)
        "$Ben0 $BenV $Ben1;" // Bengali Virama (forward)
        "$Pan0 $PanV $Pan1;" // Gurmukhi Virama (forward)
        "$Guj0 $GujV $Guj1;" // Gujarati Virama (forward)
        "$Ori0 $OriV $Ori1;" // Oriya Virama (forward)
        "$Tel0 $TelV $Tel1;" // Telugu Virama (forward)
        "$Kan0 $KanV $Kan1;" // Kannada Virama (forward)
        "$Mal0 $MalV $Mal1;" // Malayalam Virama (forward)
        "$ZWJ $EmojiForSeqs;" // Don't break in emoji ZWJ sequences
        "$EmojiForMods $EmojiVar? $EmojiMods;" // Don't break between relevant emoji (possibly with variation selector) and Fitzpatrick modifier
        "!!reverse;"
        "$LF $CR;"
        "($L | $V | $LV | $LVT) $L;"
        "($V | $T) ($LV | $V);"
        "$T ($LVT | $T);"
        "$Extend* $RI $RI / $Extend* $RI $RI;"
        "$Extend* $RI $RI;"
        "$Extend      [^$Control $CR $LF];"
        "$SpacingMark [^$Control $CR $LF];"
        "$Hin1 $HinV $Hin0;" // Devanagari Virama (backward)
        "$Ben1 $BenV $Ben0;" // Bengali Virama (backward)
        "$Pan1 $PanV $Pan0;" // Gurmukhi Virama (backward)
        "$Guj1 $GujV $Guj0;" // Gujarati Virama (backward)
        "$Ori1 $OriV $Ori0;" // Gujarati Virama (backward)
        "$Tel1 $TelV $Tel0;" // Telugu Virama (backward)
        "$Kan1 $KanV $Kan0;" // Kannada Virama (backward)
        "$Mal1 $MalV $Mal0;" // Malayalam Virama (backward)
        "$EmojiForSeqs $ZWJ;" // Don't break in emoji ZWJ sequences
        "$EmojiMods $EmojiVar? $EmojiForMods;" // Don't break between relevant emoji (possibly with variation selector) and Fitzpatrick modifier
        "!!safe_reverse;"
        "$RI $RI+;"
        "[$EmojiVar $EmojiMods]+ $EmojiForMods;"
        "!!safe_forward;"
        "$RI $RI+;"
        "$EmojiForMods [$EmojiVar $EmojiMods]+;"
    );
}
#endif

class TextBreakIteratorICU {
public:
    enum class Mode {
        Line,
        Character,
#if USE_ICU_CARET_ITERATOR
        Caret,
#endif
    };

    void set8BitText(const LChar* buffer, unsigned length)
    {
        UTextWithBuffer textLocal;
        textLocal.text = UTEXT_INITIALIZER;
        textLocal.text.extraSize = sizeof(textLocal.buffer);
        textLocal.text.pExtra = textLocal.buffer;

        UErrorCode status = U_ZERO_ERROR;
        UText* text = openLatin1UTextProvider(&textLocal, buffer, length, &status);
        ASSERT(U_SUCCESS(status));
        ASSERT(text);

        ubrk_setUText(m_iterator, text, &status);
        ASSERT(U_SUCCESS(status));

        utext_close(text);
    }

    TextBreakIteratorICU(StringView string, Mode mode, const char *locale)
    {
        UBreakIteratorType type;
        switch (mode) {
        case Mode::Line:
            type = UBRK_LINE;
            break;
        case Mode::Character:
            type = UBRK_CHARACTER;
            break;
#if USE_ICU_CARET_ITERATOR
        case Mode::Caret:
            type = UBRK_CHARACTER;
            break;
#endif
        default:
            ASSERT_NOT_REACHED();
            type = UBRK_CHARACTER;
            break;
        }

        bool requiresSet8BitText = string.is8Bit();

        const UChar *text = requiresSet8BitText ? nullptr : string.characters16();
        int32_t textLength = requiresSet8BitText ? 0 : string.length();

        // FIXME: Handle weak / normal / strict line breaking.
        UErrorCode status = U_ZERO_ERROR;
#if USE_ICU_CARET_ITERATOR
        if (mode == Mode::Caret) {
            static NeverDestroyed<String> caretRules = WTF::caretRules();
            static NeverDestroyed<StringView::UpconvertedCharacters> upconvertedRules = StringView(caretRules).upconvertedCharacters();
            UParseError parseError;
            m_iterator = ubrk_openRules(upconvertedRules.get(), caretRules.get().length(), text, textLength, &parseError, &status);
        } else
#endif
            m_iterator = ubrk_open(type, locale, text, textLength, &status);
        ASSERT(U_SUCCESS(status));

        if (requiresSet8BitText)
            set8BitText(string.characters8(), string.length());
    }

    TextBreakIteratorICU() = delete;
    TextBreakIteratorICU(const TextBreakIteratorICU&) = delete;

    TextBreakIteratorICU(TextBreakIteratorICU&& other)
        : m_iterator(other.m_iterator)
    {
        other.m_iterator = nullptr;
    }

    TextBreakIteratorICU& operator=(const TextBreakIteratorICU&) = delete;

    TextBreakIteratorICU& operator=(TextBreakIteratorICU&& other)
    {
        if (m_iterator)
            ubrk_close(m_iterator);
        m_iterator = other.m_iterator;
        other.m_iterator = nullptr;
        return *this;
    }

    ~TextBreakIteratorICU()
    {
        if (m_iterator)
            ubrk_close(m_iterator);
    }

    void setText(StringView string)
    {
        if (string.is8Bit()) {
            set8BitText(string.characters8(), string.length());
            return;
        }
        UErrorCode status = U_ZERO_ERROR;
        ubrk_setText(m_iterator, string.characters16(), string.length(), &status);
        ASSERT(U_SUCCESS(status));
    }

    std::optional<unsigned> preceding(unsigned location) const
    {
        auto result = ubrk_preceding(m_iterator, location);
        if (result == UBRK_DONE)
            return { };
        return result;
    }

    std::optional<unsigned> following(unsigned location) const
    {
        auto result = ubrk_following(m_iterator, location);
        if (result == UBRK_DONE)
            return { };
        return result;
    }

    bool isBoundary(unsigned location) const
    {
        return ubrk_isBoundary(m_iterator, location);
    }

private:
    UBreakIterator* m_iterator;
};

}
