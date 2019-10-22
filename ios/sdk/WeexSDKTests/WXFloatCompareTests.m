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
#import <WeexSDK/WeexSDK.h>

@interface WXFloatCompareTests : XCTestCase

@end

@implementation WXFloatCompareTests

- (void)setUp {
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void)testWXFloatEqual {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
    float a = 0.1;
    double b = 0.1;
    BOOL boolval = WXFloatEqual(a, b);
    XCTAssertTrue(boolval);
}

- (void)testWXFloatEqualWithPrecision {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
    float a = 0.1;
    double b = 0.1;
    BOOL boolval = WXFloatEqualWithPrecision(a, b , 0.01);
    XCTAssertTrue(boolval);
}

- (void)testWXFloatLessThan {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
    float a = 0.1;
    double b = 0.2;
    BOOL boolval = WXFloatLessThan(a, b);
    XCTAssertTrue(boolval);
}

- (void)testWXFloatLessThanWithPrecision {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
    float a = 0.1;
    double b = 0.2;
    BOOL boolval = WXFloatLessThanWithPrecision(a, b, 0.01);
    XCTAssertTrue(boolval);
}

- (void)testWXFloatGreaterThan {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
    float a = 0.2;
    double b = 0.1;
    BOOL boolval = WXFloatGreaterThan(a, b);
    XCTAssertTrue(boolval);
}

- (void)testWXFloatGreaterThanWithPrecision {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
    float a = 0.2;
    double b = 0.1;
    BOOL boolval = WXFloatGreaterThanWithPrecision(a, b,0.01);
    XCTAssertTrue(boolval);
}

@end
