#import "WXApmForInstance.h"
#import "WXApmProtocol.h"
#import "WXHandlerFactory.h"
#import "WXSDKManager.h"
#import "WXAppConfiguration.h"
#import "WXUtility.h"


#pragma mark - const static string

NSString* const WEEX_PAGE_TOPIC = @"weex_page";

/************** properties *****************/
NSString* const KEY_PROPERTIES_ERROR_CODE = @"wxErrorCode";
NSString* const KEY_PAGE_PROPERTIES_LAUNCH_ID = @"wxLaunchId";
NSString* const KEY_PAGE_PROPERTIES_BIZ_ID = @"wxBizID";
NSString* const KEY_PAGE_PROPERTIES_JSLIB_VERSION  = @"wxJSLibVersion";
NSString* const KEY_PAGE_PROPERTIES_WEEX_VERSION  = @"wxSDKVersion";
NSString* const KEY_PAGE_PROPERTIES_REQUEST_TYPE  = @"wxRequestType";
NSString* const KEY_PAGE_PROPERTIES_NET_TYPE  = @"wxNetType";
NSString* const KEY_PAGE_PROPERTIES_CACHE_TYPE  = @"wxCacheType";
NSString* const KEY_PAGE_PROPERTIES_USE_MULTI_CONTEXT  = @"wxUseMultiContext";
NSString* const KEY_PAGE_PROPERTIES_BUNDLE_TYPE = @"wxBundleType";

///************** stages *****************/
NSString* const KEY_PAGE_STAGES_START = @"wxRecordStart";
NSString* const KEY_PAGE_STAGES_DOWN_BUNDLE_START  = @"wxStartDownLoadBundle";
NSString* const KEY_PAGE_STAGES_DOWN_BUNDLE_END  = @"wxEndDownLoadBundle";
NSString* const KEY_PAGE_STAGES_RENDER_ORGIGIN  = @"wxRenderTimeOrigin";
NSString* const KEY_PAGE_STAGES_LOAD_BUNDLE_START  = @"wxStartLoadBundle";
NSString* const KEY_PAGE_STAGES_LOAD_BUNDLE_END  = @"wxEndLoadBundle";
NSString* const KEY_PAGE_STAGES_FSRENDER  = @"wxFsRender";
NSString* const KEY_PAGE_STAGES_INTERACTION  = @"wxInteraction";
NSString* const KEY_PAGE_STAGES_DESTROY  = @"wxDestroy";

///************** stats *****************/
NSString* const KEY_PAGE_STATS_BUNDLE_SIZE  = @"wxBundleSize";
NSString* const KEY_PAGE_STATS_FS_CALL_JS_TIME  = @"wxFSCallJsTotalTime";
NSString* const KEY_PAGE_STATS_FS_CALL_JS_NUM  = @"wxFSCallJsTotalNum";
NSString* const KEY_PAGE_STATS_FS_TIMER_NUM = @"wxFSTimerCount";
NSString* const KEY_PAGE_STATS_FS_CALL_NATIVE_TIME = @"wxFSCallNativeTotalTime";
NSString* const KEY_PAGE_STATS_FS_CALL_NATIVE_NUM = @"wxFSCallNativeTotalNum";
NSString* const KEY_PAGE_STATS_FS_CALL_EVENT_NUM = @"wxFSCallEventTotalNum";
NSString* const KEY_PAGE_STATS_FS_REQUEST_NUM = @"wxFSRequestNum";

NSString* const KEY_PAGE_STATS_SCROLLER_NUM = @"wxScrollerCount";
NSString* const KEY_PAGE_STATS_CELL_EXCEED_NUM = @"wxCellExceedNum";
NSString* const KEY_PAGE_STATS_CELL_UN_RE_USE_NUM = @"wxCellUnReUseCount";
NSString* const KEY_PAGE_STATS_CELL_DATA_UN_RECYCLE_NUM = @"wxCellDataUnRecycleCount";

NSString* const KEY_PAGE_STATS_MAX_DEEP_VIEW = @"wxMaxDeepViewLayer";
NSString* const KEY_PAGE_STATS_MAX_DEEP_DOM = @"wxMaxDeepVDomLayer";
NSString* const KEY_PAGE_STATS_MAX_COMPONENT_NUM = @"wxMaxComponentCount";
NSString* const KEY_PAGE_STATS_WRONG_IMG_SIZE_COUNT = @"wxWrongImgSizeCount";
NSString* const KEY_PAGE_STATS_IMG_UN_RECYCLE_NUM = @"wxImgUnRecycleCount";

NSString* const KEY_PAGE_STATS_I_SCREEN_VIEW_COUNT = @"wxInteractionScreenViewCount";
NSString* const KEY_PAGE_STATS_I_ALL_VIEW_COUNT = @"wxInteractionAllViewCount";

NSString* const KEY_PAGE_STATS_I_COMPONENT_CREATE_COUNT = @"wxInteractionComponentCreateCount";
NSString* const KEY_PAGE_ANIM_BACK_NUM = @"wxAnimationInBackCount";
NSString* const KEY_PAGE_TIMER_BACK_NUM = @"wxTimerInBackCount";
NSString* const KEY_PAGE_STATS_ACTUAL_DOWNLOAD_TIME = @"wxActualNetworkTime";

NSString* const KEY_PAGE_STATS_IMG_LOAD_NUM = @"wxImgLoadCount";
NSString* const KEY_PAGE_STATS_IMG_LOAD_SUCCESS_NUM = @"wxImgLoadSuccessCount";
NSString* const KEY_PAGE_STATS_IMG_LOAD_FAIL_NUM = @"wxImgLoadFailCount";
NSString* const KEY_PAGE_STATS_NET_NUM  = @"wxNetworkRequestCount";
NSString* const KEY_PAGE_STATS_NET_SUCCESS_NUM = @"wxNetworkRequestSuccessCount";
NSString* const KEY_PAGE_STATS_NET_FAIL_NUM = @"wxNetworkRequestFailCount";

///************** value *****************/
NSString* const VALUE_ERROR_CODE_DEFAULT = @"0";

@interface WXApmForInstance ()
@property (nonatomic,strong) id<WXApmProtocol> apmProtocolInstance;
@property (nonatomic,strong) NSString* instanceId;
@property (nonatomic,strong) NSMutableDictionary<NSString*,NSNumber*>* recordStatsMap;
@end

@implementation WXApmForInstance


- (instancetype) init
{
    self = [super init];
    if (self) {
        id<WXApmGeneratorProtocol> generater = [WXHandlerFactory handlerForProtocol:@protocol(WXApmGeneratorProtocol)];
        _apmProtocolInstance = [generater gengratorApmInstance:WEEX_PAGE_TOPIC];
        _recordStatsMap = [[NSMutableDictionary alloc] init];
    }
    return self;
}

- (void) onEvent:(NSString *)name withValue:(id)value
{
    if (nil == _apmProtocolInstance) {
        return;
    }
    [self.apmProtocolInstance onEvent:name withValue:value];
}

- (void) onStage:(NSString *)name
{
    if (nil == _apmProtocolInstance) {
        return;
    }
    [self.apmProtocolInstance onStage:name withValue:[WXUtility getUnixCurrentTimeMillis]];
}

- (void) setProperty:(NSString *)name withValue:(id)value
{
    if (nil == _apmProtocolInstance) {
        return;
    }
    [self.apmProtocolInstance addProperty:name withValue:value];
}

- (void) setStatistic:(NSString *)name withValue:(double)value
{
    if (nil == _apmProtocolInstance) {
        return;
    }
    [self.apmProtocolInstance addStatistic:name withValue:value];
}

#pragma mark - instance record

- (void) startRecord:(NSString*) instanceId
{
    if (nil == _apmProtocolInstance) {
        return;
    }
    _instanceId = instanceId;
    
    [self.apmProtocolInstance onStart:instanceId topic:WEEX_PAGE_TOPIC];
    [self onStage:KEY_PAGE_STAGES_START];
    WXSDKInstance* instance = [WXSDKManager instanceForID:instanceId];
    if (nil != instance) {
        for (NSString* key in instance.continerInfo) {
            id value = [instance.continerInfo objectForKey:key];
            [self setProperty:key withValue:value];
        }
    }
    NSString* pageUrl = instance.scriptURL.absoluteString;
    pageUrl = nil == pageUrl || [@"" isEqualToString:pageUrl]?@"unKnowUrl":pageUrl;
    
    [self setProperty:KEY_PAGE_PROPERTIES_BIZ_ID withValue:pageUrl];
    [self setProperty:KEY_PROPERTIES_ERROR_CODE withValue:VALUE_ERROR_CODE_DEFAULT];
    [self setProperty:KEY_PAGE_PROPERTIES_JSLIB_VERSION withValue:[WXAppConfiguration JSFrameworkVersion]];
    [self setProperty:KEY_PAGE_PROPERTIES_WEEX_VERSION withValue:WX_SDK_VERSION];
    
    //for apm protocl
    //iOS/Android we default recycle img when imgView disapper form screen
    //but in Android ,js can switch recycle or not
    [self updateDiffStats:KEY_PAGE_STATS_IMG_UN_RECYCLE_NUM withDiffValue:0];
}

- (void) endRecord;
{
    if (nil == _apmProtocolInstance) {
        return;
    }
    
    [self onStage:KEY_PAGE_STAGES_DESTROY];
    [self.apmProtocolInstance onEnd];
}

- (void) arriveFSRenderTime
{
    if (nil == _apmProtocolInstance || self.isFSEnd) {
        return;
    }
    self.isFSEnd = true;
    [self onStage:KEY_PAGE_STAGES_FSRENDER];
}

- (void) updateFSDiffStats:(NSString *)name withDiffValue:(double)diff
{
    if (nil == _apmProtocolInstance || self.isFSEnd) {
        return;
    }
    [self updateDiffStats:name withDiffValue:diff];
}

- (void) updateDiffStats:(NSString *)name withDiffValue:(double)diff
{
    if (nil == _apmProtocolInstance) {
        return;
    }
    NSNumber* preNumber = [self.recordStatsMap objectForKey:name];
    double preVal = nil == preNumber?0:preNumber.doubleValue;
    double currentVal = preVal + diff;
    [self.recordStatsMap setObject:@(currentVal) forKey:name];
    [self setStatistic:name withValue:currentVal];
}

- (void) updateMaxStats:(NSString *)name curMaxValue:(double)currentValue
{
    if (nil == _apmProtocolInstance) {
        return;
    }
    NSNumber* maxNumber = [self.recordStatsMap objectForKey:name];
    double maxVal = nil == maxNumber?0:maxNumber.doubleValue;
    
    if (maxVal < currentValue) {
        maxVal = currentValue;
        [self.recordStatsMap setObject:@(maxVal) forKey:name];
        [self setStatistic:name withValue:maxVal];
    }
}

- (void) updateExtInfo:(NSDictionary*) extInfo
{
    if (nil == _apmProtocolInstance || nil == extInfo) {
        return;
    }
    
    id wxRequestType = [extInfo objectForKey:KEY_PAGE_PROPERTIES_REQUEST_TYPE];
    if (nil != wxRequestType && [wxRequestType isKindOfClass: NSString.class]) {
        [self setProperty:KEY_PAGE_PROPERTIES_REQUEST_TYPE withValue:wxRequestType];
    }
    
    id wxNetType = [extInfo objectForKey:KEY_PAGE_PROPERTIES_NET_TYPE];
    if (nil != wxRequestType && [wxNetType isKindOfClass: NSString.class]) {
        [self setProperty:KEY_PAGE_PROPERTIES_NET_TYPE withValue:wxRequestType];
    }
    
    id wxCacheType = [extInfo objectForKey:KEY_PAGE_PROPERTIES_CACHE_TYPE];
    if (nil != wxCacheType && [wxCacheType isKindOfClass: NSString.class]) {
        [self setProperty:KEY_PAGE_PROPERTIES_CACHE_TYPE withValue:wxCacheType];
    }

    id wxNetLibDownBundleTime = [extInfo objectForKey:KEY_PAGE_STATS_ACTUAL_DOWNLOAD_TIME];
    if (nil != wxNetLibDownBundleTime && [wxNetLibDownBundleTime isKindOfClass: NSNumber.class]) {
        double value = ((NSNumber *)wxNetLibDownBundleTime).doubleValue;
        [self  setStatistic:KEY_PAGE_PROPERTIES_CACHE_TYPE withValue:value];
    }
}

#pragma mark - called by IWXHttpAdapter implementer

- (void) actionNetRequest
{
    if (nil == _apmProtocolInstance) {
        return;
    }
    if (!self.isFSEnd) {
        [self updateFSDiffStats:KEY_PAGE_STATS_FS_REQUEST_NUM withDiffValue:1];
    }
    [self updateDiffStats:KEY_PAGE_STATS_NET_NUM withDiffValue:1];
}

- (void) actionNetRequestResult:(bool)succeed withErrorCode:(NSString*)errorCode
{
    if (nil == _apmProtocolInstance) {
        return;
    }
    if (succeed) {
        [self updateDiffStats:KEY_PAGE_STATS_NET_SUCCESS_NUM withDiffValue:1];
    } else {
        [self updateDiffStats:KEY_PAGE_STATS_NET_FAIL_NUM withDiffValue:1];
    }
}

#pragma mark - called by IWXImgLoaderAdapter implementer

- (void) actionImgLoad
{
    if (nil == _apmProtocolInstance) {
        return;
    }
    [self updateDiffStats:KEY_PAGE_STATS_IMG_LOAD_NUM withDiffValue:1];
}

- (void) actionImgLoadResult:(bool)succeed withErrorCode:(NSString*)errorCode
{
    if (nil == _apmProtocolInstance) {
        return;
    }
    if (succeed) {
        [self updateDiffStats:KEY_PAGE_STATS_IMG_LOAD_SUCCESS_NUM withDiffValue:1];
    } else {
        [self updateDiffStats:KEY_PAGE_STATS_IMG_LOAD_FAIL_NUM withDiffValue:1];
    }
}

@end

