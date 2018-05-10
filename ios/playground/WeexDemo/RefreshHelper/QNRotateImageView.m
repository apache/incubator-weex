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

#import "QNRotateImageView.h"

@interface QNRotateImageView()

@property (assign, nonatomic) CATransform3D identityTransform;

@property (strong) NSTimer *timer;

@end

@implementation QNRotateImageView

- (id)init {
    
    self = [super init];
    if (nil == self) {
        
        return self;
    }
    
    [self _init];
    return self;
}

- (id)initWithFrame:(CGRect)frame {
    
    self = [super initWithFrame:frame];
    if (nil == self) {
        
        return self;
    }
    
    [self _init];
    return self;
}

- (void)awakeFromNib {
    
    [super awakeFromNib];
    [self _init];
}

- (void)_init {
    
    CATransform3D transform = CATransform3DIdentity;
    transform.m34 = 1.0 / -500;
    self.identityTransform = CATransform3DRotate(transform, 0, 0, 0, 1.0);
}

- (void)dealloc {
    
    [self finishRotate];
}

- (void)startRotate {
    
    [self finishRotate];
    self.timer = [NSTimer scheduledTimerWithTimeInterval:0.02 target:self selector:@selector(onTimer:) userInfo:nil repeats:YES];
}

- (void)finishRotate {
    
    [self.timer invalidate];
    self.timer = nil;
}

- (void)onTimer:(NSTimer *)timer {
    
    self.currentAngle += 0.04 * M_PI;
    [self applyEffect];
}

- (void)applyEffect {
    
    CATransform3D transform = CATransform3DRotate(self.identityTransform, self.currentAngle, 0, 0, 1.0);
    self.layer.transform = transform;
}


@end
