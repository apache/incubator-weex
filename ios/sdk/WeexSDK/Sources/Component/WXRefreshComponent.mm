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
#import "WXPullRefreshProtocol.h"
#import "WXHandlerFactory.h"

@interface WXRefreshComponent()

@property (nonatomic) BOOL displayState;
@property (nonatomic) BOOL initFinished;
@property (nonatomic) BOOL refreshEvent;
@property (nonatomic) BOOL pullingdownEvent;

@property (nonatomic) BOOL bPulling;
@property (nonatomic) BOOL bRefreshing;
@property (nonatomic) BOOL bFinalAnimation;
@property (nonatomic) CGFloat finalAnimationDuration;

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
        if (attributes[@"finalAnimationDuration"]) {
            _finalAnimationDuration = [WXConvert CGFloat:attributes[@"finalAnimationDuration"]];
        } else {
            _finalAnimationDuration = 1;
        }
//#ifndef USE_FLEX
        if (![WXComponent isUseFlex]) {
          self.cssNode->style.position_type = CSS_POSITION_ABSOLUTE;
        }
//#else
        else
        {
            self.flexCssNode->setStylePositionType(WeexCore::kAbsolute);
        }
       
//#endif
        _bFinalAnimation = NO;
        _bPulling = NO;
        _bRefreshing = NO;
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

    _bPulling = NO;
    _bRefreshing = NO;
    _bFinalAnimation = NO;
}

- (void)refresh
{
    if (!_refreshEvent || _displayState) {
        return;
    }
#ifdef DEBUG
    WXLogDebug(@"flexLayout -> refreshComponent : refresh ref:%@",self.ref);
#endif
    id <WXPullRefreshProtocol> nativeImp = [WXHandlerFactory handlerForProtocol:@protocol(WXPullRefreshProtocol)];
    if (nativeImp) {
        if ([nativeImp respondsToSelector:@selector(endPulling)]) {
            if (_bPulling) {
                [nativeImp endPulling];
                _bPulling = NO;
            }
        }
        if ([nativeImp respondsToSelector:@selector(startRefreshing:)]) {
            if (!_bRefreshing) {
                [nativeImp startRefreshing:self.view];
                _bRefreshing = YES;
            }
        }
    }
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
    id <WXPullRefreshProtocol> nativeImp = [WXHandlerFactory handlerForProtocol:@protocol(WXPullRefreshProtocol)];
    if (nativeImp && [nativeImp respondsToSelector:@selector(startPulling:)]) {
        if (!_bPulling) {
            [nativeImp startPulling:self.view];
            _bPulling = YES;
        } else if ([nativeImp respondsToSelector:@selector(pulling:)]){
            [nativeImp pulling:param];
        }
    }
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
        } else if ([attributes[@"display"] isEqualToString:@"endRefresh"]) {
            id <WXPullRefreshProtocol> nativeImp = [WXHandlerFactory handlerForProtocol:@protocol(WXPullRefreshProtocol)];
            if (nativeImp && [nativeImp respondsToSelector:@selector(endRefreshing)]) {
                if (_bRefreshing) {
                    [nativeImp endRefreshing];
                    _bRefreshing = NO;
                }
            }
            if (nativeImp && [nativeImp respondsToSelector:@selector(startFinalAnimation:duration:)]) {
                if (!_bFinalAnimation) {
                    [nativeImp startFinalAnimation:self.view duration:_finalAnimationDuration];
                    _bFinalAnimation = YES;
                }
            }
        } else {
            WXLogError(@"");
        }
        [self setDisplay];
    }

    if (attributes[@"finalAnimationDuration"]) {
        _finalAnimationDuration = [WXConvert CGFloat:attributes[@"finalAnimationDuration"]];
    } else {
        _finalAnimationDuration = 1;
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
    if (!_displayState) {
        id <WXPullRefreshProtocol> nativeImp = [WXHandlerFactory handlerForProtocol:@protocol(WXPullRefreshProtocol)];
        if (nativeImp && [nativeImp respondsToSelector:@selector(endFinalAnimation)]) {
            if (_bFinalAnimation) {
                [nativeImp endFinalAnimation];
                _bFinalAnimation = NO;
            }
        }
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
