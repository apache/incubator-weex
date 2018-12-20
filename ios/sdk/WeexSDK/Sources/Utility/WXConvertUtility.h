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

id _Nonnull GenValue(weex::core::data_render::Value* _Nonnull value);

NSString* _Nullable TO_JSON(id _Nullable object);

id _Nonnull TO_OBJECT(NSString* _Nonnull s);

NSMutableDictionary* _Nonnull NSDICTIONARY(std::map<std::string, std::string>* _Nullable map);

NSMutableDictionary* _Nonnull NSDICTIONARY(std::unordered_map<std::string, std::string>* _Nullable map);

NSMutableDictionary* _Nonnull NSDICTIONARY(std::vector<std::pair<std::string, std::string>>* _Nullable vec);

NSMutableDictionary* _Nonnull NSDICTIONARY(const std::unordered_map<std::string, weex::core::data_render::VComponent::VNodeRefs>& ref_map);

NSMutableDictionary* _Nonnull NSDICTIONARY(weex::core::data_render::Table* _Nullable table);

NSMutableArray* _Nonnull NSARRAY(std::set<std::string>* _Nullable set);

NSMutableArray* _Nonnull NSARRAY(weex::core::data_render::Array* _Nullable array);

NSMutableArray* _Nonnull NSARRAY(std::vector<std::unordered_map<std::string, std::string>> refs);

NSMutableArray* _Nonnull NSARRAY(const std::vector<weex::core::data_render::Value>& params);

void ConvertToCString(id _Nonnull obj, void (^ _Nonnull callback)(const char* _Nullable));

#endif

#endif
