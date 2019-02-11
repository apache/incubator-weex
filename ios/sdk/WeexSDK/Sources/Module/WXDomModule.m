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
#import "WXComponent+Layout.h"
#import "WXDefine.h"
#import "WXSDKManager.h"
#import "WXComponentManager.h"
#import "WXSDKInstance_private.h"
#import "WXLog.h"
#import "WXModuleProtocol.h"
#import "WXUtility.h"
#import "WXRuleManager.h"
#import "WXSDKInstance.h"
#import "WXRecycleListComponent.h"
#import "WXCoreBridge.h"
#import <objc/message.h>

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
WX_EXPORT_METHOD(@selector(getLayoutDirection:callback:))
WX_EXPORT_METHOD(@selector(updateComponentData:componentData:callback:))
WX_EXPORT_METHOD(@selector(beginBatchMark))
WX_EXPORT_METHOD(@selector(endBatchMark))

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

- (void)beginBatchMark
{
    [self performBlockOnComponentManager:^(WXComponentManager *manager) {
        [manager performBatchBegin];
    }];
}

- (void)endBatchMark
{
    [self performBlockOnComponentManager:^(WXComponentManager * manager) {
        [manager performBatchEnd];
    }];
}

- (NSThread *)targetExecuteThread
{
    return [WXComponentManager componentThread];
}

- (void)createBody:(NSDictionary *)bodyData
{
    NSString* instanceId = self.weexInstance.instanceId;
    WXPerformBlockOnComponentThread(^{
        [WXCoreBridge callCreateBody:instanceId data:bodyData];
    });
}

- (void)addElement:(NSString *)parentRef element:(NSDictionary *)elementData atIndex:(NSInteger)index
{
    NSString* instanceId = self.weexInstance.instanceId;
    WXPerformBlockOnComponentThread(^{
        [WXCoreBridge callAddElement:instanceId parentRef:parentRef data:elementData index:(int)index];
    });
}

- (void)removeElement:(NSString *)ref
{
    NSString* instanceId = self.weexInstance.instanceId;
    WXPerformBlockOnComponentThread(^{
        [WXCoreBridge callRemoveElement:instanceId ref:ref];
    });
}

- (void)moveElement:(NSString *)ref parentRef:(NSString *)parentRef index:(NSInteger)index
{
    NSString* instanceId = self.weexInstance.instanceId;
    WXPerformBlockOnComponentThread(^{
        [WXCoreBridge callMoveElement:instanceId ref:ref parentRef:parentRef index:(int)index];
    });
}

- (void)addEvent:(NSString *)ref event:(NSString *)event
{
    NSString* instanceId = self.weexInstance.instanceId;
    WXPerformBlockOnComponentThread(^{
        [WXCoreBridge callAddEvent:instanceId ref:ref event:event];
    });
}

- (void)removeEvent:(NSString *)ref event:(NSString *)event
{
    NSString* instanceId = self.weexInstance.instanceId;
    WXPerformBlockOnComponentThread(^{
        [WXCoreBridge callRemoveEvent:instanceId ref:ref event:event];
    });
}

- (void)createFinish
{
    NSString* instanceId = self.weexInstance.instanceId;
    WXPerformBlockOnComponentThread(^{
        [WXCoreBridge callCreateFinish:instanceId];
    });
}

- (void)updateFinish
{
    NSString* instanceId = self.weexInstance.instanceId;
    WXPerformBlockOnComponentThread(^{
        [WXCoreBridge callUpdateFinish:instanceId];
    });
}

- (void)refreshFinish
{
    NSString* instanceId = self.weexInstance.instanceId;
    WXPerformBlockOnComponentThread(^{
        [WXCoreBridge callRefreshFinish:instanceId];
    });
}

- (void)scrollToElement:(NSString *)elemRef options:(NSDictionary *)dict
{
    [self performBlockOnComponentManager:^(WXComponentManager *manager) {
        [manager scrollToComponent:elemRef options:dict];
    }];
}

- (void)updateStyle:(NSString *)ref styles:(NSDictionary *)styles
{
    NSString* instanceId = self.weexInstance.instanceId;
    WXPerformBlockOnComponentThread(^{
        [WXCoreBridge callUpdateStyle:instanceId ref:ref data:styles];
    });
}

- (void)updateAttrs:(NSString *)ref attrs:(NSDictionary *)attrs
{
    NSString* instanceId = self.weexInstance.instanceId;
    WXPerformBlockOnComponentThread(^{
        [WXCoreBridge callUpdateAttrs:instanceId ref:ref data:attrs];
    });
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
    if (ref == nil || ![ref isKindOfClass:[NSString class]]) {
        if (callback) {
            callback(@{@"result": @(NO), @"errMsg": @"Illegal parameter, ref must be a string."}, false);
        }
        return;
    }
    
    [self performBlockOnComponentManager:^(WXComponentManager * manager) {
        if ([ref isEqualToString:@"viewport"]) {
            dispatch_async(dispatch_get_main_queue(), ^{
                UIView* rootView = manager.weexInstance.rootView;
                CGRect rootRect = [rootView.superview convertRect:rootView.frame toView:rootView];
                NSMutableDictionary *callbackRsp = [self _componentRectInfoWithViewFrame:rootRect];
                [callbackRsp setObject:@(true) forKey:@"result"];
                if (callback) {
                    callback(callbackRsp, false);
                }
            });
        } else {
            WXComponent *component = [manager componentForRef:ref];
            dispatch_async(dispatch_get_main_queue(), ^{
                UIView *rootView = manager.weexInstance.rootView;
                NSMutableDictionary * callbackRsp = nil;
                if (!component) {
                    callbackRsp = [NSMutableDictionary new];
                    [callbackRsp setObject:@(false) forKey:@"result"];
                    [callbackRsp setObject:[NSString stringWithFormat:@"Illegal parameter, no ref about \"%@\" can be found", ref] forKey:@"errMsg"];
                } else {
                    CGRect componentRect = CGRectZero;
                    // if current component view is not loaded or it hasn't been inserted to its superview, so the position cannot be obtained correct except width and height
                    if ([component isViewLoaded] && component.view.superview) {
                        componentRect = [component.view.superview convertRect:component.view.frame toView:rootView];
                    } else {
                        componentRect = component.calculatedFrame;
                    }
                    callbackRsp = [self _componentRectInfoWithViewFrame:componentRect];
                    [callbackRsp setObject:@(true)forKey:@"result"];
                }
                if (callback) {
                    callback(callbackRsp, false);
                }
            });

        }
    }];
}

- (void)getLayoutDirection:(NSString*)ref callback:(WXModuleKeepAliveCallback)callback {
    [self performBlockOnComponentManager:^(WXComponentManager * manager) {
        if ([ref isEqualToString:@"viewport"]) {
            dispatch_async(dispatch_get_main_queue(), ^{
                NSString *direction = [WXUtility getEnvLayoutDirection] == WXLayoutDirectionRTL ? @"rtl" : @"ltr";
                if (callback) {
                    callback(direction, false);
                }
            });
        } else {
            WXComponent *component = [manager componentForRef:ref];
            dispatch_async(dispatch_get_main_queue(), ^{
                NSString *direction = @"unknow";
                if (component) {
                    direction = [component isDirectionRTL] ? @"rtl" : @"ltr";
                }
                if (callback) {
                    callback(direction, false);
                }
            });
        }
    }];
}

- (void)updateComponentData:(NSString*)componentDataId componentData:(NSDictionary*)componentData callback:(NSString*)callbackId
{
    NSString *recycleListComponentRef = [[componentDataId componentsSeparatedByString:@"@"] objectAtIndex:0];
    if (!recycleListComponentRef) {
        return;
    }
    SEL selector = _cmd;
    [self performBlockOnComponentManager:^(WXComponentManager *manager) {
        WXRecycleListComponent *recycleListComponent = (WXRecycleListComponent*)[manager componentForRef:recycleListComponentRef];
        ((void*(*)(id,SEL,NSString*,NSDictionary*,NSString*))objc_msgSend)(recycleListComponent, selector, componentDataId, componentData,callbackId);
    }];
}

- (void)destroyInstance
{
    NSString* instanceId = self.weexInstance.instanceId;
    [self performBlockOnComponentManager:^(WXComponentManager *manager) {
        [manager invalidate];
        [manager unload];
        
        // Destroy weexcore c++ page and objects.
        [WXCoreBridge closePage:instanceId];
    }];
}

- (NSMutableDictionary*)_componentRectInfoWithViewFrame:(CGRect)componentRect
{
    CGFloat scaleFactor = self.weexInstance.pixelScaleFactor;
    NSMutableDictionary *callbackRsp = [NSMutableDictionary new];
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
