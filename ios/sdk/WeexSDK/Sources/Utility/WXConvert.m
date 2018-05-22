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

#import "WXConvert.h"
#import "WXUtility.h"
#import "WXBoxShadow.h"
#import "WXLength.h"
#import "WXAssert.h"
#import "WXSDKEngine.h"

@implementation WXConvert

#pragma mark Number & String & Collection

#define WX_NUMBER_CONVERT(type, op) \
+ (type)type:(id)value {\
    if([value respondsToSelector:@selector(op)]){\
        return (type)[value op];\
    } else {\
        NSString * strval = [NSString stringWithFormat:@"%@",value];\
        return (type)[self uint64_t: strval];\
    }\
}

WX_NUMBER_CONVERT(BOOL, boolValue)
WX_NUMBER_CONVERT(int, intValue)
WX_NUMBER_CONVERT(short, shortValue)
WX_NUMBER_CONVERT(int64_t, longLongValue)
WX_NUMBER_CONVERT(uint8_t, unsignedShortValue)
WX_NUMBER_CONVERT(uint16_t, unsignedIntValue)
WX_NUMBER_CONVERT(uint32_t, unsignedLongValue)
WX_NUMBER_CONVERT(float, floatValue)
WX_NUMBER_CONVERT(double, doubleValue)
WX_NUMBER_CONVERT(NSInteger, integerValue)
WX_NUMBER_CONVERT(NSUInteger, unsignedIntegerValue)



//unsignedLongLongValue
+ (uint64_t)uint64_t:(id)value {\
    NSString * strval = [NSString stringWithFormat:@"%@",value];
    unsigned long long ullvalue = strtoull([strval UTF8String], NULL, 10);
    return ullvalue;
}

+ (CGFloat)CGFloat:(id)value
{
    if ([value isKindOfClass:[NSString class]]) {
        NSString *valueString = (NSString *)value;
        if ([valueString hasSuffix:@"px"] || [valueString hasSuffix:@"wx"]) {
            valueString = [valueString substringToIndex:(valueString.length - 2)];
        }
        if ([value hasPrefix:@"env(safe-area-inset-"] &&[value hasSuffix:@")"]){
            NSUInteger start = [value rangeOfString:@"env(safe-area-inset-"].location +@"env(safe-area-inset-".length;
            NSUInteger end = [value rangeOfString:@")" options:NSBackwardsSearch].location;
            value = [value substringWithRange:NSMakeRange(start, end-start)];
            return [self safeAreaInset:value];
        }
        return [valueString doubleValue];
    }
    
    return [self double:value];
}

+ (CGFloat)flexCGFloat:(id)value
{
    if ([value isKindOfClass:[NSString class]]) {
        NSString *valueString = (NSString *)value;
        if (valueString.length <=0) {
            return NAN;
        }
        if ([valueString hasSuffix:@"px"] || [valueString hasSuffix:@"wx"]) {
            valueString = [valueString substringToIndex:(valueString.length - 2)];
        }
        if ([value hasPrefix:@"env(safe-area-inset-"] &&[value hasSuffix:@")"]){
            NSUInteger start = [value rangeOfString:@"env(safe-area-inset-"].location +@"env(safe-area-inset-".length;
            NSUInteger end = [value rangeOfString:@")" options:NSBackwardsSearch].location;
            value = [value substringWithRange:NSMakeRange(start, end-start)];
            return [self safeAreaInset:value];
        }
        //value maybe not number ,such as 100%
        if (![WXConvert checkStringIsRealNum:valueString]) {
            return NAN;
        }
        return [valueString doubleValue];
    }
    return [self double:value];
}

+ (BOOL)checkStringIsRealNum:(NSString *)checkedNumString {
    NSScanner* scan = [NSScanner scannerWithString:checkedNumString];
    int intVal;
    BOOL isInt = [scan scanInt:&intVal] && [scan isAtEnd];
    if (isInt) {
        return YES;
    }
    float floatVal;
    BOOL isFloat = [scan scanFloat:&floatVal] && [scan isAtEnd];
    if (isFloat) {
        return YES;
    }
    
    return NO;
}

+ (CGFloat)safeAreaInset:(NSString*)value
{
    static NSArray * directionArray = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        directionArray = [NSArray arrayWithObjects:@"top",@"right",@"bottom",@"left", nil];
    });
    if ([directionArray containsObject:value]) {
        __block UIEdgeInsets safeAreaInsets = UIEdgeInsetsZero;
#if __IPHONE_11_0
        if (@available(iOS 11.0, *)) {
            WXSDKInstance * topInstance = [WXSDKEngine topInstance];
            WXPerformBlockSyncOnMainThread(^{
                safeAreaInsets = topInstance.rootView.safeAreaInsets;
            });
            
        } else {
            // Fallback on earlier versions
        }
#endif
        NSUInteger key = [directionArray indexOfObject:value];
        CGFloat retValue = 0;
        switch (key) {
            case 0:
                retValue = safeAreaInsets.top;
                break;
            case 1:
                retValue = safeAreaInsets.right;
                break;
            case 2:
                retValue = safeAreaInsets.bottom;
                break;
            case 3:
                retValue = safeAreaInsets.left;
                break;
            default:
                break;
        }
        return retValue;
    }
    return 0;
}

+ (NSString *)NSString:(id)value
{
    if([value isKindOfClass:[NSString class]]){
        return value;
    } else if([value isKindOfClass:[NSNumber class]]){
        return [((NSNumber *)value) stringValue];
    } else if (value != nil) {
        WXLogError(@"Convert Error:%@ can not be converted to string", value);
    }
    
    return nil;
}

+ (WXPixelType)WXPixelType:(id)value scaleFactor:(CGFloat)scaleFactor
{
    CGFloat pixel = [self CGFloat:value];
    
    if ([value isKindOfClass:[NSString class]] && ([value hasSuffix:@"wx"]|| [value hasPrefix:@"env(safe-area-inset-"])) {
        return pixel;
    }
    return pixel * scaleFactor;
}

+ (WXPixelType)WXFlexPixelType:(id)value scaleFactor:(CGFloat)scaleFactor
{
    CGFloat pixel = [self flexCGFloat:value];
    
    if ([value isKindOfClass:[NSString class]] && ([value hasSuffix:@"wx"]|| [value hasPrefix:@"env(safe-area-inset-"])) {
        return pixel;
    }
    return pixel * scaleFactor;
}

#pragma mark Style

+ (UIColor *)UIColor:(id)value
{
    // 1. check cache
    static NSCache *colorCache;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        colorCache = [[NSCache alloc] init];
        colorCache.countLimit = 64;
    });
    
    if ([value isKindOfClass:[NSNull class]] || !value) {
        return nil;
    }
    
    UIColor *color = [colorCache objectForKey:value];
    if (color) {
        return color;
    }
    
    // Default color is white
    double red = 255, green = 255, blue = 255, alpha = 1.0;
    
    if([value isKindOfClass:[NSString class]]){
        // 2. check if is color keyword or transparent
        static NSDictionary *knownColors;
        static dispatch_once_t onceToken;
        dispatch_once(&onceToken, ^{
            knownColors = @{
                            // https://www.w3.org/TR/css3-color/#svg-color
                            @"aliceblue": @"#f0f8ff",
                            @"antiquewhite": @"#faebd7",
                            @"aqua": @"#00ffff",
                            @"aquamarine": @"#7fffd4",
                            @"azure": @"#f0ffff",
                            @"beige": @"#f5f5dc",
                            @"bisque": @"#ffe4c4",
                            @"black": @"#000000",
                            @"blanchedalmond": @"#ffebcd",
                            @"blue": @"#0000ff",
                            @"blueviolet": @"#8a2be2",
                            @"brown": @"#a52a2a",
                            @"burlywood": @"#deb887",
                            @"cadetblue": @"#5f9ea0",
                            @"chartreuse": @"#7fff00",
                            @"chocolate": @"#d2691e",
                            @"coral": @"#ff7f50",
                            @"cornflowerblue": @"#6495ed",
                            @"cornsilk": @"#fff8dc",
                            @"crimson": @"#dc143c",
                            @"cyan": @"#00ffff",
                            @"darkblue": @"#00008b",
                            @"darkcyan": @"#008b8b",
                            @"darkgoldenrod": @"#b8860b",
                            @"darkgray": @"#a9a9a9",
                            @"darkgrey": @"#a9a9a9",
                            @"darkgreen": @"#006400",
                            @"darkkhaki": @"#bdb76b",
                            @"darkmagenta": @"#8b008b",
                            @"darkolivegreen": @"#556b2f",
                            @"darkorange": @"#ff8c00",
                            @"darkorchid": @"#9932cc",
                            @"darkred": @"#8b0000",
                            @"darksalmon": @"#e9967a",
                            @"darkseagreen": @"#8fbc8f",
                            @"darkslateblue": @"#483d8b",
                            @"darkslategray": @"#2f4f4f",
                            @"darkslategrey": @"#2f4f4f",
                            @"darkturquoise": @"#00ced1",
                            @"darkviolet": @"#9400d3",
                            @"deeppink": @"#ff1493",
                            @"deepskyblue": @"#00bfff",
                            @"dimgray": @"#696969",
                            @"dimgrey": @"#696969",
                            @"dodgerblue": @"#1e90ff",
                            @"firebrick": @"#b22222",
                            @"floralwhite": @"#fffaf0",
                            @"forestgreen": @"#228b22",
                            @"fuchsia": @"#ff00ff",
                            @"gainsboro": @"#dcdcdc",
                            @"ghostwhite": @"#f8f8ff",
                            @"gold": @"#ffd700",
                            @"goldenrod": @"#daa520",
                            @"gray": @"#808080",
                            @"grey": @"#808080",
                            @"green": @"#008000",
                            @"greenyellow": @"#adff2f",
                            @"honeydew": @"#f0fff0",
                            @"hotpink": @"#ff69b4",
                            @"indianred": @"#cd5c5c",
                            @"indigo": @"#4b0082",
                            @"ivory": @"#fffff0",
                            @"khaki": @"#f0e68c",
                            @"lavender": @"#e6e6fa",
                            @"lavenderblush": @"#fff0f5",
                            @"lawngreen": @"#7cfc00",
                            @"lemonchiffon": @"#fffacd",
                            @"lightblue": @"#add8e6",
                            @"lightcoral": @"#f08080",
                            @"lightcyan": @"#e0ffff",
                            @"lightgoldenrodyellow": @"#fafad2",
                            @"lightgray": @"#d3d3d3",
                            @"lightgrey": @"#d3d3d3",
                            @"lightgreen": @"#90ee90",
                            @"lightpink": @"#ffb6c1",
                            @"lightsalmon": @"#ffa07a",
                            @"lightseagreen": @"#20b2aa",
                            @"lightskyblue": @"#87cefa",
                            @"lightslategray": @"#778899",
                            @"lightslategrey": @"#778899",
                            @"lightsteelblue": @"#b0c4de",
                            @"lightyellow": @"#ffffe0",
                            @"lime": @"#00ff00",
                            @"limegreen": @"#32cd32",
                            @"linen": @"#faf0e6",
                            @"magenta": @"#ff00ff",
                            @"maroon": @"#800000",
                            @"mediumaquamarine": @"#66cdaa",
                            @"mediumblue": @"#0000cd",
                            @"mediumorchid": @"#ba55d3",
                            @"mediumpurple": @"#9370db",
                            @"mediumseagreen": @"#3cb371",
                            @"mediumslateblue": @"#7b68ee",
                            @"mediumspringgreen": @"#00fa9a",
                            @"mediumturquoise": @"#48d1cc",
                            @"mediumvioletred": @"#c71585",
                            @"midnightblue": @"#191970",
                            @"mintcream": @"#f5fffa",
                            @"mistyrose": @"#ffe4e1",
                            @"moccasin": @"#ffe4b5",
                            @"navajowhite": @"#ffdead",
                            @"navy": @"#000080",
                            @"oldlace": @"#fdf5e6",
                            @"olive": @"#808000",
                            @"olivedrab": @"#6b8e23",
                            @"orange": @"#ffa500",
                            @"orangered": @"#ff4500",
                            @"orchid": @"#da70d6",
                            @"palegoldenrod": @"#eee8aa",
                            @"palegreen": @"#98fb98",
                            @"paleturquoise": @"#afeeee",
                            @"palevioletred": @"#db7093",
                            @"papayawhip": @"#ffefd5",
                            @"peachpuff": @"#ffdab9",
                            @"peru": @"#cd853f",
                            @"pink": @"#ffc0cb",
                            @"plum": @"#dda0dd",
                            @"powderblue": @"#b0e0e6",
                            @"purple": @"#800080",
                            @"rebeccapurple": @"#663399",
                            @"red": @"#ff0000",
                            @"rosybrown": @"#bc8f8f",
                            @"royalblue": @"#4169e1",
                            @"saddlebrown": @"#8b4513",
                            @"salmon": @"#fa8072",
                            @"sandybrown": @"#f4a460",
                            @"seagreen": @"#2e8b57",
                            @"seashell": @"#fff5ee",
                            @"sienna": @"#a0522d",
                            @"silver": @"#c0c0c0",
                            @"skyblue": @"#87ceeb",
                            @"slateblue": @"#6a5acd",
                            @"slategray": @"#708090",
                            @"slategrey": @"#708090",
                            @"snow": @"#fffafa",
                            @"springgreen": @"#00ff7f",
                            @"steelblue": @"#4682b4",
                            @"tan": @"#d2b48c",
                            @"teal": @"#008080",
                            @"thistle": @"#d8bfd8",
                            @"tomato": @"#ff6347",
                            @"turquoise": @"#40e0d0",
                            @"violet": @"#ee82ee",
                            @"wheat": @"#f5deb3",
                            @"white": @"#ffffff",
                            @"whitesmoke": @"#f5f5f5",
                            @"yellow": @"#ffff00",
                            @"yellowgreen": @"#9acd32",
                            
                            // https://www.w3.org/TR/css3-color/#transparent
                            @"transparent": @"rgba(0,0,0,0)",
                            };
        });
        NSString *rgba = knownColors[value];
        if (!rgba) {
            rgba = value;
        }
        
        if ([rgba hasPrefix:@"#"]) {
            // #fff
            if ([rgba length] == 4) {
              unichar f =   [rgba characterAtIndex:1];
              unichar s =   [rgba characterAtIndex:2];
              unichar t =   [rgba characterAtIndex:3];
              rgba = [NSString stringWithFormat:@"#%C%C%C%C%C%C", f, f, s, s, t, t];
            }
            
            // 3. #rrggbb
            uint32_t colorValue = 0;
            sscanf(rgba.UTF8String, "#%x", &colorValue);
            red     = ((colorValue & 0xFF0000) >> 16) / 255.0;
            green   = ((colorValue & 0x00FF00) >> 8) / 255.0;
            blue    = (colorValue & 0x0000FF) / 255.0;
        } else if ([rgba hasPrefix:@"rgb("]) {
            // 4. rgb(r,g,b)
            int r,g,b;
            sscanf(rgba.UTF8String, "rgb(%d,%d,%d)", &r, &g, &b);
            red = r / 255.0;
            green = g / 255.0;
            blue = b / 255.0;
        } else if ([rgba hasPrefix:@"rgba("]) {
            // 5. rgba(r,g,b,a)
            int r,g,b;
            sscanf(rgba.UTF8String, "rgba(%d,%d,%d,%lf)", &r, &g, &b, &alpha);
            red = r / 255.0;
            green = g / 255.0;
            blue = b / 255.0;
        }
        
    } else if([value isKindOfClass:[NSNumber class]]) {
        NSUInteger colorValue = [value unsignedIntegerValue];
        red     = ((colorValue & 0xFF0000) >> 16) / 255.0;
        green   = ((colorValue & 0x00FF00) >> 8) / 255.0;
        blue    = (colorValue & 0x0000FF) / 255.0;
    }
    
    color = [UIColor colorWithRed:red green:green blue:blue alpha:alpha];
    // 6. cache color
    if (color && value) {
        [colorCache setObject:color forKey:value];
    }
    
    return color;
}

+ (CGColorRef)CGColor:(id)value
{
    UIColor *color = [self UIColor:value];
    return [color CGColor];
}

+ (NSString *)HexWithColor:(UIColor *)color
{
    uint hex;
    CGFloat red, green, blue, alpha;
    if (![color getRed:&red green:&green blue:&blue alpha:&alpha]) {
        [color getWhite:&red alpha:&alpha];
        green = red;
        blue = red;
    }
    red = roundf(red * 255.f);
    green = roundf(green * 255.f);
    blue = roundf(blue * 255.f);
    alpha = roundf(alpha * 255.f);
    hex =  ((uint)red << 16) | ((uint)green << 8) | ((uint)blue);
    return [NSString stringWithFormat:@"#%02x", hex];
}

+ (WXBorderStyle)WXBorderStyle:(id)value
{
    if([value isKindOfClass:[NSString class]]){
        if ([value isEqualToString:@"solid"]) {
            return WXBorderStyleSolid;
        } else if ([value isEqualToString:@"dotted"]) {
            return WXBorderStyleDotted;
        } else if ([value isEqualToString:@"dashed"]) {
            return WXBorderStyleDashed;
        }
    }
    
    return WXBorderStyleSolid;
}

+ (WXClipType)WXClipType:(id)value
{
    if([value isKindOfClass:[NSString class]]) {
        NSString *string = (NSString *)value;
        if ([string isEqualToString:@"visible"]) {
            return NO;
        } else if ([string isEqualToString:@"hidden"]) {
            return YES;
        }
    }
    
    return NO;
}

+ (WXPositionType)WXPositionType:(id)value
{
    if([value isKindOfClass:[NSString class]]){
        if ([value isEqualToString:@"relative"]) {
            return WXPositionTypeRelative;
        } else if ([value isEqualToString:@"absolute"]) {
            return WXPositionTypeAbsolute;
        } else if ([value isEqualToString:@"sticky"]) {
            return WXPositionTypeSticky;
        } else if ([value isEqualToString:@"fixed"]) {
            return WXPositionTypeFixed;
        }
    }
    
    return WXPositionTypeRelative;
}

#pragma mark Text

+ (NSTextAlignment)NSTextAlignment:(id)value
{
    if([value isKindOfClass:[NSString class]]){
        NSString *string = (NSString *)value;
        if ([string isEqualToString:@"left"])
            return NSTextAlignmentLeft;
        else if ([string isEqualToString:@"center"])
            return NSTextAlignmentCenter;
        else if ([string isEqualToString:@"right"])
            return NSTextAlignmentRight;
    }
    return NSTextAlignmentNatural;
}

+ (UIReturnKeyType)UIReturnKeyType:(id)value
{
    if([value isKindOfClass:[NSString class]]){
        NSString *string = (NSString *)value;
        if ([string isEqualToString:@"defalut"])
            return UIReturnKeyDefault;
        else if ([string isEqualToString:@"go"])
            return UIReturnKeyGo;
        else if ([string isEqualToString:@"next"])
            return UIReturnKeyNext;
        else if ([string isEqualToString:@"search"])
            return UIReturnKeySearch;
        else if ([string isEqualToString:@"send"])
            return UIReturnKeySend;
        else if ([string isEqualToString:@"done"])
            return UIReturnKeyDone;
    }
    return UIReturnKeyDefault;
}

+ (WXTextStyle)WXTextStyle:(id)value
{
    if([value isKindOfClass:[NSString class]]){
        NSString *string = (NSString *)value;
        if ([string isEqualToString:@"normal"])
            return WXTextStyleNormal;
        else if ([string isEqualToString:@"italic"])
            return WXTextStyleItalic;
    }
    return WXTextStyleNormal;
}

+ (CGFloat)WXTextWeight:(id)value
{
    NSString *string = [WXConvert NSString:value];
    if (!string)
        return WX_SYS_VERSION_LESS_THAN(@"8.2")?0:UIFontWeightRegular;
    else if ([string isEqualToString:@"normal"])
        return WX_SYS_VERSION_LESS_THAN(@"8.2")?0:UIFontWeightRegular;
    else if ([string isEqualToString:@"bold"])
        return WX_SYS_VERSION_LESS_THAN(@"8.2")?0.4:UIFontWeightBold;
    else if ([string isEqualToString:@"100"])
        return WX_SYS_VERSION_LESS_THAN(@"8.2")?-0.8:UIFontWeightUltraLight;
    else if ([string isEqualToString:@"200"])
        return WX_SYS_VERSION_LESS_THAN(@"8.2")?-0.6:UIFontWeightThin;
    else if ([string isEqualToString:@"300"])
        return WX_SYS_VERSION_LESS_THAN(@"8.2")?-0.4:UIFontWeightLight;
    else if ([string isEqualToString:@"400"])
        return WX_SYS_VERSION_LESS_THAN(@"8.2")?0:UIFontWeightRegular;
    else if ([string isEqualToString:@"500"])
        return WX_SYS_VERSION_LESS_THAN(@"8.2")?0.23:UIFontWeightMedium;
    else if ([string isEqualToString:@"600"])
        return WX_SYS_VERSION_LESS_THAN(@"8.2")?0.3:UIFontWeightSemibold;
    else if ([string isEqualToString:@"700"])
        return WX_SYS_VERSION_LESS_THAN(@"8.2")?0.4:UIFontWeightBold;
    else if ([string isEqualToString:@"800"])
        return WX_SYS_VERSION_LESS_THAN(@"8.2")?0.56:UIFontWeightHeavy;
    else if ([string isEqualToString:@"900"])
        return WX_SYS_VERSION_LESS_THAN(@"8.2")?0.62:UIFontWeightBlack;
        
    return WX_SYS_VERSION_LESS_THAN(@"8.2")?0:UIFontWeightRegular;
}

+ (WXTextDecoration)WXTextDecoration:(id)value
{
    if([value isKindOfClass:[NSString class]]){
        NSString *string = (NSString *)value;
        if ([string isEqualToString:@"none"])
            return WXTextDecorationNone;
        else if ([string isEqualToString:@"underline"])
            return WXTextDecorationUnderline;
        else if ([string isEqualToString:@"line-through"])
            return WXTextDecorationLineThrough;
    }
    return WXTextDecorationNone;
}

#pragma mark Image

+ (UIViewContentMode)UIViewContentMode:(id)value
{
    if([value isKindOfClass:[NSString class]]){
        NSString *string = (NSString *)value;
        if ([string isEqualToString:@"cover"])
            return UIViewContentModeScaleAspectFill;
        else if ([string isEqualToString:@"contain"])
            return UIViewContentModeScaleAspectFit;
        else if ([string isEqualToString:@"stretch"])
            return UIViewContentModeScaleToFill;
    }
    return UIViewContentModeScaleToFill;
}

+ (WXImageQuality)WXImageQuality:(id)value
{
    if([value isKindOfClass:[NSString class]]){
        
        NSString *string = (NSString *)value;
        if ([string isEqualToString:@"original"])
            return WXImageQualityOriginal;
        else if ([string isEqualToString:@"normal"])
            return  WXImageQualityNormal;
        else if ([string isEqualToString:@"low"])
            return  WXImageQualityLow;
        else if ([string isEqualToString:@"high"])
            return  WXImageQualityHigh;
    }
    
    return  WXImageQualityNone;
}

+ (WXImageSharp)WXImageSharp:(id)value
{
    if([value isKindOfClass:[NSString class]]){
        
        NSString *string = (NSString *)value;
        if ([string isEqualToString:@"sharpen"])
            return WXImageSharpening;
        else if ([string isEqualToString:@"unsharpen"])
            return WXImageSharpeningNone;
    }
    return  WXImageSharpeningNone;
}

#pragma mark Scroller

+ (WXScrollDirection)WXScrollDirection:(id)value
{
    if([value isKindOfClass:[NSString class]]){
        NSString *string = (NSString *)value;
        if ([string isEqualToString:@"none"])
            return WXScrollDirectionNone;
        else if ([string isEqualToString:@"vertical"])
            return WXScrollDirectionVertical;
        else if ([string isEqualToString:@"horizontal"])
            return WXScrollDirectionHorizontal;
    }
    return WXScrollDirectionVertical;
}

+ (UITableViewRowAnimation)UITableViewRowAnimation:(id)value
{
    if ([value isKindOfClass:[NSString class]]) {
        NSString *string = (NSString *)value;
        if ([string isEqualToString:@"none"]) {
            return UITableViewRowAnimationNone;
        } else if ([string isEqualToString:@"default"]) {
            return UITableViewRowAnimationFade;
        }
    }
    
    return UITableViewRowAnimationNone;
}

#pragma mark Animation

+ (UIViewAnimationOptions)UIViewAnimationTimingFunction:(id)value
{
    if (![value isKindOfClass:[NSString class]]) {
        return UIViewAnimationOptionCurveEaseInOut;
    }
    
    static NSDictionary *timingFunctionMapping;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        timingFunctionMapping = @{
                    @"ease-in":@(UIViewAnimationOptionCurveEaseIn),
                    @"ease-out":@(UIViewAnimationOptionCurveEaseOut),
                    @"ease-in-out":@(UIViewAnimationOptionCurveEaseInOut),
                    @"linear":@(UIViewAnimationOptionCurveLinear)
                    };
    });
    
    return [timingFunctionMapping[value] unsignedIntegerValue];
}

+ (CAMediaTimingFunction *)CAMediaTimingFunction:(id)value
{
    if (![value isKindOfClass:[NSString class]]) {
        return nil;
    }
    
    static NSDictionary *mapping;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        mapping = @{
            @"ease-in":kCAMediaTimingFunctionEaseIn,
            @"ease-out":kCAMediaTimingFunctionEaseOut,
            @"ease-in-out":kCAMediaTimingFunctionEaseInEaseOut,
            @"linear":kCAMediaTimingFunctionLinear,
            @"ease":kCAMediaTimingFunctionDefault
        };
    });
    
    NSString *timingFunction = mapping[value];
    if ([timingFunction length] > 0) {
        return [CAMediaTimingFunction functionWithName:timingFunction];
    }
    
    if ([value hasPrefix:@"cubic-bezier"]) {
        float x1, y1, x2, y2;
        sscanf(((NSString *)value).UTF8String, "cubic-bezier(%f,%f,%f,%f)", &x1, &y1, &x2, &y2);
        return [CAMediaTimingFunction functionWithControlPoints:x1 :y1 :x2 :y2];
    }
    
    return [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionLinear];
}

#pragma mark Visibility

+ (WXVisibility)WXVisibility:(id)value
{
    if ([value isKindOfClass:[NSString class]]) {
        NSString *string = (NSString *)value;
        if ([string isEqualToString:@"visible"]) {
            return WXVisibilityShow;
        } else if ([string isEqualToString:@"hidden"]) {
            return WXVisibilityHidden;
        }
    }
    
    return  WXVisibilityShow;
}

#pragma mark Gradient Color

+ (WXGradientType)gradientType:(id)value
{
    WXGradientType type = WXGradientTypeToRight;
    
    if ([value isKindOfClass:[NSString class]]) {
        NSString *string = (NSString *)value;
        
        if ([string isEqualToString:@"totop"]) {
            type = WXGradientTypeToTop;
        }
        else if ([string isEqualToString:@"tobottom"]) {
            type = WXGradientTypeToBottom;
        }
        else if ([string isEqualToString:@"toleft"]) {
            type = WXGradientTypeToLeft;
        }
        if ([string isEqualToString:@"toright"]) {
            type = WXGradientTypeToRight;
        }
        else if ([string isEqualToString:@"totopleft"]) {
            type = WXGradientTypeToTopleft;
        }
        else if ([string isEqualToString:@"tobottomright"]) {
            type = WXGradientTypeToBottomright;
        }
    }
    return type;
}

#pragma mark - Length

+ (WXLength *)WXLength:(id)value isFloat:(BOOL)isFloat scaleFactor:(CGFloat)scaleFactor
{
    if (!value) {
        return nil;
    }
    
    WXLengthType type = WXLengthTypeFixed;
    if ([value isKindOfClass:[NSString class]]) {
        if ([value isEqualToString:@"auto"]) {
            type = WXLengthTypeAuto;
        } else if ([value isEqualToString:@"normal"]){
            type = WXLengthTypeNormal;
        } else if ([value hasSuffix:@"%"]) {
            type = WXLengthTypePercent;
        }
    } else if (![value isKindOfClass:[NSNumber class]]) {
        WXAssert(NO, @"Unsupported type:%@ for WXLength", NSStringFromClass([value class]));
    }
    
    if (isFloat) {
        return [WXLength lengthWithFloat:([value floatValue] * scaleFactor) type:type];
    } else {
        return [WXLength lengthWithInt:([value intValue] * scaleFactor) type:type];
    }
}

+ (WXBoxShadow *)WXBoxShadow:(id)value scaleFactor:(CGFloat)scaleFactor
{
    NSString *boxShadow = @"";
    if([value isKindOfClass:[NSString class]]){
        boxShadow = value;
    } else if([value isKindOfClass:[NSNumber class]]){
        boxShadow =  [((NSNumber *)value) stringValue];
    } else if (value != nil) {
        boxShadow = nil;
        WXLogError(@"Convert Error:%@ can not be converted to boxshadow type", value);
    }
    if (boxShadow) {
        return [WXBoxShadow getBoxShadowFromString:boxShadow scaleFactor:scaleFactor];
    }
    return nil;
}

+ (UIAccessibilityTraits)WXUIAccessibilityTraits:(id)value
{
    UIAccessibilityTraits accessibilityTrait = UIAccessibilityTraitNone;
    if (![value isKindOfClass:[NSString class]]) {
        return accessibilityTrait;
    }
    NSString * role = value;
    if ([role isEqualToString:@"button"]) {
        accessibilityTrait = UIAccessibilityTraitButton;
    } else if ([role isEqualToString:@"link"]) {
        accessibilityTrait = UIAccessibilityTraitLink;
    } else if ([role isEqualToString:@"img"]) {
        accessibilityTrait = UIAccessibilityTraitImage;
    } else if ([role isEqualToString:@"search"]) {
        accessibilityTrait = UIAccessibilityTraitSearchField;
    } else if ([role isEqualToString:@"tab"]) {
#ifdef __IPHONE_10_0
        if (WX_SYS_VERSION_GREATER_THAN_OR_EQUAL_TO(@"10.0")) {
            accessibilityTrait = UIAccessibilityTraitTabBar;
        }
#endif
    } else if ([role isEqualToString:@"frequentUpdates"]) {
        accessibilityTrait = UIAccessibilityTraitUpdatesFrequently;
    } else if ([role isEqualToString:@"startsMedia"]) {
        accessibilityTrait = UIAccessibilityTraitStartsMediaSession;
    } else if ([role isEqualToString:@"allowsDirectInteraction"]) {
        accessibilityTrait = UIAccessibilityTraitAllowsDirectInteraction;
    } else if ([role isEqualToString:@"summary"]) {
        accessibilityTrait = UIAccessibilityTraitSummaryElement;
    } else if ([role isEqualToString:@"header"]) {
        accessibilityTrait = UIAccessibilityTraitHeader;
    } else if ([role isEqualToString:@"keyboardKey"]) {
        accessibilityTrait = UIAccessibilityTraitKeyboardKey;
    } else if ([role isEqualToString:@"disabled"]) {
        accessibilityTrait = UIAccessibilityTraitNotEnabled;
    } else if ([role isEqualToString:@"playSound"]) {
        accessibilityTrait = UIAccessibilityTraitPlaysSound;
    } else if ([role isEqualToString:@"selected"]) {
        accessibilityTrait = UIAccessibilityTraitSelected;
    } else if ([role isEqualToString:@"pageTurn"]) {
        accessibilityTrait = UIAccessibilityTraitCausesPageTurn;
    } else if ([role isEqualToString:@"text"]) {
        accessibilityTrait = UIAccessibilityTraitStaticText;
    }
    
    return accessibilityTrait;
}

@end

@implementation WXConvert (Deprecated)

+ (WXPixelType)WXPixelType:(id)value
{
    CGFloat pixel = [self WXPixelType:value scaleFactor:1.0];
    
    return pixel * WXScreenResizeRadio();
}

@end
