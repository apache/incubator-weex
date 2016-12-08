//
//  WXImageUtils.m
//  WeexSDK
//
//  Created by xiayun on 16/12/8.
//  Copyright © 2016年 taobao. All rights reserved.
//

#import "WXImageUtils.h"

@implementation WXImageUtils

+ (UIImage *)toGaussianBluredImage:(UIImage *)originalImage blurRadius:(CGFloat)blurRadius {
    CIImage *imageToBlur = [CIImage imageWithCGImage:originalImage.CGImage];
    
    CIFilter *gaussianBlurFilter = [CIFilter filterWithName:@"CIGaussianBlur"];
    [gaussianBlurFilter setValue:imageToBlur forKey:kCIInputImageKey];
    [gaussianBlurFilter setValue:[NSNumber numberWithFloat:blurRadius] forKey:kCIInputRadiusKey];
    CIImage *ciImage = gaussianBlurFilter.outputImage;
    
    CIContext *context = [CIContext contextWithOptions:nil];
    CGRect rect = CGRectMake(0, 0, originalImage.size.width * originalImage.scale, originalImage.size.height * originalImage.scale);
    CGImageRef cgImage = [context createCGImage:ciImage fromRect:rect];;
    
    if (cgImage) {
        UIImage *resultImage = [UIImage imageWithCGImage:cgImage];
        CGImageRelease(cgImage);
        
        return resultImage;
    }
    
    return nil;
}

@end
