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

@interface WXServiceFactory : NSObject

/**
 * @abstract Registers a service for a given name, js code and options
 *
 * @param name The service name to register
 *
 * @param options The service options to register
 *
 * @param code service js code to invoke
 *
 * @return script
 *
 */
+ (NSString *)registerServiceScript:(NSString *)name withRawScript:(NSString *)serviceScript withOptions:(NSDictionary *)options;


/**
 * @abstract Unregisters a service for a given name
 *
 * @param name The service name to register
 *
 * @return script
 *
 */
+ (NSString *)unregisterServiceScript:(NSString *)name;

@end
