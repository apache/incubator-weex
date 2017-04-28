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

@interface WXModuleFactory : NSObject

/**
 * @abstract Returns the class of specific module
 *
 * @param name The module name
 *
 **/
+ (Class)classWithModuleName:(NSString *)name;

/**
 * @abstract Returns the instance method implemented by the module 
 *
 * @param name The module name
 *
 * @param method The module method
 *
 **/
+ (SEL)selectorWithModuleName:(NSString *)name methodName:(NSString *)method isSync:(BOOL *)isSync;

/**
 * @abstract Registers a module for a given name and the implemented class
 *
 * @param name The module name to register
 *
 * @param clazz The module class to register
 *
 **/
+ (NSString *)registerModule:(NSString *)name withClass:(Class)clazz;

/**
 * @abstract Returns the export methods in the specific module
 *
 * @param name The module name
 **/
+ (NSMutableDictionary *)moduleMethodMapsWithName:(NSString *)name;


/**
 * @abstract Returns the registered modules.
 */
+ (NSDictionary *) moduleConfigs;
       
@end

