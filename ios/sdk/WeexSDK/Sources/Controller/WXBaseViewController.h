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
 * The WXBaseViewController class provides the infrastructure for managing the weex view in your app. It is 
 * responsible for creating a weex instance or rendering the weex view, for observing the lifecycle of the
 * view such as "appear" or "disappear"、"foreground" or "background" etc. You can initialize this controller by
 * special bundle URL.
 */

@interface WXBaseViewController : UIViewController<UIGestureRecognizerDelegate>

/**
 * @abstract initializes the viewcontroller with bundle url.
 *
 * @param sourceURL The url of bundle rendered to a weex view.
 *
 * @return a object the class of WXBaseViewController.
 *
 */
- (instancetype)initWithSourceURL:(NSURL *)sourceURL;

/**
 * @abstract refreshes the weex view in controller.
 */
- (void)refreshWeex;

@end

NS_ASSUME_NONNULL_END
