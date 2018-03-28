
#import <Foundation/Foundation.h>
#import "WXSDKInstance.h"
#import "WXMonitor.h"
#import "WXAnalyzerCenter.h"

@interface WXAnalyzerCenter (Transfer)

+(void) transDataOnState:(CommitState) timeState withInstaneId:(NSString *)instanceId data:(NSDictionary *)data ;

+ (void) transErrorInfo:(WXJSExceptionInfo *)errorInfo;

+ (BOOL) needTransfer;

@end
