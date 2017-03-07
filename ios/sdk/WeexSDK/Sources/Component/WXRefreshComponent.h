/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import "WXComponent.h"

#define  REFRESH_DISTANCE_Y                 @"dy"
#define  REFRESH_PULLINGDISTANCE            @"pullingDistance"
#define  REFRESH_VIEWHEIGHT              @"viewHeight"

@interface WXRefreshComponent : WXComponent

- (void)refresh;

- (BOOL)displayState;

- (void)pullingdown:(NSDictionary*)param;

@end
