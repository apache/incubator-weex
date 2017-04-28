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

#import "WXDomModule.h"
#import "WXDefine.h"
#import "WXSDKManager.h"
#import "WXComponentManager.h"
#import "WXSDKInstance_private.h"
#import "WXLog.h"
#import "WXModuleProtocol.h"
#import "WXUtility.h"
#import "WXRuleManager.h"
#import "WXSDKInstance.h"

@interface WXDomModule ()

@end

@implementation WXDomModule

@synthesize weexInstance;

WX_EXPORT_METHOD(@selector(createBody:))
WX_EXPORT_METHOD(@selector(addElement:element:atIndex:))
WX_EXPORT_METHOD(@selector(removeElement:))
WX_EXPORT_METHOD(@selector(moveElement:parentRef:index:))
WX_EXPORT_METHOD(@selector(addEvent:event:))
WX_EXPORT_METHOD(@selector(removeEvent:event:))
WX_EXPORT_METHOD(@selector(createFinish))
WX_EXPORT_METHOD(@selector(updateFinish))
WX_EXPORT_METHOD(@selector(refreshFinish))
WX_EXPORT_METHOD(@selector(scrollToElement:options:))
WX_EXPORT_METHOD(@selector(updateStyle:styles:))
WX_EXPORT_METHOD(@selector(updateAttrs:attrs:))
WX_EXPORT_METHOD(@selector(addRule:rule:))
WX_EXPORT_METHOD(@selector(getComponentRect:callback:))


- (void)performBlockOnComponentManager:(void(^)(WXComponentManager *))block
{
    if (!block) {
        return;
    }
    __weak typeof(self) weakSelf = self;
    
    WXPerformBlockOnComponentThread(^{
        WXComponentManager *manager = weakSelf.weexInstance.componentManager;
        if (!manager.isValid) {
            return;
        }
        [manager startComponentTasks];
        block(manager);
    });
}
- (void)performSelectorOnRuleManager:(void(^)(void))block{
    if (!block) {
        return;
    }
    WXPerformBlockOnComponentThread(^{
        block();
    });
}

- (NSThread *)targetExecuteThread
{
    return [WXComponentManager componentThread];
}

- (void)createBody:(NSDictionary *)body
{
    [self performBlockOnComponentManager:^(WXComponentManager *manager) {
        [manager createRoot:body];
    }];
}

- (void)addElement:(NSString *)parentRef element:(NSDictionary *)element atIndex:(NSInteger)index
{
    [self performBlockOnComponentManager:^(WXComponentManager *manager) {
        [manager addComponent:element toSupercomponent:parentRef atIndex:index appendingInTree:NO];
    }];
}

- (void)removeElement:(NSString *)ref
{
    [self performBlockOnComponentManager:^(WXComponentManager *manager) {
        [manager removeComponent:ref];
    }];
}

- (void)moveElement:(NSString *)elemRef parentRef:(NSString *)parentRef index:(NSInteger)index
{
    [self performBlockOnComponentManager:^(WXComponentManager *manager) {
        [manager moveComponent:elemRef toSuper:parentRef atIndex:index];
    }];
}

- (void)addEvent:(NSString *)elemRef event:(NSString *)event
{
    [self performBlockOnComponentManager:^(WXComponentManager *manager) {
        [manager addEvent:event toComponent:elemRef];
    }];
}

- (void)removeEvent:(NSString *)elemRef event:(NSString *)event
{
    [self performBlockOnComponentManager:^(WXComponentManager *manager) {
        [manager removeEvent:event fromComponent:elemRef];
    }];
}

- (void)createFinish
{
    [self performBlockOnComponentManager:^(WXComponentManager *manager) {
        [manager createFinish];
    }];
}

- (void)updateFinish
{
    [self performBlockOnComponentManager:^(WXComponentManager *manager) {
        [manager updateFinish];
    }];
}

- (void)refreshFinish
{
    [self performBlockOnComponentManager:^(WXComponentManager *manager) {
        [manager refreshFinish];
    }];
}

- (void)scrollToElement:(NSString *)elemRef options:(NSDictionary *)dict
{
    [self performBlockOnComponentManager:^(WXComponentManager *manager) {
        [manager scrollToComponent:elemRef options:dict];
    }];
}

-(void)updateStyle:(NSString *)elemRef styles:(NSDictionary *)styles
{
    [self performBlockOnComponentManager:^(WXComponentManager *manager) {
        [manager updateStyles:styles forComponent:elemRef];
    }];
}

- (void)updateAttrs:(NSString *)elemRef attrs:(NSDictionary *)attrs
{
    [self performBlockOnComponentManager:^(WXComponentManager *manager) {
        [manager updateAttributes:attrs forComponent:elemRef];
    }];
}

- (void)addRule:(NSString*)type rule:(NSDictionary *)rule {
    if ([WXUtility isBlankString:type] || ![rule count]) {
        return;
    }
    
    [self performSelectorOnRuleManager:^{
        [WXRuleManager sharedInstance].instance = weexInstance;
        [[WXRuleManager sharedInstance] addRule:type rule:rule];
        
    }];
}

- (void)getComponentRect:(NSString*)ref callback:(WXModuleKeepAliveCallback)callback {
    [self performBlockOnComponentManager:^(WXComponentManager * manager) {
        UIView *rootView = manager.weexInstance.rootView;
        CGRect rootRect = [rootView.superview convertRect:rootView.frame toView:rootView];
        if ([ref isEqualToString:@"viewport"]) {
            NSMutableDictionary * callbackRsp = nil;
            callbackRsp = [self _componentRectInfoWithViewFrame:rootRect];
            [callbackRsp setObject:@(true) forKey:@"result"];
            if (callback) {
                callback(callbackRsp, false);
            }
        } else {
            WXComponent *component = [manager componentForRef:ref];
            __weak typeof (self) weakSelf = self;
            dispatch_async(dispatch_get_main_queue(), ^{
                __strong typeof (weakSelf) strongSelf = weakSelf;
                NSMutableDictionary * callbackRsp = nil;
                if (!component) {
                    [callbackRsp setObject:@(false) forKey:@"result"];
                    [callbackRsp setObject:[NSString stringWithFormat:@"Illegal parameter, no ref about \"%@\" can be found", ref] forKey:@"errMsg"];
                } else {
                    CGRect componentRect = [component.view.superview convertRect:component.view.frame toView:rootView];
                    callbackRsp = [strongSelf _componentRectInfoWithViewFrame:componentRect];
                    [callbackRsp setObject:@(true)forKey:@"result"];
                }
                if (callback) {
                    callback(callbackRsp, false);
                }
            });

        }
    }];
}

- (void)destroyInstance
{
    [self performBlockOnComponentManager:^(WXComponentManager *manager) {
        [manager unload];
    }];
}

- (NSMutableDictionary*)_componentRectInfoWithViewFrame:(CGRect)componentRect
{
    CGFloat scaleFactor = self.weexInstance.pixelScaleFactor;
    NSMutableDictionary * callbackRsp = [NSMutableDictionary new];
    [callbackRsp setObject:@{
                             @"width":@(componentRect.size.width /scaleFactor),
                             @"height":@(componentRect.size.height / scaleFactor),
                             @"bottom":@(CGRectGetMaxY(componentRect) / scaleFactor),
                             @"left":@(componentRect.origin.x / scaleFactor),
                             @"right":@(CGRectGetMaxX(componentRect) / scaleFactor),
                             @"top":@(componentRect.origin.y / scaleFactor)
                             } forKey:@"size"];
    return callbackRsp;
}

@end
