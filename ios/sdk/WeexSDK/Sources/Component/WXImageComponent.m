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

#import "WXImageComponent.h"
#import "WXHandlerFactory.h"
#import "WXComponent_internal.h"
#import "WXImgLoaderProtocol.h"
#import "WXLayer.h"
#import "WXType.h"
#import "WXConvert.h"
#import "WXURLRewriteProtocol.h"
#import "WXRoundedRect.h"
#import "UIBezierPath+Weex.h"
#import "WXSDKEngine.h"
#import "WXUtility.h"

@interface WXImageView : UIImageView

@end

@implementation WXImageView

+ (Class)layerClass
{
    return [WXLayer class];
}

@end

static dispatch_queue_t WXImageUpdateQueue;

@interface WXImageComponent ()

@property (nonatomic, strong) NSString *imageSrc;
@property (nonatomic, strong) NSString *placeholdSrc;
@property (nonatomic, assign) CGFloat blurRadius;
@property (nonatomic, assign) UIViewContentMode resizeMode;
@property (nonatomic, assign) WXImageQuality imageQuality;
@property (nonatomic, assign) WXImageSharp imageSharp;
@property (nonatomic, strong) UIImage *image;
@property (nonatomic, strong) id<WXImageOperationProtocol> imageOperation;
@property (nonatomic, strong) id<WXImageOperationProtocol> placeholderOperation;
@property (nonatomic) BOOL imageLoadEvent;
@property (nonatomic) BOOL imageDownloadFinish;

@end

@implementation WXImageComponent

WX_EXPORT_METHOD(@selector(save:))

- (instancetype)initWithRef:(NSString *)ref type:(NSString *)type styles:(NSDictionary *)styles attributes:(NSDictionary *)attributes events:(NSArray *)events weexInstance:(WXSDKInstance *)weexInstance
{
    if (self = [super initWithRef:ref type:type styles:styles attributes:attributes events:events weexInstance:weexInstance]) {
        _async = YES;
        if (!WXImageUpdateQueue) {
            WXImageUpdateQueue = dispatch_queue_create("com.taobao.weex.ImageUpdateQueue", DISPATCH_QUEUE_SERIAL);
        }
        if (attributes[@"src"]) {
            _imageSrc = [[WXConvert NSString:attributes[@"src"]] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
        } else {
            WXLogWarning(@"image src is nil");
        }
        [self configPlaceHolder:attributes];
        _resizeMode = [WXConvert UIViewContentMode:attributes[@"resize"]];
        [self configFilter:styles];
        if (styles[@"quality"]) {
            _imageQuality = [WXConvert WXImageQuality:styles[@"quality"]];
        }
        if (attributes[@"quality"]) {
            _imageQuality = [WXConvert WXImageQuality:attributes[@"quality"]];
        }
        _imageSharp = [WXConvert WXImageSharp:styles[@"sharpen"]];
        _imageLoadEvent = NO;
        _imageDownloadFinish = NO;
    }
    
    return self;
}

- (void)configPlaceHolder:(NSDictionary*)attributes
{
    if (attributes[@"placeHolder"] || attributes[@"placeholder"]) {
        _placeholdSrc = [[WXConvert NSString:attributes[@"placeHolder"]?:attributes[@"placeholder"]]stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
    }
}

- (void)configFilter:(NSDictionary *)styles
{
    if (styles[@"filter"]) {
        NSString *filter = styles[@"filter"];
        
        NSString *pattern = @"blur\\((\\d+)(px)?\\)";
        NSError *error = nil;
        NSRegularExpression *regex = [NSRegularExpression regularExpressionWithPattern:pattern
                                                                               options:NSRegularExpressionCaseInsensitive
                                                                                 error:&error];
        NSArray *matches = [regex matchesInString:filter options:0 range:NSMakeRange(0, filter.length)];
        if (matches && matches.count > 0) {
            NSTextCheckingResult *match = matches[matches.count - 1];
            NSRange matchRange = [match rangeAtIndex:1];
            NSString *matchString = [filter substringWithRange:matchRange];
            if (matchString && matchString.length > 0) {
                _blurRadius = [matchString doubleValue];
                [self updateImage];
            }
        }
    }
}

- (void)save:(WXCallback)resultCallback
{
    NSDictionary *info = [NSBundle mainBundle].infoDictionary;
    if(!info[@"NSPhotoLibraryUsageDescription"]) {
        if (resultCallback) {
            resultCallback(@{
                             @"success" : @(false),
                             @"errorDesc": @"This maybe crash above iOS 10 because it attempted to access privacy-sensitive data without a usage description.  The app's Info.plist must contain an NSPhotoLibraryUsageDescription key with a string value explaining to the user how the app uses this data."
                             });
        }
        return ;
    }
    
    // iOS 11 needs a NSPhotoLibraryUsageDescription key for permission
    if (WX_SYS_VERSION_GREATER_THAN_OR_EQUAL_TO(@"11.0")) {
        if (!info[@"NSPhotoLibraryUsageDescription"]) {
            if (resultCallback) {
                resultCallback(@{
                                 @"success" : @(false),
                                 @"errorDesc": @"This maybe crash above iOS 10 because it attempted to access privacy-sensitive data without a usage description.  The app's Info.plist must contain an NSPhotoLibraryUsageDescription key with a string value explaining to the user how the app uses this data."
                                 });
            }
            return;
        }
    }
    
    if (![self isViewLoaded]) {
        if (resultCallback) {
            resultCallback(@{@"success": @(false),
                             @"errorDesc":@"the image is not ready"});
        }
        return;
    }
    UIImageView * imageView = (UIImageView*)self.view;
    if (!resultCallback) {
        // there is no need to callback any result;
        UIImageWriteToSavedPhotosAlbum(imageView.image, nil, nil, NULL);
    }else {
        UIImageWriteToSavedPhotosAlbum(imageView.image, self, @selector(image:didFinishSavingWithError:contextInfo:), (void*)CFBridgingRetain(resultCallback));
    }
}

// the callback for PhotoAlbum.
- (void)image:(UIImage *)image didFinishSavingWithError:(NSError *)error contextInfo:(void *)contextInfo
{
    if (!contextInfo) {
        return;
    }
    NSMutableDictionary * callbackResult = [NSMutableDictionary new];
    BOOL success = false;
    if (!error) {
        success = true;
    } else {
        [callbackResult setObject:[error description] forKey:@"errorDesc"];
    }
    if (contextInfo) {
        [callbackResult setObject:@(success) forKey:@"success"];
        ((__bridge WXCallback)contextInfo)(callbackResult);
        CFRelease(contextInfo);
    }
}

- (UIView *)loadView
{
    return [[WXImageView alloc] init];
}

- (void)addEvent:(NSString *)eventName {
    if ([eventName isEqualToString:@"load"]) {
        _imageLoadEvent = YES;
    }
}

- (void)removeEvent:(NSString *)eventName {
    if ([eventName isEqualToString:@"load"]) {
        _imageLoadEvent = NO;
    }
}

- (void)updateStyles:(NSDictionary *)styles
{
    if (styles[@"quality"]) {
        _imageQuality = [WXConvert WXImageQuality:styles[@"quality"]];
        [self updateImage];
    }
    
    if (styles[@"sharpen"]) {
        _imageSharp = [WXConvert WXImageSharp:styles[@"sharpen"]];
        [self updateImage];
    }
    [self configFilter:styles];
}

- (void)dealloc
{
    [self cancelImage];
}

- (void)updateAttributes:(NSDictionary *)attributes
{
    if (attributes[@"src"]) {
        _imageSrc = [[WXConvert NSString:attributes[@"src"]] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
        [self updateImage];
    }
    if (attributes[@"quality"]) {
        _imageQuality = [WXConvert WXImageQuality:attributes[@"quality"]];
        [self updateImage];
    }
    
    [self configPlaceHolder:attributes];
    
    if (attributes[@"resize"]) {
        _resizeMode = [WXConvert UIViewContentMode:attributes[@"resize"]];
        self.view.contentMode = _resizeMode;
    }
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    UIImageView *imageView = (UIImageView *)self.view;
    imageView.contentMode = _resizeMode;
    imageView.userInteractionEnabled = YES;
    imageView.clipsToBounds = YES;
    imageView.exclusiveTouch = YES;
    if (!_testId) {
        imageView.isAccessibilityElement = YES;
    }
    
    [self _clipsToBounds];
    
    [self updateImage];
    
}

- (BOOL)_needsDrawBorder
{
    return NO;
}

- (BOOL)needsDrawRect
{
    if (_isCompositingChild) {
        return YES;
    } else {
        return NO;
    }
}

- (UIImage *)drawRect:(CGRect)rect;
{
    if (!self.image) {
        [self updateImage];
        return nil;
    }
    
    WXRoundedRect *borderRect = [[WXRoundedRect alloc] initWithRect:rect topLeft:_borderTopLeftRadius topRight:_borderTopRightRadius bottomLeft:_borderBottomLeftRadius bottomRight:_borderBottomRightRadius];

    WXRadii *radii = borderRect.radii;    
    if ([radii hasBorderRadius]) {
        CGFloat topLeft = radii.topLeft, topRight = radii.topRight, bottomLeft = radii.bottomLeft, bottomRight = radii.bottomRight;
        UIBezierPath *bezierPath = [UIBezierPath wx_bezierPathWithRoundedRect:rect topLeft:topLeft topRight:topRight bottomLeft:bottomLeft bottomRight:bottomRight];
        [bezierPath addClip];
    }
    return self.image;
}

- (void)viewWillUnload
{
    [super viewWillUnload];
    [self cancelImage];
    _image = nil;
}

- (void)_frameDidCalculated:(BOOL)isChanged
{
    [super _frameDidCalculated:isChanged];
    
    if ([self isViewLoaded] && isChanged) {
        [self _clipsToBounds];
    }
}

- (void)setImageSrc:(NSString*)src
{
    if (![src isEqualToString:_imageSrc]) {
        _imageSrc = src;
        _imageDownloadFinish = NO;
        [self updateImage];
    }
}

- (void)updateImage
{
    __weak typeof(self) weakSelf = self;
    dispatch_async(WXImageUpdateQueue, ^{
         __strong typeof(weakSelf) strongSelf = weakSelf;
        [strongSelf cancelImage];
       
        void(^downloadFailed)(NSString *, NSError *) = ^void(NSString *url, NSError *error) {
            weakSelf.imageDownloadFinish = YES;
            WXLogError(@"Error downloading image: %@, detail:%@", url, [error localizedDescription]);
        };
        
        [strongSelf updatePlaceHolderWithFailedBlock:downloadFailed];
        [strongSelf updateContentImageWithFailedBlock:downloadFailed];
        
        if (!strongSelf.imageSrc && !strongSelf.placeholdSrc) {
            dispatch_async(dispatch_get_main_queue(), ^{
               
                strongSelf.layer.contents = nil;
                strongSelf.imageDownloadFinish = YES;
                [strongSelf readyToRender];
            });
        }
    });
}

- (void)updatePlaceHolderWithFailedBlock:(void(^)(NSString *, NSError *))downloadFailedBlock
{
    NSString *placeholderSrc = self.placeholdSrc;
    
    if ([WXUtility isBlankString:placeholderSrc]) {
//        WXLogError(@"image placeholder src is empty");
        return;
    }
    
    WXLogDebug(@"Updating image, component:%@, placeholder:%@ ", self.ref, placeholderSrc);
    NSString *newURL = [_placeholdSrc copy];
    WX_REWRITE_URL(_placeholdSrc, WXResourceTypeImage, self.weexInstance)
    
    __weak typeof(self) weakSelf = self;
    self.placeholderOperation = [[self imageLoader] downloadImageWithURL:newURL imageFrame:self.calculatedFrame userInfo:nil completed:^(UIImage *image, NSError *error, BOOL finished) {
        dispatch_async(dispatch_get_main_queue(), ^{
            __strong typeof(self) strongSelf = weakSelf;
            UIImage *viewImage = ((UIImageView *)strongSelf.view).image;
            if (error) {
                downloadFailedBlock(placeholderSrc,error);
                if ([strongSelf isViewLoaded] && !viewImage) {
                    ((UIImageView *)(strongSelf.view)).image = nil;
                    [strongSelf readyToRender];
                }
                return;
            }
            if (![placeholderSrc isEqualToString:strongSelf.placeholdSrc]) {
                return;
            }
            
            if ([strongSelf isViewLoaded] && !viewImage) {
                ((UIImageView *)strongSelf.view).image = image;
                strongSelf.imageDownloadFinish = YES;
                [strongSelf readyToRender];
            } else if (strongSelf->_isCompositingChild) {
                strongSelf->_image = image;
                strongSelf.imageDownloadFinish = YES;
            }
        });
    }];
}

- (void)updateContentImageWithFailedBlock:(void(^)(NSString *, NSError *))downloadFailedBlock
{
    NSString *imageSrc = self.imageSrc;
    
    if ([WXUtility isBlankString:imageSrc]) {
        WXLogError(@"image src is empty");
        return;
    }
    
    WXLogDebug(@"Updating image:%@, component:%@", self.imageSrc, self.ref);
    NSDictionary *userInfo = @{@"imageQuality":@(self.imageQuality), @"imageSharp":@(self.imageSharp), @"blurRadius":@(self.blurRadius)};
    NSString * newURL = [imageSrc copy];
    WX_REWRITE_URL(imageSrc, WXResourceTypeImage, self.weexInstance)
    __weak typeof(self) weakSelf = self;
    dispatch_async(dispatch_get_main_queue(), ^{
        weakSelf.imageOperation = [[weakSelf imageLoader] downloadImageWithURL:newURL imageFrame:weakSelf.calculatedFrame userInfo:userInfo completed:^(UIImage *image, NSError *error, BOOL finished) {
            dispatch_async(dispatch_get_main_queue(), ^{
                __strong typeof(self) strongSelf = weakSelf;
                
                if (strongSelf.imageLoadEvent) {
                    NSMutableDictionary *sizeDict = [NSMutableDictionary new];
                    sizeDict[@"naturalWidth"] = @0;
                    sizeDict[@"naturalHeight"] = @0;
                    if (!error) {
                        sizeDict[@"naturalWidth"] = @(image.size.width * image.scale);
                        sizeDict[@"naturalHeight"] = @(image.size.height * image.scale);
                    } else {
                        [sizeDict setObject:[error description]?:@"" forKey:@"errorDesc"];
                    }
                    [strongSelf fireEvent:@"load" params:@{ @"success": error? @false : @true,@"size":sizeDict}];
                }
                if (error) {
                    downloadFailedBlock(imageSrc, error);
                    [strongSelf readyToRender];
                    return ;
                }
                
                if (![imageSrc isEqualToString:strongSelf.imageSrc]) {
                    return ;
                }
                
                if ([strongSelf isViewLoaded]) {
                    strongSelf.imageDownloadFinish = YES;
                    ((UIImageView *)strongSelf.view).image = image;
                    [strongSelf readyToRender];
                } else if (strongSelf->_isCompositingChild) {
                    strongSelf.imageDownloadFinish = YES;
                    strongSelf->_image = image;
                    [strongSelf setNeedsDisplay];
                }
            });
        }];
    });
}

- (void)readyToRender
{
    // when image download completely (success or failed)
    if (self.weexInstance.trackComponent && _imageDownloadFinish) {
        [super readyToRender];
    }
}

- (void)cancelImage
{
    [_imageOperation cancel];
    _imageOperation = nil;
    [_placeholderOperation cancel];
    _placeholderOperation = nil;
}

- (id<WXImgLoaderProtocol>)imageLoader
{
    static id<WXImgLoaderProtocol> imageLoader;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        imageLoader = [WXHandlerFactory handlerForProtocol:@protocol(WXImgLoaderProtocol)];
    });
    return imageLoader;
}

- (void)_clipsToBounds
{
    WXRoundedRect *borderRect = [[WXRoundedRect alloc] initWithRect:self.view.bounds topLeft:_borderTopLeftRadius topRight:_borderTopRightRadius bottomLeft:_borderBottomLeftRadius bottomRight:_borderBottomRightRadius];
    // here is computed radii, do not use original style
    WXRadii *radii = borderRect.radii;
    
    if ([radii radiusesAreEqual]) {
        return;
    }
    
    CGFloat topLeft = radii.topLeft, topRight = radii.topRight, bottomLeft = radii.bottomLeft, bottomRight = radii.bottomRight;
    
    // clip to border radius
    UIBezierPath *bezierPath = [UIBezierPath wx_bezierPathWithRoundedRect:self.view.bounds topLeft:topLeft topRight:topRight bottomLeft:bottomLeft bottomRight:bottomRight];
    
    CAShapeLayer *shapeLayer = [CAShapeLayer layer];
    shapeLayer.path = bezierPath.CGPath;
    self.layer.mask = shapeLayer;
    self.layer.cornerRadius = 0;
}

#ifdef UITEST
- (NSString *)description
{
    NSString *superDescription = super.description;
    NSRange semicolonRange = [superDescription rangeOfString:@";"];
    NSString *replacement = [NSString stringWithFormat:@"; imageSrc: %@; imageQuality: %@; imageSharp: %@; ",_imageSrc,_imageQuality,_imageSharp];
    return [superDescription stringByReplacingCharactersInRange:semicolonRange withString:replacement];
}
#endif

@end
