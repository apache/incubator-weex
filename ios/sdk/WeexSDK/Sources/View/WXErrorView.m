//
//  WXErrorView.m
//  Pods
//
//  Created by bobning on 16/6/29.
//
//

#import "WXErrorView.h"

@implementation WXErrorView

- (id) initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    
    if (self) {
        UIImageView *imageView = [[UIImageView alloc]initWithFrame:self.bounds];
        NSString *file = [[NSBundle mainBundle] pathForResource:@"error@3x" ofType:@"png"];
        imageView.image = [UIImage imageWithContentsOfFile:file];
        [self addSubview:imageView];
        
        UITapGestureRecognizer *tapRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(tapGesture:)];
        [self addGestureRecognizer:tapRecognizer];
    }
    return self;
}

- (void)tapGesture:(UITapGestureRecognizer *)sender
{
    [self.delegate onclickErrorView];
}

@end
