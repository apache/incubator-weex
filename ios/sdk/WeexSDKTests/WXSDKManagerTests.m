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

@interface WXSDKManagerTests : XCTestCase

@end

@implementation WXSDKManagerTests

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
}

- (void)testWXSDKManager {
    id bridgeMgr = [WXSDKManager bridgeMgr];
    XCTAssertNotNil(bridgeMgr);
    XCTAssertTrue([bridgeMgr isKindOfClass:NSClassFromString(@"WXBridgeManager")]);
    
    [WXSDKManager storeInstance:[WXSDKInstance new] forID:@"0"];
    WXSDKInstance *instance0 = [WXSDKManager instanceForID:@"0"];
    XCTAssertNotNil(instance0);
    
    [WXSDKManager storeInstance:[WXSDKInstance new] forID:@"1"];
    WXSDKInstance *instance1 = [WXSDKManager instanceForID:@"1"];
    XCTAssertNotNil(instance1);
    
    [WXSDKManager removeInstanceforID:@"0"];
    instance0 = [WXSDKManager instanceForID:@"0"];
    XCTAssertNil(instance0);
    
    [WXSDKManager unload];
   // [TestSupportUtils waitSecs:5];
    instance1 = [WXSDKManager instanceForID:@"1"];
    //XCTAssertNil(instance1);
}

- (void)testUnload {
    
}

@end
