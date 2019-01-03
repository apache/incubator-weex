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

#include "InternalFunction.h"

namespace JSC {

class IntlDateTimeFormat;
class IntlDateTimeFormatPrototype;

class IntlDateTimeFormatConstructor : public InternalFunction {
public:
    typedef InternalFunction Base;
    static const unsigned StructureFlags = Base::StructureFlags | HasStaticPropertyTable;

    static IntlDateTimeFormatConstructor* create(VM&, Structure*, IntlDateTimeFormatPrototype*, Structure*);
    static Structure* createStructure(VM&, JSGlobalObject*, JSValue);

    DECLARE_INFO;

    Structure* dateTimeFormatStructure() const { return m_dateTimeFormatStructure.get(); }

protected:
    void finishCreation(VM&, IntlDateTimeFormatPrototype*, Structure*);

private:
    IntlDateTimeFormatConstructor(VM&, Structure*);
    static ConstructType getConstructData(JSCell*, ConstructData&);
    static CallType getCallData(JSCell*, CallData&);
    static void visitChildren(JSCell*, SlotVisitor&);
    
    WriteBarrier<Structure> m_dateTimeFormatStructure;
};

} // namespace JSC

#endif // ENABLE(INTL)
