//
//  WXRootViewTests.m
//  WeexSDK
//
//  Created by yinfeng on 2016/10/29.
//  Copyright © 2016年 taobao. All rights reserved.
//

#import <XCTest/XCTest.h>
#import "WXSDKInstance.h"
#import "TestSupportUtils.h"
#import "WXSDKEngine.h"
#import "WXLog.h"
#import "WXUtility.h"

@interface WXRootViewTests : XCTestCase

@property (nonatomic, strong) NSString *testScript;

@end

@implementation WXRootViewTests

- (void)setUp {
    [super setUp];
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void)testInstanceAndWeexRootFrame {
    CGRect instanceFrame = CGRectMake(1, 2, 345, 678);
    CGRect templateRootFrame = CGRectMake(8, 7, 654, 321);
    NSDictionary *templateRootFrameData = @{@"left":@(templateRootFrame.origin.x),
                                            @"top":@(templateRootFrame.origin.y),
                                            @"width":@(templateRootFrame.size.width),
                                            @"height":@(templateRootFrame.size.height)};
    
    NSBundle *bundle = [NSBundle bundleForClass:[self class]];
    NSString *path = [bundle pathForResource:@"main" ofType:@"js"];
    NSString *script = [NSString stringWithContentsOfFile:path encoding:NSUTF8StringEncoding error:nil];
    [WXSDKEngine initSDKEnvironment:script];
    [WXLog setLogLevel:WXLogLevelDebug];
    
    NSString *jsPath = [bundle pathForResource:@"testRootView" ofType:@"js"];
    NSString *jsScript = [NSString stringWithContentsOfFile:jsPath encoding:NSUTF8StringEncoding error:nil];
    
    // 1.instance set frame, weex template root not set frame, root container will use instance'frame, weex template root view will use instance's width and height
    WXSDKInstance *instance1 = [[WXSDKInstance alloc] init];
    instance1.frame = instanceFrame;
    [instance1 renderView:jsScript options:nil data:nil];
    XCTestExpectation *expectation1 = [self expectationWithDescription:@"instance 1"];
    instance1.renderFinish = ^(UIView *view){
        XCTAssert(WXRectApproximateToRect(view.frame, instanceFrame));
        XCTAssert(WXRectApproximateToRect(view.subviews[0].frame, CGRectMake(0, 0, instanceFrame.size.width, instanceFrame.size.height)));
        [expectation1 fulfill];
    };
    
    
    // 2.instance set frame, weex template root set frame, root container will still use instance'frame, weex template root view will use its own frame.
    WXSDKInstance *instance2 = [[WXSDKInstance alloc] init];
    instance2.frame = instanceFrame;
    [instance2 renderView:jsScript options:nil data:templateRootFrameData];
    XCTestExpectation *expectation2 = [self expectationWithDescription:@"instance 2"];
    CGFloat pixelScaleFactor = instance2.pixelScaleFactor;
    instance2.renderFinish = ^(UIView *view){
        XCTAssert(WXRectApproximateToRect(view.frame, instanceFrame));
        XCTAssert(WXRectApproximateToRect(view.subviews[0].frame,
                                    CGRectMake(
                                               WXPixelScale(templateRootFrame.origin.x, pixelScaleFactor),
                                               WXPixelScale(templateRootFrame.origin.y, pixelScaleFactor),
                                               WXPixelScale(templateRootFrame.size.width, pixelScaleFactor),
                                               WXPixelScale(templateRootFrame.size.height, pixelScaleFactor))));
    
        [expectation2 fulfill];
    };
    
    
    // 3.instance not set frame, weex template root set frame, root container will use the width and height computed by layout, weex template root view will use its own frame.
    WXSDKInstance *instance3 = [[WXSDKInstance alloc] init];
    [instance3 renderView:jsScript options:nil data:templateRootFrameData];
    XCTestExpectation *expectation3 = [self expectationWithDescription:@"instance 3"];
    XCTestExpectation *expectation31 = [self expectationWithDescription:@"instance 3 onLayoutChange"];
    __weak typeof(instance2) weakInstance = instance3;
    instance3.renderFinish = ^(UIView *view){
        XCTAssert(WXRectApproximateToRect(view.frame,
                                    CGRectMake(0,0,
                                               WXPixelScale(templateRootFrame.size.width, weakInstance.pixelScaleFactor),
                                               WXPixelScale(templateRootFrame.size.height, weakInstance.pixelScaleFactor))));
        XCTAssert(WXRectApproximateToRect(view.subviews[0].frame,
                                    CGRectMake(
                                               WXPixelScale(templateRootFrame.origin.x, weakInstance.pixelScaleFactor),
                                               WXPixelScale(templateRootFrame.origin.y, weakInstance.pixelScaleFactor),
                                               WXPixelScale(templateRootFrame.size.width, weakInstance.pixelScaleFactor),
                                               WXPixelScale(templateRootFrame.size.height, weakInstance.pixelScaleFactor))));
        [expectation3 fulfill];
        
        // 31.if weex template root frame changed, root container will change the width and height computed by layout, thus trigger instance's onLayoutChange;
        NSMutableDictionary *changedFrameData = [templateRootFrameData mutableCopy];
        [changedFrameData setObject:@(400) forKey:@"height"];
        
        [weakInstance refreshInstance:changedFrameData];
        weakInstance.onLayoutChange = ^(UIView *view) {
            XCTAssert(WXRectApproximateToRect(view.frame,
                                        CGRectMake(0,0,
                                                   WXPixelScale(templateRootFrame.size.width, weakInstance.pixelScaleFactor),
                                                   WXPixelScale(400, weakInstance.pixelScaleFactor))));
            [expectation31 fulfill];
        };
        
    };
    
    // 4.instance not set frame, weex template root not set frame, root container will use the width and height computed by layout, weex template root view will also use width and height computed by layout.
    WXSDKInstance *instance4 = [[WXSDKInstance alloc] init];
    [instance4 renderView:jsScript options:nil data:nil];
    XCTestExpectation *expectation4 = [self expectationWithDescription:@"instance 4"];
    pixelScaleFactor = instance4.pixelScaleFactor;
    instance4.renderFinish = ^(UIView *view){
        XCTAssert(WXRectApproximateToRect(view.frame,
                                    CGRectMake(0,0,WXPixelScale(100, pixelScaleFactor),WXPixelScale(200, pixelScaleFactor))));
        XCTAssert(WXRectApproximateToRect(view.subviews[0].frame,
                                    CGRectMake(0,0,WXPixelScale(100, pixelScaleFactor),WXPixelScale(200, pixelScaleFactor))));
        [expectation4 fulfill];
    };
    
    [self waitForExpectationsWithTimeout:10.0 handler:^(NSError *error) {
        if (error) {
            NSLog(@"Timeout Error: %@", error);
        }
    }];
}


@end
