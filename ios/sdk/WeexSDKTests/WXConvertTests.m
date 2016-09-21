//
//  WXConvertTests.m
//  WeexSDK
//
//  Created by Keen Zhi on 16/9/20.
//  Copyright © 2016年 taobao. All rights reserved.
//

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
    XCTAssertTrue(9==val);
    
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

- (void) testArray{
    NSMutableArray * array = [[NSMutableArray alloc]initWithCapacity:1];
    [array addObject:@"x"];
    NSUInteger val = [WXConvert NSUInteger:array];
    XCTAssertTrue(0==val);//test array
    
    NSMutableArray *array1 = [[NSMutableArray alloc]initWithCapacity:1];
    val = [WXConvert NSUInteger:array1];
    XCTAssertTrue(0==val);//test array
}

- (void) testDict{
    NSMutableDictionary * dict = [[NSMutableDictionary alloc]initWithCapacity:1];
    [dict setObject:@"sdfasd" forKey:@"3"];
    NSUInteger val = [WXConvert NSUInteger:dict];
    XCTAssertTrue(0==val);//test array
    
}

- (void)testPerformanceExample {
    // This is an example of a performance test case.
    [self measureBlock:^{
        // Put the code you want to measure the time of here.
    }];
}

@end
