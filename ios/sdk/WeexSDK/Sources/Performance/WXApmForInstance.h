#import <Foundation/Foundation.h>

#pragma mark - const static string

extern NSString* const WEEX_PAGE_TOPIC;

/************** properties *****************/
extern NSString* const KEY_PROPERTIES_ERROR_CODE;
extern NSString* const KEY_PAGE_PROPERTIES_LAUNCH_ID;
extern NSString* const KEY_PAGE_PROPERTIES_BIZ_ID;
extern NSString* const KEY_PAGE_PROPERTIES_JSLIB_VERSION;
extern NSString* const KEY_PAGE_PROPERTIES_WEEX_VERSION;
extern NSString* const KEY_PAGE_PROPERTIES_REQUEST_TYPE;
extern NSString* const KEY_PAGE_PROPERTIES_REQUEST_TYPE;
extern NSString* const KEY_PAGE_PROPERTIES_NET_TYPE;
extern NSString* const KEY_PAGE_PROPERTIES_CACHE_TYPE;
extern NSString* const KEY_PAGE_PROPERTIES_USE_MULTI_CONTEXT;
extern NSString* const KEY_PAGE_PROPERTIES_BUNDLE_TYPE;

///************** stages *****************/
extern NSString* const KEY_PAGE_STAGES_START;
extern NSString* const KEY_PAGE_STAGES_DOWN_BUNDLE_START;
extern NSString* const KEY_PAGE_STAGES_DOWN_BUNDLE_END;
extern NSString* const KEY_PAGE_STAGES_RENDER_ORGIGIN;
extern NSString* const KEY_PAGE_STAGES_LOAD_BUNDLE_START;
extern NSString* const KEY_PAGE_STAGES_LOAD_BUNDLE_END;
extern NSString* const KEY_PAGE_STAGES_FSRENDER;
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

///************** value *****************/
extern NSString* const VALUE_ERROR_CODE_DEFAULT;



@interface WXApmForInstance : NSObject

@property (nonatomic, assign) bool isFSEnd;

#pragma mark - basic method

- (void) onEvent:(NSString *)name withValue:(id)value;
- (void) onStage:(NSString *)name;
- (void) setProperty:(NSString *)name withValue:(id)value;
- (void) setStatistic:(NSString *)name withValue:(double)value;

#pragma mark - instance record 

- (void) startRecord:(NSString*) instanceId;
- (void) endRecord;
- (void) arriveFSRenderTime;
- (void) updateFSDiffStats:(NSString *)name withDiffValue:(double)diff;
- (void) updateDiffStats:(NSString *)name withDiffValue:(double)diff;
- (void) updateMaxStats:(NSString *)name curMaxValue:(double)maxValue;
- (void) updateExtInfo:(NSDictionary*) extInfo;


#pragma mark - called by IWXHttpAdapter implementer

- (void) actionNetRequest;
- (void) actionNetRequestResult:(bool)succeed withErrorCode:(NSString*)errorCode;

#pragma mark - called by IWXImgLoaderAdapter implementer
- (void) actionImgLoad;
- (void) actionImgLoadResult:(bool)succeed withErrorCode:(NSString*)errorCode;

@end
