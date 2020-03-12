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

NS_ASSUME_NONNULL_BEGIN

@protocol WXEaglePlugin <NSObject>

@required

@property (nonatomic, readonly) NSString* pluginName;
@property (nonatomic, readonly) BOOL isSkipFramework;
@property (nonatomic, readonly) BOOL isSupportInvokeJSCallBack;
@property (nonatomic, readonly) BOOL isSupportExecScript;
@property (nonatomic, readonly) BOOL isSupportFireEvent;
@property (nonatomic, readonly) BOOL isRegisterModules;
@property (nonatomic, readonly) BOOL isRegisterComponents;
@property (nonatomic, readonly) NSDictionary *options;

- (BOOL)renderWithOption:(NSDictionary *)option;

- (NSURL *)renderWithURL:(NSURL *)url;

- (void)refreshInstance:(NSString *)pageId data:(id)data;

- (void)createPage:(NSString *)pageId contents:(id)contents options:(NSDictionary * _Nullable)options data:(id)data;

- (BOOL)runPluginTask:(NSString *)pageId task:(NSString *)task options:(id)options;

@optional

- (void)fireEvent:(NSString *)pageId ref:(NSString *)ref event:(NSString *)event args:(NSDictionary * _Nullable)args domChanges:(NSDictionary * _Nullable)domChanges;

- (void)registerModules:(NSDictionary *)modules;
    
- (void)registerComponents:(NSArray *)components;
    
- (void)invokeCallBack:(NSString *)pageId function:(NSString *)functionId args:(NSDictionary * _Nullable)args keepAlive:(BOOL)keepAlive;

- (void)DispatchPageLifecycle:(NSString *)pageId;

- (void)updateInstance:(NSString *)pageId component:(NSString *)componentId jsonData:(id)jsonData;

@end

NS_ASSUME_NONNULL_END
