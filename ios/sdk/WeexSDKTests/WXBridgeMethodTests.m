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
#import "WXCallJSMethod.h"
#import "WXSDKInstance.h"

@interface WXBridgeMethodTests : XCTestCase

@end

@implementation WXBridgeMethodTests

- (void)setUp {
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void)testExample {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
    WXSDKInstance *instance = [[WXSDKInstance alloc] init];
    
    WXCallJSMethod *method = [[WXCallJSMethod alloc] initWithModuleName:@"dom" methodName:@"test" arguments:@[@"1", @"2", @"3"] instance:instance];
    
    NSDictionary *task = [method callJSTask];
    XCTAssertEqualObjects(task[@"module"], @"dom");
    XCTAssertEqualObjects(task[@"method"], @"test");
    
    NSArray *args = task[@"args"];
    XCTAssertTrue(args.count == 3);
    XCTAssertEqualObjects(args[0], @"1");
    XCTAssertEqualObjects(args[1], @"2");
    XCTAssertEqualObjects(args[2], @"3");
    
    WXCallJSMethod *method2 = [[WXCallJSMethod alloc] initWithModuleName:nil methodName:nil arguments:nil instance:[[WXSDKInstance alloc] init]];
    
    task = [method2 callJSTask];
    XCTAssertEqualObjects(task[@"module"], @"");
    XCTAssertEqualObjects(task[@"method"], @"");
    
    args = task[@"args"];
    XCTAssertNotNil(args);
    XCTAssertTrue(args.count == 0);
}

@end
