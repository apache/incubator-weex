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

#import <WeexSDK/WXModuleProtocol.h>

NS_ASSUME_NONNULL_BEGIN

@protocol WXDarkSchemeProtocol <WXModuleProtocol>

@required

/**
 Return YES so that Weex root component will be enabled for 'invertForDarkScheme' property.
*/
- (BOOL)defaultInvertValueForRootComponent;

/**
After any view of Weex component is created. Callback dark scheme handler to provide a
 chance to configure the view.
*/
- (void)configureView:(UIView*_Nonnull)view ofComponent:(WXComponent*_Nonnull)component;

/**
 Get inverted color in dark mode for input color with scene hint.

 @param color Input color.
 @param scene Scene indicating the color usage.
 @param defaultColor If no inverted one matches, return the default color.
 @return Inverted color.
 */
- (UIColor *_Nullable)getInvertedColorFor:(UIColor *_Nonnull)color ofScene:(WXColorScene)scene withDefault:(UIColor *_Nullable)defaultColor;

@optional

/**
 Using [UITraitCollection currentTraitCollection] will only get system value of
 dark mode. But application itself might also switch off or on dark mode ignoring
 setting of system.
 */
- (BOOL)isApplicationUsingDarkScheme;

@end

NS_ASSUME_NONNULL_END
