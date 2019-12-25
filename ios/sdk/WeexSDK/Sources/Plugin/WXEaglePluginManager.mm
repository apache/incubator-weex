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

#include <mach-o/getsect.h>
#include <mach-o/loader.h>
#include <mach-o/dyld.h>
#include <dlfcn.h>
#import "WXEaglePluginManager.h"
#import "WXThreadSafeMutableDictionary.h"
#import "WXComponentManager.h"

#define WX_EAGLE_PLUGIN_SECTION_NAME   "WXEaglePlugin"

static NSArray<NSString *>* pluginConfigs() {
    static NSMutableArray *configs;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        Dl_info info;
        dladdr((const void *)pluginConfigs, &info);
#ifndef __LP64__
        //        const struct mach_header *mhp = _dyld_get_image_header(0); // both works as below line
        const struct mach_header *mhp = (struct mach_header *)info.dli_fbase;
        unsigned long size = 0;
        uint32_t *memory = (uint32_t *)getsectiondata(mhp, "__DATA", WX_EAGLE_PLUGIN_SECTION_NAME, &size);
#else /* defined(__LP64__) */
        const struct mach_header_64 *mhp = (struct mach_header_64*)info.dli_fbase;
        unsigned long size = 0;
        uint64_t *memory = (uint64_t*)getsectiondata(mhp, "__DATA", WX_EAGLE_PLUGIN_SECTION_NAME, &size);
#endif /* defined(__LP64__) */
        configs = [NSMutableArray new];
        for(int idx = 0; idx < size / sizeof(void *); ++idx) {
            char *string = (char *)memory[idx];
            NSString *str = [NSString stringWithUTF8String:string];
            if (str && [str length] > 0) {
                [configs addObject:str];
            }
        }
    });
    return configs;
}

@interface WXEaglePluginManager ()

@property (nonatomic, strong) WXThreadSafeMutableDictionary *plugins;

- (void)registerModules:(NSDictionary *)modules;
- (void)registerComponents:(NSArray *)components;
- (id<WXEaglePlugin>)renderWithOption:(NSDictionary *)option;
- (id<WXEaglePlugin>)renderWithURL:(NSURL **)url;

@end

@implementation WXEaglePluginManager

- (instancetype)init {
    if (self = [super init]) {
        self.plugins = [[WXThreadSafeMutableDictionary alloc] init];
        [self registerWXPlugins];
    }
    return self;
}

+ (instancetype)sharedInstance {
    static WXEaglePluginManager* instance = nil;
    static dispatch_once_t oncePredicate;
    dispatch_once(&oncePredicate, ^{
        instance = [[self alloc] init];
    });
    return instance;
}

+ (id<WXEaglePlugin>)renderWithOption:(NSDictionary *)option {
    WXEaglePluginManager *manager = [WXEaglePluginManager sharedInstance];
    return [manager renderWithOption:option];
}


+ (id<WXEaglePlugin>)renderWithURL:(NSURL **)url {
    WXEaglePluginManager *manager = [WXEaglePluginManager sharedInstance];
    return [manager renderWithURL:url];
}


+ (void)registerModules:(NSDictionary *)modules {
    WXEaglePluginManager *manager = [WXEaglePluginManager sharedInstance];
    [manager registerModules:modules];
}

+ (void)registerComponents:(NSArray *)components {
    WXEaglePluginManager *manager = [WXEaglePluginManager sharedInstance];
    [manager registerComponents:components];
}

- (void)registerModules:(NSDictionary *)modules {
    NSArray *plugins = [self.plugins allValues];
    for (id<WXEaglePlugin>plugin in plugins) {
        if (plugin.isRegisterModules) {
            WXPerformBlockOnComponentThread(^{
                [plugin registerModules:modules];
            });
        }
    }
}

- (void)registerComponents:(NSArray *)components {
    NSArray *plugins = [self.plugins allValues];
    for (id<WXEaglePlugin>plugin in plugins) {
        if (plugin.isRegisterComponents) {
            WXPerformBlockOnComponentThread(^{
                [plugin registerComponents:components];
            });
        }
    }
}

- (id<WXEaglePlugin>)renderWithOption:(NSDictionary *)option {
    NSArray *plugins = [self.plugins allValues];
    for (id<WXEaglePlugin>plugin in plugins) {
        if ([plugin renderWithOption:option]) {
            return plugin;
        }
    }
    return nil;
}

- (id<WXEaglePlugin>)renderWithURL:(NSURL **)url {
    NSArray *plugins = [self.plugins allValues];
    for (id<WXEaglePlugin>plugin in plugins) {
        NSURL *newURL = [plugin renderWithURL:*url];
        if (newURL) {
            if (url) {
                *url = newURL;
            }
            return plugin;
        }
    }
    return nil;
}

- (void)registerWXPlugins
{
    NSArray *plugins = pluginConfigs();
    for (NSString *plugin in plugins) {
        NSArray<NSString *> *pluginInfo = [plugin componentsSeparatedByString:@"&"];
        if (pluginInfo.count != 2) {
            continue;
        }
        NSString *className = [pluginInfo objectAtIndex:1];
        if (!className.length) {
            continue;
        }
        NSString *registerName = [pluginInfo objectAtIndex:0];
        if (!registerName.length) {
            continue;
        }
        Class pluginClass = NSClassFromString(className);
        if (!pluginClass) {
            continue;
        }
        // register the plugin
        [self.plugins setObject:[[pluginClass alloc] init] forKey:registerName];
    }
}

@end
