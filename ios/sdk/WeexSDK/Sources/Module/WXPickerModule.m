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

typedef NS_ENUM(NSUInteger, WXPickerModuleType) {
    WXPickerModuleTypeNormal = 0,
    WXPickerModuleTypeDate,
    WXPickerModuleTypeTime,
};

@interface WXPickerModule()

@property (nonatomic, assign) WXPickerModuleType pickerType;
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
@property(nonatomic,copy)WXModuleCallback callback;

//date picker
@property(nonatomic,strong)UIDatePicker *datePicker;

@end

@implementation WXPickerModule
@synthesize weexInstance;

WX_EXPORT_METHOD(@selector(pick:callback:))
WX_EXPORT_METHOD(@selector(pickDate:callback:))
WX_EXPORT_METHOD(@selector(pickTime:callback:))

#pragma mark -
#pragma mark LifeCycle
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

- (NSDictionary<NSString *, NSNumber *> *)pickerTypeMap {
    return @{@"normal": @0, @"date": @1, @"time": @2};
}

- (void)paramsParser:(NSDictionary *)options {
    self.items = @[];
    self.index = 0;
    self.pickerType = WXPickerModuleTypeNormal;
    if (options[@"type"]) {
        NSString *pickerTypeValue = [WXConvert NSString:options[@"type"]];
        NSNumber *pickerTypeNum = [[self pickerTypeMap] objectForKey:pickerTypeValue];
        if (pickerTypeNum) {
            self.pickerType = pickerTypeNum.unsignedIntegerValue;
        }
    }
    if (options[@"items"]) {
        self.items = options[@"items"];
    }
    if (options[@"index"]) {
        self.index = [WXConvert NSInteger:options[@"index"]];
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
}

-(void)pick:(NSDictionary *)options callback:(WXModuleCallback)callback
{
    if (UIAccessibilityIsVoiceOverRunning()) {
        [self handleA11yFocusback:options];
    }
    
    self.callback = callback;
    [self paramsParser:options];
    
    if (self.pickerType == WXPickerModuleTypeNormal) {
        if (!(self.items && [self.items count] > 0 && [self isRightItems:self.items])) {
            if (callback) {
                callback(@{ @"result": @"error" });
            }
            self.callback = nil;
            return;
        }
    }
    
    [self configPickerView:options];
    [self show];
}

-(void)pickDate:(NSDictionary *)options callback:(WXModuleCallback)callback
{
    NSMutableDictionary *dateOptions = [NSMutableDictionary dictionaryWithDictionary:options];
    dateOptions[@"type"] = @"date";
    [self pick:dateOptions callback:callback];
}

-(void)pickTime:(NSDictionary *)options callback:(WXModuleCallback)callback
{
    NSMutableDictionary *timeOptions = [NSMutableDictionary dictionaryWithDictionary:options];
    timeOptions[@"type"] = @"time";
    [self pick:timeOptions callback:callback];
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
    if(self.datePicker == WXPickerModuleTypeNormal) {
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
        self.callback(@{ @"result": @"cancel"});
        self.callback=nil;
    }
}

-(void)done:(id)sender
{
    [self hide];
    if (!self.callback) {
        return;
    }
    switch (self.pickerType) {
        case WXPickerModuleTypeNormal:
            self.callback(@{ @"result": @"success",@"data":[NSNumber numberWithInteger:self.index]});
            break;
            
        case WXPickerModuleTypeDate:
            self.callback(@{ @"result": @"success",@"data": [WXUtility dateToString:self.datePicker.date]});
            break;
        case WXPickerModuleTypeTime:
            self.callback(@{ @"result": @"success",@"data": [WXUtility timeToString:self.datePicker.date]});
            break;
            
        default:
            break;
    }
    self.callback = nil;
}

#pragma mark -
#pragma mark Picker View

-(void)configPickerView:(NSDictionary *)options
{
    self.backgroundView = [self createbackgroundView];
    UITapGestureRecognizer *tapGesture=[[UITapGestureRecognizer alloc]initWithTarget:self action:@selector(hide)];
    [self.backgroundView addGestureRecognizer:tapGesture];
    self.pickerView = [self createPickerView];
    
    UIView *toolbar = [self createToolbar];
    [self.pickerView addSubview:toolbar];
    
    switch (self.pickerType) {
        case WXPickerModuleTypeNormal:
            [self.pickerView addSubview:self.picker];
            [self loadNormalPickerData:self.items index:self.index];
            break;
            
        case WXPickerModuleTypeDate:
            self.datePicker.datePickerMode = UIDatePickerModeDate;
            [self.pickerView addSubview:self.datePicker];
            [self loadDatePickerData:options];
            break;
            
        case WXPickerModuleTypeTime:
            self.datePicker.datePickerMode = UIDatePickerModeTime;
            [self.pickerView addSubview:self.datePicker];
            [self loadTimePickerData:options];
            break;
            
        default:
            break;
    }
    
    [self.backgroundView addSubview:self.pickerView];
}

-(UIView *)createbackgroundView
{
    UIView *view = [UIView new];
    view.frame = [UIScreen mainScreen].bounds;
    view.backgroundColor = [UIColor colorWithRed:0.0 green:0.0 blue:0.0 alpha:0.4];
    return view;
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

- (UIView *)createToolbar {
    UIView *containerView = [[UIView alloc] initWithFrame:CGRectMake(0, 0, [UIScreen mainScreen].bounds.size.width, WXPickerToolBarHeight)];
    containerView.backgroundColor = self.titleBackgroundColor ? self.titleBackgroundColor : [UIColor colorWithWhite:0.97 alpha:1];
    [containerView addGestureRecognizer:[[UITapGestureRecognizer alloc] initWithTarget:nil action:nil]];
    
    UIButton *cancelButton = [UIButton buttonWithType:UIButtonTypeSystem];
    cancelButton.titleLabel.font = [UIFont systemFontOfSize:17];
    cancelButton.contentEdgeInsets = UIEdgeInsetsMake(0, 10, 0, 20);
    NSString *cancelButtonTitle = self.cancelTitle ? self.cancelTitle : [self localizedStringForKey:@"Cancel"];
    [cancelButton setTitle:cancelButtonTitle forState:UIControlStateNormal];
    if (self.cancelTitleColor) {
        [cancelButton setTitleColor:self.cancelTitleColor forState:UIControlStateNormal];
    }
    [cancelButton addTarget:self action:@selector(cancel:) forControlEvents:UIControlEventTouchUpInside];
    [containerView addSubview:cancelButton];
    
    UILabel *titleLabel = [UILabel new];
    titleLabel.textAlignment = NSTextAlignmentCenter;
    titleLabel.text = self.title;
    titleLabel.textColor = self.titleColor;
    [containerView addSubview:titleLabel];
    
    UIButton *confirmButton = [UIButton buttonWithType:UIButtonTypeSystem];
    confirmButton.titleLabel.font = [UIFont systemFontOfSize:17 weight:UIFontWeightSemibold];
    confirmButton.contentEdgeInsets = UIEdgeInsetsMake(0, 20, 0, 10);
    NSString *confirmButtonTitle = self.confirmTitle ? self.confirmTitle : [self localizedStringForKey:@"Done"];
    [confirmButton setTitle:confirmButtonTitle forState:UIControlStateNormal];
    if (self.cancelTitleColor) {
        [confirmButton setTitleColor:self.confirmTitleColor forState:UIControlStateNormal];
    }
    [confirmButton addTarget:self action:@selector(done:) forControlEvents:UIControlEventTouchUpInside];
    [containerView addSubview:confirmButton];
    
    cancelButton.translatesAutoresizingMaskIntoConstraints = NO;
    titleLabel.translatesAutoresizingMaskIntoConstraints = NO;
    confirmButton.translatesAutoresizingMaskIntoConstraints = NO;
    
    [cancelButton setContentCompressionResistancePriority:UILayoutPriorityDefaultHigh forAxis:UILayoutConstraintAxisHorizontal];
    [titleLabel setContentCompressionResistancePriority:UILayoutPriorityDefaultLow forAxis:UILayoutConstraintAxisHorizontal];
    [confirmButton setContentCompressionResistancePriority:UILayoutPriorityDefaultHigh forAxis:UILayoutConstraintAxisHorizontal];
    
    NSDictionary *views = @{@"cancelButton": cancelButton, @"titleLabel": titleLabel, @"confirmButton": confirmButton};
    NSString *formatString = @"|-0-[cancelButton]->=10-[titleLabel]->=10-[confirmButton]-0-|";
    NSArray *constraints = [NSLayoutConstraint constraintsWithVisualFormat:formatString options: NSLayoutFormatAlignAllCenterY metrics:nil views:views];
    [containerView addConstraints:constraints];
    [containerView addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|-0-[cancelButton]-0-|" options:NSLayoutFormatAlignAllCenterY metrics:nil views:views]];
    [containerView addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|-0-[confirmButton]-0-|" options:NSLayoutFormatAlignAllCenterY metrics:nil views:views]];
    [containerView addConstraint:[NSLayoutConstraint constraintWithItem:titleLabel attribute:NSLayoutAttributeCenterX relatedBy:NSLayoutRelationEqual toItem:containerView attribute:NSLayoutAttributeCenterX multiplier:1 constant:0]];
    return containerView;
}

#pragma mark -
#pragma mark UIPickerDelegate
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
#pragma mark Data
- (void)loadNormalPickerData:(NSArray *)items index:(NSUInteger)index {
    if (items && index < [items count]) {
        [self.picker selectRow:index inComponent:0 animated:NO];
        [self performSelector:@selector(SetColorDelay:) withObject:[NSNumber numberWithInteger:self.index] afterDelay:0.3];
    } else if(items && [items count]>0) {
        [self.picker selectRow:0 inComponent:0 animated:NO];
        [self performSelector:@selector(SetColorDelay:) withObject:[NSNumber numberWithInteger:0] afterDelay:0.3];
    }
}

- (void)loadDatePickerData:(NSDictionary *)options {
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
}

- (void)loadTimePickerData:(NSDictionary *)options {
    NSString *value = [WXConvert NSString:options[@"value"]];
    if (value) {
        NSDate *date = [WXUtility timeStringToDate:value];
        if (date) {
            self.datePicker.date = date;
        }
    }
}

#pragma mark -
#pragma mark Getter and Setter
- (UIPickerView *)picker {
    if (!_picker) {
        _picker = [UIPickerView new];
        _picker.delegate = self;
        CGFloat height = WXPickerHeight;
        if (WXFloatEqual(self.height, 0)){
            height = self.height>WXPickerToolBarHeight?self.height:WXPickerHeight;
        }
        _picker.frame = CGRectMake(0, WXPickerToolBarHeight, [UIScreen mainScreen].bounds.size.width, height-WXPickerToolBarHeight);
        _picker.backgroundColor = [UIColor whiteColor];
    }
    return _picker;
}

- (UIDatePicker *)datePicker {
    if (!_datePicker) {
        _datePicker = [UIDatePicker new];
        CGFloat height = WXPickerHeight;
        if (WXFloatEqual(self.height, 0)){
            height = self.height>WXPickerToolBarHeight?self.height:WXPickerHeight;
        }
        _datePicker.frame = CGRectMake(0, WXPickerToolBarHeight, [UIScreen mainScreen].bounds.size.width, height-WXPickerToolBarHeight);
        _datePicker.backgroundColor = [UIColor whiteColor];
    }
    return _datePicker;
}

#pragma mark -
#pragma mark Helper Methods
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

- (NSString *)localizedStringForKey:(NSString *)key {
    return [[NSBundle bundleWithIdentifier:@"com.apple.UIKit"] localizedStringForKey:key value:@"" table:nil];
}

@end
