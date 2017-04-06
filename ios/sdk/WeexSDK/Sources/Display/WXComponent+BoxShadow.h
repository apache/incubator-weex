/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import <Foundation/Foundation.h>
#import "WXComponent.h"
#import "WXBoxShadow.h"

@interface WXComponent (BoxShadow)

/**
 *  @abstract compare boxshadow
 *
 *  @param boxShadow
 *
 *  @param view
 *
 *  @return if not equal return NO, if equal return YES
 */
- (BOOL)EqualBoxShadow:(WXBoxShadow *_Nullable)boxShadow withBoxShadow:(WXBoxShadow *_Nullable)compareBoxShadow;

/**
 *  @abstract config view Layer with boxshadow, lastboxshadow and originalBoxShadow
 *
 *  @param boxShadow the boxshadow will be set
 *
 *
 */
- (void)configBoxShadow:(WXBoxShadow *_Nullable)boxShadow;
@end
