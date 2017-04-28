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
#import "WXInstanceWrap.h"
#import "WXSDKInstance.h"

@interface WXInstanceWrapTests : XCTestCase

@property (nonatomic, strong) WXInstanceWrap *instanceWrap;

@property (nonatomic, strong)  XCTestExpectation *exp;

@property (nonatomic, strong)  NSError *error;

@end

@implementation WXInstanceWrapTests

- (void)setUp {
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
    
    self.instanceWrap = [[WXInstanceWrap alloc] init];
    
    __weak typeof(self) weakSelf = self;
    WXSDKInstance *instance = [[WXSDKInstance alloc] init];
    instance.onFailed = ^(NSError *error) {
        weakSelf.error = error;
        [weakSelf.exp fulfill];
    };
    
    self.instanceWrap.weexInstance = instance;
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void)testExample {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
}

- (void)testErrorCodeInfo {
    
    self.exp = [self expectationWithDescription:@" Error!"];
    
    [self.instanceWrap error:11 code:101 info:@"101"];
    
    __weak typeof(self) weakSelf = self;
    [self waitForExpectationsWithTimeout:2 handler:^(NSError * _Nullable error) {
        if (error) {
            NSLog(@"Timeout Error: %@", error);
        }
        
        XCTAssert(weakSelf.error.code == 101);
        XCTAssertEqualObjects(weakSelf.error.domain , @"11");
        XCTAssertEqualObjects(weakSelf.error.userInfo[NSLocalizedDescriptionKey], @"101");
    }];
}

@end
