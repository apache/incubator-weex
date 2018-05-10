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

#import <UIKit/UIKit.h>
#import "QNWaveLayer.h"

@interface QNWaveLayer()<CAAnimationDelegate>

@property (nonatomic, strong) CAAnimationGroup *animationGroup;

@property (strong) CAKeyframeAnimation *keyFrameAnimation;

@property (copy) QNWaveLayerOverBlock block;

@end

@implementation QNWaveLayer

- (id)init {
    self = [super init];
    if (self) {
        
        self.contentsScale = [UIScreen mainScreen].scale;
        
        self.animationDuration = 0.25f;
        self.backgroundColor = [UIColor clearColor].CGColor;
    }
    return self;
}

- (void)startAnimation:(QNWaveLayerOverBlock)block {
    
    self.block = block;
    self.backgroundColor = [UIColor colorWithRed:0xFF/255.0f green:0x00/255.0f blue:0x00/255.0f alpha:1].CGColor;
    [self setupAnimationGroup];
    [self addAnimation:self.keyFrameAnimation forKey:@"pulse"];
}

- (void)stopAnimation {
    
    [self removeAllAnimations];
    self.animationGroup = nil;
}

- (void)reset {
    
    self.backgroundColor = [UIColor clearColor].CGColor;
}

- (void)setRadius:(CGFloat)radius {
    
    _radius = radius;
    
    CGPoint tempPos = self.position;
    
    CGFloat diameter = self.radius * 2;
    
    self.bounds = CGRectMake(0, 0, diameter, diameter);
    self.cornerRadius = self.radius;
    self.position = tempPos;
}

- (void)setupAnimationGroup {
    
    CAKeyframeAnimation *scaleAnimation = [CAKeyframeAnimation animationWithKeyPath:@"transform.scale.xy"];
    scaleAnimation.values = @[@0, @0.3, @0.8, @1];
    scaleAnimation.keyTimes = @[@0, @0.5, @0.9, @1];
    scaleAnimation.duration = self.animationDuration;
    scaleAnimation.delegate = self;
    
    self.keyFrameAnimation = scaleAnimation;
}

#pragma mark CAAnimationDelegate
- (void)animationDidStop:(CAAnimation *)anim finished:(BOOL)flag {
    
    [self stopAnimation];
    if (self.block) {
        
        self.block();
    }
}

@end
