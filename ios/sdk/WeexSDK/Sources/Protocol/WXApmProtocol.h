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
