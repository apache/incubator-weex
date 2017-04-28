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

#import "WXSimulatorShortcutManager.h"
#import "WXUtility.h"
#import <objc/message.h>

#if TARGET_OS_SIMULATOR

@interface UIEvent (WXSimulatorShortcutManager)

@property (nonatomic, strong) NSString *_modifiedInput;
@property (nonatomic, strong) NSString *_unmodifiedInput;
@property (nonatomic, assign) UIKeyModifierFlags _modifierFlags;
@property (nonatomic, assign) BOOL _isKeyDown;
@property (nonatomic, assign) long _keyCode;

@end

@interface WXKeyInput : NSObject

@property (nonatomic, strong) NSString *key;
@property (nonatomic, assign) UIKeyModifierFlags flags;

@end

@implementation WXKeyInput

+ (instancetype)keyInputForKey:(NSString *)key flags:(UIKeyModifierFlags)flags
{
    WXKeyInput *keyInput = [[self alloc] init];
    if (keyInput) {
        keyInput.key = key;
        keyInput.flags = flags;
    }
    return keyInput;
}

- (BOOL)isEqual:(id)object
{
    BOOL isEqual = NO;
    if ([object isKindOfClass:[WXKeyInput class]]) {
        WXKeyInput *keyInput = (WXKeyInput *)object;
        isEqual = [self.key isEqualToString:keyInput.key] && self.flags == keyInput.flags;
    }
    return isEqual;
}

@end


@implementation WXSimulatorShortcutManager
{
    NSCache *_actions;
}

+ (instancetype)sharedManager
{
    static WXSimulatorShortcutManager *_sharedInstance = nil;
    static dispatch_once_t oncePredicate;
    dispatch_once(&oncePredicate, ^{
        _sharedInstance = [[self alloc] init];
        _sharedInstance->_actions = [[NSCache alloc] init];
        SEL handleKeyEventSelector = NSSelectorFromString(@"handleKeyUIEvent:");
        SEL replacedSelector = WXSwizzledSelectorForSelector(handleKeyEventSelector);
        WXSwizzleInstanceMethodWithBlock([UIApplication class], handleKeyEventSelector, ^(UIApplication *application, UIEvent *event) {
            [[[self class] sharedManager] handleKeyUIEvent:event];
            ((void(*)(id, SEL, id))objc_msgSend)(application, replacedSelector, event);
        }, replacedSelector);
    });
    return _sharedInstance;
}

+ (void)registerSimulatorShortcutWithKey:(NSString *)key modifierFlags:(UIKeyModifierFlags)flags action:(dispatch_block_t)action
{
    WXKeyInput *keyInput = [WXKeyInput keyInputForKey:key flags:flags];
    [[WXSimulatorShortcutManager sharedManager]->_actions setObject:action forKey:keyInput];
}

- (void)handleKeyUIEvent:(UIEvent *)event
{
    BOOL isKeyDown = NO;
    NSString *modifiedInput = nil;
    NSString *unmodifiedInput = nil;
    UIKeyModifierFlags flags = 0;
    if ([event respondsToSelector:NSSelectorFromString(@"_isKeyDown")]) {
        isKeyDown = [event _isKeyDown];
    }
    
    if ([event respondsToSelector:NSSelectorFromString(@"_modifiedInput")]) {
        modifiedInput = [event _modifiedInput];
    }
    
    if ([event respondsToSelector:NSSelectorFromString(@"_unmodifiedInput")]) {
        unmodifiedInput = [event _unmodifiedInput];
    }
    
    if ([event respondsToSelector:NSSelectorFromString(@"_modifierFlags")]) {
        flags = [event _modifierFlags];
    }
    
    if (isKeyDown && [modifiedInput length] > 0) {
        WXKeyInput *keyInput = [WXKeyInput keyInputForKey:unmodifiedInput flags:flags];
        dispatch_block_t block = [_actions objectForKey:keyInput];
        if (block) {
            block();
        }
    }
}

@end

#endif
