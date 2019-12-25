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

#ifndef WX_EAGLE_PLUGIN_MACRO_H
#define WX_EAGLE_PLUGIN_MACRO_H

#import <Foundation/Foundation.h>
#import <WeexSDK/WXEaglePlugin.h>

#define WX_EAGLE_PLUGIN_DATA  __attribute((used, section("__DATA, WXEaglePlugin")))

#define WX_EAGLE_PLUGIN_NAME_SEPARATOR(plugin, class, separator) #plugin#separator#class

#define WX_EAGLE_PLUGIN_NAME(plugin, class)  WX_EAGLE_PLUGIN_NAME_SEPARATOR(plugin, class, &)

#define WX_EAGLE_PlUGIN_EXPORT(plugin, class)  \
char const* k##class##Configuration WX_EAGLE_PLUGIN_DATA = WX_EAGLE_PLUGIN_NAME(plugin, class);

@interface WXEaglePluginManager : NSObject

/**
 * @abstract Returns the render handler for render option
 *
 **/
+ (id<WXEaglePlugin>)renderWithOption:(NSDictionary *)option;

+ (id<WXEaglePlugin>)renderWithURL:(NSURL **)url;

+ (void)registerModules:(NSDictionary *)modules;

+ (void)registerComponents:(NSArray *)components;

@end

#endif // WX_EAGLE_PLUGIN_MACRO_H
