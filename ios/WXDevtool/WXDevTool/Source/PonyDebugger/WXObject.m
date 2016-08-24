//
//  WXObject.m
//  PonyDebugger
//
//  Created by Mike Lewis on 11/6/11.
//
//  Licensed to Square, Inc. under one or more contributor license agreements.
//  See the LICENSE file distributed with this work for the terms under
//  which Square, Inc. licenses this file to you.
//

#import <objc/runtime.h>

#import "WXObject.h"
#import "WXDefinitions.h"


@implementation NSObject (WX_JSONObject)

- (id)WX_JSONObject;
{
    return self;
}

- (id)WX_JSONObjectCopy;
{
    return [[self WX_JSONObject] copy];
}

@end


@implementation WXObject

@synthesize store = _store;

+ (NSDictionary *)keysToEncode;
{
    return [[NSDictionary alloc] init];
}

- (id)init;
{
    self = [super init];
    if (!self) {
        return nil;
    }
    
    _store = [[NSMutableDictionary alloc] init];

    return self;
}

- (id)initWithStore:(NSDictionary *)store;
{
    self = [super init];
    if (!self) {
        return nil;
    }

    assert(store);
    _store = [store mutableCopy];

    return self;
}

- (id)copyWithZone:(NSZone *)zone;
{
    return [[[self class] alloc] initWithStore:_store];
}

#pragma mark - NSObject

- (BOOL)validate:(NSError **)error;
{
    return YES;
}

- (id)valueForKey:(NSString *)key;
{
	return [_store valueForKey:[[[self class] keysToEncode] objectForKey:key]];
}

- (void)setValue:(id)value forKey:(NSString *)key;
{
    [_store setValue:value forKey:[[[self class] keysToEncode] objectForKey:key]];
}

+ (BOOL)resolveInstanceMethod:(SEL)sel;
{
    NSString *selName = NSStringFromSelector(sel);
    if ([selName hasPrefix:@"set"] && selName.length > 4) {
        NSString *propertyName = [NSString stringWithFormat:@"%@%@", [[selName substringWithRange:NSMakeRange(3, 1)] lowercaseString],
                                  [selName substringWithRange:NSMakeRange(4, selName.length - 5)]];
                                 
#ifdef __IPHONE_6_0 
        class_addMethod([self class], sel, imp_implementationWithBlock((id)(^(id slf, id value) {
#else
        class_addMethod([self class], sel, imp_implementationWithBlock((__bridge void *)(^(id slf, id value) {
#endif

            [slf setValue:value forKey:propertyName];
            
        })), "v@:@");
        
        return YES;
    } else if (![selName hasPrefix:@"_"] && [selName rangeOfString:@":"].length == 0) {
    
#ifdef __IPHONE_6_0
        class_addMethod([self class], sel, imp_implementationWithBlock((id)^id(id slf) {
#else
        class_addMethod([self class], sel, imp_implementationWithBlock((__bridge void *)^id(id slf) {
#endif

            id val = [slf valueForKey:selName];
            return val;
        }), "@@:");
        
        return YES;
    }
    
    return [super resolveInstanceMethod:sel];
}

- (id)WX_JSONObject;
{
    NSMutableDictionary *ret = [[NSMutableDictionary alloc] initWithCapacity:_store.count];
    [_store enumerateKeysAndObjectsWithOptions:0 usingBlock:^(id key, id obj, BOOL *stop) {
        [ret setObject:[obj WX_JSONObject] forKey:key];
    }];
    
    return ret;
}

@end
