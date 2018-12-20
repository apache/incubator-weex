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

NSString* const JSONSTRING_SUFFIX = @"\t\n\t\r";

NSString* TO_JSON(id object)
{
    if (object == nil) {
        return nil;
    }
    
    @try {
        if ([object isKindOfClass:[NSArray class]] || [object isKindOfClass:[NSDictionary class]]) {
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

NSMutableDictionary* NSDICTIONARY(const std::unordered_map<std::string, weex::core::data_render::VComponent::VNodeRefs>& ref_map)
{
    if (ref_map.size() == 0) {
        return [[NSMutableDictionary alloc] init];
    }
    NSMutableDictionary* dic = [[NSMutableDictionary alloc] initWithCapacity:ref_map.size()];
    for (auto it : ref_map) {
        if (it.first.empty()) {
            continue;
        }
        [dic setObject:NSARRAY(it.second) forKey:NSSTRING(it.first.c_str())];
    }
    return dic;
}

NSMutableDictionary* NSDICTIONARY(weex::core::data_render::Table* table)
{
    if (table == nullptr || table->map.size() == 0)
        return [[NSMutableDictionary alloc] init];

    NSMutableDictionary* dic = [[NSMutableDictionary alloc] initWithCapacity:table->map.size()];
    for (auto it=table->map.begin(); it!=table->map.end(); ++it) {
        if (it->first.empty()) {
            continue;
        };
        [dic setObject:GenValue(&it->second) forKey:NSSTRING(it->first.c_str())];
    }
    return dic;
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

NSMutableArray* NSARRAY(weex::core::data_render::Array* array)
{
    if (array == nullptr || array->items.size() == 0)
        return [[NSMutableArray alloc] init];

    NSMutableArray* ns_array = [[NSMutableArray alloc] initWithCapacity:array->items.size()];
    for (auto it=array->items.begin(); it!=array->items.end(); ++it) {
        [ns_array addObject:GenValue(&*it)];
    }
    return ns_array;
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

NSMutableArray* NSARRAY(const std::vector<weex::core::data_render::Value>& params)
{
    if (params.size() == 0) {
        return [[NSMutableArray alloc] init];
    }
    NSMutableArray* array = [[NSMutableArray alloc] initWithCapacity:params.size()];
    for (auto it : params) {
        [array addObject:GenValue(&it)];
    }
    return array;
}

id GenValue(weex::core::data_render::Value* value)
{
    switch (value->type) {
        case weex::core::data_render::Value::Type::ARRAY:
            return NSARRAY( weex::core::data_render::ValueTo<weex::core::data_render::Array>(value));
        case weex::core::data_render::Value::Type::TABLE:
            return NSDICTIONARY( weex::core::data_render::ValueTo<weex::core::data_render::Table>(value));
        case weex::core::data_render::Value::Type::INT:
            return [NSNumber numberWithLong:
                   static_cast<long>(value->i)];
        case weex::core::data_render::Value::Type::NUMBER:
            return [NSNumber numberWithDouble:value->n];
        case weex::core::data_render::Value::Type::STRING:
            return NSSTRING(value->str->c_str());
        case weex::core::data_render::Value::Type::BOOL:
            return [NSNumber numberWithBool:value->b];
        default:
            return [NSNull null];
    }
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
