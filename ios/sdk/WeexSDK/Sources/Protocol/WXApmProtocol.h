
#import <Foundation/Foundation.h>


@protocol WXApmProtocol <NSObject>

/**
 * start record
 *
 * @param instanceId instanceId
 */
@required
- (void) onStart:(NSString*)instanceId topic:(NSString*)topic;

/**
 * pause apm record (apm ext mem / fps)
 */
@required
- (void) pauseApmRecord;

/**
 * resume apm record (apm ext mem / fps)
 */
@required
- (void) resumeApmRecord;

/**
 * end record
 */
@required
- (void) onEnd;

/**
 * record event
 */
@required
- (void) onEvent:(NSString *)name withValue:(id)value;   

/**
 * record stage
 */
@required
- (void) onStage:(NSString *)name withValue:(long)timestamp;

/**
 * record property
 */
@required
- (void) addProperty:(NSString *)name withValue:(id)value;

/**
 * record statistic
 */
@required
- (void) addStatistic:(NSString *)name withValue:(double)value;

@required
- (void) onSubProcedureBegin:(NSString*)subProcedureName;

@required
- (void) onSubProcedureEndSucceed:(NSString*)subProcedureName;

@required
- (void) onSubProcedureEndFailed:(NSString*)subProcedureName;

/**
 * record biz properties
 */
@required
- (void) addBiz:(NSString *)bizID withValue:(NSDictionary *)properties;

/**
 * record biz stage
 */
@required
- (void) addBizStage:(NSString *)bizID withValue:(NSDictionary *)stage;

@end

@protocol WXApmGeneratorProtocol <NSObject>

@required
- (id<WXApmProtocol>)gengratorApmInstance:(NSString *) type;
@end
