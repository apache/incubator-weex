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

#ifndef __WX_DEFINE_H__
#define __WX_DEFINE_H__

#define WX_SDK_VERSION @"0.24.0"

#if defined(__cplusplus)
#define WX_EXTERN extern "C" __attribute__((visibility("default")))
#else
#define WX_EXTERN extern __attribute__((visibility("default")))
#endif

/*
 * Concatenate preprocessor tokens a and b without expanding macro definitions
 * (however, if invoked from a macro, macro arguments are expanded).
 */
#define WX_CONCAT(a, b)   a ## b
/*
 * Concatenate preprocessor tokens a and b after macro-expanding them.
 */
#define WX_CONCAT_WRAPPER(a, b)    WX_CONCAT(a, b)

#define WX_CONCAT_TRIPLE(a, b, c) a ## b ## c

#define WX_NSSTRING_HELPER(x) #x
#define WX_NSSTRING(x) @WX_NSSTRING_HELPER(x)

#define WX_SDK_ROOT_REF     @"_root"

#define WX_TEXT_FONT_SIZE   (32.0 * self.weexInstance.pixelScaleFactor)

#define WX_UPDATE_CONFIG(prefix, name, configs) \
NSString *selStr = [NSString stringWithFormat:@"%@_%@", prefix, name];\
SEL selector = NSSelectorFromString(selStr);\
Class clazz = WX_COMPONENT_CLASS(_properties[@"type"]);\
if ([clazz respondsToSelector:selector]) {\
    configs = ((NSArray *(*)(id, SEL))objc_msgSend)(clazz, selector);\
}\

#define WX_TYPE_KEYPATH(config, name, type, parts, vKey) \
type = [config[0] stringByAppendingString:@":"];\
NSString *keyPath = config.count > 1 ? config[1] : nil;\
if(keyPath){\
    parts = [keyPath componentsSeparatedByString:@"."];\
    vKey = parts.lastObject;\
parts = [parts subarrayWithRange:(NSRange){0, parts.count - 1}];\
} else {\
    vKey = name;\
}

#define RGB(r, g, b) [UIColor colorWithRed:r/255.0 green:g/255.0 blue:b/255.0 alpha:1.0]
#define RGB_A(r, g, b, a) [UIColor colorWithRed:r/255.0 green:g/255.0 blue:b/255.0 alpha:a]

#define ScreenWidth  ([[UIScreen mainScreen] bounds].size.width)

#define WX_ERROR_DOMAIN @"WXErrorDomain"

#define WX_APPLICATION_WILL_RESIGN_ACTIVE @"WXApplicationWillResignActiveEvent"

#define WX_APPLICATION_DID_BECOME_ACTIVE @"WXApplicationDidBecomeActiveEvent"

#define WX_INSTANCE_NOTIFICATION_UPDATE_STATE @"WXInstUpdateState"

#define WX_COMPONENT_NOTIFICATION_VIEW_LOADED    @"WXComponentViewLoaded"

#define WX_INSTANCE_WILL_DESTROY_NOTIFICATION     @"WXSDKInstanceWillDestroyNotification"

#define WX_SDKINSTANCE_WILL_RENDER    @"WXSDKInstanceWillRender"

#define WX_COMPONENT_THREAD_NAME @"com.taobao.weex.component"

#define WX_BRIDGE_THREAD_NAME @"com.taobao.weex.bridge"

#define WX_FONT_DOWNLOAD_DIR [[WXUtility cacheDirectory] stringByAppendingPathComponent:[NSString stringWithFormat:@"wxdownload"]]

#define WX_EXPORT_METHOD_INTERNAL(method, token) \
+ (NSString *)WX_CONCAT_WRAPPER(token, __LINE__) { \
    return NSStringFromSelector(method); \
}

#define WX_MODULE_EVENT_FIRE_NOTIFICATION  @"WX_MODULE_EVENT_FIRE_NOTIFICATION"
#define WX_ICONFONT_DOWNLOAD_NOTIFICATION  @"WX_ICONFONT_DOWNLOAD_FINISH_NOTIFICATION"

#define WX_INSTANCE_JSCONTEXT_CREATE_NOTIFICATION @"WX_INSTANCE_JSCONTEXT_CREATE_NOTIFICATION"

/**
 *  @abstract export public method
 */
#define WX_EXPORT_METHOD(method) WX_EXPORT_METHOD_INTERNAL(method,wx_export_method_)

/**
 *  @abstract export public method, support sync return value
 *  @warning the method can only be called on js thread
 */
#define WX_EXPORT_METHOD_SYNC(method) WX_EXPORT_METHOD_INTERNAL(method,wx_export_method_sync_)

/** extern "C" makes a function-name in C++ have 'C' linkage (compiler does not mangle the name)
 * so that client C code can link to (i.e use) your function using a 'C' compatible header file that contains just the declaration of your function.
 *  http://stackoverflow.com/questions/1041866/in-c-source-what-is-the-effect-of-extern-c
 */
#ifdef __cplusplus
# define WX_EXTERN_C_BEGIN extern "C" {
# define WX_EXTERN_C_END   }
#else
# define WX_EXTERN_C_BEGIN
# define WX_EXTERN_C_END
#endif

/**
 *  @abstract Compared with system version of current device
 *
 *  @return YES if greater than or equal to the system verison, otherwise, NO.
 *
 */
#define WX_SYS_VERSION_GREATER_THAN_OR_EQUAL_TO(v)  ([[[UIDevice currentDevice] systemVersion] compare:v options:NSNumericSearch] != NSOrderedAscending)

/**
 *  @abstract Compared with system version of current device
 *
 *  @return YES if greater than the system verison, otherwise, NO.
 *
 */
#define WX_SYS_VERSION_GREATER_THAN(v)              ([[[UIDevice currentDevice] systemVersion] compare:v options:NSNumericSearch] == NSOrderedDescending)

/**
 *  @abstract Compared with system version of current device
 *
 *  @return YES if equal to the system verison, otherwise, NO.
 *
 */
#define WX_SYS_VERSION_EQUAL_TO(v)                  ([[[UIDevice currentDevice] systemVersion] compare:v options:NSNumericSearch] == NSOrderedSame)

/**
 *  @abstract Compared with system version of current device
 *
 *  @return YES if less than the system verison, otherwise, NO.
 *
 */
#define WX_SYS_VERSION_LESS_THAN(v)                 ([[[UIDevice currentDevice] systemVersion] compare:v options:NSNumericSearch] == NSOrderedAscending)

/**
 *  @abstract Compared with system version of current device
 *
 *  @return YES if less than or equal to the system verison, otherwise, NO.
 *
 */
#define WX_SYS_LESS_THAN_OR_EQUAL_TO(v)             ([[[UIDevice currentDevice] systemVersion] compare:v options:NSNumericSearch] != NSOrderedDescending)

/**
 *  @abstract Estimate component's type. If the type isn't equal to WXComponentTypeCommon, then return.
 */
#define WX_CHECK_COMPONENT_TYPE(type)\
do {\
if (type != WXComponentTypeCommon) {\
return;\
}\
} while (0);



#if __has_attribute(objc_requires_super)
    #define WX_REQUIRES_SUPER __attribute__((objc_requires_super))
#else
    #define WX_REQUIRES_SUPER
#endif

#endif

WX_EXTERN_C_BEGIN

NSString* GetWeexSDKVersion(void);
NSString* GetWeexSDKBuildTime(void);
unsigned long GetWeexSDKBuildTimestamp(void);

WX_EXTERN_C_END
