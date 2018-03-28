#import <Foundation/Foundation.h>
#import "WXAppMonitorProtocol.h"

#define GROUP_ANALYZER          @"WXAnalyzer"
#define MODULE_PERFORMANCE      @"WXPerformance"
#define MODULE_ERROR            @"WXError"
#define TYPE_MEASURE_REAL       @"measure_real_time"
#define TYPE_DIMEN_REAL         @"dimen_real_time"
#define  TYPE_JS_ERROR          @"js"

@protocol WXAnalyzerProtocol <NSObject>


@required
/**
 @param value  = @{
                    @"group":group,
                    @"module":module,
                    @"type":type,
                    @"data":jsonData
                };
 */
- (void)transfer:(NSDictionary *) value;

@end
