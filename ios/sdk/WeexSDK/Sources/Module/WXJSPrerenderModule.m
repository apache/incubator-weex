//
//  WXJSPrerenderModule.m
//  Pods
//
//  Created by 齐山 on 17/5/2.
//
//

#import "WXJSPrerenderModule.h"
#import "WXJSPrerenderManager.h"

@interface WXJSPrerenderModule()

@property (nonatomic, strong) NSString *instanceId;

@end

@implementation WXJSPrerenderModule

@synthesize weexInstance;

WX_EXPORT_METHOD(@selector(addTask:callback:))

- (void)dealloc {
    if(!_instanceId){
        return;
    }
    
}

- (void) addTask:(NSString *) urlStr callback:(WXModuleCallback)callback{
    if(![self weexInstance]){
        return;
    }
    
    
    NSURL *sourceURL = [NSURL URLWithString:urlStr];
    if (!sourceURL) {
        return;
    }
    
    _instanceId = [[self weexInstance] instanceId];
    __weak WXSDKInstance *weakInstance = self.weexInstance;

    
    [[WXJSPrerenderManager sharedInstance] executeTask:urlStr WXInstance:[weakInstance instanceId] callback:callback];
}

@end
