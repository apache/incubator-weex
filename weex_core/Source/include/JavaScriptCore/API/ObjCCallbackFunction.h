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
#ifndef ObjCCallbackFunction_h
#define ObjCCallbackFunction_h 

#include <JavaScriptCore/JSBase.h>

#if JSC_OBJC_API_ENABLED

#import <JavaScriptCore/JSCallbackFunction.h>

#if defined(__OBJC__)
JSObjectRef objCCallbackFunctionForMethod(JSContext *, Class, Protocol *, BOOL isInstanceMethod, SEL, const char* types);
JSObjectRef objCCallbackFunctionForBlock(JSContext *, id);
JSObjectRef objCCallbackFunctionForInit(JSContext *, Class, Protocol *, SEL, const char* types);

id tryUnwrapConstructor(JSC::VM*, JSObjectRef);
#endif

namespace JSC {

class ObjCCallbackFunctionImpl;

class ObjCCallbackFunction : public InternalFunction {
    friend struct APICallbackFunction;
public:
    typedef InternalFunction Base;

    static ObjCCallbackFunction* create(VM&, JSGlobalObject*, const String& name, std::unique_ptr<ObjCCallbackFunctionImpl>);
    static void destroy(JSCell*);

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        ASSERT(globalObject);
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
    }

    DECLARE_EXPORT_INFO;

    ObjCCallbackFunctionImpl* impl() const { return m_impl.get(); }

protected:
    ObjCCallbackFunction(VM&, Structure*, JSObjectCallAsFunctionCallback, JSObjectCallAsConstructorCallback, std::unique_ptr<ObjCCallbackFunctionImpl>);

private:
    static CallType getCallData(JSCell*, CallData&);
    static ConstructType getConstructData(JSCell*, ConstructData&);

    JSObjectCallAsFunctionCallback functionCallback() { return m_functionCallback; }
    JSObjectCallAsConstructorCallback constructCallback() { return m_constructCallback; }

    JSObjectCallAsFunctionCallback m_functionCallback;
    JSObjectCallAsConstructorCallback m_constructCallback;
    std::unique_ptr<ObjCCallbackFunctionImpl> m_impl;
};

} // namespace JSC

#endif

#endif // ObjCCallbackFunction_h 
