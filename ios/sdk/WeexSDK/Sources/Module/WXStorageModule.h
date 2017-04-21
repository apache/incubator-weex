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

#import "WXModuleProtocol.h"

@interface WXStorageModule : NSObject <WXModuleProtocol>

- (void)length:(WXModuleCallback)callback;

- (void)getAllKeys:(WXModuleCallback)callback;

- (void)setItem:(NSString *)key value:(NSString *)value callback:(WXModuleCallback)callback;

- (void)setItemPersistent:(NSString *)key value:(NSString *)value callback:(WXModuleCallback)callback;

- (void)getItem:(NSString *)key callback:(WXModuleCallback)callback;

- (void)removeItem:(NSString *)key callback:(WXModuleCallback)callback;

@end
