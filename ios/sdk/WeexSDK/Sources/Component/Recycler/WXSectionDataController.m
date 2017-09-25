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

#import "WXSectionDataController.h"
#import "WXCellComponent.h"
#import "WXHeaderComponent.h"
#import "WXAssert.h"

@implementation WXSectionDataController

- (NSInteger)numberOfItems
{
    return self.cellComponents.count;
}

- (UIView *)cellForItemAtIndex:(NSInteger)index
{
    WXAssertMainThread();
    
    WXCellComponent *cellComponent = self.cellComponents[index];
    return cellComponent.view;
}

- (CGSize)sizeForItemAtIndex:(NSInteger)index
{
    WXAssertMainThread();
    
    WXCellComponent *cellComponent = self.cellComponents[index];
    return cellComponent.calculatedFrame.size;
}

- (UIView *)viewForHeaderAtIndex:(NSInteger)index;
{
    return self.headerComponent.view;
}

- (CGSize)sizeForHeaderAtIndex:(NSInteger)index
{
    return self.headerComponent.calculatedFrame.size;
}

- (BOOL)isStickyForHeaderAtIndex:(NSInteger)index
{
    return self.headerComponent.isSticky;
}

- (NSUInteger)hash
{
    return [super hash];
}

- (BOOL)weex_isEqualTo:(id<WXDiffable>)object
{
    if ([object isKindOfClass:[WXSectionDataController class]]) {
        WXSectionDataController *controller = (WXSectionDataController *)object;
        BOOL headerEqual = (self.headerComponent && controller.headerComponent && self.headerComponent == controller.headerComponent) || (!self.headerComponent && !controller.headerComponent);
        BOOL footerEqual = (self.footerComponent && controller.footerComponent && self.footerComponent == controller.footerComponent) || (!self.footerComponent && !controller.footerComponent);
        BOOL cellEqual = self.cellComponents && controller.cellComponents && self.cellComponents.count == controller.cellComponents.count;
        if (cellEqual) {
            for (int i = 0; i < self.cellComponents.count; i ++) {
                if (self.cellComponents[i] != controller.cellComponents[i]) {
                    cellEqual = NO;
                    break;
                }
            }
        } else {
            cellEqual = !self.cellComponents && controller.cellComponents;
        }
        
        return headerEqual && footerEqual && cellEqual;
    } else {
        return NO;
    }
}

@end
