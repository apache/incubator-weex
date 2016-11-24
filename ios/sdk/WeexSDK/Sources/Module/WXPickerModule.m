/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import "WXPickerModule.h"
#import "WXConvert.h"
#import <UIKit/UIPickerView.h>
#import <UIKit/UIDatePicker.h>
#import <UIKit/UIKit.h>

#define WXPickerHeight 266
#define WXPickerToolBarHeight 44

@interface WXPickerModule()

//view
@property(nonatomic,strong)UIPickerView *picker;
@property(nonatomic,strong)UIView *backgroudView; //手势触摸，隐藏picker
@property(nonatomic,strong)UIView *pickerView;    //picker页面
    
//data
@property(nonatomic,copy)NSArray *items;
@property(nonatomic)BOOL isAnimating;
@property(nonatomic)NSInteger index;
@property(nonatomic,copy)WXModuleCallback callback;

//date picker
@property(nonatomic,strong)UIDatePicker *datePicker;
@property(nonatomic)NSString *datePickerType;

@end

@implementation WXPickerModule
@synthesize weexInstance;


WX_EXPORT_METHOD(@selector(pick:callback:))
WX_EXPORT_METHOD(@selector(datepick:callback:))

#pragma mark -
#pragma mark Single Picker
-(void)pick:(NSDictionary *)options callback:(WXModuleCallback)callback
{
    NSArray *items = @[];
    NSInteger index = 0 ;
    if(options[@"items"]) {
        items = options[@"items"];
    }
    if(options[@"index"]) {
        index = [WXConvert NSInteger:options[@"index"]];
    }
    if(items && [items count]>0) {
        [self createPicker:items index:index];
        self.callback = callback;
    }else {
        callback(@{ @"result": @"error" });
        self.callback = nil;
    }
}

-(void)createPicker:(NSArray *)items index:(NSInteger)index
{
    [self configPickerView];
    self.items = [items copy];
    self.index = index;
    if(items && index < [items count]) {
        [self.picker selectRow:index inComponent:0 animated:NO];
    }else if(items && [items count]>0) {
        [self.picker selectRow:0 inComponent:0 animated:NO];
    }
    [self show];
}

-(void)show
{
    [[[UIApplication sharedApplication] keyWindow] endEditing:YES];  //收起键盘
    UIWindow *window = [UIApplication sharedApplication].keyWindow;
    [window addSubview:self.backgroudView];
    if(self.isAnimating) {
        return;
    }
    self.isAnimating = YES;
    self.backgroudView.hidden = NO;
    [UIView animateWithDuration:0.35f animations:^{
        self.pickerView.frame = CGRectMake(0, [UIScreen mainScreen].bounds.size.height - WXPickerHeight, [UIScreen mainScreen].bounds.size.width, WXPickerHeight);
        self.backgroudView.alpha = 1;
    } completion:^(BOOL finished) {
        self.isAnimating = NO;
    }];
}

-(void)hide
{
    if(self.isAnimating) {
        return;
    }
    self.isAnimating = YES;
    [UIView animateWithDuration:0.35f animations:^{
        self.pickerView.frame = CGRectMake(0, [UIScreen mainScreen].bounds.size.height, [UIScreen mainScreen].bounds.size.width, WXPickerHeight);
        self.backgroudView.alpha = 0;
    } completion:^(BOOL finished) {
        self.backgroudView.hidden = YES;
        self.isAnimating = NO;
        [self.backgroudView removeFromSuperview];
    }];
}

-(void)cancel:(id)sender
{
    [self hide];
    self.callback(@{ @"result": @"cancel"});
    self.callback=nil;
}

-(void)done:(id)sender
{
    [self hide];
    self.callback(@{ @"result": @"success",@"data":[NSNumber numberWithInteger:self.index]});
    self.callback=nil;
}

#pragma mark -
#pragma mark Pikcer View

-(void)configPickerView
{
    if(!self.backgroudView) {
        self.backgroudView = [self createBackgroudView];
        UITapGestureRecognizer *tapGesture=[[UITapGestureRecognizer alloc]initWithTarget:self action:@selector(hide)];
        [self.backgroudView addGestureRecognizer:tapGesture];
    }
    if(!self.pickerView) {
        self.pickerView = [self createPickerView];
        UIToolbar *toolBar=[[UIToolbar alloc]initWithFrame:CGRectMake(0, 0, [UIScreen mainScreen].bounds.size.width, WXPickerToolBarHeight)];
        [toolBar setBackgroundColor:[UIColor whiteColor]];
        UIBarButtonItem* noSpace = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFixedSpace target:nil action:nil];
        noSpace.width=10;
        UIBarButtonItem* doneBtn = [[UIBarButtonItem alloc]initWithBarButtonSystemItem:UIBarButtonSystemItemDone target:self action:@selector(done:)];
        UIBarButtonItem *flexSpace = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace target:nil action:nil];
        UIBarButtonItem* cancelBtn = [[UIBarButtonItem alloc]initWithBarButtonSystemItem:UIBarButtonSystemItemCancel target:self action:@selector(cancel:)];
        [toolBar setItems:[NSArray arrayWithObjects:noSpace,cancelBtn,flexSpace,doneBtn,noSpace, nil]];
        [self.pickerView addSubview:toolBar];
    }
    if(!self.picker) {
        self.picker = [[UIPickerView alloc]init];
        self.picker.delegate = self;
        CGRect pickerFrame = CGRectMake(0, WXPickerToolBarHeight, [UIScreen mainScreen].bounds.size.width, WXPickerHeight-WXPickerToolBarHeight);
        self.picker.backgroundColor = [UIColor whiteColor];
        self.picker.frame = pickerFrame;
        [self.pickerView addSubview:self.picker];
        [self.backgroudView addSubview:self.pickerView];
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

- (NSString *)pickerView:(UIPickerView *)pickerView titleForRow:(NSInteger)row forComponent:(NSInteger)component
{
    return self.items[row];
}

- (void)pickerView:(UIPickerView *)pickerView didSelectRow:(NSInteger)row inComponent:(NSInteger)component
{
    self.index = row;
}
    
#pragma mark -
#pragma Date Picker
-(void)datepick:(NSDictionary *)options callback:(WXModuleCallback)callback
{
    if([options[@"type"] isEqualToString:@"date"] || [options[@"type"] isEqualToString:@"time"]) {
        [self createDatePicker:options callback:callback];
    }else {
        callback(@{ @"result": @"error" });
        self.callback = nil;
    }
}

- (void)createDatePicker:(NSDictionary *)options callback:(WXModuleCallback)callback
{
    self.callback = callback;
    if(!self.datePicker) {
        self.datePicker = [[UIDatePicker alloc]init];
    }
    self.datePickerType = options[@"type"];
    if([self.datePickerType isEqualToString:@"date"]) {
        self.datePicker.datePickerMode = UIDatePickerModeDate;
        NSString *value = [WXConvert NSString:options[@"value"]];
        if(value)
        {
            NSDate *date = [self inputDateStringToDate:value];
            if(date)
            {
                self.datePicker.date =date;
            }
        }
        NSString *max = [WXConvert NSString:options[@"max"]];
        if(max)
        {
            NSDate *date = [self inputDateStringToDate:max];
            if(date)
            {
                self.datePicker.maximumDate =date;
            }
        }
        NSString *min = [WXConvert NSString:options[@"min"]];
        if(min)
        {
            NSDate *date = [self inputDateStringToDate:min];
            if(date)
            {
                self.datePicker.minimumDate =date;
            }
        }
    }else if([self.datePickerType isEqualToString:@"time"])
    {
        self.datePicker.datePickerMode = UIDatePickerModeTime;
        NSString *value = [WXConvert NSString:options[@"value"]];
        if(value)
        {
            NSDate *date = [self inputTimeStringToDate:value];
            if(date)
            {
                self.datePicker.date = date;
            }
        }
    }
    [self configDatePickerView];
    [self showDatePicker];
}
    
-(void)configDatePickerView
{
    if(!self.backgroudView) {
        self.backgroudView = [self createBackgroudView];
        UITapGestureRecognizer *tapGesture=[[UITapGestureRecognizer alloc]initWithTarget:self action:@selector(hide)];
        [self.backgroudView addGestureRecognizer:tapGesture];
    }
    if(!self.pickerView) {
        self.pickerView = [self createPickerView];
        UIToolbar *toolBar=[[UIToolbar alloc]initWithFrame:CGRectMake(0, 0, [UIScreen mainScreen].bounds.size.width, WXPickerToolBarHeight)];
        [toolBar setBackgroundColor:[UIColor whiteColor]];
        UIBarButtonItem* noSpace = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFixedSpace target:nil action:nil];
        noSpace.width=10;
        UIBarButtonItem* doneBtn = [[UIBarButtonItem alloc]initWithBarButtonSystemItem:UIBarButtonSystemItemDone target:self action:@selector(doneDatePicker:)];
        UIBarButtonItem *flexSpace = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace target:nil action:nil];
        UIBarButtonItem* cancelBtn = [[UIBarButtonItem alloc]initWithBarButtonSystemItem:UIBarButtonSystemItemCancel target:self action:@selector(cancelDatePicker:)];
        [toolBar setItems:[NSArray arrayWithObjects:noSpace,cancelBtn,flexSpace,doneBtn,noSpace, nil]];
        [self.pickerView addSubview:toolBar];
    }
    CGRect pickerFrame = CGRectMake(0, WXPickerToolBarHeight, [UIScreen mainScreen].bounds.size.width, WXPickerHeight-WXPickerToolBarHeight);
    self.datePicker.backgroundColor = [UIColor whiteColor];
    self.datePicker.frame = pickerFrame;
    [self.pickerView addSubview:self.datePicker];
    [self.backgroudView addSubview:self.pickerView];
}
-(void)showDatePicker
{
    UIWindow *window = [UIApplication sharedApplication].keyWindow;
    [window addSubview:self.backgroudView];
    if(self.isAnimating)
    {
        return;
    }
    self.isAnimating = YES;
    self.backgroudView.hidden = NO;
    [UIView animateWithDuration:0.35f animations:^{
        self.pickerView.frame = CGRectMake(0, [UIScreen mainScreen].bounds.size.height - WXPickerHeight, [UIScreen mainScreen].bounds.size.width, WXPickerHeight);
        self.backgroudView.alpha = 1;
    } completion:^(BOOL finished) {
        self.isAnimating = NO;
    }];
}
    
-(void)hideDatePicker
{
    if(self.isAnimating)
    {
        return;
    }
    self.isAnimating = YES;
    [UIView animateWithDuration:0.35f animations:^{
        self.pickerView.frame = CGRectMake(0, [UIScreen mainScreen].bounds.size.height, [UIScreen mainScreen].bounds.size.width, WXPickerHeight);
        self.backgroudView.alpha = 0;
    } completion:^(BOOL finished) {
        self.backgroudView.hidden = YES;
        self.isAnimating = NO;
        [self.backgroudView removeFromSuperview];
    }];
}
    
-(void)cancelDatePicker:(id)sender
{
    [self hide];
    self.callback = nil;
}
    
-(void)doneDatePicker:(id)sender
{
    [self hide];
    NSString *value = @"";
    if([self.datePickerType isEqualToString:@"time"])
    {
        value = [self timeToString:self.datePicker.date];
    }else if([self.datePickerType isEqualToString:@"date"])
    {
        value = [self dateToString:self.datePicker.date];
    }
    self.callback(@{ @"result": @"success",@"data":value});
    self.callback=nil;
}
    
-(NSDate *)inputDateStringToDate:(NSString *)dateString
{
    NSDateFormatter *formatter = [[NSDateFormatter alloc] init] ;
    [formatter setDateFormat:@"yyyy-MM-dd"];
    NSDate *date=[formatter dateFromString:dateString];
    return date;
}
    
-(NSDate *)inputTimeStringToDate:(NSString *)dateString
{
    NSDateFormatter *formatter = [[NSDateFormatter alloc] init] ;
    [formatter setDateFormat:@"HH:mm"];
    NSDate *date=[formatter dateFromString:dateString];
    return date;
}
    
-(NSString *)dateToString:(NSDate *)date
{
    NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
    [dateFormatter setDateFormat:@"yyyy-MM-dd"];
    NSString *str = [dateFormatter stringFromDate:date];
    return str;
}
    
-(NSString *)timeToString:(NSDate *)date
{
    NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
    [dateFormatter setDateFormat:@"HH:mm"];
    NSString *str = [dateFormatter stringFromDate:date];
    return str;
}

@end
