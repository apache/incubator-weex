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

#import "WXRecycleListUpdateManager.h"
#import "WXLog.h"
#import "WXAssert.h"
#import "WXDiffUtil.h"

@interface WXRecycleListDiffResult : NSObject

@property (nonatomic, strong, readonly) NSMutableSet<NSIndexPath *> *deleteIndexPaths;
@property (nonatomic, strong, readonly) NSMutableSet<NSIndexPath *> *insertIndexPaths;
@property (nonatomic, strong, readonly) NSMutableSet<NSIndexPath *> *reloadIndexPaths;

- (BOOL)hasChanges;

@end

@implementation WXRecycleListDiffResult

- (instancetype)initWithInsertIndexPaths:(NSMutableSet<NSIndexPath *> *)insertIndexPaths
                        deleteIndexPaths:(NSMutableSet<NSIndexPath *> *)deleteIndexPaths
                        reloadIndexPaths:(NSMutableSet<NSIndexPath *> *)reloadIndexPaths
{
    if (self = [super init]) {
        _insertIndexPaths = [insertIndexPaths copy];
        _deleteIndexPaths = [deleteIndexPaths copy];
        _reloadIndexPaths = [reloadIndexPaths copy];
    }
    
    return self;
}

- (BOOL)hasChanges
{
    return _insertIndexPaths.count > 0 || _deleteIndexPaths.count > 0 || _reloadIndexPaths.count > 0;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@: %p; insert index paths: %@; delete index paths: %@; reload index paths: %@", NSStringFromClass([self class]), self, _insertIndexPaths, _deleteIndexPaths, _reloadIndexPaths];
}

@end

@interface WXRecycleListUpdateManager ()

@property (nonatomic, copy) NSArray *newerData;
@property (nonatomic, copy) NSArray *appendingData;
@property (nonatomic, copy) NSArray *olderData;
@property (nonatomic, assign) BOOL isUpdating;
@property (nonatomic, strong) NSMutableArray *completions;

@property (nonatomic, strong) NSMutableSet<NSIndexPath *> *reloadIndexPaths;

@end

@implementation WXRecycleListUpdateManager

- (instancetype)init
{
    if (self = [super init]) {
        _completions = [NSMutableArray array];
    }
    
    return self;
}

- (void)reload
{
    [_collectionView reloadData];
}

- (void)updateWithNewData:(NSArray *)newData
                  oldData:(NSArray *)oldData
               completion:(WXRecycleListUpdateCompletion)completion
                animation:(BOOL)isAnimated
{
    WXAssertMainThread();
    
    if (!_collectionView) {
        WXLogError(@"Update list with no collection view");
        completion(NO);
        return;
    }
    
    self.newerData = newData;
    self.appendingData = nil;
    self.olderData = oldData;
    
    if (completion) {
        [_completions addObject:completion];
    }
    
    [self checkUpdates];
}

- (void)updateWithAppendingData:(NSArray *)appendingData
                        oldData:(NSArray *)oldData
                     completion:(WXRecycleListUpdateCompletion)completion
                      animation:(BOOL)isAnimated
{
    if (!_collectionView) {
        WXLogError(@"Update list with no collection view");
        completion(NO);
        return;
    }
    
    self.appendingData = appendingData;
    self.olderData = oldData;
    
    if (completion) {
        [_completions addObject:completion];
    }
    
    [self checkUpdates];
}


- (void)checkUpdates
{
    dispatch_async(dispatch_get_main_queue(), ^{
        if (self.isUpdating) {
            return ;
        }
        
        [self performBatchUpdates];
    });
}

- (void)performBatchUpdates
{
    WXAssertMainThread();
    WXAssert(!self.isUpdating, @"Can not perform updates while an updating is being performed");
    
    UICollectionView *collectionView = self.collectionView;
    if (!collectionView) {
        return;
    }
    
    NSArray *newData = [self.newerData copy];
    NSArray *oldData = [self.olderData copy];
    NSArray *appendingData = [self.appendingData copy];
    //TODO use completionBlocks
//    NSArray *completionBlocks = [self.completions copy];
    
    [self cleanup];
    
    WXDiffResult *diffResult;
    if (appendingData) {
        newData = [oldData arrayByAddingObjectsFromArray:appendingData];
        NSIndexSet *inserts = [NSIndexSet indexSetWithIndexesInRange:NSMakeRange(oldData.count, appendingData.count)];
        diffResult = [[WXDiffResult alloc] initWithInserts:inserts deletes:nil updates:nil];
    } else if (newData){
        diffResult = [WXDiffUtil diffWithMinimumDistance:newData oldArray:oldData];
    }
    
    WXRecycleListDiffResult *recycleListDiffResult = [self recycleListUpdatesByDiffResult:diffResult];
    
    if (![diffResult hasChanges] && self.reloadIndexPaths.count == 0) {
        return;
    }
    
    void (^updates)() = [^{
        [self.delegate updateManager:self willUpdateData:newData];
        [UIView setAnimationsEnabled:NO];
        NSLog(@"UICollectionView update:%@", recycleListDiffResult);
        [self applyUpdateWithDiffResult:recycleListDiffResult];
    } copy];
    
    void (^completion)(BOOL) = [^(BOOL finished) {
        [UIView setAnimationsEnabled:YES];
        self.isUpdating = NO;
        [self.delegate updateManager:self didUpdateData:newData withSuccess:finished];
        
        [self.reloadIndexPaths removeAllObjects];
        [self checkUpdates];
    } copy];
    
    self.isUpdating = YES;
    
    if (!self.delegate || !collectionView.dataSource) {
        return;
    }
    
    [collectionView performBatchUpdates:updates completion:completion];
}

- (WXRecycleListDiffResult *)recycleListUpdatesByDiffResult:(WXDiffResult *)diffResult
{
    NSMutableSet<NSIndexPath *> *reloadIndexPaths = [NSMutableSet set];
    NSMutableSet<NSIndexPath *> *deleteIndexPaths = [NSMutableSet set];
    NSMutableSet<NSIndexPath *> *insertIndexPaths = [NSMutableSet set];
    
    for (WXDiffUpdateIndex *update in diffResult.updates) {
        NSIndexPath *reloadIndexPath = [NSIndexPath indexPathForItem:update.oldIndex inSection:0];
        [reloadIndexPaths addObject:reloadIndexPath];
    }
    
    [diffResult.updates enumerateObjectsUsingBlock:^(WXDiffUpdateIndex * _Nonnull update, NSUInteger idx, BOOL * _Nonnull stop) {
        NSIndexPath *reloadIndexPath = [NSIndexPath indexPathForItem:update.oldIndex inSection:0];
        [reloadIndexPaths addObject:reloadIndexPath];
    }];
    
    [diffResult.inserts enumerateIndexesUsingBlock:^(NSUInteger insertIndex, BOOL * _Nonnull stop) {
        NSIndexPath *insertIndexPath = [NSIndexPath indexPathForItem:insertIndex inSection:0];
        [insertIndexPaths addObject:insertIndexPath];
    }];
    
    [diffResult.deletes enumerateIndexesUsingBlock:^(NSUInteger deleteIndex, BOOL * _Nonnull stop) {
        NSIndexPath *deleteIndexPath = [NSIndexPath indexPathForItem:deleteIndex inSection:0];
        [deleteIndexPaths addObject:deleteIndexPath];
    }];
    
    WXRecycleListDiffResult *result = [[WXRecycleListDiffResult alloc] initWithInsertIndexPaths:insertIndexPaths deleteIndexPaths:deleteIndexPaths reloadIndexPaths:reloadIndexPaths];
    
    return result;
}


- (void)applyUpdateWithDiffResult:(WXRecycleListDiffResult *)diffResult
{
    if (!_collectionView) {
        return;
    }
    
    // reload index paths should not inculde delete index paths, otherwise it will cause crash:
    // Assertion failure in
    // -[UICollectionView _endItemAnimationsWithInvalidationContext:tentativelyForReordering:animator:]
    NSMutableSet *reloadIndexPaths = self.reloadIndexPaths ? [[diffResult.reloadIndexPaths setByAddingObjectsFromSet:self.reloadIndexPaths] mutableCopy]: [diffResult.reloadIndexPaths mutableCopy];
    [reloadIndexPaths minusSet:diffResult.deleteIndexPaths];
    
    [_collectionView deleteItemsAtIndexPaths:[diffResult.deleteIndexPaths allObjects]];
    [_collectionView insertItemsAtIndexPaths:[diffResult.insertIndexPaths allObjects]];
    [_collectionView reloadItemsAtIndexPaths:[reloadIndexPaths allObjects]];
}

- (void)cleanup
{
    self.newerData = nil;
    self.appendingData = nil;
    self.olderData = nil;
    [self.completions removeAllObjects];
}

@end
