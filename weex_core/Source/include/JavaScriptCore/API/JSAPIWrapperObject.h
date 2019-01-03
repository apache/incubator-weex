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
#ifndef JSAPIWrapperObject_h
#define JSAPIWrapperObject_h

#include "JSBase.h"
#include "JSDestructibleObject.h"
#include "WeakReferenceHarvester.h"

#if JSC_OBJC_API_ENABLED

namespace JSC {
    
class JSAPIWrapperObject : public JSDestructibleObject {
public:
    typedef JSDestructibleObject Base;
    
    void finishCreation(VM&);
    static void visitChildren(JSCell*, JSC::SlotVisitor&);
    
    void* wrappedObject() { return m_wrappedObject; }
    void setWrappedObject(void*);

protected:
    JSAPIWrapperObject(VM&, Structure*);

private:
    void* m_wrappedObject;
};

} // namespace JSC

#endif // JSC_OBJC_API_ENABLED

#endif // JSAPIWrapperObject_h
