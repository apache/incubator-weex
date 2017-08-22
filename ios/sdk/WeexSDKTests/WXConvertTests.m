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
#import "WXConvert.h"

@interface WXConvertTests : XCTestCase

@end

@implementation WXConvertTests

- (void)setUp {
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void)testBOOL {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
    BOOL boolval = [WXConvert BOOL:@"-1"];
     XCTAssertTrue(boolval);
    boolval = [WXConvert BOOL:@"true"];
    XCTAssertTrue(boolval);
    
    boolval = [WXConvert BOOL:@"false"];
    XCTAssertTrue(!boolval);
    
}

- (void) testNSUInteger{
    NSUInteger val= [WXConvert NSUInteger:@"x"];
    XCTAssertTrue(0==val);
    
    val= [WXConvert NSUInteger:@"9"];
    XCTAssertTrue(9);
    
    //test max
    NSString * unsignedIntMax = [NSString stringWithFormat:@"%lu", NSUIntegerMax ];
    val= [WXConvert NSUInteger:unsignedIntMax];
    XCTAssertTrue(val==NSUIntegerMax);
    
    //test overflow
    unsigned long long uio = NSUIntegerMax;
    uio++;
    
    NSString * ulval  = [NSString stringWithFormat:@"%llu", uio ];
    val = [WXConvert NSUInteger:ulval];
    XCTAssertTrue(0==val);//overflowed

}

- (void) testHex2Color{
    
    UIColor *redColor = [UIColor redColor];
    NSString *redString = @"#ff0000";
    //hex2color
    UIColor *redTestColor = [WXConvert UIColor:redString];
    XCTAssertTrue(CGColorEqualToColor(redTestColor.CGColor, redColor.CGColor));
}

- (void) testColor2Hex{
    
    UIColor *redColor = [UIColor redColor];
    NSString *redString = @"#ff0000";
    //color2hex
    NSString *hexString = [WXConvert HexWithColor:redColor];
    XCTAssertTrue([redString isEqualToString:hexString]);
}

@end
