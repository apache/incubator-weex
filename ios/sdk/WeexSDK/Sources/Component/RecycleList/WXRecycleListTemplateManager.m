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

#import "WXRecycleListTemplateManager.h"
#import "WXLog.h"
#import "WXAssert.h"

@interface WXReusableCollectionViewCell : UICollectionViewCell

@end

@implementation WXReusableCollectionViewCell

@end

@implementation WXRecycleListTemplateManager
{
    NSMutableSet<NSString *> *_registeredTemplates;
    NSMapTable<NSString *, WXCellSlotComponent *> *_templateTypeMap;
}

- (instancetype)init
{
    if (self = [super init]) {
        _registeredTemplates = [NSMutableSet set];
        _templateTypeMap = [NSMapTable strongToWeakObjectsMapTable];
    }
    
    return self;
}

- (void)setCollectionView:(UICollectionView *)collectionView
{
    WXAssertMainThread();
    
    if (_collectionView == collectionView) {
        return;
    }
    
    _collectionView = collectionView;
    
    for (NSString *templateType in [_templateTypeMap.keyEnumerator.allObjects copy]) {
        [self _registerCellClassForReuseID:templateType];
    }
}

- (void)addTemplate:(WXCellSlotComponent *)component
{
    WXAssertMainThread();
    
    NSString *templateType = component.templateCaseType;
    WXAssert(templateType != nil, @"cell-slot:%@ must have a template id!", component);
    
    [_templateTypeMap setObject:component forKey:templateType];
    if (_collectionView) {
        [self _registerCellClassForReuseID:templateType];
    }
}

- (WXCellSlotComponent *)dequeueCellSlotWithType:(NSString *)type forIndexPath:(NSIndexPath *)indexPath
{
    WXAssertMainThread();
    
    WXCellSlotComponent *cellSlot = [_templateTypeMap objectForKey:type];
    return [cellSlot copy];
}

- (WXCellSlotComponent *)templateWithType:(NSString *)type
{
    return [_templateTypeMap objectForKey:type];
}

- (void)_registerCellClassForReuseID:(NSString *)templateID
{
    WXLogDebug(@"register cell class for template id:%@", templateID);
    [_registeredTemplates addObject:templateID];
    [_collectionView registerClass:[WXReusableCollectionViewCell class] forCellWithReuseIdentifier:templateID];
}

- (WXCellSlotComponent *)topTemplate
{
    WXCellSlotComponent * cellTemplate = nil;
    for (NSString *templateType in [_templateTypeMap.keyEnumerator.allObjects copy]) {
        cellTemplate = [self templateWithType:templateType];
        if (!cellTemplate) {
            break;
        }
    }
    return cellTemplate;
}

- (BOOL)isTemplateRegistered:(NSString *)aTemplate
{
    WXAssertMainThread();
    return [_registeredTemplates containsObject:aTemplate];
}

- (NSString *)anyRegisteredTemplate
{
    WXAssertMainThread();
    return [_registeredTemplates anyObject];
}

@end
