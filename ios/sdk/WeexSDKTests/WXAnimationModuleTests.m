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

#import <XCTest/XCTest.h>
#import "WXSDKInstance.h"
#import "WXComponent.h"
#import "WXConvert.h"
#import "WXComponent_internal.h"
#import "WXAnimationModule.h"
#import "TestSupportUtils.h"

@interface WXAnimationModuleTests : XCTestCase

@end

@implementation WXAnimationModuleTests

- (void)setUp {
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void)testExample {
    
}

- (void)testAnimationRotate {
    WXComponent *component = [self component];
    WXAnimationModule *object = [[WXAnimationModule alloc]init];
    [object animation:component args:@{@"duration":@500, @"timingFunction":@"ease-in-out", @"styles":@{@"transform":@"rotate(90deg)"}} callback:nil];
    [TestSupportUtils waitSecs:1];
    
    CATransform3D transformToVerify = CATransform3DMakeAffineTransform(CGAffineTransformRotate(CGAffineTransformIdentity, M_PI / 2));
    XCTAssert(WXTransform3DApproximateToTransform(component.layer.transform, transformToVerify));
}

- (void)testAnimationTranslate {
    WXComponent *component = [self component];
    WXAnimationModule *object = [[WXAnimationModule alloc]init];
    [object animation:component args:@{@"duration":@500, @"timingFunction":@"linear", @"styles":@{@"transform":@"translate(50%, 50%)"}} callback:nil];
    [TestSupportUtils waitSecs:1];
    
    CGRect frame = component.layer.frame;
    XCTAssert(frame.origin.x == 50.0f && frame.origin.y == 50.0f);
}

- (void)testAnimationScale {
    WXComponent *component = [self component];
    WXAnimationModule *object = [[WXAnimationModule alloc]init];
    [object animation:component args:@{@"duration":@500, @"timingFunction":@"linear", @"styles":@{@"transform":@"scale(2)"}} callback:nil];
    [TestSupportUtils waitSecs:1];
    
    CGRect frame = component.layer.frame;
    XCTAssert(frame.size.width == 200.0f && frame.size.height == 200.0f);
}

- (void)testAnimationOpacity {
    WXComponent *component = [self component];
    WXAnimationModule *object = [[WXAnimationModule alloc]init];
    [object animation:component args:@{@"duration":@500, @"timingFunction":@"linear", @"styles":@{@"backgroundColor":@"blue"}} callback:nil];
    [TestSupportUtils waitSecs:1];
    XCTAssert(component.layer.backgroundColor==[WXConvert CGColor:@"blue"]);
}

- (WXComponent *)component {
    WXComponent *component = [[WXComponent alloc] initWithRef:@"0" type:@"div" styles:@{} attributes:@{} events:@[] weexInstance:[[WXSDKInstance alloc] init]];
    XCTAssertNotNil(component.view, @"Component's view should be created");
    
    component.view.frame = CGRectMake(0.0f, 0.0f, 100.0f, 100.0f);
    component.view.backgroundColor = [UIColor redColor];
    
    return component;
}

@end
