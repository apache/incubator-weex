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
#import "WXJSExceptionInfo.h"

#pragma mark - const static string

extern NSString* const WEEX_PAGE_TOPIC;

/************** properties *****************/
extern NSString* const KEY_PROPERTIES_ERROR_CODE;
extern NSString* const KEY_PAGE_PROPERTIES_BIZ_ID;
extern NSString* const KEY_PAGE_PROPERTIES_BUBDLE_URL;
extern NSString* const KEY_PAGE_PROPERTIES_JSLIB_VERSION;
extern NSString* const KEY_PAGE_PROPERTIES_WEEX_VERSION;
extern NSString* const KEY_PAGE_PROPERTIES_REQUEST_TYPE;
extern NSString* const KEY_PAGE_PROPERTIES_Z_CACHE_INFO;
extern NSString* const KEY_PAGE_PROPERTIES_JS_FM_INIT;
extern NSString* const KEY_PAGE_PROPERTIES_BUNDLE_TYPE;
extern NSString* const KEY_PAGE_PROPERTIES_CONTAINER_NAME;
extern NSString* const KEY_PAGE_PROPERTIES_INSTANCE_TYPE;
extern NSString* const KEY_PAGE_PROPERTIES_PARENT_PAGE;
extern NSString* const KEY_PAGE_PROPERTIES_RENDER_TYPE;


///************** stages *****************/
extern NSString* const KEY_PAGE_STAGES_START;
extern NSString* const KEY_PAGE_STAGES_DOWN_BUNDLE_START;
extern NSString* const KEY_PAGE_STAGES_DOWN_BUNDLE_END;
extern NSString* const KEY_PAGE_STAGES_CUSTOM_PREPROCESS_START;
extern NSString* const KEY_PAGE_STAGES_CUSTOM_PREPROCESS_END;
extern NSString* const KEY_PAGE_STAGES_RENDER_ORGIGIN;
extern NSString* const KEY_PAGE_STAGES_LOAD_BUNDLE_START;
extern NSString* const KEY_PAGE_STAGES_LOAD_BUNDLE_END;
extern NSString* const KEY_PAGE_STAGES_CREATE_FINISH;
extern NSString* const KEY_PAGE_STAGES_FSRENDER;
extern NSString* const KEY_PAGE_STAGES_NEW_FSRENDER;
extern NSString* const KEY_PAGE_STAGES_INTERACTION;
extern NSString* const KEY_PAGE_STAGES_DESTROY;

///************** stats *****************/
extern NSString* const KEY_PAGE_STATS_BUNDLE_SIZE;
extern NSString* const KEY_PAGE_STATS_FS_CALL_JS_TIME;
extern NSString* const KEY_PAGE_STATS_FS_CALL_JS_NUM;
extern NSString* const KEY_PAGE_STATS_FS_TIMER_NUM;
extern NSString* const KEY_PAGE_STATS_FS_CALL_NATIVE_TIME;
extern NSString* const KEY_PAGE_STATS_FS_CALL_NATIVE_NUM;
extern NSString* const KEY_PAGE_STATS_FS_CALL_EVENT_NUM;
extern NSString* const KEY_PAGE_STATS_FS_REQUEST_NUM;

extern NSString* const KEY_PAGE_STATS_SCROLLER_NUM;
extern NSString* const KEY_PAGE_STATS_CELL_EXCEED_NUM;
extern NSString* const KEY_PAGE_STATS_CELL_UN_RE_USE_NUM;
extern NSString* const KEY_PAGE_STATS_CELL_DATA_UN_RECYCLE_NUM;
extern NSString* const KEY_PAGE_STATS_EMBED_COUNT;
extern NSString* const KEY_PAGE_STATS_LARGE_IMG_COUNT;

extern NSString* const KEY_PAGE_STATS_MAX_DEEP_VIEW;
extern NSString* const KEY_PAGE_STATS_MAX_DEEP_DOM;
extern NSString* const KEY_PAGE_STATS_MAX_COMPONENT_NUM;
extern NSString* const KEY_PAGE_STATS_WRONG_IMG_SIZE_COUNT;
extern NSString* const KEY_PAGE_STATS_IMG_UN_RECYCLE_NUM;

extern NSString* const KEY_PAGE_STATS_I_SCREEN_VIEW_COUNT;
extern NSString* const KEY_PAGE_STATS_I_ALL_VIEW_COUNT;

extern NSString* const KEY_PAGE_STATS_I_COMPONENT_CREATE_COUNT;
extern NSString* const KEY_PAGE_ANIM_BACK_NUM;
extern NSString* const KEY_PAGE_TIMER_BACK_NUM;
extern NSString* const KEY_PAGE_STATS_ACTUAL_DOWNLOAD_TIME;

extern NSString* const KEY_PAGE_STATS_IMG_LOAD_NUM;
extern NSString* const KEY_PAGE_STATS_IMG_LOAD_SUCCESS_NUM;
extern NSString* const KEY_PAGE_STATS_IMG_LOAD_FAIL_NUM;
extern NSString* const KEY_PAGE_STATS_NET_NUM;
extern NSString* const KEY_PAGE_STATS_NET_SUCCESS_NUM;
extern NSString* const KEY_PAGE_STATS_NET_FAIL_NUM;
extern NSString* const KEY_PAGE_STAGES_FIRST_INTERACTION_VIEW;
extern NSString* const KEY_PAGE_STATS_BODY_RATIO;

///************** value *****************/
extern NSString* const VALUE_ERROR_CODE_DEFAULT;



@interface WXApmForInstance : NSObject

@property (nonatomic, assign) BOOL isFSEnd;
@property (nonatomic,assign)  BOOL isOpenApm;
@property (nonatomic, assign) BOOL isStartRecord;
@property (nonatomic, assign) BOOL hasAddView;
@property (nonatomic, assign) BOOL isDegrade;
@property (nonatomic, assign) BOOL isStartRender;
@property (nonatomic,assign)  BOOL  hasRecordFirstInterationView;
@property (nonatomic, assign) BOOL isDownLoadFailed;
@property (nonatomic,assign) double pageRatio;

#pragma mark - basic method

- (void) onEvent:(NSString *)name withValue:(id)value;
- (void) onStage:(NSString *)name;
- (void) onStageWithTime:(NSString*)name time:(long)unixTime;
- (void) setProperty:(NSString *)name withValue:(id)value;
- (void) setStatistic:(NSString *)name withValue:(double)value;

#pragma mark - instance record 

- (void) startRecord:(NSString*) instanceId;
- (void) endRecord;
- (void) updateFSDiffStats:(NSString *)name withDiffValue:(double)diff;
- (void) updateDiffStats:(NSString *)name withDiffValue:(double)diff;
- (void) updateMaxStats:(NSString *)name curMaxValue:(double)maxValue;
- (void) updateExtInfoFromResponseHeader:(NSDictionary*) extInfo;
- (void) forceSetInteractionTime:(long) unixTime;


#pragma mark - called by IWXHttpAdapter implementer

- (void) actionNetRequest;
- (void) actionNetRequestResult:(bool)succeed withErrorCode:(NSString*)errorCode;

#pragma mark - called by IWXImgLoaderAdapter implementer
- (void) actionImgLoad;
- (void) actionImgLoadResult:(bool)succeed withErrorCode:(NSString*)errorCode;

#pragma mark record top5 errorMsg
- (void) recordErrorMsg:(WXJSExceptionInfo *)exception;
- (NSDictionary<NSString*,NSNumber*>*) stageDic;

#pragma mark templateinfo
- (NSString*) templateInfo;
@end
