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
#import "WXStreamModule.h"
#import "WeexSDK.h"
#import "WXResourceRequestHandlerDefaultImpl.h"

@interface WXStreamModuleTests : XCTestCase
@property (nonatomic, strong)  WXStreamModule *streamModule;
@property (nonatomic, strong)XCTestExpectation *exp;

@end

@implementation WXStreamModuleTests

- (void)setUp {
    [super setUp];
    _streamModule = [[WXStreamModule alloc] init];
    [WXSDKEngine registerHandler:[WXResourceRequestHandlerDefaultImpl new] withProtocol:@protocol(WXResourceRequestHandler)];
    _exp = [self expectationWithDescription:@"SendRequestSuccess Unit Test Error!"];
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void)fetch:(NSDictionary*)options {
    __block id callbackRet = nil, progressCallbackRet = nil;
    [_streamModule fetch:options callback:^(id result) {
        callbackRet = result;
    } progressCallback:^(id result, BOOL keepAlive) {
        progressCallbackRet = result;
        [_exp fulfill];
    }];
    
    [self waitForExpectationsWithTimeout:10 handler:^(NSError * error) {
        XCTAssertNotNil(callbackRet); // final result
    }];
}

@end
