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

#import "WXConvertUtility.h"
#import "WXLog.h"
#import "WXAssert.h"
#import "WXExceptionUtils.h"
#import "WXSDKError.h"

#include <vector>
#include <string>

static NSString* const JSONSTRING_SUFFIX = @"\t\n\t\r";
static NSString* const OBJC_MRC_SUFFIX = @"\t\t\n\r";

static BOOL bIsIOS13 = NO;
static BOOL bUseMRCForInvalidJSONObject = NO;
static BOOL bAlwaysUseMRC = NO;

static NSString* sCurrentPage = nil;

void SetConvertCurrentPage(NSString* pageId)
{
    sCurrentPage = pageId;
}

void ConvertSwitches(BOOL isIOS13, BOOL invalidJSONObjectUseMRC, BOOL alwaysUseMRC)
{
    bIsIOS13 = isIOS13;
    bUseMRCForInvalidJSONObject = invalidJSONObjectUseMRC;
    bAlwaysUseMRC = alwaysUseMRC;
}

#if 0

void _detectObjectRecursion(id object, NSMutableSet* nodes)
{
    if (object == nil) {
        return;
    }
    if ([object isKindOfClass:[NSString class]] ||
        [object isKindOfClass:[NSNumber class]] ||
        [object isKindOfClass:[NSNull class]]) {
        return;
    }
    
    if ([object isKindOfClass:[NSArray class]]) {
        for (id subobj in object) {
            if ([nodes containsObject:subobj]) {
                NSLog(@"Find recursion.");
            }
            [nodes addObject:subobj];
            _detectObjectRecursion(subobj, nodes);
            [nodes removeObject:subobj];
        }
    }
    else if ([object isKindOfClass:[NSDictionary class]]) {
        NSArray* allKeys = [object allKeys];
        for (id key in allKeys) {
            id subobj = object[key];
            if ([nodes containsObject:subobj]) {
                NSLog(@"Find recursion.");
            }
            [nodes addObject:subobj];
            _detectObjectRecursion(subobj, nodes);
            [nodes removeObject:subobj];
        }
    }
}

#endif

NSString* TO_JSON(id object)
{
    if (object == nil) {
        return nil;
    }
    
    @try {
        if ([object isKindOfClass:[NSArray class]] || [object isKindOfClass:[NSDictionary class]]) {
            
#if 0
            NSMutableSet* nodes = [[NSMutableSet alloc] init];
            _detectObjectRecursion(object, nodes);
#endif
            
            if (bAlwaysUseMRC) {
                return [NSString stringWithFormat:@"%p%@", (__bridge_retained void*)object, OBJC_MRC_SUFFIX];
            }
            
            if (bIsIOS13) {
                if (![NSJSONSerialization isValidJSONObject:object]) {
                    [WXExceptionUtils commitCriticalExceptionRT:sCurrentPage
                                                        errCode:[NSString stringWithFormat:@"%d", WX_KEY_EXCEPTION_INVALID_JSON_OBJECT]
                                                       function:@""
                                                      exception:@"Invalid JSON object."
                                                      extParams:nil];
                    
                    // Report for instance.
                    if (bUseMRCForInvalidJSONObject) {
                        return [NSString stringWithFormat:@"%p%@", (__bridge_retained void*)object, OBJC_MRC_SUFFIX];
                    }
                    else {
                        return nil;
                    }
                }
            }
            
            NSError *error = nil;
            NSData *data = [NSJSONSerialization dataWithJSONObject:object
                                                           options:0
                                                             error:&error];
            
            if (error) {
                WXLogError(@"%@", error);
                WXAssert(NO, @"Fail to convert object to json. %@", error);
                return nil;
            }
            
            return [[[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding] stringByAppendingString:JSONSTRING_SUFFIX]; // add suffix so that we know this is a json string
        }
    } @catch (NSException *exception) {
        WXLogError(@"%@", exception);
        WXAssert(NO, @"Fail to convert object to json. %@", exception);
        return nil;
    }
    
    return nil;
}

id TO_OBJECT(NSString* s)
{
    if ([s hasSuffix:JSONSTRING_SUFFIX]) {
        if ([s length] == [JSONSTRING_SUFFIX length]) {
            return [NSNull null];
        }
        
        // s is a json string
        @try {
            NSError* error = nil;
            id jsonObj = [NSJSONSerialization JSONObjectWithData:[s dataUsingEncoding:NSUTF8StringEncoding]
                                                         options:NSJSONReadingMutableContainers | NSJSONReadingMutableLeaves
                                                           error:&error];
            
            if (jsonObj == nil) {
                WXLogError(@"%@", error);
                WXAssert(NO, @"Fail to convert json to object. %@", error);
            }
            else {
                return jsonObj;
            }
        } @catch (NSException *exception) {
            WXLogError(@"%@", exception);
            WXAssert(NO, @"Fail to convert json to object. %@", exception);
        }
    }
    else if ([s hasSuffix:OBJC_MRC_SUFFIX]) {
        NSScanner* scanner = [NSScanner scannerWithString:s];
        unsigned long long address = 0;
        [scanner scanHexLongLong:&address];
        if (address != 0) {
            return (__bridge_transfer id)((void*)address);
        }
    }
    
    return s; // return s instead
}

NSMutableDictionary* NSDICTIONARY(std::map<std::string, std::string>* map)
{
    if (map == nullptr || map->size() == 0)
        return [[NSMutableDictionary alloc] init];
    
    NSMutableDictionary* result = [[NSMutableDictionary alloc] initWithCapacity:map->size()];
    for (auto it = map->begin(); it != map->end(); it ++) {
        id object = TO_OBJECT(NSSTRING(it->second.c_str()));
        if (object) {
            [result setObject:object forKey:NSSTRING(it->first.c_str())];
        }
    }
    return result;
}

NSMutableDictionary* NSDICTIONARY(std::unordered_map<std::string, std::string>* map)
{
    if (map == nullptr || map->size() == 0)
        return [[NSMutableDictionary alloc] init];
    
    NSMutableDictionary* result = [[NSMutableDictionary alloc] initWithCapacity:map->size()];
    for (auto it = map->begin(); it != map->end(); it ++) {
        id object = TO_OBJECT(NSSTRING(it->second.c_str()));
        if (object) {
            [result setObject:object forKey:NSSTRING(it->first.c_str())];
        }
    }
    return result;
}

NSMutableDictionary* NSDICTIONARY(std::vector<std::pair<std::string, std::string>>* vec)
{
    if (vec == nullptr || vec->size() == 0)
        return [[NSMutableDictionary alloc] init];
    
    NSMutableDictionary* result = [[NSMutableDictionary alloc] initWithCapacity:vec->size()];
    for (auto& p : *vec) {
        id object = TO_OBJECT(NSSTRING(p.second.c_str()));
        if (object) {
            [result setObject:object forKey:NSSTRING(p.first.c_str())];
        }
    }
    return result;
}

NSMutableArray* NSARRAY(std::set<std::string>* set)
{
    if (set == nullptr || set->size() == 0)
        return [[NSMutableArray alloc] init];
    
    NSMutableArray* result = [[NSMutableArray alloc] initWithCapacity:set->size()];
    for (auto& s : *set) {
        id object = TO_OBJECT(NSSTRING(s.c_str()));
        if (object) {
            [result addObject:object];
        }
    }
    return result;
}

NSMutableArray* NSARRAY(std::vector<std::unordered_map<std::string, std::string>> refs)
{
    if (refs.size() == 0)
        return [[NSMutableArray alloc] init];

    NSMutableArray* ns_array = [[NSMutableArray alloc] initWithCapacity:refs.size()];
    for (auto it : refs) {
        [ns_array addObject:NSDICTIONARY(&it)];
    }
    return ns_array;
}

void ConvertToCString(id _Nonnull obj, void (^callback)(const char*))
{
    if ([obj isKindOfClass:[NSString class]]) {
        callback([obj UTF8String]);
    }
    else if ([obj isKindOfClass:[NSNumber class]]) {
        callback([[(NSNumber*)obj stringValue] UTF8String]);
    }
    else if ([obj isKindOfClass:[NSNull class]]) {
        callback([JSONSTRING_SUFFIX UTF8String]);
    }
    else {
        NSString* jsonstring = TO_JSON(obj);
        if (jsonstring != nil) {
            callback([jsonstring UTF8String]);
        }
    }
}
