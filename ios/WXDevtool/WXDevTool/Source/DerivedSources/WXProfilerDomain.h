//
//  WXProfilerDomain.h
//  PonyDebuggerDerivedSources
//
//  Generated on 8/23/12
//
//  Licensed to Square, Inc. under one or more contributor license agreements.
//  See the LICENSE file distributed with this work for the terms under
//  which Square, Inc. licenses this file to you.
//

#import "WXObject.h"
#import "WXDebugger.h"
#import "WXDynamicDebuggerDomain.h"

@class WXRuntimeRemoteObject;
@class WXProfilerProfile;
@class WXProfilerProfileHeader;

@protocol WXProfilerCommandDelegate;

@interface WXProfilerDomain : WXDynamicDebuggerDomain 

@property (nonatomic, assign) id <WXProfilerCommandDelegate, WXCommandDelegate> delegate;

// Events
- (void)addProfileHeaderWithHeader:(WXProfilerProfileHeader *)header;
- (void)addHeapSnapshotChunkWithUid:(NSNumber *)uid chunk:(NSString *)chunk;
- (void)finishHeapSnapshotWithUid:(NSNumber *)uid;
- (void)setRecordingProfileWithIsProfiling:(NSNumber *)isProfiling;
- (void)resetProfiles;
- (void)reportHeapSnapshotProgressWithDone:(NSNumber *)done total:(NSNumber *)total;

@end

@protocol WXProfilerCommandDelegate <WXCommandDelegate>
@optional
- (void)domain:(WXProfilerDomain *)domain causesRecompilationWithCallback:(void (^)(NSNumber *result, id error))callback;
- (void)domain:(WXProfilerDomain *)domain isSamplingWithCallback:(void (^)(NSNumber *result, id error))callback;
- (void)domain:(WXProfilerDomain *)domain hasHeapProfilerWithCallback:(void (^)(NSNumber *result, id error))callback;
- (void)domain:(WXProfilerDomain *)domain enableWithCallback:(void (^)(id error))callback;
- (void)domain:(WXProfilerDomain *)domain disableWithCallback:(void (^)(id error))callback;
- (void)domain:(WXProfilerDomain *)domain startWithCallback:(void (^)(id error))callback;
- (void)domain:(WXProfilerDomain *)domain stopWithCallback:(void (^)(id error))callback;
- (void)domain:(WXProfilerDomain *)domain getProfileHeadersWithCallback:(void (^)(NSArray *headers, id error))callback;
- (void)domain:(WXProfilerDomain *)domain getProfileWithType:(NSString *)type uid:(NSNumber *)uid callback:(void (^)(WXProfilerProfile *profile, id error))callback;
- (void)domain:(WXProfilerDomain *)domain removeProfileWithType:(NSString *)type uid:(NSNumber *)uid callback:(void (^)(id error))callback;
- (void)domain:(WXProfilerDomain *)domain clearProfilesWithCallback:(void (^)(id error))callback;
- (void)domain:(WXProfilerDomain *)domain takeHeapSnapshotWithCallback:(void (^)(id error))callback;
- (void)domain:(WXProfilerDomain *)domain collectGarbageWithCallback:(void (^)(id error))callback;
// Param objectGroup: Symbolic group name that can be used to release multiple objects.
// Callback Param result: Evaluation result.
- (void)domain:(WXProfilerDomain *)domain getObjectByHeapObjectIdWithObjectId:(NSString *)objectId objectGroup:(NSString *)objectGroup callback:(void (^)(WXRuntimeRemoteObject *result, id error))callback;
// Param objectId: Identifier of the object to get heap object id for.
// Callback Param heapSnapshotObjectId: Id of the heap snapshot object corresponding to the passed remote object id.
- (void)domain:(WXProfilerDomain *)domain getHeapObjectIdWithObjectId:(NSString *)objectId callback:(void (^)(NSString *heapSnapshotObjectId, id error))callback;

@end

@interface WXDebugger (WXProfilerDomain)

@property (nonatomic, readonly, strong) WXProfilerDomain *profilerDomain;

@end
