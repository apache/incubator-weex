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

#import "WXRefreshComponent.h"
#import "WXScrollerComponent.h"
#import "WXLoadingIndicator.h"
#import "WXComponent_internal.h"
#import "WXLog.h"
#import "WXComponent+Layout.h"

@interface WXRefreshComponent()

@property (nonatomic) BOOL displayState;
@property (nonatomic) BOOL initFinished;
@property (nonatomic) BOOL refreshEvent;
@property (nonatomic) BOOL pullingdownEvent;

@property (nonatomic, weak) WXLoadingIndicator *indicator;

@end

@implementation WXRefreshComponent

- (instancetype)initWithRef:(NSString *)ref type:(NSString *)type styles:(NSDictionary *)styles attributes:(NSDictionary *)attributes events:(NSArray *)events weexInstance:(WXSDKInstance *)weexInstance
{
    self = [super initWithRef:ref type:type styles:styles attributes:attributes events:events weexInstance:weexInstance];
    if (self) {
        _refreshEvent = NO;
        _pullingdownEvent = NO;
        if (attributes[@"display"]) {
            if ([attributes[@"display"] isEqualToString:@"show"]) {
                _displayState = YES;
            } else if ([attributes[@"display"] isEqualToString:@"hide"]) {
                _displayState = NO;
            } else {
                WXLogError(@"");
            }
        }
        self.flexCssNode->setStylePositionType(WeexCore::kAbsolute);
    }
    return self;
}

- (void)viewDidLoad
{
     _initFinished = YES;
    
    if (!_displayState) {
        [_indicator.view setHidden:YES];
    }
}

- (void)layoutDidFinish
{
    [self.view setFrame: (CGRect) {
        .size = self.calculatedFrame.size,
        .origin.x = self.calculatedFrame.origin.x,
        .origin.y = self.view.frame.origin.y - CGRectGetHeight(self.calculatedFrame)
    }];
}

- (void)viewWillUnload
{
    _displayState = NO;
    _refreshEvent = NO;
    _initFinished = NO;
}

- (void)refresh
{
    if (!_refreshEvent || _displayState) {
        return;
    }
#ifdef DEBUG
    WXLogDebug(@"flexLayout -> refreshComponent : refresh ref:%@",self.ref);
#endif
    [self fireEvent:@"refresh" params:nil];
}

- (void)pullingdown:(NSDictionary*)param
{
    if (!_pullingdownEvent) {
        return ;
    }
#ifdef DEBUG
    WXLogDebug(@"flexLayout -> refreshComponent : pullingdown ,ref:%@",self.ref);
#endif
    
    [self fireEvent:@"pullingdown" params:param];
}

- (void)_insertSubcomponent:(WXComponent *)subcomponent atIndex:(NSInteger)index
{
    if (subcomponent) {
        [super _insertSubcomponent:subcomponent atIndex:index];
        if ([subcomponent isKindOfClass:[WXLoadingIndicator class]]) {
            _indicator = (WXLoadingIndicator*)subcomponent;
        }
    }
}

- (void)updateAttributes:(NSDictionary *)attributes
{
    if (attributes[@"display"]) {
        if ([attributes[@"display"] isEqualToString:@"show"]) {
            _displayState = YES;
        } else if ([attributes[@"display"] isEqualToString:@"hide"]) {
            _displayState = NO;
        } else {
            WXLogError(@"");
        }
        [self setDisplay];
    }
}

- (void)addEvent:(NSString *)eventName
{
    if ([eventName isEqualToString:@"refresh"]) {
        _refreshEvent = YES;
    }
    if ([eventName isEqualToString:@"pullingdown"]) {
        _pullingdownEvent = YES;
    }
}

- (void)removeEvent:(NSString *)evetName
{
    if ([evetName isEqualToString:@"refresh"]) {
        _refreshEvent = NO;
    }
    if ([evetName isEqualToString:@"pullingdown"]) {
        _pullingdownEvent = NO;
    }
}

- (void)setDisplay
{
    id<WXScrollerProtocol> scrollerProtocol = self.ancestorScroller;
    if (scrollerProtocol == nil || !_initFinished)
        return;
    
    if ([scrollerProtocol respondsToSelector:@selector(refreshType)] &&
        [[scrollerProtocol refreshType] isEqualToString:@"refreshForAppear"]) {
        UIEdgeInsets inset = [scrollerProtocol contentInset];
        if (_displayState) {
            inset.top = self.calculatedFrame.size.height;
            if ([_indicator.view isHidden]) {
                [_indicator.view setHidden:NO];
            }
            [_indicator start];
        } else {
            inset.top = 0;
            [_indicator stop];
        }
        [scrollerProtocol setContentInset:inset];
    } else {
        CGPoint offset = [scrollerProtocol contentOffset];
        if (_displayState) {
            offset.y = -self.calculatedFrame.size.height;
            if ([_indicator.view isHidden]) {
                [_indicator.view setHidden:NO];
            }
            [_indicator start];
        } else {
            offset.y = 0;
            [_indicator stop];
        }
        [scrollerProtocol setContentOffset:offset animated:YES];
    }
  
}

- (BOOL)displayState
{
    return _displayState;
}

- (void)setIndicatorHidden:(BOOL)hidden {
    [_indicator.view setHidden:hidden];
    
    id<WXScrollerProtocol> scrollerProtocol = self.ancestorScroller;
    if (scrollerProtocol == nil || !_initFinished)
        return;
    if ([scrollerProtocol respondsToSelector:@selector(refreshType)] &&
        [[scrollerProtocol refreshType] isEqualToString:@"refreshForAppear"]) {
        UIEdgeInsets inset = [scrollerProtocol contentInset];
        if (!hidden) {
            inset.top = self.calculatedFrame.size.height;
            [_indicator start];
        } else {
            inset.top = 0;
            [_indicator stop];
        }
        [scrollerProtocol setContentInset:inset];
    }
}

@end
