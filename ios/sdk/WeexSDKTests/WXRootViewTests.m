//
//  WXRootViewTests.m
//  WeexSDK
//
//  Created by yinfeng on 2016/10/29.
//  Copyright © 2016年 taobao. All rights reserved.
//

#import <XCTest/XCTest.h>
#import "WXSDKInstance.h"




@interface WXRootViewTests : XCTestCase

@property (nonatomic, strong) NSString *testScript;

@end

@implementation WXRootViewTests

- (void)setUp {
    [super setUp];
    
    /**
     <template>
        <div style='width:{{width}};height:{{height}}'>
            <div style='width:100;height:200'>
            </div>
        </div>
     </template>
     */
    _testScript = @"";
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void)testInstanceSetFrame {
    WXSDKInstance *instance = [[WXSDKInstance alloc] init];
    instance.frame = CGRectMake(1, 2, 345, 678);
    [instance renderView:_testScript options:nil data:nil];
}

- (void)testInstanceNotSetFrame {
    
}

- (void)testInstanceChangeFrame {
    
}


@end
