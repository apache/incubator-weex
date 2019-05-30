/*
 * Copyright (C) 2015 Andy VanWagoner (thetalecrafter@gmail.com)
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

#if ENABLE(INTL)

#include "JSDestructibleObject.h"

struct UCollator;

namespace JSC {

class IntlCollatorConstructor;
class JSBoundFunction;

class IntlCollator : public JSDestructibleObject {
public:
    typedef JSDestructibleObject Base;

    static IntlCollator* create(VM&, Structure*);
    static Structure* createStructure(VM&, JSGlobalObject*, JSValue);

    DECLARE_INFO;

    void initializeCollator(ExecState&, JSValue locales, JSValue optionsValue);
    JSValue compareStrings(ExecState&, StringView, StringView);
    JSObject* resolvedOptions(ExecState&);

    JSBoundFunction* boundCompare() const { return m_boundCompare.get(); }
    void setBoundCompare(VM&, JSBoundFunction*);

protected:
    IntlCollator(VM&, Structure*);
    void finishCreation(VM&);
    static void destroy(JSCell*);
    static void visitChildren(JSCell*, SlotVisitor&);

private:
    enum class Usage { Sort, Search };
    enum class Sensitivity { Base, Accent, Case, Variant };

    struct UCollatorDeleter {
        void operator()(UCollator*) const;
    };

    void createCollator(ExecState&);
    static const char* usageString(Usage);
    static const char* sensitivityString(Sensitivity);

    Usage m_usage;
    String m_locale;
    String m_collation;
    Sensitivity m_sensitivity;
    WriteBarrier<JSBoundFunction> m_boundCompare;
    std::unique_ptr<UCollator, UCollatorDeleter> m_collator;
    bool m_numeric;
    bool m_ignorePunctuation;
    bool m_initializedCollator { false };
};

} // namespace JSC

#endif // ENABLE(INTL)
