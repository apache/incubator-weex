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

#import "WXScrollerComponent.h"
#import "WXComponent_internal.h"
#import "WXSDKInstance_private.h"
#import "WXComponent.h"
#import "WXDefine.h"
#import "WXConvert.h"
#import "WXSDKInstance.h"
#import "WXUtility.h"
#import "WXLoadingComponent.h"
#import "WXRefreshComponent.h"
#import "WXConfigCenterProtocol.h"
#import "WXSDKEngine.h"
#import "WXComponent+Events.h"
#import "WXPageEventNotifyEvent.h"
#import "WXComponent+Layout.h"
#import "WXUtility.h"

@interface WXScrollerComponentView : UIScrollView
@end

@implementation WXScrollerComponentView
- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldReceiveTouch:(UITouch *)touch
{
    if ([(id <WXScrollerProtocol>) self.wx_component respondsToSelector:@selector(requestGestureShouldStopPropagation:shouldReceiveTouch:)]) {
        return [(id <WXScrollerProtocol>) self.wx_component requestGestureShouldStopPropagation:gestureRecognizer shouldReceiveTouch:touch];
    }
    else{
        return YES;
    }
}

- (void)setContentOffset:(CGPoint)contentOffset animated:(BOOL)animated
{
    [super setContentOffset:contentOffset animated:animated];
    BOOL scrollStartEvent = [[self.wx_component valueForKey:@"_scrollStartEvent"] boolValue];
    id scrollEventListener = [self.wx_component valueForKey:@"_scrollEventListener"];
    
    if (animated && (scrollStartEvent ||scrollEventListener)  && !WXPointEqualToPoint(contentOffset, self.contentOffset)) {
        CGFloat scaleFactor = self.wx_component.weexInstance.pixelScaleFactor;
        NSDictionary *contentSizeData = @{@"width":@(self.contentSize.width / scaleFactor),
                                          @"height":@(self.contentSize.height / scaleFactor)};
        NSDictionary *contentOffsetData = @{@"x":@(-self.contentOffset.x / scaleFactor),
                                            @"y":@(-self.contentOffset.y / scaleFactor)};
        if (scrollStartEvent) {
            [self.wx_component fireEvent:@"scrollstart" params:@{@"contentSize":contentSizeData, @"contentOffset":contentOffsetData} domChanges:nil];
        }
        if (scrollEventListener) {
            WXScrollerComponent *component = (WXScrollerComponent *)self.wx_component;
            component.scrollEventListener(component, @"scrollstart", @{@"contentSize":contentSizeData, @"contentOffset":contentOffsetData});
        }
    }
}

@end

@interface WXScrollToTarget : NSObject

@property (nonatomic, weak)   WXComponent *target;
@property (nonatomic, assign) BOOL hasAppear;

@end

@implementation WXScrollToTarget

@end


@interface WXScrollerComponent()

@property (nonatomic, strong) NSMutableArray *  stickyArray;
@property (nonatomic, strong) NSMutableArray * listenerArray;
@property (nonatomic, weak) WXRefreshComponent *refreshComponent;
@property (nonatomic, weak) WXLoadingComponent *loadingComponent;

@end

@implementation WXScrollerComponent
{
    CGSize _contentSize;
    BOOL _needsPlatformLayout;
    BOOL _listenLoadMore;
    BOOL _scrollEvent;
    BOOL _scrollStartEvent;
    BOOL _scrollEndEvent;
    BOOL _isScrolling;
    BOOL _isDragging;
    CGFloat _pageSize;
    CGFloat _loadMoreOffset;
    CGFloat _previousLoadMoreContentHeight;
    CGFloat _offsetAccuracy;
    CGPoint _lastContentOffset;
    CGPoint _lastScrollEventFiredOffset;
    BOOL _scrollable;
    NSString * _alwaysScrollableVertical;
    NSString * _alwaysScrollableHorizontal;
    BOOL _bounces;
    
    // refreshForAppear: load more when refresh component begin appear(if scroll is dragging or decelerating, should delay)
    // refreshForWholeVisible: load more until the whole refresh component visible
    NSString *_refreshType;

    // vertical & horizontal
    WXScrollDirection _scrollDirection;
    // left & right & up & down
    NSString *_direction;
    BOOL _showScrollBar;
    BOOL _pagingEnabled;
    
    BOOL _shouldNotifiAppearDescendantView;
    BOOL _shouldRemoveScrollerListener;
    CGPoint _scrollStartPoint;
    CGPoint _scrollEndPoint;

    //css_node_t *_scrollerCSSNode;
    
    NSHashTable* _delegates;
}

WX_EXPORT_METHOD(@selector(resetLoadmore))

- (void)resetLoadmore
{
    _previousLoadMoreContentHeight=0;
}

- (BOOL)_insertSubcomponent:(WXComponent *)subcomponent atIndex:(NSInteger)index
{
    BOOL inserted = [super _insertSubcomponent:subcomponent atIndex:index];
    
    if ([subcomponent isKindOfClass:[WXRefreshComponent class]]) {
        _refreshComponent = (WXRefreshComponent*)subcomponent;
    }
    else if ([subcomponent isKindOfClass:[WXLoadingComponent class]]) {
        _loadingComponent = (WXLoadingComponent*)subcomponent;
    }
    
    // If a vertical list is added to a horizontal scroller, we need platform dependent layout
    if (_flexCssNode && [self isKindOfClass:[WXScrollerComponent class]] &&
        [subcomponent isKindOfClass:[WXScrollerComponent class]] &&
        subcomponent->_positionType != WXPositionTypeFixed &&
        (((WXScrollerComponent*)subcomponent).scrollDirection == WXScrollDirectionVertical)) {
        if (subcomponent->_flexCssNode) {
            if (subcomponent->_flexCssNode->getFlex() > 0 && !isnan(subcomponent->_flexCssNode->getStyleWidth())) {
                _needsPlatformLayout = YES;
                _flexCssNode->setNeedsPlatformDependentLayout(true);
            }
        }
    }

    return inserted;
}

-(instancetype)initWithRef:(NSString *)ref type:(NSString *)type styles:(NSDictionary *)styles attributes:(NSDictionary *)attributes events:(NSArray *)events weexInstance:(WXSDKInstance *)weexInstance
{
    self = [super initWithRef:ref type:type styles:styles attributes:attributes events:events weexInstance:weexInstance];
    if (self) {
        
        _stickyArray = [NSMutableArray array];
        _listenerArray = [NSMutableArray array];
        _scrollEvent = NO;
        _scrollStartEvent = NO;
        _scrollEndEvent = NO;
        _lastScrollEventFiredOffset = CGPointMake(0, 0);
        _scrollDirection = attributes[@"scrollDirection"] ? [WXConvert WXScrollDirection:attributes[@"scrollDirection"]] : WXScrollDirectionVertical;
        _showScrollBar = attributes[@"showScrollbar"] ? [WXConvert BOOL:attributes[@"showScrollbar"]] : YES;
        
        if (attributes[@"alwaysScrollableVertical"]) {
            _alwaysScrollableVertical = [WXConvert NSString:attributes[@"alwaysScrollableVertical"]];
        }
        if (attributes[@"alwaysScrollableHorizontal"]) {
            _alwaysScrollableHorizontal = [WXConvert NSString:attributes[@"alwaysScrollableHorizontal"]];
        }
        _bounces = attributes[@"bounce"]?[WXConvert BOOL:attributes[@"bounce"]]:YES;
        _refreshType = [WXConvert NSString:attributes[@"refreshType"]]?:@"refreshForWholeVisible";
        _pagingEnabled = attributes[@"pagingEnabled"] ? [WXConvert BOOL:attributes[@"pagingEnabled"]] : NO;
        _pageSize = attributes[@"pageSize"] ? [WXConvert WXPixelType:attributes[@"pageSize"] scaleFactor:self.weexInstance.pixelScaleFactor] : 0;
        if (_pageSize < 0) {
            _pageSize = 0;
        }
        _loadMoreOffset = attributes[@"loadmoreoffset"] ? [WXConvert WXPixelType:attributes[@"loadmoreoffset"] scaleFactor:self.weexInstance.pixelScaleFactor] : 0;
        _loadmoreretry = attributes[@"loadmoreretry"] ? [WXConvert NSUInteger:attributes[@"loadmoreretry"]] : 0;
        _listenLoadMore = [events containsObject:@"loadmore"];
        _scrollable = attributes[@"scrollable"] ? [WXConvert BOOL:attributes[@"scrollable"]] : YES;
        _offsetAccuracy = attributes[@"offsetAccuracy"] ? [WXConvert WXPixelType:attributes[@"offsetAccuracy"] scaleFactor:self.weexInstance.pixelScaleFactor] : 0;

        /* let scroller fill the rest space if it is a child component and has no fixed height & width.
         WeexCore also does this in C++, but only for "scroller" and "list" not including for
         subclasses of WXScrollerComponent. */
        if (_flexCssNode != nullptr) {
            if (((_scrollDirection == WXScrollDirectionVertical &&
                  flexIsUndefined(_flexCssNode->getStyleHeight())) ||
                 (_scrollDirection == WXScrollDirectionHorizontal &&
                  flexIsUndefined(_flexCssNode->getStyleWidth()))) &&
                _flexCssNode->getFlex() <= 0.0) {
                _flexCssNode->set_flex(1.0);
            }
        }
        
        id configCenter = [WXSDKEngine handlerForProtocol:@protocol(WXConfigCenterProtocol)];
        if ([configCenter respondsToSelector:@selector(configForKey:defaultValue:isDefault:)]) {
            BOOL shouldNotifiAppearDescendantView = [[configCenter configForKey:@"iOS_weex_ext_config.shouldNotifiAppearDescendantView" defaultValue:@(YES) isDefault:NULL] boolValue];
            _shouldNotifiAppearDescendantView = shouldNotifiAppearDescendantView;
            BOOL shouldRemoveScrollerListener = [[configCenter configForKey:@"iOS_weex_ext_config.shouldRemoveScrollerListener" defaultValue:@(YES) isDefault:NULL] boolValue];
            _shouldRemoveScrollerListener = shouldRemoveScrollerListener;
            
        }
        //may be list
        if ([@"scroller" isEqualToString:type]) {
            [weexInstance.apmInstance updateDiffStats:KEY_PAGE_STATS_SCROLLER_NUM withDiffValue:1];
        }        
    }
    
    return self;
}

- (UIView *)loadView
{
    return [[WXScrollerComponentView alloc] init];
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    [self setContentSize:_contentSize];
    WXScrollerComponentView *scrollView = (WXScrollerComponentView *)self.view;
    scrollView.delegate = self;
    scrollView.exclusiveTouch = YES;
    scrollView.autoresizesSubviews = NO;
    scrollView.clipsToBounds = YES;
    scrollView.showsVerticalScrollIndicator = _showScrollBar;
    scrollView.showsHorizontalScrollIndicator = _showScrollBar;
    scrollView.scrollEnabled = _scrollable;
    scrollView.pagingEnabled = _pagingEnabled;
    
    if (scrollView.bounces != _bounces) {
        scrollView.bounces = _bounces;
    }
    
    if (_alwaysScrollableHorizontal) {
        scrollView.alwaysBounceHorizontal = [WXConvert BOOL:_alwaysScrollableHorizontal];
    }
    if (_alwaysScrollableVertical) {
        scrollView.alwaysBounceVertical = [WXConvert BOOL:_alwaysScrollableVertical];
    }
    
    if (@available(iOS 11.0, *)) {
        if ([scrollView respondsToSelector:@selector(setContentInsetAdjustmentBehavior:)]) {
            scrollView.contentInsetAdjustmentBehavior = UIScrollViewContentInsetAdjustmentNever;
        }
    } else {
        // Fallback on earlier versions
    }
    
    if (self.ancestorScroller) {
        scrollView.scrollsToTop = NO;
    } else {
        scrollView.scrollsToTop = YES;
    }
    
    if (_pagingEnabled && _pageSize > 0) {
        scrollView.pagingEnabled = NO; // turn off system default paging
        scrollView.decelerationRate = UIScrollViewDecelerationRateFast;
    }
    else {
        scrollView.decelerationRate = UIScrollViewDecelerationRateNormal;
    }
}

- (void)layoutDidFinish
{
    if ([self isViewLoaded]) {
        [self setContentSize:_contentSize];
        [self adjustSticky];
        [self handleAppear];
    }
    
    [_loadingComponent resizeFrame];
}

- (void)_buildViewHierarchyLazily
{
    [super _buildViewHierarchyLazily];
    [self handleAppear];
}

- (void)viewWillUnload
{
    ((UIScrollView *)_view).delegate = nil;
}

- (void)dealloc
{
    ((UIScrollView *)_view).delegate = nil;
    [self.stickyArray removeAllObjects];
    [self.listenerArray removeAllObjects];
}

- (void)updateAttributes:(NSDictionary *)attributes
{
    if (attributes[@"showScrollbar"]) {
        _showScrollBar = [WXConvert BOOL:attributes[@"showScrollbar"]];
        ((UIScrollView *)self.view).showsHorizontalScrollIndicator = _showScrollBar;
        ((UIScrollView *)self.view).showsVerticalScrollIndicator = _showScrollBar;
    }
    
    if (attributes[@"pagingEnabled"]) {
        _pagingEnabled = [WXConvert BOOL:attributes[@"pagingEnabled"]];
        ((UIScrollView *)self.view).pagingEnabled = _pagingEnabled;
    }
    
    if (attributes[@"pageSize"]) {
        _pageSize = [WXConvert WXPixelType:attributes[@"pageSize"]
                               scaleFactor:self.weexInstance.pixelScaleFactor];
        if (_pageSize < 0) {
            _pageSize = 0;
        }
    }
    
    if ([self isViewLoaded]) {
        if (_pagingEnabled && _pageSize > 0) {
            ((UIScrollView *)self.view).pagingEnabled = NO; // turn off system default paging
            ((UIScrollView *)self.view).decelerationRate = UIScrollViewDecelerationRateFast;
        }
        else {
            ((UIScrollView *)self.view).decelerationRate = UIScrollViewDecelerationRateNormal;
        }
    }
    
    if (attributes[@"loadmoreoffset"]) {
        _loadMoreOffset = [WXConvert WXPixelType:attributes[@"loadmoreoffset"] scaleFactor:self.weexInstance.pixelScaleFactor];
    }
    
    if (attributes[@"bounce"]) {
        _bounces = [WXConvert BOOL:attributes[@"bounce"]];
        ((UIScrollView *)self.view).bounces = _bounces;
    }
    
    if (attributes[@"loadmoreretry"]) {
        NSUInteger loadmoreretry = [WXConvert NSUInteger:attributes[@"loadmoreretry"]];
        if (loadmoreretry != _loadmoreretry) {
            _previousLoadMoreContentHeight = 0;
        }
        self.loadmoreretry = loadmoreretry;
    }
    
    if (attributes[@"scrollable"]) {
        _scrollable = attributes[@"scrollable"] ? [WXConvert BOOL:attributes[@"scrollable"]] : YES;
        ((UIScrollView *)self.view).scrollEnabled = _scrollable;
    }
    if (attributes[@"alwaysScrollableHorizontal"]) {
        _alwaysScrollableHorizontal = [WXConvert NSString:attributes[@"alwaysScrollableHorizontal"]];
        ((UIScrollView*)self.view).alwaysBounceHorizontal = [WXConvert BOOL:_alwaysScrollableHorizontal];
    }
    
    if (attributes[@"alwaysScrollableVertical"]) {
        _alwaysScrollableVertical = [WXConvert NSString:attributes[@"alwaysScrollableVertical"]];
        ((UIScrollView*)self.view).alwaysBounceVertical = [WXConvert BOOL:_alwaysScrollableVertical];
    }
    
    if (attributes[@"refreshType"]) {
        _refreshType = [WXConvert NSString:attributes[@"refreshType"]];
    }
    
    if (attributes[@"offsetAccuracy"]) {
        _offsetAccuracy = [WXConvert WXPixelType:attributes[@"offsetAccuracy"] scaleFactor:self.weexInstance.pixelScaleFactor];
    }
    
    if (attributes[@"scrollDirection"]) {
        _scrollDirection = attributes[@"scrollDirection"] ? [WXConvert WXScrollDirection:attributes[@"scrollDirection"]] : WXScrollDirectionVertical;
    }
}

- (void)addEvent:(NSString *)eventName
{
    if ([eventName isEqualToString:@"loadmore"]) {
        _listenLoadMore = YES;
    }
    if ([eventName isEqualToString:@"scroll"]) {
        _scrollEvent = YES;
    }
    if ([eventName isEqualToString:@"scrollstart"]) {
        _scrollStartEvent = YES;
    }
    if ([eventName isEqualToString:@"scrollend"]) {
        _scrollEndEvent = YES;
    }
}

- (void)removeEvent:(NSString *)eventName
{
    if ([eventName isEqualToString:@"loadmore"]) {
        _listenLoadMore = NO;
    }
    if ([eventName isEqualToString:@"scroll"]) {
        _scrollEvent = NO;
    }
    if ([eventName isEqualToString:@"scrollstart"]) {
        _scrollStartEvent = NO;
    }
    if ([eventName isEqualToString:@"scrollend"]) {
        _scrollEndEvent = NO;
    }
}

#pragma mark WXScrollerProtocol

- (void)addStickyComponent:(WXComponent *)sticky
{
    if(![self.stickyArray containsObject:sticky]) {
        [self.stickyArray addObject:sticky];
        [self adjustSticky];
    }
}

- (void)removeStickyComponent:(WXComponent *)sticky
{
    if([self.stickyArray containsObject:sticky]) {
        [self.stickyArray removeObject:sticky];
		WXPerformBlockOnMainThread(^{
			[self adjustSticky];
		});
    }
}

- (void)adjustForRTL
{
    if (![WXUtility enableRTLLayoutDirection]) return;
    
    // this is scroll rtl solution.
    // scroll layout not use direction, use self tranform
    if (self.view && [self isDirectionRTL]) {
        if (_transform) {
            self.view.layer.transform = CATransform3DConcat(self.view.layer.transform, CATransform3DScale(CATransform3DIdentity, -1, 1, 1));
        } else {
            self.view.layer.transform = CATransform3DScale(CATransform3DIdentity, -1, 1, 1);
        }
    } else {
        if (!_transform) {
            self.view.layer.transform = CATransform3DIdentity;
        }
    }
}

- (void)_adjustForRTL {
    if (![WXUtility enableRTLLayoutDirection]) return;
    
    [super _adjustForRTL];
    [self adjustForRTL];
}

- (void)adjustSticky
{
    if (![self isViewLoaded]) {
        return;
    }
    CGFloat scrollOffsetY = ((UIScrollView *)self.view).contentOffset.y;
    for(WXComponent *component in self.stickyArray) {
        if (isnan(component->_absolutePosition.x) && isnan(component->_absolutePosition.y)) {
            component->_absolutePosition = [component.supercomponent.view convertPoint:component.view.frame.origin toView:self.view];
        }
        CGPoint relativePosition = component->_absolutePosition;
        if (isnan(relativePosition.y)) {
            continue;
        }
        
        WXComponent *supercomponent = component.supercomponent;
        if(supercomponent != self && component.view.superview != self.view) {
            [component.view removeFromSuperview];
            [self.view addSubview:component.view];
        } else {
            [self.view bringSubviewToFront:component.view];
        }
        
        CGFloat relativeY = relativePosition.y;
        BOOL needSticky = NO;
        
        if (scrollOffsetY >= relativeY) {
            needSticky = YES;
        } else {
            // important: reset views' frame
            component.view.frame = CGRectMake(relativePosition.x, relativePosition.y, component.calculatedFrame.size.width, component.calculatedFrame.size.height);
        }
        
        if (!needSticky) {
            continue;
        }
        
        // The minimum Y sticky view can reach is its original position
        CGFloat minY = relativeY;
        CGPoint superRelativePosition = supercomponent == self ? CGPointZero : [supercomponent.supercomponent.view convertPoint:supercomponent.view.frame.origin toView:self.view];
        CGFloat maxY = superRelativePosition.y + supercomponent.calculatedFrame.size.height - component.calculatedFrame.size.height;
        
        CGFloat stickyY = scrollOffsetY;
        if (stickyY < minY) {
            stickyY = minY;
        } else if (stickyY > maxY && ![supercomponent conformsToProtocol:@protocol(WXScrollerProtocol)]) {
            // Sticky component can not go beyond its parent's bounds when its parent is not scroller;
            stickyY = maxY;
        }
        
        UIView *stickyView = component.view;
        CGPoint origin = stickyView.frame.origin;
        origin.y = stickyY;
        stickyView.frame = (CGRect){origin,stickyView.frame.size};
    }
}

- (void)addScrollToListener:(WXComponent *)target
{
    BOOL has = NO;
    NSMutableArray *listenerArray = [self.listenerArray copy];
    for (WXScrollToTarget *targetData in listenerArray) {
        if (targetData.target == target) {
            has = YES;
            break;
        }
    }
    if (!has) {
        WXScrollToTarget *scrollTarget = [[WXScrollToTarget alloc] init];
        scrollTarget.target = target;
        scrollTarget.hasAppear = NO;
        [self.listenerArray addObject:scrollTarget];
    }
}

- (void)removeScrollToListener:(WXComponent *)target
{
    if (_shouldRemoveScrollerListener) {
        WXScrollToTarget *targetData = nil;
        NSMutableArray *listenerArray = [self.listenerArray copy];
        for (WXScrollToTarget *targetDataTemp in listenerArray) {
            if (targetDataTemp.target == target) {
                targetData = targetDataTemp;
                break;
            }
        }
        if(targetData) {
            [self.listenerArray removeObject:targetData];
        }
    }
}

- (void)scrollToComponent:(WXComponent *)component withOffset:(CGFloat)offset animated:(BOOL)animated
{
    UIScrollView *scrollView = (UIScrollView *)self.view;
    // http://dotwe.org/vue/aa1af34e5fc745c0f1520e346904682a
    // ignore scroll action if contentSize smaller than scroller frame
    if (_scrollDirection == WXScrollDirectionVertical && scrollView.contentSize.height < scrollView.frame.size.height) {
        return;
    }
    if (_scrollDirection == WXScrollDirectionHorizontal && scrollView.contentSize.width < scrollView.frame.size.width) {
        return;
    }


    CGPoint contentOffset = scrollView.contentOffset;
    CGFloat scaleFactor = self.weexInstance.pixelScaleFactor;
    
    if (_scrollDirection == WXScrollDirectionHorizontal) {
        CGFloat contentOffetX = 0;
        if (self.isDirectionRTL && component.supercomponent != self) {
            contentOffetX = [component.supercomponent.view convertPoint:CGPointMake(CGRectGetMaxX(component.view.frame), component.view.frame.origin.y) toView:self.view].x;
        } else {
            contentOffetX = [component.supercomponent.view convertPoint:component.view.frame.origin toView:self.view].x;
        }
        contentOffetX += offset * scaleFactor;
        
        if (scrollView.contentSize.width >= scrollView.frame.size.width && contentOffetX > scrollView.contentSize.width - scrollView.frame.size.width) {
            contentOffset.x = scrollView.contentSize.width - scrollView.frame.size.width;
        } else {
            contentOffset.x = contentOffetX;
        }
    } else {
        CGFloat contentOffetY = [component.supercomponent.view convertPoint:component.view.frame.origin toView:self.view].y;
        contentOffetY += offset * scaleFactor;
        
        if (scrollView.contentSize.height >= scrollView.frame.size.height && contentOffetY > scrollView.contentSize.height - scrollView.frame.size.height) {
            contentOffset.y = scrollView.contentSize.height - scrollView.frame.size.height;
        } else {
            contentOffset.y = contentOffetY;
        }
    }
    
    [scrollView setContentOffset:contentOffset animated:animated];
}

- (BOOL)isNeedLoadMore
{
    if (WXScrollDirectionVertical == _scrollDirection) {
        if (_loadMoreOffset >= 0.0 && ((UIScrollView *)self.view).contentOffset.y >= 0) {
            return _previousLoadMoreContentHeight != ((UIScrollView *)self.view).contentSize.height && ((UIScrollView *)self.view).contentSize.height - ((UIScrollView *)self.view).contentOffset.y -  self.view.frame.size.height <= _loadMoreOffset;
        }
    } else if (WXScrollDirectionHorizontal == _scrollDirection) {
        if (_loadMoreOffset >= 0.0 && ((UIScrollView *)self.view).contentOffset.x >= 0) {
            return _previousLoadMoreContentHeight != ((UIScrollView *)self.view).contentSize.width && ((UIScrollView *)self.view).contentSize.width - ((UIScrollView *)self.view).contentOffset.x -  self.view.frame.size.width <= _loadMoreOffset;
        }
    }
    
    return NO;
}

- (void)loadMore
{
    [self fireEvent:@"loadmore" params:nil];
    
    if (WXScrollDirectionVertical == _scrollDirection) {
        _previousLoadMoreContentHeight = ((UIScrollView *)self.view).contentSize.height;
    } else if (WXScrollDirectionHorizontal == _scrollDirection) {
        _previousLoadMoreContentHeight = ((UIScrollView *)self.view).contentSize.width;
    }
}

- (CGPoint)contentOffset
{
    CGPoint rtv = CGPointZero;
    UIScrollView *scrollView = (UIScrollView *)self.view;
    if (scrollView) {
        rtv = scrollView.contentOffset;
    }
    return rtv;
}

- (void)setContentOffset:(CGPoint)contentOffset
{
    UIScrollView *scrollView = (UIScrollView *)self.view;
    [scrollView setContentOffset:contentOffset];
}

- (void)setContentOffset:(CGPoint)contentOffset animated:(BOOL)animated
{
    UIScrollView *scrollView = (UIScrollView *)self.view;
    [scrollView setContentOffset:contentOffset animated:animated];
}

- (CGSize)contentSize
{
    return ((UIScrollView *)self.view).contentSize;
}

- (void)setContentSize:(CGSize)size
{
    UIScrollView *scrollView = (UIScrollView *)self.view;
    scrollView.contentSize = size;
}

- (UIEdgeInsets)contentInset
{
    UIEdgeInsets rtv = UIEdgeInsetsZero;
    UIScrollView *scrollView = (UIScrollView *)self.view;
    if (scrollView) {
        rtv = scrollView.contentInset;
    }
    return rtv;
}

- (void)setContentInset:(UIEdgeInsets)contentInset
{
    UIScrollView *scrollView = (UIScrollView *)self.view;
    [scrollView setContentInset:contentInset];
}

- (void)addScrollDelegate:(id<UIScrollViewDelegate>)delegate
{
    if (delegate) {
        if (!_delegates) {
            _delegates = [NSHashTable hashTableWithOptions:NSPointerFunctionsWeakMemory];
        }
        [_delegates addObject:delegate];
    }
}

- (void)removeScrollDelegate:(id<UIScrollViewDelegate>)delegate
{
    if (delegate) {
        [_delegates removeObject:delegate];
    }
}

- (WXScrollDirection)scrollDirection
{
    return _scrollDirection;
}

- (NSString*)refreshType
{
    return _refreshType;
}
- (BOOL)requestGestureShouldStopPropagation:(UIGestureRecognizer *)gestureRecognizer shouldReceiveTouch:(UITouch *)touch
{
    return [self gestureShouldStopPropagation:gestureRecognizer shouldReceiveTouch:touch];
}

#pragma mark UIScrollViewDelegate

- (void)scrollViewWillBeginDragging:(UIScrollView *)scrollView
{
    _isDragging = YES;
    
    if ([_refreshType isEqualToString:@"refreshForAppear"] && _refreshComponent) {
        [_refreshComponent setIndicatorHidden:NO];
    }
    
    _scrollStartPoint = scrollView.contentOffset;
    
    if (_scrollStartEvent || _scrollEventListener) {
        CGFloat scaleFactor = self.weexInstance.pixelScaleFactor;
        NSDictionary *contentSizeData = @{@"width":@(scrollView.contentSize.width / scaleFactor),
                                          @"height":@(scrollView.contentSize.height / scaleFactor)};
        NSDictionary *contentOffsetData = @{@"x":@(-scrollView.contentOffset.x / scaleFactor),
                                            @"y":@(-scrollView.contentOffset.y / scaleFactor)};
        NSDictionary *params = @{@"contentSize":contentSizeData, @"contentOffset":contentOffsetData, @"isDragging":@(scrollView.isDragging)};
        
        if (_scrollStartEvent) {
            [self fireEvent:@"scrollstart" params:params domChanges:nil];
        }
        if (_scrollEventListener) {
            _scrollEventListener(self, @"scrollstart", params);
        }
    }
    
    NSHashTable *delegates = [_delegates copy];
    for (id<UIScrollViewDelegate> delegate in delegates) {
        if ([delegate respondsToSelector:@selector(scrollViewWillBeginDragging:)]) {
            [delegate scrollViewWillBeginDragging:scrollView];
        }
    }
}

- (void)scrollViewDidScroll:(UIScrollView *)scrollView
{
    //apply block which are registered
    WXSDKInstance *instance = self.weexInstance;
    if ([self.ref isEqualToString:WX_SDK_ROOT_REF] &&
        [self isKindOfClass:[WXScrollerComponent class]]) {
        if (instance.onScroll) {
            instance.onScroll(scrollView.contentOffset);
        }
    }
    
    if (_lastContentOffset.x > scrollView.contentOffset.x) {
        _direction = @"right";
    } else if (_lastContentOffset.x < scrollView.contentOffset.x) {
        _direction = @"left";
        if (WXScrollDirectionHorizontal == _scrollDirection) {
            [self handleLoadMore];
        }
    } else if(_lastContentOffset.y > scrollView.contentOffset.y) {
        _direction = @"down";
    } else if(_lastContentOffset.y < scrollView.contentOffset.y) {
        _direction = @"up";
        if (WXScrollDirectionVertical == _scrollDirection) {
            [self handleLoadMore];
        }
    }
    
    CGFloat scaleFactor = self.weexInstance.pixelScaleFactor;
    if (_isDragging) {
        // only trigger pullingDown event when user is dragging
        [_refreshComponent pullingdown:@{
                                         REFRESH_DISTANCE_Y: @(fabs((scrollView.contentOffset.y - _lastContentOffset.y)/scaleFactor)),
                                         REFRESH_VIEWHEIGHT: @(_refreshComponent.view.frame.size.height/scaleFactor),
                                         REFRESH_PULLINGDISTANCE: @(fabs(scrollView.contentOffset.y/scaleFactor)),
                                         @"type":@"pullingdown"
                                         }];
    }
    _lastContentOffset = scrollView.contentOffset;
    // check sticky
    [self adjustSticky];
    [self handleAppear];
    
    if (self.onScroll) {
        self.onScroll(scrollView);
    }
    
    if (_scrollEvent || _scrollEventListener) {
        CGFloat distance = 0;
        if (_scrollDirection == WXScrollDirectionHorizontal) {
            distance = scrollView.contentOffset.x - _lastScrollEventFiredOffset.x;
        } else {
            distance = scrollView.contentOffset.y - _lastScrollEventFiredOffset.y;
        }
        if (fabs(distance) >= _offsetAccuracy) {
            NSDictionary *contentSizeData = @{@"width": @(scrollView.contentSize.width / scaleFactor),
                                              @"height": @(scrollView.contentSize.height / scaleFactor)};
            //contentOffset values are replaced by (-contentOffset.x,-contentOffset.y), in order to be consistent with Android client.
            NSDictionary *contentOffsetData = @{@"x": @(-scrollView.contentOffset.x / scaleFactor),
                                                @"y": @(-scrollView.contentOffset.y / scaleFactor)};
            
            if (_scrollEvent) {
                [self fireEvent:@"scroll" params:@{@"contentSize":contentSizeData, @"contentOffset":contentOffsetData} domChanges:nil];
            }
            if (_scrollEventListener) {
                _scrollEventListener(self, @"scroll", @{@"contentSize":contentSizeData, @"contentOffset":contentOffsetData});
            }
            _lastScrollEventFiredOffset = scrollView.contentOffset;
        }
    }
    
    NSHashTable *delegates = [_delegates copy];
    for (id<UIScrollViewDelegate> delegate in delegates) {
        if ([delegate respondsToSelector:@selector(scrollViewDidScroll:)]) {
            [delegate scrollViewDidScroll:scrollView];
        }
    }
}

- (void)scrollViewDidEndScrollingAnimation:(UIScrollView *)scrollView
{
    [self dispatchScrollEndEvent:scrollView];
    
    UIEdgeInsets inset = [scrollView contentInset];
    
//  currently only set contentInset when loading
//    if ([_refreshComponent displayState]) {
//        inset.top = _refreshComponent.view.frame.size.height;
//    }
//    else {
//        inset.top = 0;
//    }
    
    if ([_loadingComponent displayState]) {
        inset.bottom = _loadingComponent.view.frame.size.height;
    } else {
        inset.bottom = 0;
    }
    [scrollView setContentInset:inset];
    
    NSHashTable *delegates = [_delegates copy];
    for (id<UIScrollViewDelegate> delegate in delegates) {
        if ([delegate respondsToSelector:@selector(scrollViewDidEndScrollingAnimation:)]) {
            [delegate scrollViewDidEndScrollingAnimation:scrollView];
        }
    }
}

- (void)scrollViewDidEndDecelerating:(UIScrollView *)scrollView
{
    if (!_isScrolling) {
        [self dispatchScrollEndEvent:scrollView];
        _scrollEndPoint = scrollView.contentOffset;
        id<WXPageEventNotifyEventProtocol> eventNotify = [WXSDKEngine handlerForProtocol:@protocol(WXPageEventNotifyEventProtocol)];
        if ([eventNotify respondsToSelector:@selector(notifyScrollEvent:from:to:)]) {
            [eventNotify notifyScrollEvent:self.weexInstance.instanceId from:_scrollStartPoint to:_scrollEndPoint];
        }
    }
    
    NSHashTable *delegates = [_delegates copy];
    for (id<UIScrollViewDelegate> delegate in delegates) {
        if ([delegate respondsToSelector:@selector(scrollViewDidEndDecelerating:)]) {
            [delegate scrollViewDidEndDecelerating:scrollView];
        }
    }
}

- (void)scrollViewWillEndDragging:(UIScrollView *)scrollView withVelocity:(CGPoint)velocity targetContentOffset:(inout CGPoint *)targetContentOffset
{
    // Page stop effect
    if (_pagingEnabled && _pageSize > 0) {
        if (_scrollDirection == WXScrollDirectionVertical) {
            CGFloat targetY = scrollView.contentOffset.y + velocity.y * 120.0;
            CGFloat targetIndex = round(targetY / _pageSize);
            
            /*
             When user's finger departs from screen with any velocity (like swipe gesture).
             We make sure that target index is changed.
             */
            CGFloat sourceIndex = round(_scrollStartPoint.y / _pageSize);
            if (velocity.y > 0.3) {
                if (targetIndex <= sourceIndex) {
                    targetIndex = sourceIndex + 1;
                }
            }
            else if (velocity.y < -0.3) {
                if (targetIndex >= sourceIndex) {
                    targetIndex = sourceIndex - 1;
                }
            }
            
            if (targetIndex < 0)
                targetIndex = 0;
            
            targetContentOffset->y = targetIndex * _pageSize;
        }
        else {
            CGFloat targetX = scrollView.contentOffset.x + velocity.x * 120.0;
            CGFloat targetIndex = round(targetX / _pageSize);
            
            /*
             When user's finger departs from screen with any velocity (like swipe gesture).
             We make sure that target index is changed.
             */
            CGFloat sourceIndex = round(_scrollStartPoint.x / _pageSize);
            if (velocity.x > 0.3) {
                if (targetIndex <= sourceIndex) {
                    targetIndex = sourceIndex + 1;
                }
            }
            else if (velocity.x < -0.3) {
                if (targetIndex >= sourceIndex) {
                    targetIndex = sourceIndex - 1;
                }
            }
            
            if (targetIndex < 0)
                targetIndex = 0;
            targetContentOffset->x = targetIndex * _pageSize;
        }
    }
    
    if ([_refreshType isEqualToString:@"refreshForAppear"]) {
        if(targetContentOffset == nil)
            return;
        CGPoint offset = *targetContentOffset;
        if(velocity.y <= 0) {
            // drop down
            if( offset.y <= _refreshComponent.calculatedFrame.size.height ) {
                [self loadMoreIfNeed];
            }
        } else if (velocity.y > 0) {
            // drop up
        }
    }
    
    NSHashTable *delegates = [_delegates copy];
    for (id<UIScrollViewDelegate> delegate in delegates) {
        if ([delegate respondsToSelector:@selector(scrollViewWillEndDragging:withVelocity:targetContentOffset:)]) {
            [delegate scrollViewWillEndDragging:scrollView withVelocity:velocity targetContentOffset:targetContentOffset];
        }
    }
}

- (void)scrollViewDidEndDragging:(UIScrollView *)scrollView willDecelerate:(BOOL)decelerate
{
    [_loadingComponent.view setHidden:NO];
    [_refreshComponent.view setHidden:NO];
    
    //refresh
    if ([_refreshType isEqualToString:@"refreshForWholeVisible"]) {
        if (_refreshComponent && scrollView.contentOffset.y < 0 && scrollView.contentOffset.y + _refreshComponent.calculatedFrame.size.height < _refreshComponent.calculatedFrame.origin.y) {
            [_refreshComponent refresh];
        }
    }
    
    //loading
    if (_loadingComponent && scrollView.contentOffset.y > 0 &&
        scrollView.contentOffset.y + scrollView.frame.size.height > _loadingComponent.view.frame.origin.y + _loadingComponent.calculatedFrame.size.height) {
        [_loadingComponent loading];
    }
    if (!decelerate) {
        _isScrolling = NO;
        [self performSelector:@selector(scrollViewDidEndDecelerating:) withObject:scrollView afterDelay:0.1];
    }
    
    NSHashTable *delegates = [_delegates copy];
    for (id<UIScrollViewDelegate> delegate in delegates) {
        if ([delegate respondsToSelector:@selector(scrollViewDidEndDragging:willDecelerate:)]) {
            [delegate scrollViewDidEndDragging:scrollView willDecelerate:decelerate];
        }
    }
    
    _isDragging = NO;
}

- (void)loadMoreIfNeed
{
    WXScrollerComponentView* scrollView = (WXScrollerComponentView *)self.view;
    if (scrollView.isDragging || scrollView.isTracking || scrollView.isDecelerating) {
        [self performSelector:@selector(loadMoreIfNeed) withObject:nil afterDelay:0.1];
        return;
    }
    [_refreshComponent refresh];
}

- (void)handleAppear
{
    if (![self isViewLoaded]) {
        return;
    }
    UIScrollView *scrollView = (UIScrollView *)self.view;
    CGFloat vx = scrollView.contentInset.left + scrollView.contentOffset.x;
    CGFloat vy = scrollView.contentInset.top + scrollView.contentOffset.y;
    CGFloat vw = scrollView.frame.size.width - scrollView.contentInset.left - scrollView.contentInset.right;
    CGFloat vh = scrollView.frame.size.height - scrollView.contentInset.top - scrollView.contentInset.bottom;
    CGRect scrollRect = CGRectMake(vx, vy, vw, vh);;
    
    // notify action for appear
    NSArray *listenerArrayCopy = [self.listenerArray copy];
    for(WXScrollToTarget *target in listenerArrayCopy){
        if (_shouldNotifiAppearDescendantView) {
            // if target component is descendant of scrollerview, it should notify the appear event handler, or here will skip this appear calculation.
            if ([target.target isViewLoaded] && [target.target.view isDescendantOfView:self.view]) {
                [self scrollToTarget:target scrollRect:scrollRect];
            }
        } else {
            [self scrollToTarget:target scrollRect:scrollRect];
        }
    }
}

- (CGPoint)absolutePositionForComponent:(WXComponent *)component
{
    return [component->_view.superview convertPoint:component->_view.frame.origin toView:_view];
}

#pragma mark  Private Methods
- (void)dispatchScrollEndEvent:(UIScrollView *)scrollView
{
    if (_scrollEndEvent || _scrollEventListener) {
        CGFloat scaleFactor = self.weexInstance.pixelScaleFactor;
        NSDictionary *contentSizeData = @{@"width":@(scrollView.contentSize.width / scaleFactor),
                                          @"height":@(scrollView.contentSize.height / scaleFactor)};
        NSDictionary *contentOffsetData = @{@"x":@(-scrollView.contentOffset.x / scaleFactor),
                                            @"y":@(-scrollView.contentOffset.y / scaleFactor)};
        if (_scrollEndEvent) {
            [self fireEvent:@"scrollend" params:@{@"contentSize":contentSizeData, @"contentOffset":contentOffsetData} domChanges:nil];
        }
        if (_scrollEventListener) {
            _scrollEventListener(self, @"scrollend", @{@"contentSize":contentSizeData, @"contentOffset":contentOffsetData});
        }
    }
}

- (void)scrollToTarget:(WXScrollToTarget *)target scrollRect:(CGRect)rect
{
    WXComponent *component = target.target;
    if (![component isViewLoaded]) { 
        return;
    }
    
    CGFloat ctop;
    if (component && component->_view && component->_view.superview) {
        ctop = [self absolutePositionForComponent:component].y;
    } else {
        ctop = 0.0;
    }
    CGFloat cbottom = ctop + CGRectGetHeight(component.calculatedFrame);
    CGFloat cleft;
    if (component && component->_view && component->_view.superview) {
        cleft = [self absolutePositionForComponent:component].x;
    } else {
        cleft = 0.0;
    }
    CGFloat cright = cleft + CGRectGetWidth(component.calculatedFrame);
    
    CGFloat vtop = CGRectGetMinY(rect), vbottom = CGRectGetMaxY(rect), vleft = CGRectGetMinX(rect), vright = CGRectGetMaxX(rect);
    if(cbottom > vtop && ctop <= vbottom && cleft <= vright && cright > vleft){
        if(!target.hasAppear && component){
            target.hasAppear = YES;
            if (component->_appearEvent) {
//                NSLog(@"appear:%@, %.2f", component, ctop);
                [component fireEvent:@"appear" params:_direction ? @{@"direction":_direction} : nil];
            }
        }
    } else {
        if(target.hasAppear && component){
            target.hasAppear = NO;
            if(component->_disappearEvent){
//                NSLog(@"disappear:%@", component);
                [component fireEvent:@"disappear" params:_direction ? @{@"direction":_direction} : nil];
            }
        }
    }
}

- (void)handleLoadMore
{
    if (_listenLoadMore && [self isNeedLoadMore]) {
        [self loadMore];
    }
}

#pragma mark Layout

- (CGFloat)_getInnerContentMainSize
{
    if (_scrollDirection == WXScrollDirectionVertical) {
        return _contentSize.height;
    }
    else if (_scrollDirection == WXScrollDirectionHorizontal) {
        return _contentSize.width;
    }
    else {
        return -1.0f;
    }
}

- (void)_assignInnerContentMainSize:(CGFloat)value
{
    if (_scrollDirection == WXScrollDirectionVertical) {
        _contentSize.height = value;
    }
    else if (_scrollDirection == WXScrollDirectionHorizontal) {
        _contentSize.width = value;
    }
}

- (void)_layoutPlatform
{
    /* Handle multiple vertical scrollers inside horizontal scroller case. In weexcore,
     a verticall list with NAN height will be set flex=1, which suppresses its style-width property.
     This will cause two lists with style-width 750px in a horizontal scroller sharing one screen width.
     Here we respect its style-width property so that the two lists will both be screen width wide. */
    
    if (_needsPlatformLayout) {
        if (_flexCssNode) {
            float top = _flexCssNode->getLayoutPositionTop();
            float left = _flexCssNode->getLayoutPositionLeft();
            float width = _flexCssNode->getLayoutWidth();
            float height = _flexCssNode->getLayoutHeight();

            if (_scrollDirection == WXScrollDirectionVertical) {
                _flexCssNode->setFlexDirection(WeexCore::kFlexDirectionColumn, NO);
                _flexCssNode->setStyleWidth(_flexCssNode->getLayoutWidth(), NO);
                _flexCssNode->setStyleHeight(FlexUndefined);
            } else {
                _flexCssNode->setFlexDirection(WeexCore::kFlexDirectionRow, NO);
                _flexCssNode->setStyleHeight(_flexCssNode->getLayoutHeight());
                _flexCssNode->setStyleWidth(FlexUndefined, NO);
            }

            _flexCssNode->markAllDirty();
            
            // this is scroll rtl solution.
            // scroll layout not use direction, use self tranform
            // but we need inherit direction in CSS, so we set children layout diretion manually
            _flexCssNode->determineChildLayoutDirection(_flexCssNode->getLayoutDirectionFromPathNode());
            
            std::pair<float, float> renderPageSize;
            renderPageSize.first = self.weexInstance.frame.size.width;
            renderPageSize.second = self.weexInstance.frame.size.height;
            auto parent = _flexCssNode->getParent(); // clear parent temporarily
            _flexCssNode->setParent(nullptr, _flexCssNode);
            _flexCssNode->calculateLayout(renderPageSize);
            _flexCssNode->setParent(parent, _flexCssNode);
            
            // set origin and size back
            _flexCssNode->rewriteLayoutResult(left, top, width, height);
        }
    }
    else {
        // should not happen, set platform layout to false
        _flexCssNode->setNeedsPlatformDependentLayout(false);
    }
}

@end
