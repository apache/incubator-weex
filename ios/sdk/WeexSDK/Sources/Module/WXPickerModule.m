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

#import "WXPickerModule.h"
#import "WXConvert.h"
#import "WXUtility.h"
#import "WXComponentManager.h"
#import <UIKit/UIPickerView.h>
#import <UIKit/UIDatePicker.h>
#import <UIKit/UIKit.h>

#define WXPickerHeight 266
#define WXPickerToolBarHeight 44

@interface WXPickerModule() <UIGestureRecognizerDelegate>

@property (nonatomic, strong)NSString * pickerType;
// when resign the picker ,then the focus will be.
@property (nonatomic, strong)UIView * focusToView;
//picker
@property(nonatomic,strong)UIPickerView *picker;
@property(nonatomic,strong)UIView *backgroundView;
@property(nonatomic,strong)UIView *pickerView;

//custom
@property(nonatomic,copy)NSString *title;
@property(nonatomic,strong)UIColor *titleColor;
@property(nonatomic,copy)NSString *cancelTitle;
@property(nonatomic,copy)NSString *confirmTitle;
@property(nonatomic,strong)UIColor *cancelTitleColor;
@property(nonatomic,strong)UIColor *confirmTitleColor;
@property(nonatomic,strong)UIColor *titleBackgroundColor;
@property(nonatomic)CGFloat height;
@property(nonatomic,strong)UIColor *textColor;
@property(nonatomic,strong)UIColor *selectionColor;
//data
@property(nonatomic,copy)NSArray *items;
@property(nonatomic)BOOL isAnimating;
@property(nonatomic)NSInteger index;
@property(nonatomic,copy)WXModuleKeepAliveCallback callback;

//date picker
@property(nonatomic,strong)UIDatePicker *datePicker;
@property(nonatomic)UIDatePickerMode datePickerMode;

@end

@implementation WXPickerModule
@synthesize weexInstance;

WX_EXPORT_METHOD(@selector(pick:callback:))
WX_EXPORT_METHOD(@selector(pickDate:callback:))
WX_EXPORT_METHOD(@selector(pickTime:callback:))

#pragma mark -
#pragma mark Single Picker
-(void)dealloc
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self];

    if (nil != _backgroundView.superview) {
        UIView* backgroundView =  _backgroundView;
        dispatch_async(dispatch_get_main_queue(), ^{
            [backgroundView removeFromSuperview];
        });
    }
}

-(void)pick:(NSDictionary *)options callback:(WXModuleKeepAliveCallback)callback
{
    if (UIAccessibilityIsVoiceOverRunning()) {
        [self handleA11yFocusback:options];
    }
    
    _pickerType = @"picker";
    NSArray *items = @[];
    NSInteger index = 0 ;
  
    if (options[@"items"]) {
        items = options[@"items"];
    }
    if (options[@"index"]) {
        index = [WXConvert NSInteger:options[@"index"]];
    }
    if (options[@"title"]) {
        self.title = [WXConvert NSString:options[@"title"]];
    }
    if (options[@"titleColor"]) {
        self.titleColor = [WXConvert UIColor:options[@"titleColor"]];
    }
    if (options[@"cancelTitle"]) {
        self.cancelTitle = [WXConvert NSString:options[@"cancelTitle"]];
    }
    if (options[@"confirmTitle"]) {
        self.confirmTitle = [WXConvert NSString:options[@"confirmTitle"]];
    }
    if (options[@"cancelTitleColor"]) {
        self.cancelTitleColor = [WXConvert UIColor:options[@"cancelTitleColor"]];
    }
    if (options[@"confirmTitleColor"]) {
        self.confirmTitleColor = [WXConvert UIColor:options[@"confirmTitleColor"]];
    }
    if (options[@"titleBackgroundColor"]) {
        self.titleBackgroundColor = [WXConvert UIColor:options[@"titleBackgroundColor"]];
    }
    if (options[@"textColor"]) {
        self.textColor = [WXConvert UIColor:options[@"textColor"]];
    }
    if (options[@"selectionColor"]) {
        self.selectionColor = [WXConvert UIColor:options[@"selectionColor"]];
    }
    if (options[@"height"]) {
        self.height = [WXConvert CGFloat:options[@"height"]];
    }
    if (items && [items count]>0 && [self isRightItems:items]) {
        [self createPicker:items index:index];
        self.callback = callback;
    } else {
        if (callback) {
            callback(@{ @"result": @"error" },NO);
        }
        self.callback = nil;
    }
}

- (void)handleA11yFocusback:(NSDictionary*)options
{
    __weak typeof(self) weakSelf = self;
    if (options[@"sourceRef"] && [options[@"sourceRef"] isKindOfClass:[NSString class]]) {
        WXPerformBlockOnComponentThread(^{
            WXComponent * focusBackComponent = [weakSelf.weexInstance componentForRef:options[@"sourceRef"]];
            WXPerformBlockOnMainThread(^{
                weakSelf.focusToView = focusBackComponent.view;
            });
        });
    }
}

-(void)SetColorDelay:(NSNumber *)number
{
    if(self.selectionColor) {
        UILabel *labelSelected = (UILabel*)[self.picker viewForRow:[number integerValue] forComponent:0.3];
        [labelSelected setBackgroundColor:self.selectionColor];
    }
}

-(void)createPicker:(NSArray *)items index:(NSInteger)index
{
    [self configPickerView];
    self.items = [items copy];
    self.index = index;
    if (items && index < [items count]) {
        [self.picker selectRow:index inComponent:0 animated:NO];
        [self performSelector:@selector(SetColorDelay:) withObject:[NSNumber numberWithInteger:self.index] afterDelay:0.3];
        
    } else if(items && [items count]>0) {
        [self.picker selectRow:0 inComponent:0 animated:NO];
        [self performSelector:@selector(SetColorDelay:) withObject:[NSNumber numberWithInteger:0] afterDelay:0.3];

    }
    [self show];
}

-(void)show
{
    [[[UIApplication sharedApplication] keyWindow] endEditing:YES];  //hide keyboard
    UIWindow *window = [UIApplication sharedApplication].keyWindow;
    [window addSubview:self.backgroundView];
    if (self.isAnimating) {
        return;
    }
    self.isAnimating = YES;
    self.backgroundView.hidden = NO;
    UIView * focusView = self.picker;
    if([_pickerType isEqualToString:@"picker"]) {
        focusView = self.picker;
    } else {
        focusView = self.datePicker;
    }
    UIAccessibilityPostNotification(UIAccessibilityLayoutChangedNotification, focusView);
    [UIView animateWithDuration:0.35f animations:^{
        self.pickerView.frame = CGRectMake(0, [UIScreen mainScreen].bounds.size.height - WXPickerHeight, [UIScreen mainScreen].bounds.size.width, WXPickerHeight);
        self.backgroundView.alpha = 1;
        
    } completion:^(BOOL finished) {
        self.isAnimating = NO;
        
    }];
}

-(void)hide
{
    if (self.isAnimating) {
        return;
    }
    self.isAnimating = YES;
    [UIView animateWithDuration:0.35f animations:^{
        self.pickerView.frame = CGRectMake(0, [UIScreen mainScreen].bounds.size.height, [UIScreen mainScreen].bounds.size.width, WXPickerHeight);
        self.backgroundView.alpha = 0;
    } completion:^(BOOL finished) {
        self.backgroundView.hidden = YES;
        self.isAnimating = NO;
        if (!_focusToView) {
            _focusToView = self.backgroundView.superview;
        }
        UIAccessibilityPostNotification(UIAccessibilityScreenChangedNotification, _focusToView);
        [self.backgroundView removeFromSuperview];
    }];
}

-(void)cancel:(id)sender
{
    [self hide];
    if (self.callback) {
        self.callback(@{ @"result": @"cancel"},NO);
        self.callback=nil;
    }
}

-(void)done:(id)sender
{
    [self hide];
    if (self.callback) {
        self.callback(@{ @"result": @"success",@"data":[NSNumber numberWithInteger:self.index]},NO);
        self.callback=nil;
    }
}

-(BOOL)isRightItems:(NSArray *)array
{
    for (id value in array) {
        if([value isKindOfClass:[NSString class]] || [value isKindOfClass:[NSNumber class]]) {
            continue;
        }else {
            return NO;
        }
    }
    return YES;
}

-(NSString *)convertItem:(id)value
{
    if ([value isKindOfClass:[NSNumber class]]) {
        return [NSString stringWithFormat:@"%ld",[value longValue]];
    }
    return value;
}

#pragma mark -
#pragma mark Picker View

-(void)configPickerView
{
    self.backgroundView = [self createbackgroundView];
    UITapGestureRecognizer *tapGesture=[[UITapGestureRecognizer alloc]initWithTarget:self action:@selector(hide)];
    if (WX_SYS_VERSION_GREATER_THAN_OR_EQUAL_TO(@"11.0") && WX_SYS_VERSION_LESS_THAN(@"11.1")) {
        tapGesture.delegate = self;
    }
    [self.backgroundView addGestureRecognizer:tapGesture];
    self.pickerView = [self createPickerView];
    UIToolbar *toolBar=[[UIToolbar alloc]initWithFrame:CGRectMake(0, 0, [UIScreen mainScreen].bounds.size.width, WXPickerToolBarHeight)];
    toolBar.barTintColor = self.titleBackgroundColor?self.titleBackgroundColor:[UIColor whiteColor];
    
    
    
    UIBarButtonItem* noSpace = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFixedSpace target:nil action:nil];
    noSpace.width=10;
    
    UIBarButtonItem* doneBtn ;
    if (self.confirmTitle.length >0) {
        doneBtn = [[UIBarButtonItem alloc] initWithTitle:self.confirmTitle style:UIBarButtonItemStylePlain target:self action:@selector(done:)];
    }else {
       doneBtn = [[UIBarButtonItem alloc]initWithBarButtonSystemItem:UIBarButtonSystemItemDone target:self action:@selector(done:)];
    }
    if(self.confirmTitleColor){
        doneBtn.tintColor = self.confirmTitleColor;
    }
    UIBarButtonItem *cancelBtn;
    if (self.cancelTitle.length >0) {
        cancelBtn = [[UIBarButtonItem alloc] initWithTitle:self.cancelTitle style:UIBarButtonItemStylePlain target:self action:@selector(cancel:)];
    }else {
        cancelBtn = [[UIBarButtonItem alloc]initWithBarButtonSystemItem:UIBarButtonSystemItemCancel target:self action:@selector(cancel:)];
    }
    if(self.cancelTitleColor){
        cancelBtn.tintColor = self.cancelTitleColor;
    }
    UIBarButtonItem* flexSpace = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace target:nil action:nil];
    [toolBar setItems:[NSArray arrayWithObjects:noSpace,cancelBtn,flexSpace,doneBtn,noSpace, nil]];
    UILabel *titleLabel = [UILabel new];
    titleLabel.frame = CGRectMake(0, 0, 200, WXPickerToolBarHeight);
    titleLabel.center = toolBar.center;
    titleLabel.textAlignment = NSTextAlignmentCenter;
    if(self.titleColor){
        titleLabel.textColor = self.titleColor;
    }
    if(self.title.length>0){
        titleLabel.text = self.title;
        [toolBar addSubview:titleLabel];
    }
    [self.pickerView addSubview:toolBar];
    self.picker = [[UIPickerView alloc]init];
    self.picker.delegate = self;
    CGFloat height = WXPickerHeight;
    if (WXFloatEqual(self.height, 0)){
        height = self.height>WXPickerToolBarHeight?self.height:WXPickerHeight;
    }
    CGRect pickerFrame = CGRectMake(0, WXPickerToolBarHeight, [UIScreen mainScreen].bounds.size.width, height-WXPickerToolBarHeight);
    self.picker.backgroundColor = [UIColor whiteColor];
    self.picker.frame = pickerFrame;
    [self.pickerView addSubview:self.picker];
    [self.backgroundView addSubview:self.pickerView];
}

-(UIView *)createPickerView
{
    UIView *view = [UIView new];
    CGFloat height = WXPickerHeight;
    if (WXFloatEqual(self.height, 0)){
        height = self.height>WXPickerToolBarHeight?self.height:WXPickerHeight;
    }
    view.frame = CGRectMake(0, [UIScreen mainScreen].bounds.size.height, [UIScreen mainScreen].bounds.size.width, height);
    view.backgroundColor = [UIColor whiteColor];
    return view;
}

-(UIView *)createbackgroundView
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
    return [self convertItem:self.items[row]];
}

- (void)pickerView:(UIPickerView *)pickerView didSelectRow:(NSInteger)row inComponent:(NSInteger)component
{
    self.index = row;
    if(self.selectionColor) {
        UILabel *labelSelected = (UILabel*)[pickerView viewForRow:row forComponent:component];
        [labelSelected setBackgroundColor:self.selectionColor];
    }
}

-(UIView *)pickerView:(UIPickerView *)pickerView viewForRow:(NSInteger)row forComponent:(NSInteger)component reusingView:(UIView *)view
{
    
    UILabel *label = (id)view;
    
    if (!label)
    {
        
        label= [[UILabel alloc] initWithFrame:CGRectMake(0.0f, 0.0f, [pickerView rowSizeForComponent:component].width, [pickerView rowSizeForComponent:component].height)];
        label.textAlignment = NSTextAlignmentCenter;
        UIColor *color = self.textColor?self.textColor:[UIColor blackColor];
        label.textColor = color;
        label.text = [self convertItem:self.items[row]];
    }
    
    return label;
}


#pragma mark -
#pragma Date & Time Picker
-(void)pickDate:(NSDictionary *)options callback:(WXModuleKeepAliveCallback)callback
{
    if (UIAccessibilityIsVoiceOverRunning()) {
        [self handleA11yFocusback:options];
    }
    _pickerType = @"pickDate";
    self.datePickerMode = UIDatePickerModeDate;
    [self datepick:options callback:callback];
}

-(void)pickTime:(NSDictionary *)options callback:(WXModuleKeepAliveCallback)callback
{
    if (UIAccessibilityIsVoiceOverRunning()) {
        [self handleA11yFocusback:options];
    }
    _pickerType = @"pickTime";
    self.datePickerMode = UIDatePickerModeTime;
    [self datepick:options callback:callback];
}
    
-(void)datepick:(NSDictionary *)options callback:(WXModuleKeepAliveCallback)callback
{
    if ((UIDatePickerModeTime == self.datePickerMode) || (UIDatePickerModeDate == self.datePickerMode)) {
        [self createDatePicker:options callback:callback];
    } else {
        if (callback) {
            callback(@{ @"result": @"error" },NO);
        }
        self.callback = nil;
    }
}

- (void)createDatePicker:(NSDictionary *)options callback:(WXModuleKeepAliveCallback)callback
{
    self.callback = callback;
    self.datePicker = [[UIDatePicker alloc]init];
    if (UIDatePickerModeDate == self.datePickerMode) {
        self.datePicker.datePickerMode = UIDatePickerModeDate;
        NSString *value = [WXConvert NSString:options[@"value"]];
        if (value) {
            NSDate *date = [WXUtility dateStringToDate:value];
            if (date) {
                self.datePicker.date =date;
            }
        }
        NSString *max = [WXConvert NSString:options[@"max"]];
        if (max) {
            NSDate *date = [WXUtility dateStringToDate:max];
            if (date) {
                self.datePicker.maximumDate =date;
            }
        }
        NSString *min = [WXConvert NSString:options[@"min"]];
        if (min) {
            NSDate *date = [WXUtility dateStringToDate:min];
            if (date) {
                self.datePicker.minimumDate =date;
            }
        }
    } else if (UIDatePickerModeTime == self.datePickerMode) {
        self.datePicker.datePickerMode = UIDatePickerModeTime;
        NSString *value = [WXConvert NSString:options[@"value"]];
        if (value) {
            NSDate *date = [WXUtility timeStringToDate:value];
            if (date) {
                self.datePicker.date = date;
            }
        }
    }
    [self configDatePickerView];
    [self show];
}

-(void)configDatePickerView
{
    self.backgroundView = [self createbackgroundView];
    UITapGestureRecognizer *tapGesture=[[UITapGestureRecognizer alloc]initWithTarget:self action:@selector(hide)];
    if (WX_SYS_VERSION_GREATER_THAN_OR_EQUAL_TO(@"11.0") && WX_SYS_VERSION_LESS_THAN(@"11.1")) {
        tapGesture.delegate = self;
    }
    [self.backgroundView addGestureRecognizer:tapGesture];
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
    CGRect pickerFrame = CGRectMake(0, WXPickerToolBarHeight, [UIScreen mainScreen].bounds.size.width, WXPickerHeight-WXPickerToolBarHeight);
    self.datePicker.frame = pickerFrame;
    self.datePicker.backgroundColor = [UIColor whiteColor];
    [self.pickerView addSubview:self.datePicker];
    [self.backgroundView addSubview:self.pickerView];
}
    
-(void)cancelDatePicker:(id)sender
{
    [self hide];
    if (self.callback) {
        self.callback(@{ @"result": @"cancel"},NO);
        self.callback = nil;
    }
}

-(void)doneDatePicker:(id)sender
{
    [self hide];
    NSString *value = @"";
    if (UIDatePickerModeTime == self.datePicker.datePickerMode) {
        value = [WXUtility timeToString:self.datePicker.date];
    } else if(UIDatePickerModeDate == self.datePicker.datePickerMode)
    {
        value = [WXUtility dateToString:self.datePicker.date];
    }
    if (self.callback) {
        self.callback(@{ @"result": @"success",@"data":value},NO);
        self.callback=nil;
    }
}

#pragma mark - UIGestureRecognizerDelegate
- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldReceiveTouch:(UITouch *)touch {
    if (self.pickerView && [touch.view isDescendantOfView:self.pickerView]) {
        return NO;
    }
    return YES;
}

@end
