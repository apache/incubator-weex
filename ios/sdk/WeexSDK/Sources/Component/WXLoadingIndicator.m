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

#import "WXLoadingIndicator.h"
#import "WXConvert.h"

@implementation WXLoadingIndicator {
    UIActivityIndicatorView *_indicator;
    UIColor *_indicatorColor;
    id _animating;
}

- (instancetype)initWithRef:(NSString *)ref type:(NSString *)type styles:(NSDictionary *)styles attributes:(NSDictionary *)attributes events:(NSArray *)events weexInstance:(WXSDKInstance *)weexInstance {
    self = [super initWithRef:ref type:type styles:styles attributes:attributes events:events weexInstance:weexInstance];
    if (self) {
        if (styles[@"color"]) {
            _indicatorColor = [WXConvert UIColor:styles[@"color"]];
        }
        if (attributes[@"animating"]) {
            _animating = attributes[@"animating"];
        }
    }
    return self;
}

- (UIView *)loadView {
    return [[UIActivityIndicatorView alloc] init];
}

- (void)viewDidLoad {
    _indicator = (UIActivityIndicatorView *)self.view;
    
    if (_indicatorColor) {
        _indicator.color = _indicatorColor;
    }
    if (_animating) {
        if ([WXConvert BOOL:_animating]) {
            [_indicator startAnimating];
        } else {
            [_indicator stopAnimating];
        }
    }
}

- (void)updateStyles:(NSDictionary *)styles {
    if (styles[@"color"]) {
        _indicatorColor = [WXConvert UIColor:styles[@"color"]];
        _indicator.color = _indicatorColor;
    }
}

- (void)updateAttributes:(NSDictionary *)attributes
{
    if (attributes[@"animating"]) {
        _animating = attributes[@"animating"];
        
        if ([WXConvert BOOL:_animating]) {
            [_indicator startAnimating];
        } else {
            [_indicator stopAnimating];
        }
    }
}

#pragma mark - lifeCircle

- (void)viewWillUnload {
    if (_indicator) {
        [_indicator stopAnimating];
        _indicator = nil;
    }
}

- (void)start {
    if (_indicator) {
        [_indicator startAnimating];
    }
}

- (void)stop {
    if (_indicator) {
        [_indicator stopAnimating];
    }
}

#pragma mark -reset color

- (void)resetStyles:(NSArray *)styles {
    if ([styles containsObject:@"color"]) {
        _indicatorColor = [UIColor blackColor];
        _indicator.color = _indicatorColor;
    }
}

@end
