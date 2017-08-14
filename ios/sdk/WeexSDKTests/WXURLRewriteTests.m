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
#import "WXURLRewriteProtocol.h"
#import "WXHandlerFactory.h"
#import "WXSDKInstance.h"
#import "WXSDKEngine.h"

@interface WXURLRewriteTests : XCTestCase

@property (nonatomic, strong) id<WXURLRewriteProtocol> rewriteHandler;
@property (nonatomic, strong) WXSDKInstance *instance;

@end

@implementation WXURLRewriteTests

- (void)setUp {
    [super setUp];
    [WXSDKEngine registerHandler:[[NSClassFromString(@"WXURLRewriteDefaultImpl") alloc] init] withProtocol:@protocol(WXURLRewriteProtocol)];
    _rewriteHandler = [WXHandlerFactory handlerForProtocol:@protocol(WXURLRewriteProtocol)];
    _instance = [[WXSDKInstance alloc] init];
    _instance.scriptURL = [NSURL URLWithString:@"https://www.weex.com/test/test.js"];
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
    _rewriteHandler = nil;
}

- (void)testHttpURL {
    NSString *testURL1 = @"http://www.weex2.net/test.jpg";
    NSURL *rewriteURL1 = [_rewriteHandler rewriteURL:testURL1 withResourceType:WXResourceTypeImage withInstance:_instance];
    XCTAssertEqualObjects(testURL1, rewriteURL1.absoluteString);
    
    NSString *testURL2 = @"https://www.weex3.org/test.ttf";
    NSURL *rewriteURL2 = [_rewriteHandler rewriteURL:testURL2 withResourceType:WXResourceTypeFont withInstance:_instance];
    XCTAssertEqualObjects(testURL2, rewriteURL2.absoluteString);
}

- (void)testNoSchemeURL {
    NSString *testURL = @"//www.weex.net/test.jpg";
    NSURL *rewriteURL = [_rewriteHandler rewriteURL:testURL withResourceType:WXResourceTypeImage withInstance:_instance];
    XCTAssertEqualObjects(@"https://www.weex.net/test.jpg", [rewriteURL absoluteString]);
}

- (void)testRelativeURL {
    NSString *testURL1 = @"./test.jpg";
    NSURL *rewriteURL1 = [_rewriteHandler rewriteURL:testURL1 withResourceType:WXResourceTypeImage withInstance:_instance];
    XCTAssertEqualObjects(@"https://www.weex.com/test/test.jpg", rewriteURL1.absoluteString);
    
    NSString *testURL2 = @"../test.jpg";
    NSURL *rewriteURL2 = [_rewriteHandler rewriteURL:testURL2 withResourceType:WXResourceTypeImage withInstance:_instance];
    XCTAssertEqualObjects(@"https://www.weex.com/test.jpg", rewriteURL2.absoluteString);
    
    NSString *testURL3 = @"/test.jpg";
    NSURL *rewriteURL3 = [_rewriteHandler rewriteURL:testURL3 withResourceType:WXResourceTypeImage withInstance:_instance];
    XCTAssertEqualObjects(@"https://www.weex.com/test.jpg", rewriteURL3.absoluteString);
    
    NSString *testURL4 = @"test.jpg";
    NSURL *rewriteURL4 = [_rewriteHandler rewriteURL:testURL4 withResourceType:WXResourceTypeImage withInstance:_instance];
    XCTAssertEqualObjects(@"https://www.weex.com/test/test.jpg", rewriteURL4.absoluteString);
}


@end
