//
//  WXLayoutConstraint.m
//  Pods-WeexDemo
//
//  Created by 邹明 on 2018/6/26.
//

#import "WXLayoutConstraint.h"

@implementation WXLayoutMargin

- (instancetype)initWithTop:(float)top bottom:(float)bottom left:(float)left right:(float)right{
    if (self = [super init]) {
        _top = top;
        _bottom = bottom;
        _left = left;
        _right = right;
    }
    return self;
}

@end

@implementation WXLayoutPadding

- (instancetype)initWithTop:(float)top bottom:(float)bottom left:(float)left right:(float)right{
    if (self = [super init]) {
        _top = top;
        _bottom = bottom;
        _left = left;
        _right = right;
    }
    return self;
}

@end

@implementation WXLayoutBorderWidth

- (instancetype)initWithTop:(float)top bottom:(float)bottom left:(float)left right:(float)right{
    if (self = [super init]) {
        _top = top;
        _bottom = bottom;
        _left = left;
        _right = right;
    }
    return self;
}

@end
