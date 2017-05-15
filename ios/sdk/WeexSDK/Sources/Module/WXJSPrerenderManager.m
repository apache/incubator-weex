//
//  WXJSPrerenderManager.m
//  Pods
//
//  Created by 齐山 on 17/5/2.
//
//

#import "WXJSPrerenderManager.h"
#import "WXJSPrerenderConfigProtocol.h"
#import "WXModuleMethod.h"
#import "WXBridgeManager.h"

static NSString *const MSG_PRERENDER_SWITCH_OFF = @"switch_off";
static NSString *const MSG_PRERENDER_EXCEED = @"exceed";
static NSString *const MSG_PRERENDER_ERROR_CONNECTION = @"error_connection_type";
static NSString *const MSG_PRERENDER_ALREADY_REQUESTED = @"already_requested";
static NSString *const MSG_PRERENDER_INTERNAL_ERROR = @"internal_error";

@interface WXJSPrerenderManager()

@property (nonatomic, strong) NSMutableArray *cachedUrlList;
@property (nonatomic, strong) dispatch_queue_t queue;
@property (nonatomic, strong) NSMutableDictionary<NSString *, NSMutableDictionary*> *prerenderTasks;
@property (nonatomic, strong) NSMutableDictionary<NSString *, NSMutableArray*> *prerenderModuleTasks;
@property (nonatomic, strong) id<WXJSPrerenderConfigProtocol> configHandler;

@end

@implementation WXJSPrerenderManager

+ (instancetype) sharedInstance{
    
    static WXJSPrerenderManager *instance = nil;
    static dispatch_once_t once;
    
    dispatch_once(&once, ^{
        instance = [[WXJSPrerenderManager alloc] initPrivate];
    });
    
    return instance;
}

- (instancetype) initPrivate{
    self = [super init];
    if(self){
        self.cachedUrlList = [[NSMutableArray alloc] init];
        self.queue = dispatch_queue_create("JSPrerender", DISPATCH_QUEUE_SERIAL);
        self.prerenderTasks = [[NSMutableDictionary alloc] init];
        self.prerenderModuleTasks = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (void) dealloc{
    self.cachedUrlList = nil;
    self.prerenderTasks = nil;
    self.prerenderModuleTasks = nil;
}

- (id<WXJSPrerenderConfigProtocol>) handler{
    if(!_configHandler){
        _configHandler = [WXSDKEngine handlerForProtocol:@protocol(WXJSPrerenderConfigProtocol)];
    }
    
    return _configHandler;
}

- (void) cancelTask:(NSString *)instanceId{
    dispatch_async(self.queue, ^{
        for(NSURLSessionDataTask *task in _prerenderTasks[instanceId]){
            [task cancel];
        }
        
        [_prerenderTasks removeObjectForKey:instanceId];
    });
}

- (void) executeTask:(NSString *)urlStr WXInstance:(NSString *)instanceId callback:(WXModuleCallback)callback{
    NSURL *url = [NSURL URLWithString:urlStr];
    if(!url){
        callback(@{@"url":urlStr,@"error":MSG_PRERENDER_INTERNAL_ERROR});
        return;
    }
    
    __weak WXJSPrerenderManager *weakSelf = self;
    dispatch_async(self.queue, ^{
        [weakSelf prerender:url WXInstance:instanceId callback:callback];
    });
}

-(NSString *)prerenderUrl:(NSURL *)scriptUrl
{
    NSURL *sourceURL = nil;
    //    if([AliWeexURLResolver isValidWeexURL:url request:&urlRequest mode:&requestMode]){
    //        parsedUrl = [urlRequest URL];
    //    } else {
    //        NSString *scheme = [url scheme];
    //        NSString *host = [url host];
    //        NSString *path = [url path];
    //        if(!scheme || !host || !path){
    //            callback(JSPrerenderFail, [url absoluteString], MSG_PRERENDER_INTERNAL_ERROR);
    //
    //            return;
    //        }
    //
    //        parsedUrl = [[NSURL alloc] initWithScheme:scheme host:host path:path];
    //    }
    NSURL *url = scriptUrl;
    NSString *scheme = [url scheme];
    NSString *host = [url host];
    NSString *path = [url path];
    if(!scheme || !host || !path){
        return nil;
    }
    NSString *str = [NSString stringWithFormat:@"%@://%@:%@%@",scheme,host,[url port],path];
    return str;
}

- (void) prerender:(NSURL *)url WXInstance:(NSString *)instanceId callback:(WXModuleCallback) callback{
    
    
    
    NSMutableURLRequest *urlRequest = nil;
//    WXRequestTemplateMode requestMode = WXRequestTemplateMode_Unknown;
    NSURL *sourceURL = nil;
//    if([AliWeexURLResolver isValidWeexURL:url request:&urlRequest mode:&requestMode]){
//        parsedUrl = [urlRequest URL];
//    } else {
//        NSString *scheme = [url scheme];
//        NSString *host = [url host];
//        NSString *path = [url path];
//        if(!scheme || !host || !path){
//            callback(JSPrerenderFail, [url absoluteString], MSG_PRERENDER_INTERNAL_ERROR);
//            
//            return;
//        }
//        
//        parsedUrl = [[NSURL alloc] initWithScheme:scheme host:host path:path];
//    }
    
//    NSString *scheme = [url scheme];
//    NSString *host = [url host];
//    NSString *path = [url path];
//    if(!scheme || !host || !path){
//        callback(@{@"url":[url absoluteString],@"error":MSG_PRERENDER_INTERNAL_ERROR});
//        return;
//    }
    NSString *str = [self prerenderUrl:url];
    if(str.length==0){
        callback(@{@"url":[url absoluteString],@"error":MSG_PRERENDER_INTERNAL_ERROR});
    }
    sourceURL = [NSURL URLWithString:str];
    id<WXJSPrerenderConfigProtocol> configHandler = [self handler];
    //step 1.判断Orange开关是否打开
    BOOL switchOn = [configHandler isSwitchOn];
//    if(!switchOn){
//        callback(@{@"url":[url absoluteString],@"error":MSG_PRERENDER_INTERNAL_ERROR});
//        return;
//    }
    
    
    WXSDKInstance *instance = [[WXSDKInstance alloc] init];
    //    _instance.frame = CGRectMake(0.0f, 0.0f, self.view.bounds.size.width, self.view.bounds.size.height);
    //    _instance.pageObject = self;
    //    _instance.pageName = [[WXUtility urlByDeletingParameters:sourceURL] absoluteString];
    //    _instance.viewController = self;
    instance.needPrerender = YES;
    NSString *newURL = nil;
    NSMutableDictionary *m = [NSMutableDictionary new];
    [m setObject:instance forKey:@"instance"];
    
    if ([sourceURL.absoluteString rangeOfString:@"?"].location != NSNotFound) {
        newURL = [NSString stringWithFormat:@"%@&random=%d", sourceURL.absoluteString, arc4random()];
    } else {
        newURL = [NSString stringWithFormat:@"%@?random=%d", sourceURL.absoluteString, arc4random()];
    }
    [instance renderWithURL:[NSURL URLWithString:newURL] options:@{@"bundleUrl":sourceURL.absoluteString} data:nil];
    
    [self.prerenderTasks setObject:m forKey:url.absoluteString];
    __weak typeof(self) weakSelf = self;
    instance.onCreate = ^(UIView *view) {
        //        [weakSelf.weexView removeFromSuperview];
        //        weakSelf.weexView = view;
        //        [weakSelf.view addSubview:weakSelf.weexView];
        NSMutableDictionary *m = [weakSelf.prerenderTasks objectForKey:url.absoluteString];
        [m setObject:view forKey:@"view"];
        [weakSelf.prerenderTasks setObject:m forKey:url.absoluteString];
    };
    
    instance.onFailed = ^(NSError *error) {
        
    };
    
    instance.renderFinish = ^(UIView *view) {
        //        [weakSelf _updateInstanceState:WeexInstanceAppear];
        NSMutableDictionary *m = [weakSelf.prerenderTasks objectForKey:url.absoluteString];
        [m setObject:@(WeexInstanceAppear) forKey:@"WeexInstanceAppear"];
        [weakSelf.prerenderTasks setObject:m forKey:url.absoluteString];
    };
    
    
}

- (id)prerenderTasksForUrl:(NSString *)url
{
    return [self.prerenderTasks objectForKey:url];
}

- (void)storePrerenderTasks:(NSMutableDictionary *)prerenderTasks forUrl:(NSString *)url
{
    [self.prerenderTasks setObject:prerenderTasks forKey:url];
}

- (void)removePrerenderTaskforUrl:(NSString *)url
{
    if (url) {
        [self.prerenderTasks removeObjectForKey:url];
    }
}


- (id)prerenderModuleTasksForUrl:(NSString *)url
{
    return [self.prerenderModuleTasks objectForKey:url];
}

- (void)storePrerenderModuleTasks:(WXModuleMethod *)method forUrl:(NSString *)url
{
    NSMutableArray *m = [self.prerenderModuleTasks objectForKey:url];
    if (!m){
        m = [NSMutableArray new];
    }
    [m addObject:method];
    [self.prerenderModuleTasks setObject:m forKey:url];}

- (void)removePrerenderModuleTaskforUrl:(NSString *)url
{
    if (url) {
        [self.prerenderModuleTasks removeObjectForKey:url];
    }
}

- (void)excuteModuleTasksForUrl:(NSString *)url
{
    NSMutableArray *m = [self.prerenderModuleTasks objectForKey:url];
    
    if (m && [m count]>0){
        for (WXModuleMethod *method in m) {
            [method invoke];
        }
        
    }

    
}

@end
