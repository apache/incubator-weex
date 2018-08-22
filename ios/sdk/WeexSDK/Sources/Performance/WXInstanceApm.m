#import "WXApmForInstance.h"
#import "WXApmProtocol.h"
#import "WXHandlerFactory.h"
#import "WXSDKManager.h"
#import "WXAppConfiguration.h"
#import "WXUtility.h"
#import "WXComponentManager.h"
#import "WXConfigCenterProtocol.h"
#import "WXSDKEngine.h"
#import "WXSDKError.h"
#import "WXExceptionUtils.h"


#pragma mark - const static string

NSString* const WEEX_PAGE_TOPIC = @"weex_page";

/************** properties *****************/
NSString* const KEY_PROPERTIES_ERROR_CODE = @"wxErrorCode";
NSString* const KEY_PAGE_PROPERTIES_BIZ_ID = @"wxBizID";
NSString* const KEY_PAGE_PROPERTIES_BUBDLE_URL = @"wxBundleUrl";
NSString* const KEY_PAGE_PROPERTIES_JSLIB_VERSION  = @"wxJSLibVersion";
NSString* const KEY_PAGE_PROPERTIES_WEEX_VERSION  = @"wxSDKVersion";
NSString* const KEY_PAGE_PROPERTIES_REQUEST_TYPE  = @"wxRequestType";
NSString* const KEY_PAGE_PROPERTIES_Z_CACHE_INFO  = @"wxZCacheInfo";
NSString* const KEY_PAGE_PROPERTIES_JS_FM_INIT  = @"wxJsFrameworkInit";
NSString* const KEY_PAGE_PROPERTIES_BUNDLE_TYPE = @"wxBundleType";
NSString* const KEY_PAGE_PROPERTIES_CONTAINER_NAME = @"wxContainerName";
NSString* const KEY_PAGE_PROPERTIES_INSTANCE_TYPE = @"wxInstanceType";
NSString* const KEY_PAGE_PROPERTIES_PARENT_PAGE = @"wxParentPage";


///************** stages *****************/
NSString* const KEY_PAGE_STAGES_START = @"wxRecordStart";
NSString* const KEY_PAGE_STAGES_DOWN_BUNDLE_START  = @"wxStartDownLoadBundle";
NSString* const KEY_PAGE_STAGES_DOWN_BUNDLE_END  = @"wxEndDownLoadBundle";
NSString* const KEY_PAGE_STAGES_RENDER_ORGIGIN  = @"wxRenderTimeOrigin";
NSString* const KEY_PAGE_STAGES_LOAD_BUNDLE_START  = @"wxStartLoadBundle";
NSString* const KEY_PAGE_STAGES_LOAD_BUNDLE_END  = @"wxEndLoadBundle";
NSString* const KEY_PAGE_STAGES_CREATE_FINISH = @"wxJSBundleCreateFinish";
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
NSString* const KEY_PAGE_STATS_EMBED_COUNT=@"wxEmbedCount";
NSString* const KEY_PAGE_STATS_LARGE_IMG_COUNT=@"wxLargeImgMaxCount";

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
@property (nonatomic,assign) BOOL isRecord;
@property (nonatomic,strong) NSMutableDictionary<NSString*,NSNumber*>* recordStageMap;
@property (nonatomic,strong) NSMutableArray<WXJSExceptionInfo*>* errorList;
@end

@implementation WXApmForInstance


- (instancetype) init
{
    self = [super init];
    if (self) {
        _recordStatsMap = [[NSMutableDictionary alloc] init];
        _recordStageMap = [[NSMutableDictionary alloc] init];
        _errorList = [[NSMutableArray alloc] init];
        _isOpenApm = [self _loadApmSwitch];
        if (_isOpenApm) {
            id<WXApmGeneratorProtocol> generater = [WXHandlerFactory handlerForProtocol:@protocol(WXApmGeneratorProtocol)];
            _apmProtocolInstance = [generater gengratorApmInstance:WEEX_PAGE_TOPIC];
        }
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
    long time = [WXUtility getUnixFixTimeMillis];
    [self.apmProtocolInstance onStage:name withValue:time];
    __weak typeof(self) weakSelf = self;
    WXPerformBlockOnComponentThread(^{
        [weakSelf.recordStageMap setObject:@(time) forKey:name];
    });
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
    if (nil == _apmProtocolInstance || self.isRecord) {
        return;
    }
    self.isRecord = YES;
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
    NSString* pageName = instance.pageName?:@"unKnowPageName";
    NSString* vcName = @"unKnowVCName";
    if (nil != instance.viewController) {
        vcName = NSStringFromClass(instance.viewController.class);
    }
    [self setProperty:KEY_PAGE_PROPERTIES_CONTAINER_NAME withValue:vcName];
    [self setProperty:KEY_PAGE_PROPERTIES_INSTANCE_TYPE withValue:@"page"];
    [self setProperty:KEY_PAGE_PROPERTIES_BIZ_ID withValue:pageName];
    [self setProperty:KEY_PAGE_PROPERTIES_BUBDLE_URL withValue:pageUrl];
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
    [self _checkScreenEmptyAndReport];
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
    __weak typeof(self) weakSelf = self;
    WXPerformBlockOnComponentThread(^{
        NSNumber* preNumber = [self.recordStatsMap objectForKey:name];
        double preVal = nil == preNumber?0:preNumber.doubleValue;
        double currentVal = preVal + diff;
        [weakSelf.recordStatsMap setObject:@(currentVal) forKey:name];
        [weakSelf setStatistic:name withValue:currentVal];
    });
}

- (void) updateMaxStats:(NSString *)name curMaxValue:(double)currentValue
{
    if (nil == _apmProtocolInstance) {
        return;
    }
    __weak typeof(self) weakSelf = self;
    WXPerformBlockOnComponentThread(^{
        NSNumber* maxNumber = [weakSelf.recordStatsMap objectForKey:name];
        double maxVal = nil == maxNumber?0:maxNumber.doubleValue;
        
        if (maxVal < currentValue) {
            maxVal = currentValue;
            [weakSelf.recordStatsMap setObject:@(maxVal) forKey:name];
            [weakSelf setStatistic:name withValue:maxVal];
        }
    });
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
    
    id wxNetLibDownBundleTime = [extInfo objectForKey:KEY_PAGE_STATS_ACTUAL_DOWNLOAD_TIME];
    if (nil != wxNetLibDownBundleTime && [wxNetLibDownBundleTime isKindOfClass: NSNumber.class]) {
        double value = ((NSNumber *)wxNetLibDownBundleTime).doubleValue;
        [self  setStatistic:KEY_PAGE_STATS_ACTUAL_DOWNLOAD_TIME withValue:value];
    }
    
    id wxZcacheInfo = [extInfo objectForKey:KEY_PAGE_PROPERTIES_Z_CACHE_INFO];
    if (nil !=wxZcacheInfo && [wxZcacheInfo isKindOfClass: NSString.class]) {
        [self setProperty:KEY_PAGE_PROPERTIES_Z_CACHE_INFO withValue:wxZcacheInfo];
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

- (void) recordErrorMsg:(WXJSExceptionInfo *)exception
{
    if (nil == exception || !self.isOpenApm) {
        return;
    }
    __weak typeof(self) weakSelf = self;
    WXPerformBlockOnComponentThread(^{
        if (weakSelf.errorList.count > 5) {
            [weakSelf.errorList removeObjectAtIndex:0];
            [weakSelf.errorList addObject:exception];
        }
    });
}

- (NSArray<WXJSExceptionInfo*>*) topExceptionList
{
    return self.errorList;
}

- (void) _checkScreenEmptyAndReport
{
    if(self.hasAddView || !self.isStartRender || self.isDegrade){
        return;
    }
    WXSDKErrCode code = WX_KEY_EXCEPTION_EMPTY_SCREEN_NATIVE;
    
    
    NSNumberFormatter *formater = [[NSNumberFormatter alloc] init];
    formater.numberStyle = NSNumberFormatterDecimalStyle;
    
    for (WXJSExceptionInfo* exception in self.errorList) {
        NSNumber *codeNumber = [formater numberFromString:exception.errorCode];
        if (nil == codeNumber) {
            continue;
        }
        WXSDKErrorGroup group = [WXSDKErrCodeUtil getErrorGroupByCode:codeNumber.intValue];
        if(group == WX_JS){
            code = WX_KEY_EXCEPTION_EMPTY_SCREEN_JS;
            break;
        }
    }
    NSString *codeStr = [NSString stringWithFormat:@"%d",code];
    [WXExceptionUtils commitCriticalExceptionRT:self.instanceId errCode:codeStr function:@"_checkScreenEmptyAndReport"
                                      exception:[self _convertTopExceptionListToStr] extParams:nil];
    
}

- (NSString *)_convertTopExceptionListToStr
{
    if (!self.isOpenApm) {
        return @"";
    }
    if (!self.errorList || self.errorList.count <= 0) {
        return @"emptyTopErrorlist";
    }
    NSString* errorStr = @"========== topErrorList start ============";
    for (WXJSExceptionInfo* exception in self.errorList) {
        errorStr = [errorStr stringByAppendingFormat:
                    @"%@ -> code:%@,func:%@,exception:%@ \n",
                    errorStr,exception.errorCode,exception.functionName,exception.exception
                    ];
    }
    errorStr = [errorStr stringByAppendingFormat:@"%@ ========== topErrorList start end ============\n",errorStr];
    return errorStr;
}

- (NSDictionary<NSString*,NSNumber*>*) stageDic
{
    return self.recordStageMap;
}

- (BOOL) _loadApmSwitch
{
    BOOL openApm = YES;
    id<WXConfigCenterProtocol> configCenter = [WXSDKEngine handlerForProtocol:@protocol(WXConfigCenterProtocol)];
    
    if ([configCenter respondsToSelector:@selector(configForKey:defaultValue:isDefault:)]) {
        openApm = [[configCenter configForKey:@"iOS_weex_ext_config.open_apm" defaultValue:@(YES) isDefault:NULL] boolValue];
    }
    return openApm;
}

@end

