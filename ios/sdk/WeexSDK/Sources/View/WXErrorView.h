//
//  WXErrorView.h
//  Pods
//
//  Created by bobning on 16/6/29.
//
//

#import <UIKit/UIKit.h>

@protocol WXErrorViewDelegate <NSObject>

- (void)onclickErrorView;

@end

@interface WXErrorView : UIView

@property (nonatomic, weak) id<WXErrorViewDelegate>  delegate;

@end
