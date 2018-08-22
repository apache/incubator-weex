
#import "WXApmImpl.h"
#import "WXUtility.h"
#import "WXComponentManager.h"

@interface WXApmImpl()
@property(nonatomic,strong) NSMutableDictionary<NSString*,NSNumber*>* stageMap;
@property(nonatomic,strong) NSMutableDictionary<NSString*,id>* propertyMap;
@property(nonatomic,strong) NSMutableDictionary<NSString*,NSNumber*>* statisticMap;
@property(nonatomic,strong) NSMutableDictionary<NSString*,id>* eventMap;

@end

@implementation WXApmImpl

- (instancetype)init
{
    self = [super init];
    if (self) {
        _stageMap = [[NSMutableDictionary alloc] init];
        _propertyMap = [[NSMutableDictionary alloc] init];
        _statisticMap = [[NSMutableDictionary alloc] init];
        _eventMap = [[NSMutableDictionary alloc] init];
    }
    return self;
}


- (void) onStart:(NSString *)instanceId topic:(NSString *)topic
{
    
}

- (void) onEnd
{
    __weak typeof(self) weakSelf = self;
    WXPerformBlockOnComponentThread(^{
        [weakSelf _printApmInfo];
    });
    
}

- (void) onEvent:(NSString *)name withValue:(id)value
{
    __weak typeof(self) weakSelf = self;
    WXPerformBlockOnComponentThread(^{
        [weakSelf.eventMap setObject:value forKey:name];
    });
}

- (void) onStage:(NSString *)name withValue:(long)timestamp
{
    __weak typeof(self) weakSelf = self;
    WXPerformBlockOnComponentThread(^{
         [weakSelf.stageMap setObject:[NSNumber numberWithLong:timestamp] forKey:name];
    });
}

- (void) addProperty:(NSString *)name withValue:(id)value
{
    __weak typeof(self) weakSelf = self;
    WXPerformBlockOnComponentThread(^{
         [weakSelf.propertyMap setObject:value forKey:name];
    });
}

- (void) addStatistic:(NSString *)name withValue:(double)value
{
    __weak typeof(self) weakSelf = self;
    WXPerformBlockOnComponentThread(^{
        [weakSelf.statisticMap setObject:[NSNumber numberWithDouble:value] forKey:name];
    });
}

- (void) addBiz:(NSString *)bizID withValue:(NSDictionary *)properties
{
    
}

- (void) addBizAbTest:(NSString *)bizID withValue:(NSDictionary *)abTest
{
    
}

- (void) addBizStage:(NSString *)bizID withValue:(NSDictionary *)stage
{
    
}

- (void) onSubProcedureBegin:(NSString *)subProcedureName
{
    
}

- (void) onSubProcedureEndFailed:(NSString *)subProcedureName
{
    
}

- (void) onSubProcedureEndSucceed:(NSString *)subProcedureName
{
    
}

- (void)pauseApmRecord {
    
}


- (void)resumeApmRecord {
    
}


- (void) _printApmInfo
{
    NSDictionary* InfoMap = @{
                              @"stage":self.stageMap,
                              @"property":self.propertyMap,
                              @"statistic":self.statisticMap,
                              @"event":self.eventMap
                              };
    NSString* jsonStr = [WXUtility JSONString:InfoMap];
    NSLog(@"wxApmForInstance :%@",jsonStr);
}

@end
