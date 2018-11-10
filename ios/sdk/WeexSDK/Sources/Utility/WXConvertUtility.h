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

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#ifndef __WX_CONVERT_UTILITY_H__
#define __WX_CONVERT_UTILITY_H__

#if defined __cplusplus
#include "core/data_render/vnode/vnode_render_manager.h"

#define NSSTRING(cstr) ((__bridge_transfer NSString*)(CFStringCreateWithCString(NULL, (const char *)(cstr), kCFStringEncodingUTF8)))
#define NSSTRING_NO_COPY(cstr) ((__bridge_transfer NSString*)(CFStringCreateWithCStringNoCopy(NULL, (const char *)(cstr), kCFStringEncodingUTF8, kCFAllocatorNull)))

id GenValue(weex::core::data_render::Value* value);

NSString* TO_JSON(id object);

id TO_OBJECT(NSString* s);

NSMutableDictionary* NSDICTIONARY(std::map<std::string, std::string>* map);

NSMutableDictionary* NSDICTIONARY(std::unordered_map<std::string, std::string>* map);

NSMutableDictionary* NSDICTIONARY(std::vector<std::pair<std::string, std::string>>* vec);

NSMutableDictionary* NSDICTIONARY(const std::unordered_map<std::string, weex::core::data_render::VComponent::VNodeRefs>& ref_map);

NSMutableDictionary* NSDICTIONARY(weex::core::data_render::Table* table);

NSMutableArray* NSARRAY(std::set<std::string>* set);

NSMutableArray* NSARRAY(weex::core::data_render::Array* array);

NSMutableArray* NSARRAY(std::vector<std::unordered_map<std::string, std::string>> refs);

NSMutableArray* NSARRAY(const std::vector<weex::core::data_render::Value>& params);

id GenValue(weex::core::data_render::Value* value);

void ConvertToCString(id _Nonnull obj, void (^callback)(const char*));

#endif

#endif
