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

#import "WXSwitchComponent.h"
#import "WXConvert.h"
#import "WXComponent_internal.h"

@interface WXSwitchView : UISwitch

@end

@implementation WXSwitchView

@end

@interface WXSwitchComponent()

@property (nonatomic, strong)   WXSwitchView    *switchView;
@property (nonatomic, assign)   BOOL    changeEvent;
@property (nonatomic, assign)   BOOL    checked;
@property (nonatomic, assign)   BOOL    disabled;

@end

@implementation WXSwitchComponent

- (instancetype)initWithRef:(NSString *)ref type:(NSString *)type styles:(NSDictionary *)styles attributes:(NSDictionary *)attributes events:(NSArray *)events weexInstance:(WXSDKInstance *)weexInstance
{
    if (self = [super initWithRef:ref type:type styles:styles attributes:attributes events:events weexInstance:weexInstance]) {
        _checked = attributes[@"checked"] ? [WXConvert BOOL:attributes[@"checked"]] : NO;
        _disabled = attributes[@"disabled"] ? [WXConvert BOOL:attributes[@"disabled"]] : NO;
        
        self.cssNode->style.dimensions[CSS_WIDTH] = 51;
        self.cssNode->style.dimensions[CSS_HEIGHT] = 31;
    }
    return self;
}

- (UIView *)loadView
{
    return [[WXSwitchView alloc] init];
}

- (void)viewDidLoad
{
    _switchView = (WXSwitchView *)self.view;
    _switchView.isAccessibilityElement = YES;
    [_switchView setOn:_checked animated:YES];
    [_switchView setEnabled:!_disabled];
    [_switchView addTarget:self action:@selector(checkChanged) forControlEvents:UIControlEventValueChanged];
}

- (void)addEvent:(NSString *)eventName
{
    if ([eventName isEqualToString:@"change"]) {
        _changeEvent = YES;
    }
}

- (void)removeEvent:(NSString *)eventName
{
    if ([eventName isEqualToString:@"change"]) {
        _changeEvent = NO;
    }
}

- (void)updateAttributes:(NSDictionary *)attributes
{
    if (attributes[@"checked"]) {
        _checked = [WXConvert BOOL:attributes[@"checked"]];
        [_switchView setOn:_checked animated:YES];
    }
    else if (attributes[@"disabled"]) {
        _disabled = [WXConvert BOOL:attributes[@"disabled"]];
        [_switchView setEnabled:!_disabled];
    }
}

- (void)checkChanged
{
   if (_changeEvent) {
        [self fireEvent:@"change" params:@{@"value":@([_switchView isOn])} domChanges:@{@"attrs": @{@"checked": @([_switchView isOn])}}];
   }
}

@end
