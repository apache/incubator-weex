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

#include "ArrayBuffer.h"
#include "InternalFunction.h"

namespace JSC {

class JSArrayBufferPrototype;
class GetterSetter;

class JSArrayBufferConstructor : public InternalFunction {
public:
    typedef InternalFunction Base;

protected:
    JSArrayBufferConstructor(VM&, Structure*, ArrayBufferSharingMode);
    void finishCreation(VM&, JSArrayBufferPrototype*, GetterSetter* speciesSymbol);

public:
    static JSArrayBufferConstructor* create(VM&, Structure*, JSArrayBufferPrototype*, GetterSetter* speciesSymbol, ArrayBufferSharingMode);
    
    DECLARE_INFO;
    
    static Structure* createStructure(VM&, JSGlobalObject*, JSValue prototype);
    
    ArrayBufferSharingMode sharingMode() const { return m_sharingMode; }

protected:
    static ConstructType getConstructData(JSCell*, ConstructData&);
    static CallType getCallData(JSCell*, CallData&);

private:
    ArrayBufferSharingMode m_sharingMode;
};

} // namespace JSC
