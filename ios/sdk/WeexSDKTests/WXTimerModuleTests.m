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
#import "WXTimerModule.h"
#import "TestSupportUtils.h"

@interface WXTimerModuleTests : XCTestCase

@property (nonatomic, strong)  WXTimerModule *timerModule;

@property (nonatomic, strong)  NSMutableDictionary *timers;

@property (nonatomic, strong)  XCTestExpectation *exp;

@end

@implementation WXTimerModuleTests

- (void)setUp {
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
    
    self.timerModule = [[WXTimerModule alloc] init];
    self.timers = [self.timerModule timers];
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void)testExample {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
}

- (void)testSetTimeout{
    
    self.exp = [self expectationWithDescription:@"Set Timeout Unit Test Error!"];
    
    [self.timerModule createTimerWithCallback:@"0" time:1000 target:self selector:@selector(trigger) shouldRepeat:YES];

    __weak typeof(self) weakSelf = self;
    [self waitForExpectationsWithTimeout:2 handler:^(NSError * _Nullable error) {
        if (error) {
            NSLog(@"Timeout Error: %@", error);
        }
        
        NSTimer *timer = weakSelf.timers[@"0"];
        XCTAssertNotNil(timer);
        XCTAssertEqual(timer.timeInterval, 1);
        XCTAssertTrue(timer.isValid);
    }];
}

- (void)testClearTimeout {
    
    NSTimer *timer = [NSTimer scheduledTimerWithTimeInterval:0.5 target:self selector:@selector(trigger) userInfo:nil repeats:YES];
    self.timers[@"1"] = timer;
    
    [self.timerModule clearTimeout:@"1"];
    
    [TestSupportUtils waitSecs:1];
    
    XCTAssert(self.timers[@"1"] == nil);
}

- (void)trigger {
    [self.exp fulfill];
}

@end
