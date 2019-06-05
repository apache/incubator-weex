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
//
//  wsonjsc.cpp
//  JavaScriptCore
//
//  Created by furture on 2017/8/30.
//
//

#include "android/jsengine/wson/wson_jsc.h"

#include "include/JavaScriptCore/runtime/ObjectConstructor.h"
#include "include/JavaScriptCore/runtime/JSONObject.h"
#include "include/JavaScriptCore/runtime/JSCJSValueInlines.h"
#include <wtf/Vector.h>
#include <wtf/HashMap.h>


#define LOGE(format, ...) LOGE(format, ##__VA_ARGS__)

#ifdef  __ANDROID__
    //#define WSON_JSC_DEBUG  true;
    #include <wtf/unicode/WTFUTF8.h>
    #include "base/log_defines.h"
#else
  #include <wtf/unicode/UTF8.h>
#endif


using namespace JSC;


/**
 * max deep, like JSONObject.cpp's maximumFilterRecursion default 40000
 */
#define WSON_MAX_DEEP  40000
#define WSON_SYSTEM_IDENTIFIER_CACHE_COUNT (1024*4)
#define WSON_LOCAL_IDENTIFIER_CACHE_COUNT 32

namespace wson {
    struct IdentifierCache{
        Identifier identifer = Identifier::EmptyIdentifier;
        size_t length = 0;
        uint32_t key = 0;
        const UChar* utf16 = NULL;
    };

    static  IdentifierCache* systemIdentifyCache = nullptr;
    static  VM* systemIdentifyCacheVM = nullptr;
    void wson_push_js_value(ExecState* exec, JSValue val, wson_buffer* buffer, Vector<JSObject*, 16>& objectStack);
    JSValue wson_to_js_value(ExecState* state, wson_buffer* buffer, IdentifierCache* localIdentifiers, const int& localCount);
    inline void wson_push_js_string(ExecState* exec,  JSValue val, wson_buffer* buffer);
    inline void wson_push_js_identifier(Identifier val, wson_buffer* buffer);
    JSValue call_object_js_value_to_json(ExecState* exec, JSValue val, VM& vm, Identifier* identifier);
    JSValue call_object_js_value_to_json(ExecState* exec, JSValue val, VM& vm, uint32_t index);



    wson_buffer* toWson(ExecState* exec, JSValue val){
        
#ifdef  WSON_JSC_DEBUG  
         LOGE("weex wson pre %s", JSONStringify(exec, val, 0).utf8().data());
#endif
        VM& vm = exec->vm();
        LocalScope localScope(exec->vm());
        Identifier emptyIdentifier = vm.propertyNames->emptyIdentifier;
        if(val.isObject()){
            val = call_object_js_value_to_json(exec, val, vm, &emptyIdentifier);
        }
        wson_buffer* buffer = wson_buffer_new();
        Vector<JSObject*, 16> objectStack;
        wson_push_js_value(exec, val, buffer, objectStack);
        
        
#ifdef  WSON_JSC_DEBUG
        int position = buffer->position;
        buffer->position = 0;
        buffer->length = position;
        JSValue back = toJSValue(exec, buffer);
        LOGE("weex wson back %s", JSONStringify(exec, back, 0).utf8().data());
        buffer->position = position;
#endif
        
        
        return buffer;
    }

    /**
     * if has system cache, local cache size 64, if not, local cache size 128
     */
    JSValue toJSValue(ExecState* exec, wson_buffer* buffer){
        VM& vm =exec->vm();
        LocalScope scope(vm);
        if(systemIdentifyCacheVM && systemIdentifyCacheVM == &vm){
             return wson_to_js_value(exec, buffer, systemIdentifyCache, WSON_SYSTEM_IDENTIFIER_CACHE_COUNT);
        }

        if(buffer->length < 256){
               IdentifierCache localIdentifiers[WSON_LOCAL_IDENTIFIER_CACHE_COUNT];
               return wson_to_js_value(exec, buffer, localIdentifiers, WSON_LOCAL_IDENTIFIER_CACHE_COUNT);
        }

        if(buffer->length < 1024*2){
               IdentifierCache localIdentifiers[WSON_LOCAL_IDENTIFIER_CACHE_COUNT*2];
               return wson_to_js_value(exec, buffer, localIdentifiers, WSON_LOCAL_IDENTIFIER_CACHE_COUNT*2);
        }
        IdentifierCache localIdentifiers[WSON_LOCAL_IDENTIFIER_CACHE_COUNT*4];
        JSValue value =  wson_to_js_value(exec, buffer, localIdentifiers, WSON_LOCAL_IDENTIFIER_CACHE_COUNT*4);
               
        return value;
    }

     JSValue toJSValue(ExecState* state, void* data, int length){
         wson_buffer* buffer = wson_buffer_from(data, length);
#ifdef  WSON_JSC_DEBUG
        LOGE("weex wson toJSValue length %d  %d", buffer->position, buffer->length);
#endif  
         JSValue ret = toJSValue(state, buffer);

#ifdef  WSON_JSC_DEBUG
        LOGE("weex wson toJSValue %s", JSONStringify(state, ret, 0).utf8().data());
        wson_buffer* backBuffer = toWson(state, ret);
        LOGE("weex wson toJSValue backBuffer length %d",  backBuffer->position);
        wson_buffer_free(backBuffer);
#endif  
         buffer->data = nullptr;
         wson_buffer_free(buffer);
         return ret;
     }


    void init(VM* vm){
        if(systemIdentifyCache){
            destory();
        }
        if(systemIdentifyCacheVM){
            systemIdentifyCacheVM = nullptr;
        }
        systemIdentifyCache = new IdentifierCache[WSON_SYSTEM_IDENTIFIER_CACHE_COUNT];
        for(int i=0; i<WSON_SYSTEM_IDENTIFIER_CACHE_COUNT; i++){
            systemIdentifyCache[i].identifer = vm->propertyNames->nullIdentifier;
            systemIdentifyCache[i].length = 0;
            systemIdentifyCache[i].utf16 = NULL;
            systemIdentifyCache[i].key = 0;
        }
        systemIdentifyCacheVM  = vm;
    }


    void destory(){
        if(systemIdentifyCache){
            for(int i=0; i<WSON_SYSTEM_IDENTIFIER_CACHE_COUNT; i++){
                if(systemIdentifyCache[i].length > 0){
                    if(systemIdentifyCache[i].utf16){
                        free((void*)systemIdentifyCache[i].utf16);
                        systemIdentifyCache[i].utf16 = nullptr;
                    }
                    systemIdentifyCache[i].key = 0;
                    systemIdentifyCache[i].length = 0;
                }
            }
            delete[] systemIdentifyCache; 
            systemIdentifyCache = nullptr;
        }
        if(systemIdentifyCacheVM){
            systemIdentifyCacheVM = nullptr;
        }
    }



    /**
     * check is circle reference and  max deep
     */
    inline bool check_js_deep_and_circle_reference(JSObject* object, Vector<JSObject*, 16>& objectStack){
        for (unsigned i = 0; i < objectStack.size(); i++) {
            if (objectStack.at(i) == object) {
                return true;
            }
        }
        if(objectStack.size() > WSON_MAX_DEEP){
            return true;
        }
        return false;
    }

    /**
      * djb2-hash-function
      */
    inline uint32_t hash(const UChar* utf16, const int length){
        uint32_t  hash = 5381;
        for(int i=0;i<length; i++){
            hash = ((hash << 5) + hash) + utf16[i];
        }
        return hash;
    }

    inline uint32_t hash2(const UChar* utf16, const int length){
        uint32_t  hash = 5381;
        hash = ((hash << 5) + hash) + utf16[0];
        if(length > 1){
            hash = ((hash << 5) + hash) + utf16[length-1];
        }
        return hash;
    }

    /**
     * most of json identifer is repeat, cache can improve performance fixme 
     * cache global, improve performance
     */
    inline Identifier makeIdentifer(VM* vm, IdentifierCache* localIdentifiers, const int& localCount, const UChar* utf16, const size_t length){
        if(length <= 0){
           return vm->propertyNames->emptyIdentifier;
        }
        if (utf16[0] <= 0 || utf16[0] >= 127 || length > 32){//only cache short identifier
            return  Identifier::fromString(vm, utf16, length);
        }

        IdentifierCache cache;
        uint32_t key= hash(utf16, length);
        uint32_t localIndex = (localCount - 1) & key;
        cache = localIdentifiers[localIndex];
        if(cache.length == length
           && cache.key == key
           && memcmp(
               (void*)cache.utf16,
               (void*)utf16, length*sizeof(UChar)) == 0
           && !cache.identifer.isNull()){
            return cache.identifer;
        }
        UChar* destination = static_cast<UChar*>(malloc(length*sizeof(UChar) + sizeof(UChar)));
        memset((void*)destination, 0, length*sizeof(UChar) + sizeof(UChar));
        memcpy((void*)destination, (void*)utf16, length*sizeof(UChar));
        Identifier identifier = Identifier::fromString(vm, destination, length);
        cache.identifer = identifier;
        cache.length = length;
        cache.key = key;
        cache.utf16 = destination;
        localIdentifiers[localIndex] = cache;
        return identifier;
    }

    JSValue wson_to_js_value(ExecState* exec, wson_buffer* buffer,  IdentifierCache* localIdentifiers, const int& localCount){
        uint8_t  type = wson_next_type(buffer);
        switch (type) {
            case WSON_UINT8_STRING_TYPE: {
                int size = wson_next_uint(buffer);
                uint8_t *utf8 = wson_next_bts(buffer, size);
                String s = String::fromUTF8(reinterpret_cast<char *>(utf8), size);
                return jsString(exec, s);
            }
            case WSON_STRING_TYPE:
            case WSON_NUMBER_BIG_INT_TYPE:
            case WSON_NUMBER_BIG_DECIMAL_TYPE:{
                    uint32_t length = wson_next_uint(buffer);
                    UChar* destination;
                    String s = String::createUninitialized(length/sizeof(UChar), destination);
                    void* src = wson_next_bts(buffer, length);
                    memcpy(destination, src, length);
                    return jsString(exec, s);
                }
                break;
            case WSON_ARRAY_TYPE:{
                    uint32_t length = wson_next_uint(buffer);
                    JSArray* array = constructEmptyArray(exec, 0);
                    for(uint32_t i=0; i<length; i++){
                        if(wson_has_next(buffer)){
                            array->putDirectIndex(exec, i, wson_to_js_value(exec, buffer, localIdentifiers, localCount));
                        }else{
                            break;
                        }
                    }
                   return array;
                }
                break;
            case WSON_MAP_TYPE:{
                  uint32_t length = wson_next_uint(buffer);
                  JSObject* object = constructEmptyObject(exec);
                  VM& vm = exec->vm();
                  for(uint32_t i=0; i<length; i++){
                      if(wson_has_next(buffer)){
                          int propertyLength = wson_next_uint(buffer);
                          const UChar* data = (const UChar*)wson_next_bts(buffer, propertyLength);
                          Identifier  identifer = makeIdentifer(&vm, localIdentifiers, localCount, data, propertyLength/sizeof(UChar));
                          PropertyName name = identifer;
                          if (std::optional<uint32_t> index = parseIndex(name)){
                              object->putDirectIndex(exec, index.value(), wson_to_js_value(exec, buffer, localIdentifiers, localCount));
                          }else{
                              object->putDirect(vm, name, wson_to_js_value(exec, buffer, localIdentifiers, localCount));
                          }
                      }else{
                          break;
                      }
                   }
                   return object;
                }
                break;   
           case WSON_NUMBER_INT_TYPE:{
                    int32_t  num =  wson_next_int(buffer);
                    return  jsNumber(num);
                } 
                break;  
            case WSON_BOOLEAN_TYPE_TRUE:{
                  return jsBoolean(true);
               }
               break;
            case WSON_BOOLEAN_TYPE_FALSE:{
                return jsBoolean(false);
                 }
                 break;   
            case WSON_NUMBER_DOUBLE_TYPE:{
                 double  num = wson_next_double(buffer);
                 return  jsNumber(num);
                }
                break;
           case WSON_NUMBER_FLOAT_TYPE:{
                 float  num = wson_next_float(buffer);
                 return  jsNumber(num);
                }
                break;
            case WSON_NUMBER_LONG_TYPE:{
                  int64_t  num = wson_next_long(buffer);
                  return  jsNumber(num);
                }
                break;
            case WSON_NULL_TYPE:{
                 return  jsNull();
                }
                break;    
            default:
#ifdef __ANDROID__
                LOGE("weex weex wson err  wson_to_js_value  unhandled type %d buffer position  %d length %d", type, buffer->position, buffer->length);
#endif
                break;
        }
        return jsNull();
    }

    
    JSValue call_object_js_value_to_json(ExecState* exec, JSValue val, VM& vm, Identifier* identifier){
     
        JSObject* object = asObject(val);
        PropertySlot slot(object, PropertySlot::InternalMethodType::Get);
        bool hasProperty = object->getPropertySlot(exec, vm.propertyNames->toJSON, slot);
        if (hasProperty){
            JSValue toJSONFunction = slot.getValue(exec, vm.propertyNames->toJSON);
            CallType callType;
            CallData callData;
            if (toJSONFunction.isCallable(callType, callData)){
                MarkedArgumentBuffer args;
                args.append(jsString(exec, identifier->string()));
                return call(exec, asObject(toJSONFunction), callType, callData, val, args);
            }
        }
        return val;
    }
    
    JSValue call_object_js_value_to_json(ExecState* exec, JSValue val, VM& vm, uint32_t index){
        JSObject* object = asObject(val);
        PropertySlot slot(object, PropertySlot::InternalMethodType::Get);
        bool hasProperty = object->getPropertySlot(exec, vm.propertyNames->toJSON, slot);
        if (hasProperty){
            JSValue toJSONFunction = slot.getValue(exec, vm.propertyNames->toJSON);
            CallType callType;
            CallData callData;
            if (toJSONFunction.isCallable(callType, callData)){
                MarkedArgumentBuffer args;
                if(index <= 9){
                    args.append(vm.smallStrings.singleCharacterString(index + '0'));
                }else{
                    args.append(jsNontrivialString(&vm, vm.numericStrings.add(index)));
                }
                return call(exec, asObject(toJSONFunction), callType, callData, val, args);
            }
        }
        return val;
    }
    
    void wson_push_js_value(ExecState* exec, JSValue val, wson_buffer* buffer, Vector<JSObject*, 16>& objectStack){
        // check json function
        if(val.isNull() || val.isUndefined() || val.isEmpty()){
            wson_push_type_null(buffer);
            return;
        }

        if(val.isString()){
            wson_push_js_string(exec, val, buffer);
            return;
        }

        if(val.isNumber()){
            if(val.isInt32()){
                wson_push_type_int(buffer, val.asInt32());
                return;
            }
            
            if(val.isAnyInt()){
                int64_t int64Number = val.asAnyInt();
                wson_push_type_long(buffer, int64Number);
                return;
            }
            
            if(val.isDouble()){
                double number = val.asDouble();
                wson_push_type_double(buffer, number);
                return;
            }
            wson_push_js_string(exec, val, buffer);
            return;
        }

        if(isJSArray(val)){
            JSArray* array = asArray(val);
            if(check_js_deep_and_circle_reference(array, objectStack)){
                wson_push_type_null(buffer);
                return;
            }
            VM& vm = exec->vm();
            uint32_t length = array->length();
            wson_push_type_array(buffer, length);
            objectStack.append(array);
            for(uint32_t index=0; index<length; index++){
                JSValue ele = array->getIndex(exec, index);
                if(ele.isObject()){
                     ele = call_object_js_value_to_json(exec, ele, vm, index);
                }
                wson_push_js_value(exec, ele, buffer, objectStack);
            }
            objectStack.removeLast();
            return;
        }

        if(val.isObject() && !val.isFunction()){
            JSObject* object = asObject(val);
            VM& vm = exec->vm();

            if (object->inherits(vm, StringObject::info())){
                wson_push_js_string(exec, object->toString(exec), buffer);
                return;
            }
            if (object->inherits(vm, NumberObject::info())){
                JSValue number = jsNumber(object->toNumber(exec));
                if(number.isInt32()){
                    wson_push_type_int(buffer, number.asInt32());
                    return;
                }
                
                if(number.isAnyInt()){
                    int64_t int64Number = number.asAnyInt();
                    wson_push_type_long(buffer, int64Number);
                    return;
                }
                
                if(val.isDouble()){
                    double d = val.asDouble();
                    wson_push_type_double(buffer, d);
                    return;
                }
                wson_push_js_string(exec, number, buffer);
                return;
            }

            if (object->inherits(vm, BooleanObject::info())){
                JSValue boolVal =  object->toPrimitive(exec);
                if(boolVal.isTrue()){
                    wson_push_type_boolean(buffer, 1);
                }else{
                    wson_push_type_boolean(buffer, 0);
                }
                return;
            }

            if(check_js_deep_and_circle_reference(object, objectStack)){
                wson_push_type_null(buffer);
                return;
            }
#ifdef __ANDROID__
            PropertyNameArray objectPropertyNames(exec, PropertyNameMode::Strings);
#else
            PropertyNameArray objectPropertyNames(&vm, PropertyNameMode::Strings, PrivateSymbolMode::Exclude);
#endif
            const MethodTable* methodTable = object->methodTable();
            methodTable->getOwnPropertyNames(object, exec, objectPropertyNames, EnumerationMode());
            PropertySlot slot(object, PropertySlot::InternalMethodType::Get);
            uint32_t size = objectPropertyNames.size();
            /**map should skip null or function */
            /** first check skip null or function,calc null or function */
            uint32_t undefinedOrFunctionSize  = 0;
            for(uint32_t i=0; i<size; i++){
                 Identifier& propertyName = objectPropertyNames[i];
                 if(methodTable->getOwnPropertySlot(object, exec, propertyName, slot)){
                     JSValue propertyValue = slot.getValue(exec, propertyName);
                     if(propertyValue.isUndefined() || propertyValue.isFunction()){
                         undefinedOrFunctionSize++;
                     }
                 }else{
                      undefinedOrFunctionSize++;
                 }
            }
            /** skip them undefined or function value */
            wson_push_type_map(buffer, size - undefinedOrFunctionSize);
            objectStack.append(object);
            for(uint32_t i=0; i<size; i++){
                 Identifier& propertyName = objectPropertyNames[i];
                 if(methodTable->getOwnPropertySlot(object, exec, propertyName, slot)){
                     JSValue propertyValue = slot.getValue(exec, propertyName);
                     if(propertyValue.isUndefined() || propertyValue.isFunction()){
                          continue;
                     }
                     if(propertyValue.isObject()){
                         propertyValue = call_object_js_value_to_json(exec, propertyValue, vm, &propertyName);
                     }
                     wson_push_js_identifier(propertyName , buffer);
                     wson_push_js_value(exec, propertyValue, buffer,objectStack);
                 }
            }
            objectStack.removeLast();
            return;
        }
        
        if(val.isBoolean()){
            if(val.isTrue()){
                 wson_push_type_boolean(buffer, 1);
            }else{
                wson_push_type_boolean(buffer, 0);
            }
            return;
        }
        if(val.isFunction()){
            wson_push_type_null(buffer);
            return;
        }
#ifdef __ANDROID__
        LOGE("weex wson err value type is not handled, treat as null, json value %s %d %d ", JSONStringify(exec, val, 0).utf8().data(), val.isFunction(), val.tag());
        for(size_t i=0; i<objectStack.size(); i++){
            LOGE("weex wson err value type is not handled, treat as null, root json value %s", JSONStringify(exec, objectStack[i], 0).utf8().data());
        }
#endif
        wson_push_type_null(buffer);
    }

    inline void wson_push_js_string(ExecState* exec,  JSValue val, wson_buffer* buffer){
        String s = val.toWTFString(exec);
        size_t length = s.length();
        if (s.is8Bit()) {
            // Convert latin1 chars to unicode.
            wson_push_type(buffer, WSON_STRING_TYPE);
            wson_push_uint(buffer, length*sizeof(UChar));
            wson_buffer_require(buffer, length*sizeof(UChar));
            UChar* jchars = (UChar*)((uint8_t*)buffer->data + buffer->position);
            for (unsigned i = 0; i < length; i++) {
#ifdef __ANDROID__
                jchars[i] = s.at(i);
#else
                jchars[i] = s.characterAt(i);
#endif
            }
            buffer->position +=length*sizeof(UChar);
       } else {
           wson_push_type(buffer, WSON_STRING_TYPE);
           wson_push_uint(buffer, length*sizeof(UChar));
           wson_push_bytes(buffer, s.characters16(), s.length()*sizeof(UChar));
        }

    }

    inline void wson_push_js_identifier(Identifier val, wson_buffer* buffer){
         String s = val.string();
         size_t  length = s.length();
         if (s.is8Bit()) {
            // Convert latin1 chars to unicode.
             wson_push_uint(buffer, length*sizeof(UChar));
             wson_buffer_require(buffer, length*sizeof(UChar));
             UChar* jchars = (UChar*)((uint8_t*)buffer->data + buffer->position);
             
             for (unsigned i = 0; i < length; i++) {
#ifdef __ANDROID__
                 jchars[i] = s.at(i);
#else
                 jchars[i] = s.characterAt(i);
#endif
             }
             buffer->position +=length*sizeof(UChar);
        } else { 
           wson_push_uint(buffer, length*sizeof(UChar));
           wson_push_bytes(buffer, s.characters16(), s.length()*sizeof(UChar));
        }
    }
}
