/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import "WXPickerModule.h"
#import <UIKit/UIPickerView.h>
#import <UIKit/UIKit.h>

#define WXPickerHeight 266

@interface WXPickerModule()

//view
@property(nonatomic,strong)UIPickerView *picker;
@property(nonatomic,strong)UIView *backgroudView; //手势触摸，隐藏picker
@property(nonatomic,strong)UIView *pickerView;    //picker页面

//data
@property(nonatomic,copy)NSArray *items;

@end

@implementation WXPickerModule
@synthesize weexInstance;

WX_EXPORT_METHOD(@selector(pick:index:callback:))

#pragma mark -
#pragma mark Single Picker
-(void)pick:(NSArray *)items index:(long long)index callback:(WXModuleCallback)callback
{
    
}

-(void)createPicker:(NSArray *)items index:(long long)index
{
    [self configPickerView];
    self.items = [items copy];
}

#pragma mark -
#pragma mark Pikcer View

-(void *)configPickerView
{
    if(!self.backgroudView)
    {
        self.backgroudView = [self createBackgroudView];
        UITapGestureRecognizer *tapGesture=[[UITapGestureRecognizer alloc]initWithTarget:self action:@selector(hide)];
        [self.backgroudView addGestureRecognizer:tapGesture];
    }
    
    if(!self.pickerView)
    {
        self.pickerView = [self createPickerView];
    }
    
    if(!self.picker)
    {
        self.picker = [[UIDatePicker alloc]init];
        CGRect pickerFrame = CGRectMake(0, 44, [UIScreen mainScreen].bounds.size.width, WXPickerHeight-44);
        self.picker.backgroundColor = [UIColor whiteColor];
        self.picker.frame = pickerFrame;
    }
}

-(UIView *)createPickerView
{
    UIView *view = [UIView new];
    view.frame = CGRectMake(0, [UIScreen mainScreen].bounds.size.height, [UIScreen mainScreen].bounds.size.width, WXPickerHeight);
    view.backgroundColor = [UIColor whiteColor];
    return view;
}

-(UIView *)createBackgroudView
{
    UIView *view = [UIView new];
    view.frame = [UIScreen mainScreen].bounds;
    view.backgroundColor = [UIColor colorWithRed:0.0 green:0.0 blue:0.0 alpha:0.4];
    return view;
}

#pragma mark -
#pragma UIPickerDelegate
- (NSInteger)pickerView:(UIPickerView *)pickerView numberOfRowsInComponent:(NSInteger)component
{
    return [self.items count];
}

- (CGFloat)pickerView:(UIPickerView *)pickerView rowHeightForComponent:(NSInteger)component
{
    return 44.0f;
}
- (void)pickerView:(UIPickerView *)pickerView didSelectRow:(NSInteger)row inComponent:(NSInteger)component
{
    
}
@end
