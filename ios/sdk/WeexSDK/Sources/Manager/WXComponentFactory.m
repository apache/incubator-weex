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

#import "WXComponentFactory.h"
#import "WXAssert.h"
#import "WXLog.h"
#import "WXCoreBridge.h"
#import "WXComponent.h"
#import <objc/runtime.h>

@interface WXComponentBaseType : NSObject

@property (nonatomic, strong) NSString* type;
@property (nonatomic, strong) Class clazz;

@end

@implementation WXComponentBaseType

@end

@implementation WXComponentConfig

- (instancetype)initWithName:(NSString *)name class:(NSString *)clazz pros:(NSDictionary *)pros
{
    if (self = [super initWithName:name class:clazz]) {
        _properties = pros;
    }
    
    return self;
}

- (BOOL)isValid
{
    if (self.name == nil || self.clazz == nil) {
        return NO;
    }
    return YES;
}

@end

@implementation WXComponentFactory
{
    NSMutableArray<WXComponentBaseType*> *_baseTypes;
    NSMutableDictionary *_componentConfigs;
    NSLock *_configLock;
}

#pragma mark Life Cycle

+ (instancetype)sharedInstance {
    static id _sharedInstance = nil;
    static dispatch_once_t oncePredicate;
    dispatch_once(&oncePredicate, ^{
        _sharedInstance = [[self alloc] init];
    });
    return _sharedInstance;
}

- (instancetype)init
{
    if(self = [super init]){
        _baseTypes = [NSMutableArray array];
        _componentConfigs = [NSMutableDictionary dictionary];
        _configLock = [[NSLock alloc] init];
    }
    return self;
}

#pragma mark Public

+ (Class)classWithComponentName:(NSString *)name
{
    WXComponentConfig *config = [self configWithComponentName:name];
    if(!config || !config.clazz) {
        return nil;
    }
    
    return NSClassFromString(config.clazz);
}

+ (WXComponentConfig *)configWithComponentName:(NSString *)name
{
    return [[self sharedInstance] configWithComponentName:name];
}

+ (void)registerBaseType:(NSString *)typeName withClass:(Class)clazz
{
    [[self sharedInstance] registerBaseType:typeName withClass:clazz];
}

+ (void)registerComponent:(NSString *)name withClass:(Class)clazz withPros:(NSDictionary *)pros
{
    [[self sharedInstance] registerComponent:name withClass:clazz withPros:pros];
}

+ (void)registerComponents:(NSArray *)components
{
    [[self sharedInstance] registerComponents:components];
}

+ (void)unregisterAllComponents
{
    [[self sharedInstance] unregisterAllComponents];
}

+ (NSDictionary *)componentConfigs {
    return [[self sharedInstance] getComponentConfigs];
}

+ (SEL)methodWithComponentName:(NSString *)name withMethod:(NSString *)method {
    return [[self sharedInstance] _methodWithComponetName:name withMethod:method isSync:NULL];
}

+ (SEL)methodWithComponentName:(NSString *)name withMethod:(NSString *)method isSync:(BOOL *)isSync
{
    return [[self sharedInstance] _methodWithComponetName:name withMethod:method isSync:isSync];
}

+ (NSMutableDictionary *)componentMethodMapsWithName:(NSString *)name
{
    return [[self sharedInstance] _componentMethodMapsWithName:name];
}

+ (NSMutableDictionary *)componentSelectorMapsWithName:(NSString *)name
{
    return [[self sharedInstance] _componentSelectorMapsWithName:name];
}

#pragma mark Private

- (NSMutableDictionary *)_componentMethodMapsWithName:(NSString *)name
{
    NSMutableDictionary *dict = [NSMutableDictionary dictionary];
    NSMutableArray *methods = [NSMutableArray array];
    
    [_configLock lock];
    [dict setValue:methods forKey:@"methods"];
    
    WXComponentConfig *config = _componentConfigs[name];
    void (^mBlock)(id, id, BOOL *) = ^(id mKey, id mObj, BOOL * mStop) {
        [methods addObject:mKey];
    };
    [config.asyncMethods enumerateKeysAndObjectsUsingBlock:mBlock];
    [config.syncMethods enumerateKeysAndObjectsUsingBlock:mBlock];
    [_configLock unlock];
    
    return dict;
}

- (NSMutableDictionary *)_componentSelectorMapsWithName:(NSString *)name
{
    NSMutableDictionary *dict = [NSMutableDictionary dictionary];
    NSMutableArray *methods = [NSMutableArray array];
    
    [_configLock lock];
    [dict setValue:methods forKey:@"methods"];
    
    WXComponentConfig *config = _componentConfigs[name];
    void (^mBlock)(id, id, BOOL *) = ^(id mKey, id mObj, BOOL * mStop) {
        [methods addObject:mObj];
    };
    [config.asyncMethods enumerateKeysAndObjectsUsingBlock:mBlock];
    [config.syncMethods enumerateKeysAndObjectsUsingBlock:mBlock];
    [_configLock unlock];
    
    return dict;
}

- (SEL)_methodWithComponetName:(NSString *)name withMethod:(NSString *)method isSync:(BOOL *)isSync
{
    WXAssert(name && method, @"Fail to find selector with module name and method, please check if the parameters are correct ！");
    
    NSString *selStr = nil; SEL selector = nil;
    WXComponentConfig *config = nil;
    [_configLock lock];
    config = [_componentConfigs objectForKey:name];
    if (config.asyncMethods) {
        selStr = [config.asyncMethods objectForKey:method];
    }
    if (isSync && !selStr && config.syncMethods) {
        selStr = [config.syncMethods objectForKey:method];
        if (selStr.length > 0) {
            *isSync = YES;
        }
    }
    if (selStr) {
        selector = NSSelectorFromString(selStr);
    }
    [_configLock unlock];
    return selector;
}

- (NSDictionary *)getComponentConfigs {
    NSMutableDictionary *componentDic = [[NSMutableDictionary alloc] init];
    void (^componentBlock)(id, id, BOOL *) = ^(id mKey, id mObj, BOOL * mStop) {
        WXComponentConfig *componentConfig = (WXComponentConfig *)mObj;
        if ([componentConfig isValid]) {
            NSMutableDictionary *configDic = [[NSMutableDictionary alloc] init];
            [configDic setObject:componentConfig.name forKey:@"name"];
            [configDic setObject:componentConfig.clazz forKey:@"clazz"];
            if (componentConfig.properties) {
                [configDic setObject:componentConfig.properties forKey:@"pros"];
            }
            [componentDic setObject:configDic forKey:componentConfig.name];
        }
    };
    [_componentConfigs enumerateKeysAndObjectsUsingBlock:componentBlock];
    return componentDic;
}

- (WXComponentConfig *)configWithComponentName:(NSString *)name
{
    WXAssert(name, @"Can not find config for a nil component name");
    
    WXComponentConfig *config = nil;
    
    [_configLock lock];
    config = [_componentConfigs objectForKey:name];
    if (!config) {
        WXLogWarning(@"No component config for name:%@, use default config", name);
        config = [_componentConfigs objectForKey:@"div"];
    }
    [_configLock unlock];
    
    return config;
}

- (void)registerBaseType:(NSString *)typeName withClass:(Class)clazz
{
    if ([typeName length] > 0 && clazz != Nil) {
        WXComponentBaseType* typePair = [[WXComponentBaseType alloc] init];
        typePair.type = typeName;
        typePair.clazz = clazz;
        [_baseTypes addObject:typePair];
    }
}

- (void)registerAffineType:(NSString *)typeName withClass:(Class)clazz
{
    // iterates super classes of clazz and check if any super class matches registerd base types
    if ([_baseTypes count] > 0 && [typeName length] > 0 && clazz != Nil) {
        Class supercls = [clazz superclass];
        while (supercls) {
            if (supercls == [WXComponent class]) {
                break;
            }
            
            for (WXComponentBaseType* typePair in _baseTypes) {
                if (supercls == typePair.clazz) {
                    WXLogInfo(@"Type '%@' is registerd as affine type of '%@' because '%@' is subclass of '%@'.", typeName, typePair.type, clazz, supercls);
                    [WXCoreBridge registerComponentAffineType:typeName asType:typePair.type];
                    return; // done, use the first found affine type
                }
            }
            supercls = [supercls superclass];
        }
    }
}

- (void)registerComponent:(NSString *)name withClass:(Class)clazz withPros:(NSDictionary *)pros
{
    WXAssert(name && clazz, @"name or clazz must not be nil for registering component.");
    
    WXComponentConfig *config = nil;
    [_configLock lock];
    config = [_componentConfigs objectForKey:name];
    
    if(config){
        WXLogInfo(@"Overrider component name:%@ class:%@, to name:%@ class:%@",
                  config.name, config.class, name, clazz);
    }
    
    config = [[WXComponentConfig alloc] initWithName:name class:NSStringFromClass(clazz) pros:pros];
    [_componentConfigs setValue:config forKey:name];
    [config registerMethods];
    [self registerAffineType:name withClass:clazz];
    [_configLock unlock];
}

- (void)registerComponents:(NSArray *)components
{
    WXAssert(components, @"components array must not be nil for registering component.");
    
    [_configLock lock];
    for(NSDictionary *dict in components){
        NSString *name = dict[@"name"];
        NSString *clazz = dict[@"class"];
        WXAssert(name && clazz, @"name or clazz must not be nil for registering components.");
        
        WXComponentConfig *config = [[WXComponentConfig alloc] initWithName:name class:clazz pros:nil];
        if(config){
            [_componentConfigs setValue:config forKey:name];
            [config registerMethods];
            [self registerAffineType:name withClass:NSClassFromString(clazz)];
        }
    }
    [_configLock unlock];
}

- (void)unregisterAllComponents
{
    [_configLock lock];
    [_componentConfigs removeAllObjects];
    [_configLock unlock];
}

@end
