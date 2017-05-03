//
//  WXJSPrerenderManager.h
//  Pods
//
//  Created by 齐山 on 17/5/2.
//
//

#import <Foundation/Foundation.h>
#import <WeexSDk/WeexSDK.h>

typedef NS_ENUM(NSUInteger, JSPrerenderStatus) {
    JSPrerenderSucceed = 0,
    JSPrerenderFail = 1,
};
typedef void (^JSPrerenderCallback) (JSPrerenderStatus status, NSString* urlStr, NSString * msg);

@interface WXJSPrerenderManager : NSObject

- (instancetype) init NS_UNAVAILABLE;
+ (instancetype) sharedInstance;
- (void) executeTask:(NSString *) urlStr WXInstance:(NSString *) instanceId callback:(WXModuleCallback)callback;
- (void) cancelTask:(NSString *) instanceId;


- (id)prerenderTasksForUrl:(NSString *)url;

- (void)storePrerenderInstance:(WXSDKInstance *)instance forUrl:(NSString *)url;

-  (void)removePrerenderTaskforUrl:(NSString *)url;

- (id)prerenderModuleTasksForUrl:(NSString *)url;

- (void)storePrerenderModuleTasks:(NSMutableDictionary *)prerenderModuleTask forUrl:(NSString *)url;

- (void)removePrerenderModuleTaskforUrl:(NSString *)url;
-(NSString *)prerenderUrl:(NSURL *)scriptUrl;
- (void)excuteModuleTasksForUrl:(NSString *)url;
@end
