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

#import "WXTitleBarModule.h"
#import "WXDemoViewController.h"

@implementation WXTitleBarModule
@synthesize weexInstance;

WX_EXPORT_METHOD(@selector(setTitle:))
WX_EXPORT_METHOD(@selector(showTitleBar:))
WX_EXPORT_METHOD(@selector(setStyle:))

- (void)setTitle:(NSString*)title
{
    if (title) {
        [weexInstance.viewController.navigationItem setTitle:title];
    }
}

- (void)setStyle:(NSDictionary*)styles
{
    if (styles[@"backgroundColor"]) {
        weexInstance.viewController.navigationController.navigationBar.barTintColor = [WXConvert UIColor:styles[@"backgroundColor"]];
    }
    if (styles[@"foregroundColor"]) {
        weexInstance.viewController.navigationController.navigationBar.tintColor = [WXConvert UIColor:styles[@"foregroundColor"]];
        [weexInstance.viewController.navigationController.navigationBar setTitleTextAttributes:@{NSForegroundColorAttributeName:[WXConvert UIColor:styles[@"foregroundColor"]]}];
    }
}

- (void)dealloc
{
    weexInstance.viewController.navigationController.navigationBar.tintColor = [UIColor whiteColor];
}

- (void)showTitleBar:(bool)show
{
    [weexInstance.viewController.navigationController setNavigationBarHidden:!show];
    ((WXDemoViewController*)weexInstance.viewController).showNavigationBar = !show;
}

@end
