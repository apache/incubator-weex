/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
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

- (BOOL)isEqualToWXObject:(id<WXDiffable>)object
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
