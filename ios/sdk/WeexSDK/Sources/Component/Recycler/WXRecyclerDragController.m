//
//  WXRecyclerDragController.m
//  Pods
//
//  Created by ximu on 2017/7/31.
//
//

#import "WXRecyclerDragController.h"


@implementation WXRecyclerDragController

- (instancetype)init{
    self = [super init];
    if (self) {
        _excludedAry = [[NSMutableArray alloc] init];
        _isDragAnchor = NO;

    }
    return self;
}

- (void)setCollectionView:(UICollectionView *)collectionView{
    _collectionView = collectionView;
    _currentLongPress = [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(longPressMethod:)];
    _currentLongPress.minimumPressDuration = 0.3f;
    [_collectionView addGestureRecognizer:_currentLongPress];
}

- (void)setDragingCell:(UICollectionViewCell *)dragingCell{
    _dragingCell = dragingCell;
    _dragingCell.hidden = true;
    [_collectionView addSubview:_dragingCell];
}


- (void)goThroughAnchor:(WXComponent *)wxComponent indexPath:(NSIndexPath *)indexPath{
    if (wxComponent.attributes && [wxComponent.attributes[@"dragExcluded"] boolValue]){
        [_excludedAry addObject:indexPath];
        NSSet *set = [NSSet setWithArray:_excludedAry];
        [_excludedAry removeAllObjects];
        [_excludedAry addObjectsFromArray:[set allObjects]];
    }
    
    //遍历获取锚点
    NSMutableArray *subviewComponents = [[NSMutableArray alloc] init];
    [subviewComponents addObjectsFromArray:wxComponent.subcomponents];
    WXComponent *anchorComponent;
    for (int i = 0 ; i < subviewComponents.count ; i++){
        WXComponent *compoent = subviewComponents[i];
        if (compoent.attributes[@"dragAnchor"]) {
            anchorComponent = compoent;
            _isDragAnchor = YES;
            break;
        }
        
        if (compoent.subcomponents && compoent.subcomponents.count && compoent.subcomponents.count > 0) {
            [subviewComponents addObjectsFromArray:compoent.subcomponents];
        }
    }
    
    if (anchorComponent) {
        //去除全局UILongPressGestureRecognizer手势
        if (_currentLongPress) {
            [self.collectionView removeGestureRecognizer:_currentLongPress];
            _currentLongPress = nil;
        }
        
        //添加锚点的手势
        if (_dragTriggerType == WXRecyclerDragTriggerPan) {
            UIPanGestureRecognizer *panGestureRecognizer = [[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(longPressMethod:)];
            [anchorComponent.view addGestureRecognizer:panGestureRecognizer];
            
        }else{
            UILongPressGestureRecognizer *longPressGestureRecognizer = [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(longPressMethod:)];
            [anchorComponent.view addGestureRecognizer:longPressGestureRecognizer];
        }
    }
}

#pragma mark - dragMethod
- (void)longPressMethod:(UILongPressGestureRecognizer*)gesture
{
    if (_isDragable) {
        switch (gesture.state) {
            case UIGestureRecognizerStateBegan:
                [self dragBegin:gesture];
                break;
            case UIGestureRecognizerStateChanged:
                [self dragChanged:gesture];
                break;
            case UIGestureRecognizerStateEnded:
                [self dragEnd:gesture];
                break;
            default:
                break;
        }
    }
}

- (void)dragBegin:(UILongPressGestureRecognizer *)gesture{
    
    CGPoint point = [gesture locationInView:_collectionView];
    
    _startIndexPath = [self getDragingIndexPathWithPoint:point];
    if (!_startIndexPath) {
        return;
    }
    
    if ([self.delegate respondsToSelector:@selector(fireEvent:params:)]) {
        [self.delegate dragFireEvent:@"dragstart" params:@{@"fromIndex":[NSString stringWithFormat:@"%ld",(long)_startIndexPath.row]}];
    }
    
    _dragingIndexPath = [self getDragingIndexPathWithPoint:point];
    if (!_dragingIndexPath) {
        return;
    }
    
    [_collectionView bringSubviewToFront:_dragingCell];
    _dragingCell.frame = [_collectionView cellForItemAtIndexPath:_dragingIndexPath].frame;
    _dragingCell.hidden = false;
    [UIView animateWithDuration:0.3 animations:^{
        [_dragingCell setTransform:CGAffineTransformMakeScale(1.2, 1.2)];
    }];
}

- (void)dragChanged:(UILongPressGestureRecognizer *)gesture{
    
    if (!_startIndexPath) {
        return;
    }
    CGPoint point = [gesture locationInView:_collectionView];
    _dragingCell.center = point;
    _targetIndexPath = [self getTargetIndexPathWithPoint:point];
    
    if (_targetIndexPath && _dragingIndexPath && (_targetIndexPath.section == _startIndexPath.section)){
        [_collectionView moveItemAtIndexPath:_dragingIndexPath toIndexPath:_targetIndexPath];
        _dragingIndexPath = _targetIndexPath;
    }
}

- (void)dragEnd:(UILongPressGestureRecognizer *)gesture{
    
    if (!_startIndexPath || !_dragingIndexPath) {
        return;
    }
    if ([self.delegate respondsToSelector:@selector(fireEvent:params:)]) {
        [self.delegate dragFireEvent:@"dragend" params:@{@"toIndex":[NSString stringWithFormat:@"%ld",(long)_dragingIndexPath.row],@"fromIndex":[NSString stringWithFormat:@"%ld",(long)_startIndexPath.row]}];
    }
    
    CGRect endFrame = [_collectionView cellForItemAtIndexPath:_dragingIndexPath].frame;
    
    __weak typeof(self) weakSelf = self;
    [UIView animateWithDuration:0.3 animations:^{
        [weakSelf.dragingCell setTransform:CGAffineTransformMakeScale(1.0, 1.0)];
        weakSelf.dragingCell.frame = endFrame;
    } completion:^(BOOL finished) {
        weakSelf.dragingCell.hidden = YES;
        if ([weakSelf.delegate respondsToSelector:@selector(updateDataSource)]) {
            [weakSelf.delegate updateDataSource];
        }
        
    }];
}

- (NSIndexPath *)getDragingIndexPathWithPoint:(CGPoint)point{
    NSIndexPath *dragingIndexPath = nil;
    for (NSIndexPath *indexPath in [_collectionView indexPathsForVisibleItems]){
        if (CGRectContainsPoint([_collectionView cellForItemAtIndexPath:indexPath].frame,point)) {
            dragingIndexPath = indexPath;
            break;
        }
    }
    
    BOOL isExcluded = NO;
    if (dragingIndexPath) {
        for (NSIndexPath *indexPath in _excludedAry) {
            if (indexPath.row == dragingIndexPath.row) {
                isExcluded = YES;
            }
        }
    }
    return isExcluded?nil:dragingIndexPath;
}

- (NSIndexPath *)getTargetIndexPathWithPoint:(CGPoint)point{
    NSIndexPath *targetIndexPath = nil;
    for (NSIndexPath *indexPath in _collectionView.indexPathsForVisibleItems) {
        if (CGRectContainsPoint([_collectionView cellForItemAtIndexPath:indexPath].frame, point)) {
            targetIndexPath = indexPath;
        }
    }
    return targetIndexPath;
}

- (void)dealloc{
    
}
@end
