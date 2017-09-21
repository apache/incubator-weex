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

#import "WXDiffUtil.h"
#import "WXLog.h"

typedef enum : NSUInteger {
    WXDiffOperationDoNothing,
    WXDiffOperationUpdate,
    WXDiffOperationDelete,
    WXDiffOperationInsert
} WXDiffOperation;

@implementation WXDiffUpdateIndex

- (instancetype)initWithOldIndex:(NSUInteger)oldIndex newIndex:(NSUInteger)newIndex
{
    if (self = [super init]) {
        _oldIndex = oldIndex;
        _newIndex = newIndex;
    }
    
    return self;
}

@end

@implementation WXDiffResult

- (instancetype)initWithInserts:(NSIndexSet *)inserts
                        deletes:(NSIndexSet *)deletes
                        updates:(NSArray<WXDiffUpdateIndex *> *)updates
{
    if (self = [super init]) {
        _inserts = [inserts copy];
        _deletes = [deletes copy];
        _updates = [updates copy];
    }
    
    return self;
}

- (BOOL)hasChanges
{
    return _updates.count > 0 || _inserts.count > 0 || _deletes.count > 0;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@: %p; %zi inserts; %zi deletes; %zi updates", NSStringFromClass([self class]), self, _inserts.count, _deletes.count, _updates.count];
}


@end

@implementation WXDiffUtil

+ (WXDiffResult *)diffWithMinimumDistance:(NSArray<id<WXDiffable>> *)newArray oldArray:(NSArray<id<WXDiffable>> *)oldArray
{
    // Using the levenshtein algorithm
    // https://en.wikipedia.org/wiki/Levenshtein_distance
    
    int oldSize = (int)(oldArray.count + 1);
    int newSize = (int)(newArray.count + 1);
    
    int **matrix = malloc(oldSize * sizeof(int *));
    for (int i = 0; i < oldSize; i++) {
        matrix[i] = malloc(newSize * sizeof(int));
    }
    
    matrix[0][0] = 0;
    
    for (int i = 1; i < oldSize; i++) {
        matrix[i][0] = i;
    }
    
    for (int j = 1; j < newSize; j++) {
        matrix[0][j] = j;
    }
    
    for (int oldIndex = 1; oldIndex < oldSize; oldIndex ++) {
        for (int newIndex = 1; newIndex < newSize; newIndex ++) {
            if ([oldArray[oldIndex - 1] weex_isEqualTo:newArray[newIndex - 1]]) {
                matrix[oldIndex][newIndex] = matrix[oldIndex - 1][newIndex - 1];
            } else {
                int updateCost = matrix[oldIndex - 1][newIndex - 1] + 1;
                int insertCost = matrix[oldIndex][newIndex - 1] + 1;
                int deleteCost = matrix[oldIndex - 1][newIndex] + 1;
                matrix[oldIndex][newIndex] = MIN(MIN(insertCost, deleteCost), updateCost);
            }
        }
    }
    
#if DEBUG
    [self _printMatrix:matrix rowSize:oldSize columnSize:newSize];
#endif
    
    NSMutableArray *updates = [NSMutableArray array];
    NSMutableIndexSet *inserts = [NSMutableIndexSet indexSet];
    NSMutableIndexSet *deletes = [NSMutableIndexSet indexSet];
    int oldIndex = oldSize - 1;
    int newIndex = newSize - 1;
    while (oldIndex != 0 || newIndex != 0) {
        WXDiffOperation operation = [self _operationInMatrix:matrix newIndex:newIndex oldIndex:oldIndex];
        switch (operation) {
            case WXDiffOperationUpdate:
                newIndex --;
                oldIndex --;
                [updates addObject:[[WXDiffUpdateIndex alloc] initWithOldIndex:oldIndex newIndex:newIndex]];
                break;
            case WXDiffOperationDelete:
                oldIndex --;
                [deletes addIndex:oldIndex];
                break;
            case WXDiffOperationInsert:
                newIndex --;
                [inserts addIndex:newIndex];
                break;
            case WXDiffOperationDoNothing:
                newIndex --;
                oldIndex --;
                break;
        }
    }
    
    for (int i = 0; i < oldSize; i++) {
        free(matrix[i]);
    }
    free(matrix);
    
    WXDiffResult *result = [[WXDiffResult alloc] initWithInserts:inserts deletes:deletes updates:updates];
    return result;
}

+ (WXDiffOperation)_operationInMatrix:(int **)matrix newIndex:(int)newIndex oldIndex:(int)oldIndex
{
    if (newIndex == 0) {
        return WXDiffOperationDelete;
    }
    
    if (oldIndex == 0) {
        return WXDiffOperationInsert;
    }
    
    int cost = matrix[oldIndex][newIndex];
    
    int costBeforeInsert = matrix[oldIndex][newIndex - 1];
    if (costBeforeInsert + 1 == cost) {
        return WXDiffOperationInsert;
    }
    
    int costBeforDelete = matrix[oldIndex - 1][newIndex];
    if (costBeforDelete + 1 == cost) {
        return WXDiffOperationDelete;
    }
    
    int costBeforUpdate = matrix[oldIndex - 1][newIndex - 1];
    if (costBeforUpdate + 1 == cost) {
        return WXDiffOperationUpdate;
    }
    
    return WXDiffOperationDoNothing;
}

+ (void)_printMatrix:(int **)matrix rowSize:(int)rowSize columnSize:(int)columnSize
{
    for (int i = 0; i < rowSize; i ++) {
        NSMutableArray *array = [NSMutableArray array];
        for (int j = 0; j < columnSize; j ++) {
            int value = matrix[i][j];
            NSString *result;
            if (value < 10) {
                result = [NSString stringWithFormat:@"0%zi", value];
            } else {
                result = [NSString stringWithFormat:@"%zi", value];
            }
            [array addObject:result];
        }
        WXLogDebug(@"%@", [array componentsJoinedByString:@" "]);
    }
}

@end

@implementation NSNumber (WXDiffable)

- (BOOL)weex_isEqualTo:(id<WXDiffable>)object
{
    return [self isEqual:object];
}

@end

@implementation NSString (WXDiffable)

- (BOOL)weex_isEqualTo:(id<WXDiffable>)object
{
    return [self isEqual:object];
}

@end

@implementation NSArray (WXDiffable)

- (BOOL)weex_isEqualTo:(id<WXDiffable>)object
{
    if (![object isKindOfClass:[NSArray class]]) {
        return NO;
    }
    
    NSArray *array = (NSArray *)object;
    if (self.count != array.count) {
        return NO;
    }
    
    __block BOOL isEqual = YES;
    [array enumerateObjectsUsingBlock:^(id  _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        id obj2 = [self objectAtIndex:idx];
        if ([obj conformsToProtocol:@protocol(WXDiffable)] && [obj2 conformsToProtocol:@protocol(WXDiffable)]) {
            if (![obj weex_isEqualTo:obj2]) {
                isEqual = NO;
                *stop = YES;
            }
        } else {
            isEqual = NO;
            *stop = YES;
        }
    }];
    
    return isEqual;
}

@end

@implementation NSDictionary (WXDiffable)

- (BOOL)weex_isEqualTo:(id<WXDiffable>)object
{
    if (![object isKindOfClass:[NSDictionary class]]) {
        return NO;
    }
    
    NSDictionary *dictionary = (NSDictionary *)object;
    if (self.count != dictionary.count) {
        return NO;
    }
    
    __block BOOL isEqual = YES;
    [dictionary enumerateKeysAndObjectsUsingBlock:^(id  _Nonnull key, id  _Nonnull obj, BOOL * _Nonnull stop) {
        id obj2 = [self objectForKey:key];
        if (obj2 && [obj2 conformsToProtocol:@protocol(WXDiffable)] && [obj conformsToProtocol:@protocol(WXDiffable)]) {
            if (![obj weex_isEqualTo:obj2]) {
                isEqual = NO;
                *stop = YES;
            }
        } else {
            isEqual = NO;
            *stop = YES;
        }
    }];
    
    return isEqual;
}

@end
