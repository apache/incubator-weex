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

- (instancetype) init
{
    _cacheMap = [NSMutableDictionary new];
    _glcontext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    [EAGLContext setCurrentContext:_glcontext];
    return [super init];
}

- (void) dealloc
{
    [_cacheMap removeAllObjects];
    _cacheMap = nil;
}

- (void)performBlockWithCanvas:(NSString *)elemRef block:(void (^)(WXCanvasComponent *))block
{
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


- (void)doBlock:(void (^)(void))block
{
    block();
}

- (void)addDrawActions:(NSString *)elemRef actions:(NSArray *)actions
{
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

-(void) initTexture:(NSString *)imgURL callbackId:(NSInteger)callbackId
{
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
