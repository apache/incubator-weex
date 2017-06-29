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
#import "WXSDKInstance.h"

@interface WXComponentTracing:NSObject

@property (nonatomic, copy) NSString *ref;
@property (nonatomic, copy) NSString *classStr;
@property (nonatomic, copy) NSString *name;
@property (nonatomic, copy) NSString *ph;
@property (nonatomic) NSTimeInterval ts;
@property (nonatomic, strong) WXSDKInstance *instance;

@end

@interface WXTracingTask:NSObject

@property (nonatomic, strong) WXSDKInstance *instance;
@property (nonatomic, strong) NSMutableArray *componentTracings;

@end

@interface WXTracingManager : NSObject
+(void)switchTracing:(BOOL)isTracing;
+(BOOL)isTracing;
+(void)monitorComponent:(WXComponentTracing *)componentTracing;
+(double)getMonitorComponent:(WXComponentTracing *)componentTracing;

@end
