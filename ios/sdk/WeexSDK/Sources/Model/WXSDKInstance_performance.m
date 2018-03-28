
#import <Foundation/Foundation.h>
#import "WXSDKInstance_performance.h"
#import "WXMonitor.h"

@implementation WXPerformance

@end

@implementation WXSDKInstance (Performance)

- (void) updatePerDicAfterCreateFinish
{
    [WXMonitor performanceFinishWithState:DebugAfterFSFinish instance:self];
    self.performance.jsCreateFinishTime =  CACurrentMediaTime()*1000;
    self.isJSCreateFinish = TRUE;
    WX_MONITOR_PERF_SET(WXPTFsCallJsNum, self.performance.fsCallJsNum, self);
    WX_MONITOR_PERF_SET(WXPTFsCallJsTime, self.performance.fsCallJsTime, self);
    WX_MONITOR_PERF_SET(WXPTFsCallNativeNum, self.performance.fsCallNativeNum, self);
    WX_MONITOR_PERF_SET(WXPTFsCallNativeTime, self.performance.fsCallNativeTime, self);
    WX_MONITOR_PERF_SET(WXPTFsReqNetNum, self.performance.fsReqNetNum, self);
    WX_MONITOR_PERF_SET(WXPTTimerNum, self.performance.timerNum, self);
}


- (void)updatePerDicBeforExit
{
    WX_MONITOR_PERF_SET(WXPTCellExceedNum,self.performance.cellExceedNum,self);
    WX_MONITOR_PERF_SET(WXPTMaxDeepVDom,self.performance.maxVdomDeep,self);
    WX_MONITOR_PERF_SET(WXPTImgWrongSizeNum,self.performance.imgWrongSizeNum,self);
    WX_MONITOR_PERF_SET(WXPTImgWrongSizeNum,self.performance.imgWrongSizeNum,self);
    WX_MONITOR_PERF_SET(WXPTInteractionTime,self.performance.interactionTime,self);
}

@end
