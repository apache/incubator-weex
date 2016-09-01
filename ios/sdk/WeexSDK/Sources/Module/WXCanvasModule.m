//
//  WXCanvasModule.m
//  WeexSDK
//
//  Created by yuankong on 16/6/28.
//  Copyright © 2016年 taobao. All rights reserved.
//

#import "WXCanvasModule.h"
#import "WXCanvasComponent.h"
#import "WXSDKManager.h"
#import "WXSDKInstance_private.h"
#import "WXUtility.h"

@interface WXCanvasModule()
@property (nonatomic, strong) NSMutableDictionary *cacheMap;
@property (nonatomic, strong) EAGLContext *glcontext;
@end

@implementation WXCanvasModule

@synthesize weexInstance;

WX_EXPORT_METHOD(@selector(addDrawActions:actions:))
WX_EXPORT_METHOD(@selector(initTexture:callbackId:))

- (instancetype) init {
    _cacheMap = [NSMutableDictionary new];
    _glcontext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    [EAGLContext setCurrentContext:_glcontext];
    return [super init];
}

- (void) dealloc {
    [_cacheMap removeAllObjects];
    _cacheMap = nil;
}

- (void)performBlockWithCanvas:(NSString *)elemRef block:(void (^)(WXCanvasComponent *))block {
    if (!elemRef) {
        return;
    }

    __weak typeof(self) weakSelf = self;

    WXPerformBlockOnComponentThread(^{
        WXCanvasComponent *canvas = (WXCanvasComponent *)[weakSelf.weexInstance componentForRef:elemRef];
        if (!canvas) {
            return;
        }

        [weakSelf performSelectorOnMainThread:@selector(doBlock:) withObject:^() {
            block(canvas);
        } waitUntilDone:NO];
    });
}


- (void)doBlock:(void (^)())block {
    block();
}

- (void)addDrawActions:(NSString *)elemRef actions:(NSArray *)actions {
    [self performBlockWithCanvas:elemRef block:^(WXCanvasComponent *canvas) {
        [canvas addDrawActions:actions canvasModule:self];
    }];
}


-(UIImage *) getImage:(NSString *)imageURL {
    NSString *key = [NSString stringWithFormat:@"image~%@", imageURL];
    if ([_cacheMap objectForKey:key]) {
        return (UIImage *)[_cacheMap objectForKey:key];
    }
    UIImage *image = [UIImage imageWithData:[NSData dataWithContentsOfURL:[NSURL URLWithString:imageURL]]];

    if (image != nil) {
        [_cacheMap setObject:image forKey:key];
    }
    return image;
}

-(void) initTexture:(NSString *)imgURL callbackId:(NSInteger)callbackId {
    __weak typeof(self) weakSelf = self;
    WXPerformBlockOnComponentThread(^{
        UIImage *image = [weakSelf getImage:imgURL];
        NSDictionary *data = @{
                               @"url": imgURL,
                               @"width": @(image.size.width),
                               @"height": @(image.size.height)
                               };

        [[WXSDKManager bridgeMgr] callBack:self.weexInstance.instanceId funcId:[@(callbackId) stringValue] params:[WXUtility JSONString:data]];
    });
}

@end
