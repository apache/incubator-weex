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

#import "WXComponent.h"

typedef enum
{
    WXPointIndicatorAlignCenter,    // point indicator align center
    WXPointIndicatorAlignLeft,      // point indicator align left
    WXPointIndicatorAlignRight,     // point indicator align right
} WXPointIndicatorAlignStyle;

@interface WXIndicatorView : UIView

@property (nonatomic, assign)   NSInteger   pointCount;         // total count point of point indicator
@property (nonatomic, assign)   NSInteger   currentPoint;       // current light index of point at point indicator
@property (nonatomic, strong)   UIColor *pointColor;        // normal point color of point indicator
@property (nonatomic, strong)   UIColor *lightColor;        // highlight point color of point indicator
@property (nonatomic, assign)   WXPointIndicatorAlignStyle  alignStyle;    //align style of point indicator
@property (nonatomic, assign)   CGFloat pointSize;          // point size of point indicator
@property (nonatomic, assign)   CGFloat pointSpace;         // point space of point indicator

@end


@protocol WXIndicatorComponentDelegate <NSObject>

-(void)setIndicatorView:(WXIndicatorView *)indicatorView;

@end

@interface WXIndicatorComponent : WXComponent

@property (nonatomic, weak) id<WXIndicatorComponentDelegate> delegate;

@end
