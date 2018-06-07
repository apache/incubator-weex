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
#import "WXScrollerComponent+Layout.h"
#import "WXCoreLayout.h"
#import "WXPageEventNotifyEvent.h"

@interface WXScrollerComponentView:UIScrollView
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
    BOOL _listenLoadMore;
    BOOL _scrollEvent;
    BOOL _scrollStartEvent;
    BOOL _scrollEndEvent;
    BOOL _isScrolling;
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

- (void)_insertSubcomponent:(WXComponent *)subcomponent atIndex:(NSInteger)index
{
    [super _insertSubcomponent:subcomponent atIndex:index];
    
    if ([subcomponent isKindOfClass:[WXRefreshComponent class]]) {
        _refreshComponent = (WXRefreshComponent*)subcomponent;
    }
    else if ([subcomponent isKindOfClass:[WXLoadingComponent class]]) {
        _loadingComponent = (WXLoadingComponent*)subcomponent;
    }
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
        _loadMoreOffset = attributes[@"loadmoreoffset"] ? [WXConvert WXPixelType:attributes[@"loadmoreoffset"] scaleFactor:self.weexInstance.pixelScaleFactor] : 0;
        _loadmoreretry = attributes[@"loadmoreretry"] ? [WXConvert NSUInteger:attributes[@"loadmoreretry"]] : 0;
        _listenLoadMore = [events containsObject:@"loadmore"];
        _scrollable = attributes[@"scrollable"] ? [WXConvert BOOL:attributes[@"scrollable"]] : YES;
        _offsetAccuracy = attributes[@"offsetAccuracy"] ? [WXConvert WXPixelType:attributes[@"offsetAccuracy"] scaleFactor:self.weexInstance.pixelScaleFactor] : 0;
        
            _flexScrollerCSSNode = new WeexCore::WXCoreLayoutNode();
            // let scroller fill the rest space if it is a child component and has no fixed height & width
            if (((_scrollDirection == WXScrollDirectionVertical &&
                  flexIsUndefined(self.flexCssNode->getStyleHeight())) ||
                 (_scrollDirection == WXScrollDirectionHorizontal &&
                  flexIsUndefined(self.flexCssNode->getStyleWidth()))) &&
                self.flexCssNode->getFlex() <= 0.0) {
                self.flexCssNode->setFlex(1.0);
            }
        id configCenter = [WXSDKEngine handlerForProtocol:@protocol(WXConfigCenterProtocol)];
        if ([configCenter respondsToSelector:@selector(configForKey:defaultValue:isDefault:)]) {
            BOOL shouldNotifiAppearDescendantView = [[configCenter configForKey:@"iOS_weex_ext_config.shouldNotifiAppearDescendantView" defaultValue:@(YES) isDefault:NULL] boolValue];
            _shouldNotifiAppearDescendantView = shouldNotifiAppearDescendantView;
            BOOL shouldRemoveScrollerListener = [[configCenter configForKey:@"iOS_weex_ext_config.shouldRemoveScrollerListener" defaultValue:@(YES) isDefault:NULL] boolValue];
            _shouldRemoveScrollerListener = shouldRemoveScrollerListener;
            
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
    WXScrollerComponentView* scrollView = (WXScrollerComponentView *)self.view;
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

- (void)viewWillUnload
{
    ((UIScrollView *)_view).delegate = nil;
}

- (void)dealloc
{
    ((UIScrollView *)_view).delegate = nil;
    [self.stickyArray removeAllObjects];
    [self.listenerArray removeAllObjects];
        if(_flexScrollerCSSNode){
            [WXComponent recycleNodeOnComponentThread:_flexScrollerCSSNode gabRef:self.ref];
            
            _flexScrollerCSSNode=nullptr;
        }
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
        [self adjustSticky];
    }
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

    CGPoint contentOffset = scrollView.contentOffset;
    CGFloat scaleFactor = self.weexInstance.pixelScaleFactor;
    
    if (_scrollDirection == WXScrollDirectionHorizontal) {
        CGFloat contentOffetX = [component.supercomponent.view convertPoint:component.view.frame.origin toView:self.view].x;
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
    if ([_refreshType isEqualToString:@"refreshForAppear"] && _refreshComponent) {
        [_refreshComponent setIndicatorHidden:NO];
    }
    
    _scrollStartPoint = scrollView.contentOffset;
    
    if (_scrollStartEvent) {
        CGFloat scaleFactor = self.weexInstance.pixelScaleFactor;
        NSDictionary *contentSizeData = @{@"width":[NSNumber numberWithFloat:scrollView.contentSize.width / scaleFactor],@"height":[NSNumber numberWithFloat:scrollView.contentSize.height / scaleFactor]};
        NSDictionary *contentOffsetData = @{@"x":[NSNumber numberWithFloat:-scrollView.contentOffset.x / scaleFactor],@"y":[NSNumber numberWithFloat:-scrollView.contentOffset.y / scaleFactor]};
        [self fireEvent:@"scrollstart" params:@{@"contentSize":contentSizeData,@"contentOffset":contentOffsetData} domChanges:nil];
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
    [_refreshComponent pullingdown:@{
             REFRESH_DISTANCE_Y: @(fabs((scrollView.contentOffset.y - _lastContentOffset.y)/scaleFactor)),
             REFRESH_VIEWHEIGHT: @(_refreshComponent.view.frame.size.height/scaleFactor),
             REFRESH_PULLINGDISTANCE: @(scrollView.contentOffset.y/scaleFactor),
             @"type":@"pullingdown"
    }];
    _lastContentOffset = scrollView.contentOffset;
    // check sticky
    [self adjustSticky];
    [self handleAppear];
    
    if (self.onScroll) {
        self.onScroll(scrollView);
    }
    if (_scrollEvent) {
        NSDictionary *contentSizeData = [[NSDictionary alloc] initWithObjectsAndKeys:[NSNumber numberWithFloat:scrollView.contentSize.width / scaleFactor],@"width",[NSNumber numberWithFloat:scrollView.contentSize.height / scaleFactor],@"height", nil];
        //contentOffset values are replaced by (-contentOffset.x,-contentOffset.y) ,in order to be consistent with Android client.
        NSDictionary *contentOffsetData = [[NSDictionary alloc] initWithObjectsAndKeys:[NSNumber numberWithFloat:-scrollView.contentOffset.x / scaleFactor],@"x",[NSNumber numberWithFloat:-scrollView.contentOffset.y / scaleFactor],@"y", nil];
        CGFloat distance = 0;
        if (_scrollDirection == WXScrollDirectionHorizontal) {
            distance = scrollView.contentOffset.x - _lastScrollEventFiredOffset.x;
        } else {
            distance = scrollView.contentOffset.y - _lastScrollEventFiredOffset.y;
        }
        if (fabs(distance) >= _offsetAccuracy) {
            [self fireEvent:@"scroll" params:@{@"contentSize":contentSizeData,@"contentOffset":contentOffsetData} domChanges:nil];
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
    if (_scrollEndEvent) {
        if (!_isScrolling) {
            CGFloat scaleFactor = self.weexInstance.pixelScaleFactor;
            NSDictionary *contentSizeData = @{@"width":[NSNumber numberWithFloat:scrollView.contentSize.width / scaleFactor],@"height":[NSNumber numberWithFloat:scrollView.contentSize.height / scaleFactor]};
            NSDictionary *contentOffsetData = @{@"x":[NSNumber numberWithFloat:-scrollView.contentOffset.x / scaleFactor],@"y":[NSNumber numberWithFloat:-scrollView.contentOffset.y / scaleFactor]};
            [self fireEvent:@"scrollend" params:@{@"contentSize":contentSizeData,@"contentOffset":contentOffsetData} domChanges:nil];
        }
    }
    if (!_isScrolling) {
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
        [self performSelector:@selector(scrollViewDidEndDecelerating:) withObject:nil afterDelay:0.1];
    }
    
    NSHashTable *delegates = [_delegates copy];
    for (id<UIScrollViewDelegate> delegate in delegates) {
        if ([delegate respondsToSelector:@selector(scrollViewDidEndDragging:willDecelerate:)]) {
            [delegate scrollViewDidEndDragging:scrollView willDecelerate:decelerate];
        }
    }
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

- (NSUInteger)_childrenCountForLayout;
{
    return 0;
}

- (NSUInteger)childrenCountForScrollerLayout
{
    return [super _childrenCountForLayout];
}

- (void)_calculateFrameWithSuperAbsolutePosition:(CGPoint)superAbsolutePosition
                          gatherDirtyComponents:(NSMutableSet<WXComponent *> *)dirtyComponents
{
    /**
     *  Pretty hacky way
     *  layout from root to scroller to get scroller's frame,
     *  layout from children to scroller to get scroller's contentSize
     */
    if ([self needsLayout]) {
            _flexScrollerCSSNode->copyStyle(_flexCssNode);
            _flexScrollerCSSNode->copyMeasureFunc(_flexCssNode);
            
            if (_scrollDirection == WXScrollDirectionVertical) {
                _flexScrollerCSSNode->setFlexDirection(WeexCore::kFlexDirectionColumn,NO);
                _flexScrollerCSSNode->setStyleWidth(self.flexCssNode->getLayoutWidth(),NO);
                _flexScrollerCSSNode->setStyleHeight(FlexUndefined);
            } else {
                _flexScrollerCSSNode->setFlexDirection(WeexCore::kFlexDirectionRow,NO);
                _flexScrollerCSSNode->setStyleHeight(self.flexCssNode->getLayoutHeight());
                _flexScrollerCSSNode->setStyleWidth(FlexUndefined,NO);
            }
            _flexScrollerCSSNode->markDirty();
            std::pair<float, float> renderPageSize;
            renderPageSize.first = self.weexInstance.frame.size.width;
            renderPageSize.second = self.weexInstance.frame.size.height;
            _flexScrollerCSSNode->calculateLayout(renderPageSize);
            CGSize size = {
                WXRoundPixelValue(_flexScrollerCSSNode->getLayoutWidth()),
                WXRoundPixelValue(_flexScrollerCSSNode->getLayoutHeight())
            };
            
            if (!CGSizeEqualToSize(size, _contentSize)) {
                // content size
                _contentSize = size;
                [dirtyComponents addObject:self];
            }
    }
    
    [super _calculateFrameWithSuperAbsolutePosition:superAbsolutePosition gatherDirtyComponents:dirtyComponents];
}

@end
