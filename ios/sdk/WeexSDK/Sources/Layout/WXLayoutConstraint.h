//
//  WXLayoutConstraint.h
//  Pods-WeexDemo
//
//  Created by 邹明 on 2018/6/26.
//

#import <Foundation/Foundation.h>

@interface WXLayoutMargin : NSObject

@property (nonatomic,assign,readonly) float top;
@property (nonatomic,assign,readonly) float bottom;
@property (nonatomic,assign,readonly) float left;
@property (nonatomic,assign,readonly) float right;

- (instancetype)initWithTop:(float)top bottom:(float)bottom left:(float)left right:(float)right;

@end

@interface WXLayoutPadding : NSObject

@property (nonatomic,assign,readonly) float top;
@property (nonatomic,assign,readonly) float bottom;
@property (nonatomic,assign,readonly) float left;
@property (nonatomic,assign,readonly) float right;

- (instancetype)initWithTop:(float)top bottom:(float)bottom left:(float)left right:(float)right;

@end

@interface WXLayoutBorderWidth : NSObject

@property (nonatomic,assign,readonly) float top;
@property (nonatomic,assign,readonly) float bottom;
@property (nonatomic,assign,readonly) float left;
@property (nonatomic,assign,readonly) float right;

- (instancetype)initWithTop:(float)top bottom:(float)bottom left:(float)left right:(float)right;

@end
