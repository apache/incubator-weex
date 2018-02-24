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

#import "WXTransform.h"
#import "math.h"
#import "WXLength.h"
#import "WXUtility.h"
#import "WXSDKInstance.h"
#import "WXConvert.h"

@interface WXTransform()

@property (nonatomic, weak) WXSDKInstance *weexInstance;

@end

@implementation WXTransform
{
    float _rotateAngle; //for rotate
    float _scaleX;
    float _scaleY;
    WXLength *_translateX;
    WXLength *_translateY;
    WXLength *_originX;
    WXLength *_originY;
    
    //3d rotate
    float _rotateX;
    float _rotateY;
    float _rotateZ;
    float _perspective;
    
    CATransform3D _nativeTransform;
    BOOL _useNativeTransform;
}

- (instancetype)initWithCSSValue:(NSString *)cssValue origin:(NSString *)origin instance:(WXSDKInstance *)instance
{
    if (self = [super init]) {
        _weexInstance = instance;
        _scaleX = 1.0;
        _scaleY = 1.0;
        _rotateX = 0.0;
        _rotateY = 0.0;
        _rotateZ = 0.0;
        _rotateAngle = 0.0;
        
        // default is parallel projection
        _perspective = CGFLOAT_MAX;
        
        [self parseTransform:cssValue];
        [self parseTransformOrigin:origin];
    }
    
    return self;
}

- (instancetype)initWithNativeTransform:(CATransform3D)transform instance:(WXSDKInstance *)instance
{
    if (self = [super init]) {
        _weexInstance = instance;
        _nativeTransform = transform;
        _useNativeTransform = YES;
    }
    return self;
}

- (float)rotateAngle
{
    if (_useNativeTransform) {
        return atan2(_nativeTransform.m11, _nativeTransform.m12);
    }
    return _rotateAngle;
}

- (float)rotateX
{
    if (_useNativeTransform) {
        return atan2(_nativeTransform.m22, _nativeTransform.m23);
    }
    return _rotateX;
}

- (float)rotateY
{
    if (_useNativeTransform) {
        return atan2(_nativeTransform.m11, _nativeTransform.m31);
    }
    return _rotateY;
}

- (float)rotateZ
{
    return _rotateZ;
}

- (WXLength *)translateX
{
    if (_useNativeTransform) {
        return [WXLength lengthWithFloat:_nativeTransform.m41 type:WXLengthTypeFixed];
    }
    return _translateX;
}

- (WXLength *)translateY
{
    if (_useNativeTransform) {
        return [WXLength lengthWithFloat:_nativeTransform.m42 type:WXLengthTypeFixed];
    }
    return _translateY;
}

- (float)scaleX
{
    if (_useNativeTransform) {
        return sqrt(_nativeTransform.m11 * _nativeTransform.m11 + _nativeTransform.m21 * _nativeTransform.m21);
    }
    return _scaleX;
}

- (float)scaleY
{
    if (_useNativeTransform) {
        return sqrt(_nativeTransform.m12 * _nativeTransform.m12 + _nativeTransform.m22 * _nativeTransform.m22);
    }
    return _scaleY;
}

- (void)setTransformOrigin:(NSString *)transformOriginCSS
{
    [self parseTransformOrigin:transformOriginCSS];
}

- (CATransform3D)nativeTransformWithView:(UIView *)view
{
    if (_useNativeTransform) {
        return _nativeTransform;
    }
    
    CATransform3D nativeTransform3d = [self nativeTransformWithoutRotateWithView:view];

    if (_rotateAngle != 0 || _rotateZ != 0) {
        nativeTransform3d = CATransform3DRotate(nativeTransform3d, _rotateAngle?:_rotateZ, 0, 0, 1);
    }
    
    if (_rotateY != 0) {
        nativeTransform3d = CATransform3DRotate(nativeTransform3d, _rotateY, 0, 1, 0);
    }
    
    if (_rotateX != 0) {
        nativeTransform3d = CATransform3DRotate(nativeTransform3d, _rotateX, 1, 0, 0);
    }
    
    return nativeTransform3d;
}

- (CATransform3D)nativeTransformWithoutRotateWithView:(UIView *)view
{
    CATransform3D nativeTansform3D = CATransform3DIdentity;
    
    // CGFLOAT_MAX is not INF on 32-bit device
    if(_perspective && _perspective != CGFLOAT_MAX && !isinf(_perspective)) {
        nativeTansform3D.m34 = -1.0/_perspective;
    }
    if (!view || view.bounds.size.width <= 0 || view.bounds.size.height <= 0) {
        return nativeTansform3D;
    }
    
    if (_translateX || _translateY) {
        
        nativeTansform3D = CATransform3DTranslate(nativeTansform3D, _translateX ? [_translateX valueForMaximum:view.bounds.size.width] : 0, _translateY ? [_translateY valueForMaximum:view.bounds.size.height]:0, 0);
    }
    nativeTansform3D = CATransform3DScale(nativeTansform3D, _scaleX, _scaleY, 1.0);
    
    return nativeTansform3D;
}

-(void)setAnchorPoint:(CGPoint)anchorPoint forView:(UIView *)view
{
    CGPoint newPoint = CGPointMake(view.bounds.size.width * anchorPoint.x,
                                   view.bounds.size.height * anchorPoint.y);
    CGPoint oldPoint = CGPointMake(view.bounds.size.width * view.layer.anchorPoint.x,
                                   view.bounds.size.height * view.layer.anchorPoint.y);
    
    newPoint = CGPointApplyAffineTransform(newPoint, view.transform);
    oldPoint = CGPointApplyAffineTransform(oldPoint, view.transform);
    
    CGPoint position = view.layer.position;
    
    position.x -= oldPoint.x;
    position.x += newPoint.x;
    
    position.y -= oldPoint.y;
    position.y += newPoint.y;
    
    view.layer.position = position;
    view.layer.anchorPoint = anchorPoint;
}


- (void)applyTransformForView:(UIView *)view
{
    if (!view || view.bounds.size.width <= 0 || view.bounds.size.height <= 0) {
        return;
    }
    
    BOOL applyTransformOrigin = _originX || _originY;
    if (applyTransformOrigin) {
        /**
          * Waiting to fix the issue that transform-origin behaves in rotation
          * http://ronnqvi.st/translate-rotate-translate/
          **/
        CGPoint anchorPoint = CGPointMake(
                                          _originX ? [_originX valueForMaximum:view.bounds.size.width] / view.bounds.size.width : 0.5,
                                          _originY ? [_originY valueForMaximum:view.bounds.size.height] / view.bounds.size.height : 0.5);
        [self setAnchorPoint:anchorPoint forView:view];
    }
    CATransform3D nativeTransform3d = [self nativeTransformWithView:view];
    if (!CATransform3DEqualToTransform(view.layer.transform, nativeTransform3d)){
        view.layer.transform = nativeTransform3d;
    }
}

- (void)parseTransform:(NSString *)cssValue
{
    if (!cssValue || cssValue.length == 0 || [cssValue isEqualToString:@"none"]) {
        return;
    }
    
    NSError *error = NULL;
    NSRegularExpression *regex = [NSRegularExpression regularExpressionWithPattern:@"(\\w+)\\((.+?)\\)"
                                                                           options:NSRegularExpressionCaseInsensitive
                                                                             error:&error];
    
    NSArray *matches = [regex matchesInString:cssValue options:0 range:NSMakeRange(0, cssValue.length)];
    
    for (NSTextCheckingResult *match in matches) {
        NSString *name = [cssValue substringWithRange:[match rangeAtIndex:1]];
        NSArray *value = [[cssValue substringWithRange:[match rangeAtIndex:2]] componentsSeparatedByString:@","];
        
        SEL method = NSSelectorFromString([NSString stringWithFormat:@"parse%@:", [name capitalizedString]]);
        if ([self respondsToSelector:method]) {
            @try {
                [self performSelectorOnMainThread:method withObject:value waitUntilDone:YES];
            }
            @catch (NSException *exception) {
                WXLogError(@"WXTransform exception:%@", [exception reason]);
            }
        }
    }
}

- (void)parseTransformOrigin:(NSString *)cssValue
{
    if (!cssValue || cssValue.length == 0 || [cssValue isEqualToString:@"none"]) {
        return;
    }
    
    NSArray *values = [cssValue componentsSeparatedByString:@" "];

    double originX = 50;
    double originY = 50;
    WXLengthType typeX = WXLengthTypePercent;
    WXLengthType typeY = WXLengthTypePercent;
    for (NSInteger i = 0; i < values.count; i++) {
        NSString *value = values[i];
        if ([value isEqualToString:@"left"]) {
            originX = 0;
        } else if ([value isEqualToString:@"right"]) {
            originX = 100;
        } else if ([value isEqualToString:@"top"]) {
            originY = 0;
        } else if ([value isEqualToString:@"bottom"]) {
            originY = 100;
        } else if ([value isEqualToString:@"center"]) {
            if (i == 0) {
                originX = 50;
            } else {
                originY = 50;
            }
        } else {
            double val = [value doubleValue];
            if (i == 0) {
                if ([value hasSuffix:@"%"]) {
                    originX = val;
                } else {
                    typeX = WXLengthTypeFixed;
                    originX = WXPixelScale(val, self.weexInstance.pixelScaleFactor);
                }
            } else {
                if ([value hasSuffix:@"%"]) {
                    originY = val;
                } else {
                    typeY = WXLengthTypeFixed;
                    originY = WXPixelScale(val, self.weexInstance.pixelScaleFactor);
                }
            }
        }
    }
    _originX = [WXLength lengthWithFloat:originX type:typeX];
    _originY = [WXLength lengthWithFloat:originY type:typeY];
}

- (void)parseRotate:(NSArray *)value
{
    float rotateAngle = [self getAngle:value[0]];
    _rotateAngle = rotateAngle;
}

- (void)parseRotatex:(NSArray *)value
{
    _rotateX = [self getAngle:value[0]];
}

- (void)parseRotatey:(NSArray *)value
{
    _rotateY = [self getAngle:value[0]];
}

- (void)parseRotatez:(NSArray *)value
{
   _rotateZ = [self getAngle:value[0]];
}

- (void)parsePerspective:(NSArray *)value
{
    _perspective = [WXConvert WXPixelType:value[0] scaleFactor:self.weexInstance.pixelScaleFactor];
    if (_perspective <= 0 ) {
        _perspective = CGFLOAT_MAX;
    }
}

- (void)parseTranslate:(NSArray *)value
{
    [self parseTranslatex:@[value[0]]];
    if (value.count > 1) {
        [self parseTranslatey:@[value[1]]];
    }
}

- (void)parseTranslatex:(NSArray *)value
{
    WXLength *translateX;
    double x = [value[0] doubleValue];
    if ([value[0] hasSuffix:@"%"]) {
        translateX = [WXLength lengthWithFloat:x type:WXLengthTypePercent];
    } else {
        x = WXPixelScale(x, self.weexInstance.pixelScaleFactor);
        translateX = [WXLength lengthWithFloat:x type:WXLengthTypeFixed];
    }
    _translateX = translateX;
}

- (void)parseTranslatey:(NSArray *)value
{
    WXLength *translateY;
    double y = [value[0] doubleValue];
    if ([value[0] hasSuffix:@"%"]) {
        translateY = [WXLength lengthWithFloat:y type:WXLengthTypePercent];
    } else {
        y = WXPixelScale(y, self.weexInstance.pixelScaleFactor);
        translateY = [WXLength lengthWithFloat:y type:WXLengthTypeFixed];
    }
    _translateY = translateY;
}

- (void)parseScale:(NSArray *)value
{
    double x = [value[0] doubleValue];
    double y = x;
    if (value.count == 2) {
        y = [value[1] doubleValue];
    }
    _scaleX = x;
    _scaleY = y;
}

- (void)parseScalex:(NSArray *)value
{
    [self parseScale:@[value[0], @1]];
}

- (void)parseScaley:(NSArray *)value
{
    [self parseScale:@[@1, value[0]]];
}

// Angle in radians
- (double)getAngle:(NSString *)value
{
    double angle = [value doubleValue];
    if ([value hasSuffix:@"deg"]) {
        return [self deg2rad:angle];
    } else {
        return angle;
    }
}

- (double)deg2rad:(double)deg
{
    return deg * M_PI / 180.0;
}

@end
