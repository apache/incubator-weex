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

#import "WXRecycleListDataManager.h"
#import "NSArray+Weex.h"
#import "WXLog.h"
#import "WXAssert.h"

@implementation WXRecycleListDataManager
{
    NSArray *_data;
    NSMapTable<NSString*, NSDictionary*>* _virtualComponentData;
    NSMapTable<NSIndexPath*, NSMutableSet*>* _renderStatus;
}

- (instancetype)init
{
    if (self = [super init]) {
        _virtualComponentData = [NSMapTable strongToStrongObjectsMapTable];
        _renderStatus = [NSMapTable strongToStrongObjectsMapTable];
    }
    return self;
}

- (instancetype)initWithData:(NSArray *)data
{
    if (self = [super init]) {
        if (![data isKindOfClass:[NSArray class]]) {
            WXLogError(@"list data must be an array!");
        } else {
            _data = data;
        }
    }
    
    return self;
}

- (void)updateData:(NSArray *)data
{
    WXAssertMainThread();
    
    _data = data;
}

- (NSArray *)data
{
    WXAssertMainThread();
    
    return _data;
}

- (NSDictionary *)dataAtIndex:(NSInteger)index
{
    WXAssertMainThread();
    
    return [_data wx_safeObjectAtIndex:index];
}

- (NSInteger)numberOfItems
{
    WXAssertMainThread();
    
    return [_data count];
}

- (void)updateVirtualComponentData:(NSString *)componentId data:(NSDictionary *)data
{
    if (!componentId) {
        return;
    }
    [_virtualComponentData setObject:data forKey:componentId];

    NSIndexPath *indexPath = [data objectForKey:@"indexPath"];
    NSMutableSet *ids = [_renderStatus objectForKey:indexPath];
    if (!ids) {
        ids = [NSMutableSet set];
    }
    [ids addObject:componentId];
    [_renderStatus setObject:ids forKey:indexPath];
}

- (void)deleteVirtualComponentAtIndexPaths:(NSArray<NSIndexPath*>*)indexPaths
{
    [_virtualComponentData removeAllObjects];
    [_renderStatus removeAllObjects];
}

- (NSDictionary *)virtualComponentDataWithId:(NSString *)componentId
{
    return [_virtualComponentData objectForKey:componentId];
}

- (NSString *)virtualComponentIdWithIndexPath:(NSIndexPath *)indexPath templateId:(NSString *)templateId
{
    if (!templateId) {
        return nil;
    }
    NSSet *ids = [_renderStatus objectForKey:indexPath];
    if (!ids) {
        return nil;
    }
    for (NSString *componentId in ids) {
        if ([componentId containsString:templateId]) {
            return componentId;
        }
    }
    return nil;
}

- (NSDictionary *)virtualComponentDataWithIndexPath:(NSIndexPath*)indexPath templateId:(NSString *)templateId
{
    NSString *componentId = [self virtualComponentIdWithIndexPath:indexPath templateId:templateId];
    return [self virtualComponentDataWithId:componentId];
}

- (NSInteger)numberOfVirtualComponent
{
    return [_virtualComponentData count];
}

@end
