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

#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * The WXRootViewController class inherited from UINavigationController class which implements a specialized
 * view controller that manages the navigation of hierarchical content. Developing an iOS application, you
 * need a series of customer pages which will be render by weex bundle. Sometimes, these pages are embedded in
 * viewcontroller. This navigation controller makes it possible to present your page efficiently and makes it
 * easier for the user to navigate that content.
 */

@interface WXRootViewController : UINavigationController

/**
 * @abstract initialize the RootViewController with bundle url.
 *
 * @param sourceURL The bundle url which can be render to a weex view.
 *
 * @return a object the class of WXRootViewController.
 *
 * @discussion initialize this controller in function 'application:didFinishLaunchingWithOptions', and make it as rootViewController of window. In the
 * weex application, all page content can be managed by the navigation, such as push or pop.
 */
- (id)initWithSourceURL:(NSURL *)sourceURL;

@end

NS_ASSUME_NONNULL_END
