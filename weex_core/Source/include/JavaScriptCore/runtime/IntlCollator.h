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
