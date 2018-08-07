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

#import "WXErrorView.h"

@implementation WXErrorView

- (id) initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    
    if (self) {
        UIImageView *imageView = [[UIImageView alloc]initWithFrame:self.bounds];
        NSBundle *bundle = [NSBundle bundleForClass:self.class];
        NSString *file = [bundle pathForResource:@"wx_load_error@3x" ofType:@"png"];
		if (file == nil) {
			file = [[NSBundle mainBundle] pathForResource:@"wx_load_error@3x" ofType:@"png"];
		}
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
