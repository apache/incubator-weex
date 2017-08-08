//
//  WXExtendCallNativeManager.m
//  WeexSDK
//
//  Created by 齐山 on 2017/7/27.
//  Copyright © 2017年 taobao. All rights reserved.
//

#import "WXExtendCallNativeManager.h"
#import "WXExtendCallNativeProtocol.h"
#import <objc/runtime.h>

@interface WXExtendCallNativeManager ()

@property (nonatomic, strong) NSMutableDictionary *names;

@end

@implementation WXExtendCallNativeManager
{
    NSLock *_configLock;
}
+ (instancetype) sharedInstance{
    
    static WXExtendCallNativeManager *instance = nil;
    static dispatch_once_t once;
    
    dispatch_once(&once, ^{
        instance = [[WXExtendCallNativeManager alloc] initPrivate];
    });
    
    return instance;
}

- (instancetype) initPrivate{
    self = [super init];
    if(self){
        _configLock = [[NSLock alloc] init];
        self.names = [NSMutableDictionary new];
    }
    
    return self;
}

+ (void)registerExtendCallNative:(NSString *)name withClass:(Class)clazz
{
    [[WXExtendCallNativeManager sharedInstance].names setObject:NSStringFromClass(clazz) forKey:name];
}

+ (Class)classWithExtendCallNativeName:(NSString *)name
{
    NSString *clazz = [[WXExtendCallNativeManager sharedInstance].names objectForKey:name];
    return NSClassFromString(clazz);
}

+(id)sendExtendCallNativeEvent:(NSDictionary *)parameters
{
    if(parameters[@"className"]){
        Class<WXExtendCallNativeProtocol> receiveClass = [self classWithExtendCallNativeName:parameters[@"className"]];
        if(receiveClass && class_conformsToProtocol(receiveClass,@protocol(WXExtendCallNativeProtocol))){
            BOOL receivedItem = [receiveClass checkParameters:parameters];
            if(receivedItem){
                return [receiveClass excuteCallNative:parameters];
            }
        }
    }
    return @(-1);
}
@end
