/*
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

#import "JSValue+Weex.h"
#import <objc/runtime.h>

@implementation JSValue (Weex)

+ (JSValue *)wx_valueWithReturnValueFromInvocation:(NSInvocation *)invocation inContext:(JSContext *)context
{
    if (!invocation || !context) {
        return nil;
    }
    
    const char * returnType = [invocation.methodSignature methodReturnType];
    
    JSValue *returnValue;
    switch (returnType[0] == _C_CONST ? returnType[1] : returnType[0]) {
        case _C_VOID: {
            // 1.void
            returnValue = [JSValue valueWithUndefinedInContext:context];
            break;
        }
        
        case _C_ID: {
            // 2.id
            void *value;
            [invocation getReturnValue:&value];
            id object = (__bridge id)value;
        
            returnValue = [JSValue valueWithObject:[object copy] inContext:context];
            break;
        }
        
#define WX_JS_VALUE_RET_CASE(typeString, type) \
        case typeString: {                      \
            type value;                         \
            [invocation getReturnValue:&value];  \
            returnValue = [JSValue valueWithObject:@(value) inContext:context]; \
            break; \
        }
        // 3.number
        WX_JS_VALUE_RET_CASE(_C_CHR, char)
        WX_JS_VALUE_RET_CASE(_C_UCHR, unsigned char)
        WX_JS_VALUE_RET_CASE(_C_SHT, short)
        WX_JS_VALUE_RET_CASE(_C_USHT, unsigned short)
        WX_JS_VALUE_RET_CASE(_C_INT, int)
        WX_JS_VALUE_RET_CASE(_C_UINT, unsigned int)
        WX_JS_VALUE_RET_CASE(_C_LNG, long)
        WX_JS_VALUE_RET_CASE(_C_ULNG, unsigned long)
        WX_JS_VALUE_RET_CASE(_C_LNG_LNG, long long)
        WX_JS_VALUE_RET_CASE(_C_ULNG_LNG, unsigned long long)
        WX_JS_VALUE_RET_CASE(_C_FLT, float)
        WX_JS_VALUE_RET_CASE(_C_DBL, double)
        WX_JS_VALUE_RET_CASE(_C_BOOL, BOOL)
            
        case _C_STRUCT_B: {
            NSString *typeString = [NSString stringWithUTF8String:returnType];
            
#define WX_JS_VALUE_RET_STRUCT(_type, _methodName)                             \
            if ([typeString rangeOfString:@#_type].location != NSNotFound) {   \
                _type value;                                                   \
                [invocation getReturnValue:&value];                            \
                returnValue = [JSValue _methodName:value inContext:context]; \
                break;                                                         \
            }
            // 4.struct
            WX_JS_VALUE_RET_STRUCT(CGRect, valueWithRect)
            WX_JS_VALUE_RET_STRUCT(CGPoint, valueWithPoint)
            WX_JS_VALUE_RET_STRUCT(CGSize, valueWithSize)
            WX_JS_VALUE_RET_STRUCT(NSRange, valueWithRange)
            
        }
        case _C_CHARPTR:
        case _C_PTR:
        case _C_CLASS: {
            returnValue = [JSValue valueWithUndefinedInContext:context];
            break;
        }
    }
    
    return returnValue;
}

@end
