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

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "WXDefine.h"
#import "WXType.h"
#import "WXLog.h"
#import "WXSDKInstance.h"

// The default screen width which helps us to calculate the real size or scale in different devices.
static const CGFloat WXDefaultScreenWidth = 750.0;

#define WX_ENUMBER_CASE(_invoke, idx, code, obj, _type, op, _flist) \
case code:{\
    _type *_tmp = malloc(sizeof(_type));\
    memset(_tmp, 0, sizeof(_type));\
    *_tmp = [obj op];\
    [_invoke setArgument:_tmp atIndex:(idx) + 2];\
    *(_flist + idx) = _tmp;\
    break;\
}
#define WX_EPCHAR_CASE(_invoke, idx, code, obj, _type, op, _flist) \
case code:{\
    _type *_tmp = (_type  *)[obj op];\
    [_invoke setArgument:&_tmp atIndex:(idx) + 2];\
    *(_flist + idx) = 0;\
    break;\
}\

#define WX_ALLOC_FLIST(_ppFree, _count) \
do {\
    _ppFree = (void *)malloc(sizeof(void *) * (_count));\
    memset(_ppFree, 0, sizeof(void *) * (_count));\
} while(0)

#define WX_FREE_FLIST(_ppFree, _count) \
do {\
    for(int i = 0; i < _count; i++){\
        if(*(_ppFree + i ) != 0) {\
            free(*(_ppFree + i));\
        }\
    }\
    free(_ppFree);\
}while(0)

#define WX_ARGUMENTS_SET(_invocation, _sig, idx, _obj, _ppFree) \
do {\
    const char *encode = [_sig getArgumentTypeAtIndex:(idx) + 2];\
    switch(encode[0]){\
        WX_EPCHAR_CASE(_invocation, idx, _C_CHARPTR, _obj, char *, UTF8String, _ppFree)\
        WX_ENUMBER_CASE(_invocation, idx, _C_INT, _obj, int, intValue, _ppFree)\
        WX_ENUMBER_CASE(_invocation, idx, _C_SHT, _obj, short, shortValue, _ppFree)\
        WX_ENUMBER_CASE(_invocation, idx, _C_LNG, _obj, long, longValue, _ppFree)\
        WX_ENUMBER_CASE(_invocation, idx, _C_LNG_LNG, _obj, long long, longLongValue, _ppFree)\
        WX_ENUMBER_CASE(_invocation, idx, _C_UCHR, _obj, unsigned char, unsignedCharValue, _ppFree)\
        WX_ENUMBER_CASE(_invocation, idx, _C_UINT, _obj, unsigned int, unsignedIntValue, _ppFree)\
        WX_ENUMBER_CASE(_invocation, idx, _C_USHT, _obj, unsigned short, unsignedShortValue, _ppFree)\
        WX_ENUMBER_CASE(_invocation, idx, _C_ULNG, _obj, unsigned long, unsignedLongValue, _ppFree)\
        WX_ENUMBER_CASE(_invocation, idx, _C_ULNG_LNG, _obj,unsigned long long, unsignedLongLongValue, _ppFree)\
        WX_ENUMBER_CASE(_invocation, idx, _C_FLT, _obj, float, floatValue, _ppFree)\
        WX_ENUMBER_CASE(_invocation, idx, _C_DBL, _obj, double, doubleValue, _ppFree)\
        WX_ENUMBER_CASE(_invocation, idx, _C_BOOL, _obj, bool, boolValue, _ppFree)\
        default: { [_invocation setArgument:&_obj atIndex:(idx) + 2]; *(_ppFree + idx) = 0; break;}\
    }\
}while(0)

#ifdef __cplusplus
extern "C" {
#endif
    
/**
 * @abstract execute asynchronous action block on the main thread.
 *
 */
void WXPerformBlockOnMainThread( void (^ _Nonnull block)(void));

/**
 * @abstract execute synchronous action block on the main thread.
 *
 */
void WXPerformBlockSyncOnMainThread( void (^ _Nonnull block)(void));

/**
 * @abstract execute action block on the specific thread.
 *
 */
void WXPerformBlockOnThread(void (^ _Nonnull block)(void), NSThread *_Nonnull thread);

/**
 * @abstract swizzling methods.
 *
 */
void WXSwizzleInstanceMethod(_Nonnull Class className, _Nonnull SEL original, _Nonnull SEL replaced);

void WXSwizzleInstanceMethodWithBlock(_Nonnull Class className, _Nonnull SEL original, _Nonnull id block, _Nonnull SEL replaced);

_Nonnull SEL WXSwizzledSelectorForSelector(_Nonnull SEL selector);
    
#ifdef __cplusplus
}
#endif

@interface WXUtility : NSObject

+ (void)performBlock:(void (^_Nonnull)(void))block onThread:(NSThread *_Nonnull)thread;

/**
 * @abstract Returns the environment of current application, you can get some necessary properties such as appVersion、sdkVersion、appName etc.
 *
 * @return A dictionary object which contains these properties.
 *
 */
+ (NSDictionary *_Nonnull)getEnvironment;

+ (NSDictionary *_Nonnull)getDebugEnvironment;

+ (WXLayoutDirection)getEnvLayoutDirection;

/**
 * @abstract UserAgent Generation
 *
 * @return A ua string by splicing (deviceName、appVersion、sdkVersion、externalField、screenSize)
 *
 */
+ (NSString *_Nonnull)userAgent;

/**
 * @abstract JSON Decode Method
 *
 * @param json String.
 *
 * @return A json object by decoding json string.
 *
 */
+ (id _Nullable)objectFromJSON:(NSString * _Nonnull)json;

#define WXDecodeJson(json)  [WXUtility objectFromJSON:json]

/**
 * @abstract JSON Encode Method
 *
 * @param object Object.
 *
 * @return A json string by encoding json object.
 *
 */
+ (NSString * _Nullable)JSONString:(id _Nonnull)object;

#define WXEncodeJson(obj)  [WXUtility JSONString:obj]

/**
 * @abstract Returns a Foundation object from given JSON data. A Foundation object from the JSON data in data, or nil if an error occurs.
 *
 * @param data A data object containing JSON data.
 * @param error If an error occurs, upon return contains an NSError object that describes the problem.
 *
 * @return A Foundation object from the JSON data in data, or nil if an error occurs.
 *
 */
+ (id _Nullable)JSONObject:(NSData * _Nonnull)data error:(NSError * __nullable * __nullable)error;

#define WXJSONObjectFromData(data) [WXUtility JSONObject:data error:nil]
/**
 * @abstract JSON Object Copy Method
 *
 * @param object Object.
 *
 * @return A json object by copying.
 *
 */
+ (id _Nullable)copyJSONObject:(id _Nonnull)object;

#define WXCopyJson(obj)     [WXUtility copyJSONObject:obj]

/**
 *
 *  Checks if a String is whitespace, empty ("") or nil
 *  @code
 *    [WXUtility isBlankString: nil]       = true
 *    [WXUtility isBlankString: ""]        = true
 *    [WXUtility isBlankString: " "]       = true
 *    [WXUtility isBlankString: "bob"]     = false
 *    [WXUtility isBlankString: "  bob  "] = false
 *  @endcode
 *  @param string the String to check, may be null
 *
 *  @return true if the String is null, empty or whitespace
 */
+ (BOOL)isBlankString:(NSString * _Nullable)string ;


/**
 check a point is valid or not. A zero point is also valid

 @param point a point value to check
 @return true if point.x and point.y are all valid value for a number.
 */
+ (BOOL)isValidPoint:(CGPoint)point;

/**
 * @abstract Returns a standard error object
 *
 * @param code code.
 *
 * @param message message.
 *
 * @return A error object type of NSError.
 *
 */
+ (NSError * _Nonnull)errorWithCode:(NSInteger)code message:(NSString * _Nullable)message;

/**
 * @abstract Returns a Font Object by setting some properties such as size、weight、style and fontFamily.
 *
 * @param size font size
 *
 * @param textWeight font weight
 *
 * @param textStyle  The type of WXTextStyle (Normal or Italic).
 *
 * @param fontFamily font family
 *
 * @param scaleFactor please use instance's scale factor
 *
 * @return A font object according to the above params.
 *
 */
+ (UIFont *_Nonnull)fontWithSize:(CGFloat)size textWeight:(CGFloat)textWeight textStyle:(WXTextStyle)textStyle fontFamily:(NSString *_Nullable)fontFamily scaleFactor:(CGFloat)scaleFactor;

+ (UIFont *_Nonnull)fontWithSize:(CGFloat)size textWeight:(CGFloat)textWeight textStyle:(WXTextStyle)textStyle fontFamily:(NSString *_Nullable)fontFamily scaleFactor:(CGFloat)scaleFactor useCoreText:(BOOL)useCoreText;

/**
 * @abstract download remote font from specified url
 * @param fontURL for remote font
 *
 */
+ (void)getIconfont:(NSURL * _Nonnull)fontURL completion:( void(^ _Nullable )(NSURL * _Nonnull url, NSError * _Nullable error)) completionBlock;

/**
 * @abstract Returns the main screen's size when the device is in portrait mode,.
 */
+ (CGSize)portraitScreenSize;

/**
 * @abstract Returns the default pixel scale factor
 * @discussion If orientation is equal to landscape, the value is caculated as follows: WXScreenSize().height / WXDefaultScreenWidth, otherwise, WXScreenSize().width / WXDefaultScreenWidth.
 */
+ (CGFloat)defaultPixelScaleFactor;

#if defined __cplusplus
extern "C" {
#endif
/**
 * @abstract Returns the scale of the main screen.
 *
 */
CGFloat WXScreenScale(void);

/**
 * @abstract Returns a Round float coordinates to the main screen pixel.
 *
 */
CGFloat WXRoundPixelValue(CGFloat value);

/**
 * @abstract Returns a Floor float coordinates to the main screen pixel.
 *
 */
CGFloat WXFloorPixelValue(CGFloat value);

/**
 * @abstract Returns a Ceil float coordinates to the main screen pixel.
 *
 */
CGFloat WXCeilPixelValue(CGFloat value);
    
#if defined __cplusplus
};
#endif
    
/**
 *  @abstract check whether the file is exist
 *
 */

+ (BOOL)isFileExist:(NSString * _Nonnull)filePath;
/**
 *  @abstract Returns the document directory path.
 *
 */
+ (NSString *_Nonnull)documentDirectory;

#define WXDocumentPath     [WXUtility documentDirectory]

/**
 *  @abstract Returns the system cache directory path.
 *
 */
+ (NSString *_Nonnull)cacheDirectory;

#define WXCachePath     [WXUtility cacheDirectory]

/**
 *  @abstract Returns the system library directory path.
 *
 */
+ (NSString *_Nonnull)libraryDirectory;

#define WXLibraryPath  [WXUtility libraryDirectory]

/**
 *  @abstract Returns the global cache whose size is 5M.
 *
 */
+ (NSCache *_Nonnull)globalCache;

#define WXGlobalCache   [WXUtility globalCache]

+ (NSURL *_Nonnull)urlByDeletingParameters:(NSURL *_Nonnull)url;

/**
 *  @abstract Returns the contents of file.
 *
 */
+ (NSString *_Nullable)stringWithContentsOfFile:(NSString *_Nonnull)filePath;

/**
 *  @abstract Returns md5 string.
 *
 */
+ (NSString *_Nullable)md5:(NSString *_Nullable)string;

/**
 *  @abstract Returns Creates a Universally Unique Identifier (UUID) string.
 *
 */
+ (NSString *_Nullable)uuidString;

/**
 *  @abstract convert date string with formatter yyyy-MM-dd to date.
 *
 */
+ (NSDate *_Nullable)dateStringToDate:(NSString *_Nullable)dateString;

/**
 *  @abstract convert time string with formatter HH:mm to date.
 *
 */
+ (NSDate *_Nullable)timeStringToDate:(NSString *_Nullable)timeString;

/**
 *  @abstract convert date to date string with formatter yyyy-MM-dd .
 *
 */
+ (NSString *_Nullable)dateToString:(NSDate *_Nullable)date;

/**
 *  @abstract convert date to time string with formatter HH:mm .
 *
 */
+ (NSString *_Nullable)timeToString:(NSDate *_Nullable)date;

/**
 *  @abstract get the repeat substring number of string.
 *
 */
+ (NSUInteger)getSubStringNumber:(NSString *_Nullable)string subString:(NSString *_Nullable)subString;

/**
 *  @abstract Returns a resized pixel which is calculated according to the WXScreenResizeRadio.
 *
 */
CGFloat WXPixelScale(CGFloat value, CGFloat scaleFactor);

CGFloat WXScreenResizeRadio(void) DEPRECATED_MSG_ATTRIBUTE("Use [WXUtility defaultPixelScaleFactor] instead");
CGFloat WXPixelResize(CGFloat value) DEPRECATED_MSG_ATTRIBUTE("Use WXPixelScale Instead");
CGRect WXPixelFrameResize(CGRect value) DEPRECATED_MSG_ATTRIBUTE("Use WXPixelScale Instead");
CGPoint WXPixelPointResize(CGPoint value) DEPRECATED_MSG_ATTRIBUTE("Use WXPixelScale Instead");
+ (UIFont  * _Nullable )fontWithSize:(CGFloat)size textWeight:(CGFloat)textWeight textStyle:(WXTextStyle)textStyle fontFamily:(NSString * _Nullable)fontFamily DEPRECATED_MSG_ATTRIBUTE("Use +[WXUtility fontWithSize:textWeight:textStyle:fontFamily:scaleFactor:]");


/**
 @discusstion construct a gradientLayer from the colors locations, gradientType
 @param colors The array of UIColor objects defining the color of each gradient
 stop. Defaults to nil
 @param locations An optional array of NSNumber objects defining the location of each
  gradient stop as a value in the range [0,1].
 @param frame the layer frame
 @param gradientType WXGradientType value specify the gradient location
 @return gradient layer
 */
+ (CAGradientLayer *_Nullable)gradientLayerFromColors:(NSArray*_Nullable)colors
                                           locations:(NSArray*_Nullable)locations
                                               frame:(CGRect)frame
                                        gradientType:(WXGradientType)gradientType;

/**
 @discusstion parse gradient-color string to a dictionary, then you can get gradientLayer from @see gradientLayerFromColors:colors:locations:frame:locations
 @param backgroundImage  linear-gradient string like linear-gradient(to right, #a80077,rgba(200, 54, 54, 0.5))
 @return dictionary with endColor, startColor and gradientType value
 @code
    NSDictionary * linearGradient = [self linearGradientWithBackgroundImage:@"linear-gradient(to right, #a80077,rgba(200, 54, 54, 0.5))"];
    CAGradientLayer * gradientLayer = [self gradientLayerFromColors:@[linearGradient[@"startColor"], linearGradient[@"endColor"]],nil,bounds,[linearGradient[@"gradientType"] integerValue]];
 @endcode
 */
+ (NSDictionary *_Nullable)linearGradientWithBackgroundImage:(NSString *_Nullable)backgroundImage;

#if defined __cplusplus
extern "C" {
#endif
    
/**
 *  @abstract compare float a and b, if a equal b, return true,or reture false.
 *
 */
BOOL WXFloatEqual(CGFloat a, CGFloat b);
/**
 *  @abstract compare float a and b, user give the compare precision, if a equal b, return true,or reture false.
 *
 */
BOOL WXFloatEqualWithPrecision(CGFloat a, CGFloat b ,double precision);
/**
 *  @abstract compare float a and b, if a less than b, return true,or reture false.
 *
 */
BOOL WXFloatLessThan(CGFloat a, CGFloat b);
/**
 *  @abstract compare float a and b,user give the compare precision, if a less than b,return true,or reture false.
 *
 */
BOOL WXFloatLessThanWithPrecision(CGFloat a, CGFloat b,double precision);
/**
 *  @abstract compare float a and b, if a great than b, return true,or reture false.
 *
 */
BOOL WXFloatGreaterThan(CGFloat a, CGFloat b);
/**
 *  @abstract compare float a and b, user give the compare precision,if a great than b, return true,or reture false.
 *
 */
BOOL WXFloatGreaterThanWithPrecision(CGFloat a,CGFloat b,double precision);

#if defined __cplusplus
};
#endif

/**
 *  @abstract convert returnKeyType to type string .
 *
 */
+ (NSString *_Nullable)returnKeyType:(UIReturnKeyType)type;

/**
 *  @abstract custorm monitor info
 *
 */
+ (void)customMonitorInfo:(WXSDKInstance *_Nullable)instance key:(NSString * _Nonnull)key value:(id _Nonnull)value;

/**
 *  @abstract format to base64 dictionary
 *
 */
+ (NSDictionary *_Nonnull)dataToBase64Dict:(NSData *_Nullable)data;

/**
 *  @abstract format to data
 *
 */
+ (NSData *_Nonnull)base64DictToData:(NSDictionary *_Nullable)base64Dict;

+ (void)setUnregisterFontWhenCollision:(BOOL)value;

+ (void)setUseJSCApiForCreateInstance:(BOOL)value;

+ (BOOL)useJSCApiForCreateInstance;

+ (void)setEnableRTLLayoutDirection:(BOOL)value;

+ (BOOL)enableRTLLayoutDirection;

+ (long) getUnixFixTimeMillis;

+ (NSArray<NSString *> *_Nullable)extractPropertyNamesOfJSValueObject:(JSValue *_Nullable)jsvalue;

@end
