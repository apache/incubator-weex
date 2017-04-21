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

#import "WXListComponent.h"
#import "WXCellComponent.h"
#import "WXHeaderComponent.h"
#import "WXComponent.h"
#import "WXComponent_internal.h"
#import "NSArray+Weex.h"
#import "WXAssert.h"
#import "WXMonitor.h"
#import "WXUtility.h"
#import "NSObject+WXSwizzle.h"
#import "WXSDKInstance_private.h"
#import "WXRefreshComponent.h"
#import "WXLoadingComponent.h"

@interface WXTableView : UITableView

@end

@implementation WXTableView

+ (BOOL)requiresConstraintBasedLayout
{
    return NO;
}

- (void)layoutSubviews
{
    [super layoutSubviews];
    [self.wx_component layoutDidFinish];
}

- (void)setContentOffset:(CGPoint)contentOffset
{
    // FIXME: side effect caused by hooking _adjustContentOffsetIfNecessary.
    // When UITableView is pulled down and finger releases，contentOffset will be set from -xxxx to about -0.5(greater than -0.5), then contentOffset will be reset to zero by calling _adjustContentOffsetIfNecessary.
    // So hooking _adjustContentOffsetIfNecessary will always cause remaining 1px space between list's top and navigator.
    // Demo: http://dotwe.org/895630945793a9a044e49abe39cbb77f
    // Have to reset contentOffset to zero manually here.
    if (fabs(contentOffset.y) < 0.5) {
        contentOffset.y = 0;
    }
    
    [super setContentOffset:contentOffset];
}

@end


@interface WXSection : NSObject<NSCopying>

@property (nonatomic, strong) WXHeaderComponent *header;
@property (nonatomic, strong) NSMutableArray<WXCellComponent *> *rows;

@end

@implementation WXSection

- (instancetype)init
{
    if (self = [super init]) {
        _rows = [NSMutableArray array];
    }
    
    return self;
}

- (id)copyWithZone:(NSZone *)zone
{
    WXSection *newSection = [[[self class] allocWithZone:zone] init];
    newSection.header = _header;
    newSection.rows = [_rows mutableCopyWithZone:zone];
    
    return newSection;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"%@\n%@", [_header description], [_rows description]];
}
@end

@interface WXListComponent () <UITableViewDataSource, UITableViewDelegate, WXCellRenderDelegate, WXHeaderRenderDelegate>

@end

@implementation WXListComponent
{
    __weak UITableView * _tableView;

    // Only accessed on component thread
    NSMutableArray<WXSection *> *_sections;
    // Only accessed on main thread
    NSMutableArray<WXSection *> *_completedSections;
    NSUInteger _previousLoadMoreRowNumber;
    
    BOOL _isUpdating;
    NSMutableArray<void(^)()> *_updates;
    NSTimeInterval _reloadInterval;
}

- (instancetype)initWithRef:(NSString *)ref type:(NSString *)type styles:(NSDictionary *)styles attributes:(NSDictionary *)attributes events:(NSArray *)events weexInstance:(WXSDKInstance *)weexInstance
{
    if (self = [super initWithRef:ref type:type styles:styles attributes:attributes events:events weexInstance:weexInstance]) {
        _sections = [NSMutableArray array];
        _completedSections = [NSMutableArray array];
        _reloadInterval = attributes[@"reloadInterval"] ? [WXConvert CGFloat:attributes[@"reloadInterval"]]/1000 : 0;
        [self fixFlicker];
    }
    
    return self;
}

- (void)dealloc
{
    if (_tableView) {
        _tableView.delegate = nil;
        _tableView.dataSource = nil;
    }
}

- (UIView *)loadView
{
    return [[WXTableView alloc] init];
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    _tableView = (UITableView *)self.view;
    _tableView.allowsSelection = NO;
    _tableView.allowsMultipleSelection = NO;
    _tableView.separatorStyle = UITableViewCellSeparatorStyleNone;
    _tableView.delegate = self;
    _tableView.dataSource = self;
    _tableView.userInteractionEnabled = YES;
}

- (void)viewWillUnload
{
    [super viewWillUnload];
    
    _tableView.delegate = nil;
    _tableView.dataSource = nil;
}

- (void)updateAttributes:(NSDictionary *)attributes
{
    [super updateAttributes:attributes];
    
    if (attributes[@"reloadInterval"]) {
        _reloadInterval = [WXConvert CGFloat:attributes[@"reloadInterval"]] / 1000;
    }
}

- (void)setContentSize:(CGSize)contentSize
{
    // Do Nothing
}

- (void)_handleFirstScreenTime
{
    // Do Nothing， firstScreenTime is set by cellDidRendered:
}

- (void)scrollToComponent:(WXComponent *)component withOffset:(CGFloat)offset animated:(BOOL)animated
{
    CGPoint contentOffset = _tableView.contentOffset;
    CGFloat contentOffsetY = 0;
    
    WXComponent *cellComponent = component;
    CGRect cellRect;
    while (cellComponent) {
        if ([cellComponent isKindOfClass:[WXCellComponent class]]) {
            NSIndexPath *toIndexPath = [self indexPathForCell:(WXCellComponent*)cellComponent sections:_completedSections];
            cellRect = [_tableView rectForRowAtIndexPath:toIndexPath];
            break;
        }
        if ([cellComponent isKindOfClass:[WXHeaderComponent class]]) {
            NSUInteger toIndex = [self indexForHeader:(WXHeaderComponent *)cellComponent sections:_completedSections];
            cellRect = [_tableView rectForSection:toIndex];
            break;
        }
        contentOffsetY += cellComponent.calculatedFrame.origin.y;
        cellComponent = cellComponent.supercomponent;
    }
    
    contentOffsetY += cellRect.origin.y;
    contentOffsetY += offset * self.weexInstance.pixelScaleFactor;
    
    if (_tableView.contentSize.height >= _tableView.frame.size.height && contentOffsetY > _tableView.contentSize.height - _tableView.frame.size.height) {
        contentOffset.y = _tableView.contentSize.height - _tableView.frame.size.height;
    } else {
        contentOffset.y = contentOffsetY;
    }
    
    [_tableView setContentOffset:contentOffset animated:animated];
}


#pragma mark - Inheritance

- (void)_insertSubcomponent:(WXComponent *)subcomponent atIndex:(NSInteger)index
{
    if ([subcomponent isKindOfClass:[WXCellComponent class]]) {
        ((WXCellComponent *)subcomponent).delegate = self;
    } else if ([subcomponent isKindOfClass:[WXHeaderComponent class]]) {
        ((WXHeaderComponent *)subcomponent).delegate = self;
    } else if (![subcomponent isKindOfClass:[WXRefreshComponent class]]
               && ![subcomponent isKindOfClass:[WXLoadingComponent class]]
               && subcomponent->_positionType != WXPositionTypeFixed) {
        WXLogError(@"list only support cell/header/refresh/loading/fixed-component as child.");
        return;
    }
    
    [super _insertSubcomponent:subcomponent atIndex:index];
    
    if (![subcomponent isKindOfClass:[WXHeaderComponent class]]
        && ![subcomponent isKindOfClass:[WXCellComponent class]]) {
        // Don't insert section if subcomponent is not header or cell
        return;
    }
    
    NSIndexPath *indexPath = [self indexPathForSubIndex:index];
    
    if ([subcomponent isKindOfClass:[WXHeaderComponent class]] || _sections.count <= indexPath.section) {
        // conditions to insert section: insert a header or insert first cell of table view
        // this will be updated by recycler's update controller in the future
        WXSection *insertSection = [WXSection new];
        BOOL keepScrollPosition = NO;
        if ([subcomponent isKindOfClass:[WXHeaderComponent class]]) {
            WXHeaderComponent *header = (WXHeaderComponent*)subcomponent;
            insertSection.header = header;
            keepScrollPosition = header.keepScrollPosition;
        }
        
        NSUInteger insertIndex = indexPath.section;
        WXSection *reloadSection;
        if (insertIndex > 0 && insertIndex <= _sections.count
            && [subcomponent isKindOfClass:[WXHeaderComponent class]]) {
            // insert a header in the middle, one section may divide into two
            // so the original section need to be reloaded
            NSIndexPath *indexPathBeforeHeader = [self indexPathForSubIndex:index - 1];
            if (_sections[insertIndex - 1].rows.count != 0 && indexPathBeforeHeader.row < _sections[insertIndex - 1].rows.count - 1) {
                reloadSection = _sections[insertIndex - 1];
                NSArray *rowsToSeparate = reloadSection.rows;
                insertSection.rows = [[rowsToSeparate subarrayWithRange:NSMakeRange(indexPathBeforeHeader.row + 1, rowsToSeparate.count - indexPathBeforeHeader.row - 1)] mutableCopy];
                reloadSection.rows = [[rowsToSeparate subarrayWithRange:NSMakeRange(0, indexPathBeforeHeader.row + 1)]  mutableCopy];
            }
        }
    
        [_sections insertObject:insertSection atIndex:insertIndex];
        WXSection *completedInsertSection = [insertSection copy];
        WXSection *completedReloadSection;
        if (reloadSection) {
            completedReloadSection = [reloadSection copy];
        }
        
        [self.weexInstance.componentManager _addUITask:^{
            WXLogDebug(@"Insert section:%ld", insertIndex);
            [_completedSections insertObject:completedInsertSection atIndex:insertIndex];
            if (completedReloadSection) {
                WXLogDebug(@"Reload section:%ld", insertIndex - 1);
                _completedSections[insertIndex - 1] = completedReloadSection;
            }
            
            [UIView performWithoutAnimation:^{
                [_tableView beginUpdates];
                
                [self _insertTableViewSectionAtIndex:insertIndex keepScrollPosition:keepScrollPosition animation:UITableViewRowAnimationNone];
                
                if (completedReloadSection) {
                    [_tableView reloadSections:[NSIndexSet indexSetWithIndex:insertIndex - 1] withRowAnimation:UITableViewRowAnimationNone];
                }
                
                [_tableView endUpdates];
            }];
            
        }];
    }
}

- (void)insertSubview:(WXComponent *)subcomponent atIndex:(NSInteger)index
{
    //Here will not insert cell or header's view again
    if (![subcomponent isKindOfClass:[WXCellComponent class]]
        && ![subcomponent isKindOfClass:[WXHeaderComponent class]]) {
        [super insertSubview:subcomponent atIndex:index];
    }
}

#pragma mark - WXHeaderRenderDelegate

- (float)headerWidthForLayout:(WXHeaderComponent *)cell
{
    return self.scrollerCSSNode->style.dimensions[CSS_WIDTH];
}

- (void)headerDidLayout:(WXHeaderComponent *)header
{
    [self.weexInstance.componentManager _addUITask:^{
        // trigger section header update
        [UIView performWithoutAnimation:^{
            [_tableView beginUpdates];
            
            NSUInteger reloadIndex = [self indexForHeader:header sections:_completedSections];
            [_tableView reloadSections:[NSIndexSet indexSetWithIndex:reloadIndex] withRowAnimation:UITableViewRowAnimationNone];
            
            [_tableView endUpdates];
        }];
    }];
}

- (void)headerDidRemove:(WXHeaderComponent *)header
{
    NSUInteger deleteIndex = [self indexForHeader:header sections:_sections];
    // this will be updated by recycler's update controller in the future
    WXSection *deleteSection = _sections[deleteIndex];
    WXSection *reloadSection;
    if (deleteIndex > 0 && deleteSection.rows.count > 0) {
        // delete a header in the middle, two sections merge into one
        // so the one section need to be reloaded
        reloadSection = _sections[deleteIndex - 1];
        reloadSection.rows = [[reloadSection.rows arrayByAddingObjectsFromArray:deleteSection.rows] mutableCopy];
    }
    
    [_sections removeObjectAtIndex:deleteIndex];
    WXSection *completedReloadSection;
    if (reloadSection) {
        completedReloadSection = [reloadSection copy];
    }
    BOOL keepScrollPosition = header.keepScrollPosition;
    
    [self.weexInstance.componentManager _addUITask:^{
        WXLogDebug(@"delete section:%ld", deleteIndex);
        [_completedSections removeObjectAtIndex:deleteIndex];
        if (completedReloadSection) {
            WXLogDebug(@"Reload section:%ld", deleteIndex - 1);
            _completedSections[deleteIndex - 1] = completedReloadSection;
        }
        
        [UIView performWithoutAnimation:^{
            [_tableView beginUpdates];
            [self _deleteTableViewSectionAtIndex:deleteIndex keepScrollPosition:keepScrollPosition animation:UITableViewRowAnimationNone];
            if (completedReloadSection) {
                [_tableView reloadSections:[NSIndexSet indexSetWithIndex:deleteIndex - 1] withRowAnimation:UITableViewRowAnimationNone];
            }
            
            [_tableView endUpdates];
        }];
        
    }];
}

#pragma mark - WXCellRenderDelegate

- (float)containerWidthForLayout:(WXCellComponent *)cell
{
    return self.scrollerCSSNode->style.dimensions[CSS_WIDTH];
}

- (void)cellDidRemove:(WXCellComponent *)cell
{
    WXAssertComponentThread();
    
    NSIndexPath *indexPath = [self indexPathForCell:cell sections:_sections];
    [self removeCellForIndexPath:indexPath withSections:_sections];
    
    [self.weexInstance.componentManager _addUITask:^{
        [self removeCellForIndexPath:indexPath withSections:_completedSections];
        
        WXLogDebug(@"Delete cell:%@ at indexPath:%@", cell.ref, indexPath);
        if (cell.deleteAnimation == UITableViewRowAnimationNone) {
            [UIView performWithoutAnimation:^{
                [self _deleteTableViewCellAtIndexPath:indexPath keepScrollPosition:cell.keepScrollPosition animation:UITableViewRowAnimationNone];
            }];
        } else {
            [self _deleteTableViewCellAtIndexPath:indexPath keepScrollPosition:cell.keepScrollPosition animation:cell.deleteAnimation];
        }
    }];
}

- (void)cellDidLayout:(WXCellComponent *)cell
{
    WXAssertComponentThread() ;
    
    NSUInteger index = [self.subcomponents indexOfObject:cell];
    NSIndexPath *indexPath = [self indexPathForSubIndex:index];

    NSInteger sectionNum = indexPath.section;
    NSInteger row = indexPath.row;
    NSMutableArray *sections = _sections;
    WXSection *section = sections[sectionNum];
    WXAssert(section, @"no section found for section number:%ld", sectionNum);
    NSMutableArray *completedSections;
    BOOL isReload = [section.rows containsObject:cell];
    if (!isReload && row > [section.rows count]) {
        // protect crash when row out of bounds
        return ;
    }
    if (!isReload) {
        [section.rows insertObject:cell atIndex:row];
        // deep copy
        completedSections = [[NSMutableArray alloc] initWithArray:sections copyItems:YES];;
    }
    
    [self.weexInstance.componentManager _addUITask:^{
        if (!isReload) {
            WXLogDebug(@"Insert cell:%@ at indexPath:%@", cell.ref, indexPath);
            _completedSections = completedSections;
            if (cell.insertAnimation == UITableViewRowAnimationNone) {
                [UIView performWithoutAnimation:^{
                    [self _insertTableViewCellAtIndexPath:indexPath keepScrollPosition:cell.keepScrollPosition animation:UITableViewRowAnimationNone];
                }];
            } else {
                [self _insertTableViewCellAtIndexPath:indexPath keepScrollPosition:cell.keepScrollPosition animation:cell.insertAnimation];
            }
        } else {
            WXLogInfo(@"Reload cell:%@ at indexPath:%@", cell.ref, indexPath);
            [UIView performWithoutAnimation:^{
                [_tableView reloadRowsAtIndexPaths:[NSArray arrayWithObject:indexPath] withRowAnimation:UITableViewRowAnimationNone];
                [self handleAppear];
            }];
        }
    }];
}

- (void)cellDidRendered:(WXCellComponent *)cell
{
    WXAssertMainThread();
    
    if (WX_MONITOR_INSTANCE_PERF_IS_RECORDED(WXPTFirstScreenRender, self.weexInstance) && !self.weexInstance.onRenderProgress) {
        // improve performance
        return;
    }
    
    NSIndexPath *indexPath = [self indexPathForCell:cell sections:_completedSections];
    if (!indexPath || indexPath.section >= [_tableView numberOfSections] ||
        indexPath.row < 0 || indexPath.row >= [_tableView numberOfRowsInSection:indexPath.section]) {
        WXLogWarning(@"Rendered cell:%@ out of range, sections:%@", cell, _completedSections);
        return;
    }
    
    CGRect cellRect = [_tableView rectForRowAtIndexPath:indexPath];
    if (cellRect.origin.y + cellRect.size.height >= _tableView.frame.size.height) {
        WX_MONITOR_INSTANCE_PERF_END(WXPTFirstScreenRender, self.weexInstance);
    }
    
    if (self.weexInstance.onRenderProgress) {
        CGRect renderRect = [_tableView convertRect:cellRect toView:self.weexInstance.rootView];
        self.weexInstance.onRenderProgress(renderRect);
    }
}

- (void)cell:(WXCellComponent *)cell didMoveToIndex:(NSUInteger)index
{
    WXAssertComponentThread();
    
    NSIndexPath *fromIndexPath = [self indexPathForCell:cell sections:_sections];
    NSIndexPath *toIndexPath = [self indexPathForSubIndex:index];
    [self removeCellForIndexPath:fromIndexPath withSections:_sections];
    [self insertCell:cell forIndexPath:toIndexPath withSections:_sections];
    
    [self.weexInstance.componentManager _addUITask:^{
        if (_reloadInterval > 0) {
            // use [UITableView reloadData] to do batch updates, will move to recycler's update controller
            __weak typeof(self) weakSelf = self;
            if (!_updates) {
                _updates = [NSMutableArray array];
            }
            [_updates addObject:^{
                __strong typeof(weakSelf) strongSelf = weakSelf;
                [strongSelf removeCellForIndexPath:fromIndexPath withSections:strongSelf->_completedSections];
                [strongSelf insertCell:cell forIndexPath:toIndexPath withSections:strongSelf->_completedSections];
            }];
            
            [self checkReloadData];
        } else {
            [self removeCellForIndexPath:fromIndexPath withSections:_completedSections];
            [self insertCell:cell forIndexPath:toIndexPath withSections:_completedSections];
            [UIView performWithoutAnimation:^{
                [_tableView beginUpdates];
                [_tableView moveRowAtIndexPath:fromIndexPath toIndexPath:toIndexPath];
                [self handleAppear];
                [_tableView endUpdates];
            }];
        }
    }];
}

- (void)checkReloadData
{
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(_reloadInterval * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        if (_isUpdating || _updates.count == 0) {
            return ;
        }
        
        _isUpdating = YES;
        NSArray *updates = [_updates copy];
        [_updates removeAllObjects];
        for (void(^update)() in updates) {
            update();
        }
        [_tableView reloadData];
        _isUpdating = NO;
        
        [self checkReloadData];
    });
}

- (void)addStickyComponent:(WXComponent *)sticky
{
    
}

- (void)removeStickyComponent:(WXComponent *)sticky
{

}
#pragma mark - TableView delegate

- (void)tableView:(UITableView *)tableView willDisplayCell:(UITableViewCell *)cell forRowAtIndexPath:(NSIndexPath *)indexPath
{
    
}

- (void)tableView:(UITableView *)tableView didEndDisplayingCell:(UITableViewCell *)cell forRowAtIndexPath:(NSIndexPath *)indexPath
{
    WXLogDebug(@"Did end displaying cell:%@, at index path:%@", cell, indexPath);
    NSArray *visibleIndexPaths = [tableView indexPathsForVisibleRows];
    if (![visibleIndexPaths containsObject:indexPath]) {
        if (cell.contentView.subviews.count > 0) {
            UIView *wxCellView = [cell.contentView.subviews firstObject];
            // Must invoke synchronously otherwise it will remove the view just added.
            WXCellComponent *cellComponent = (WXCellComponent *)wxCellView.wx_component;
            if (cellComponent.isRecycle) {
                [wxCellView.wx_component _unloadViewWithReusing:YES];
            }
        }
    }
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    WXCellComponent *cell = [self cellForIndexPath:indexPath];
    return cell.calculatedFrame.size.height;
}

- (CGFloat)tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger)section
{
    WXHeaderComponent *header = ((WXSection *)_completedSections[section]).header;
    if (header) {
        return header.calculatedFrame.size.height;
    } else {
        return 0.0;
    }
}

- (UIView *)tableView:(UITableView *)tableView viewForHeaderInSection:(NSInteger)section
{
    WXHeaderComponent *header = ((WXSection *)_completedSections[section]).header;
    WXLogDebug(@"header view for section %ld:%@", (long)section, header.view);
    return header.view;
}

#pragma mark - TableView Data Source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return _completedSections.count;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return ((WXSection *)[_completedSections wx_safeObjectAtIndex:section]).rows.count;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    WXLogDebug(@"Getting cell at indexPath:%@", indexPath);
    static NSString *reuseIdentifier = @"WXTableViewCell";
    
    UITableViewCell *cellView = [_tableView dequeueReusableCellWithIdentifier:reuseIdentifier];
    if (!cellView) {
        cellView = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:reuseIdentifier];
        cellView.backgroundColor = [UIColor clearColor];
    }
    
    WXCellComponent *cell = [self cellForIndexPath:indexPath];
    
    if (!cell) {
        return cellView;
    }
    
    if (cell.view.superview == cellView.contentView) {
        return cellView;
    }
    
    for (UIView *view in cellView.contentView.subviews) {
        [view removeFromSuperview];
    }
    
    [cellView.contentView addSubview:cell.view];
    
    [cellView setAccessibilityIdentifier:cell.view.accessibilityIdentifier];
    
    WXLogDebug(@"Created cell:%@ view:%@ cellView:%@ at indexPath:%@", cell.ref, cell.view, cellView, indexPath);
    return cellView;
}

#pragma mark - Load More Event

- (void)setLoadmoreretry:(NSUInteger)loadmoreretry
{
    if (loadmoreretry != self.loadmoreretry) {
        _previousLoadMoreRowNumber = 0;
    }
    
    [super setLoadmoreretry:loadmoreretry];
}

- (void)loadMore
{
    [super loadMore];
    
    _previousLoadMoreRowNumber = [self totalNumberOfRows];
}

- (BOOL)isNeedLoadMore
{
    BOOL superNeedLoadMore = [super isNeedLoadMore];
    return superNeedLoadMore && _previousLoadMoreRowNumber != [self totalNumberOfRows];
}

- (NSUInteger)totalNumberOfRows
{
    NSUInteger rowNumber = 0;
    NSUInteger sectionCount = [_tableView numberOfSections];
    for (int section = 0; section < sectionCount; section ++) {
        rowNumber += [_tableView numberOfRowsInSection:section];
    }
    
    return rowNumber;
}

- (void)resetLoadmore{
    [super resetLoadmore];
    _previousLoadMoreRowNumber=0;
}

#pragma mark Private

- (WXCellComponent *)cellForIndexPath:(NSIndexPath *)indexPath
{
    WXSection *section = [_completedSections wx_safeObjectAtIndex:indexPath.section];
    if (!section) {
        WXLogError(@"No section found for num:%ld, completed sections:%ld", (long)indexPath.section, (unsigned long)_completedSections.count);
        return nil;
    }
    
    WXCellComponent *cell = [section.rows wx_safeObjectAtIndex:indexPath.row];
    if (!cell) {
        WXLogError(@"No cell found for num:%ld, completed rows:%ld", (long)indexPath.row, (unsigned long)section.rows.count);
        return nil;
    }
    
    return cell;
}

- (void)insertCell:(WXCellComponent *)cell forIndexPath:(NSIndexPath *)indexPath withSections:(NSMutableArray *)sections
{
    WXSection *section = [sections wx_safeObjectAtIndex:indexPath.section];
    WXAssert(section, @"inserting cell at indexPath:%@ section has not been inserted to list before, sections:%@", indexPath, sections);
    WXAssert(indexPath.row <= section.rows.count, @"inserting cell at indexPath:%@ outof range, sections:%@", indexPath, sections);
    [section.rows insertObject:cell atIndex:indexPath.row];
}

- (void)removeCellForIndexPath:(NSIndexPath *)indexPath withSections:(NSMutableArray *)sections
{
    WXSection *section = [sections wx_safeObjectAtIndex:indexPath.section];
    WXAssert(section, @"Removing cell at indexPath:%@ has not been inserted to cell list before, sections:%@", indexPath, sections);
    WXAssert(indexPath.row < section.rows.count, @"Removing cell at indexPath:%@ outof range, sections:%@", indexPath, sections);
    [section.rows removeObjectAtIndex:indexPath.row];
}

- (NSIndexPath *)indexPathForCell:(WXCellComponent *)cell sections:(NSMutableArray<WXSection *> *)sections
{
    __block NSIndexPath *indexPath;
    [sections enumerateObjectsUsingBlock:^(WXSection * _Nonnull section, NSUInteger sectionIndex, BOOL * _Nonnull sectionStop) {
        [section.rows enumerateObjectsUsingBlock:^(WXCellComponent * _Nonnull row, NSUInteger rowIndex, BOOL * _Nonnull stop) {
            if (row == cell) {
                indexPath = [NSIndexPath indexPathForRow:rowIndex inSection:sectionIndex];
                *stop = YES;
                *sectionStop = YES;
            }
        }];
    }];
    
    return indexPath;
}

- (NSUInteger)indexForHeader:(WXHeaderComponent *)header sections:(NSMutableArray<WXSection *> *)sections
{
    __block NSUInteger index;
    [sections enumerateObjectsUsingBlock:^(WXSection * _Nonnull section, NSUInteger sectionIndex, BOOL * _Nonnull stop) {
        if (section.header == header) {
            index = sectionIndex;
            *stop = YES;
        }
    }];
    
    return index;
}

- (NSIndexPath *)indexPathForSubIndex:(NSUInteger)index
{
    NSInteger section = 0;
    NSInteger row = -1;
    WXComponent *firstComponent;
    for (int i = 0; i <= index; i++) {
        WXComponent* component = [self.subcomponents wx_safeObjectAtIndex:i];
        if (!component) {
            continue;
        }
        if (([component isKindOfClass:[WXHeaderComponent class]]
            || [component isKindOfClass:[WXCellComponent class]])
            && !firstComponent) {
            firstComponent = component;
        }
        
        if (component != firstComponent && [component isKindOfClass:[WXHeaderComponent class]]) {
            section ++;
            row = -1;
        }
        
        if ([component isKindOfClass:[WXCellComponent class]]) {
            row ++;
        }
    }

    return [NSIndexPath indexPathForRow:row inSection:section];
}

- (void)_performUpdates:(void(^)())updates withKeepScrollPosition:(BOOL)keepScrollPosition adjustmentBlock:(CGFloat(^)(NSIndexPath *topVisibleCell))adjustmentBlock
{
    CGFloat adjustment = 0;
    
    // keep the scroll position when inserting or deleting sections/rows by adjusting the content offset
    if (keepScrollPosition) {
        NSIndexPath *top = _tableView.indexPathsForVisibleRows.firstObject;
        adjustment = adjustmentBlock(top);
    }
    
    updates();
    
    if (keepScrollPosition) {
        CGPoint afterContentOffset = _tableView.contentOffset;
        CGPoint newContentOffset = CGPointMake(afterContentOffset.x, afterContentOffset.y + adjustment);
        _tableView.contentOffset = newContentOffset;
    }
    
    [self handleAppear];
}

- (void)_insertTableViewSectionAtIndex:(NSUInteger)section keepScrollPosition:(BOOL)keepScrollPosition animation:(UITableViewRowAnimation)animation
{
    [self _performUpdates:^{
        [_tableView insertSections:[NSIndexSet indexSetWithIndex:section] withRowAnimation:animation];
    } withKeepScrollPosition:keepScrollPosition adjustmentBlock:^CGFloat(NSIndexPath *top) {
        if (section <= top.section) {
            return [self tableView:_tableView heightForHeaderInSection:section];
        } else {
            return 0.0;
        }
    }];
}

- (void)_deleteTableViewSectionAtIndex:(NSUInteger)section keepScrollPosition:(BOOL)keepScrollPosition animation:(UITableViewRowAnimation)animation
{
    [self _performUpdates:^{
        [_tableView deleteSections:[NSIndexSet indexSetWithIndex:section] withRowAnimation:animation];
    } withKeepScrollPosition:keepScrollPosition adjustmentBlock:^CGFloat(NSIndexPath *top) {
        if (section <= top.section) {
            return [self tableView:_tableView heightForHeaderInSection:section];
        } else {
            return 0.0;
        }
    }];
}

- (void)_insertTableViewCellAtIndexPath:(NSIndexPath *)indexPath keepScrollPosition:(BOOL)keepScrollPosition animation:(UITableViewRowAnimation)animation
{
    [self _performUpdates:^{
        [_tableView insertRowsAtIndexPaths:[NSArray arrayWithObject:indexPath] withRowAnimation:animation];
    } withKeepScrollPosition:keepScrollPosition adjustmentBlock:^CGFloat(NSIndexPath *top) {
        if ([indexPath compare:top] <= 0) {
            return [self tableView:_tableView heightForRowAtIndexPath:indexPath];
        } else {
            return 0.0;
        }
    }];
}

- (void)_deleteTableViewCellAtIndexPath:(NSIndexPath *)indexPath keepScrollPosition:(BOOL)keepScrollPosition animation:(UITableViewRowAnimation)animation
{
    [self _performUpdates:^{
        [_tableView deleteRowsAtIndexPaths:[NSArray arrayWithObject:indexPath] withRowAnimation:animation];
    } withKeepScrollPosition:keepScrollPosition adjustmentBlock:^CGFloat(NSIndexPath *top) {
        if ([indexPath compare:top] <= 0) {
            return [self tableView:_tableView heightForRowAtIndexPath:indexPath];
        } else {
            return 0.0;
        }
    }];
}

- (void)fixFlicker
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        // FIXME:(ง •̀_•́)ง┻━┻ Stupid scoll view, always reset content offset to zero by calling _adjustContentOffsetIfNecessary after insert cells.
        // So if you pull down list while list is rendering, the list will be flickering.
        // Demo:    
        // Have to hook _adjustContentOffsetIfNecessary here.
        // Any other more elegant way?
        NSString *a = @"ntOffsetIfNe";
        NSString *b = @"adjustConte";
        
        NSString *originSelector = [NSString stringWithFormat:@"_%@%@cessary", b, a];
        [[self class] weex_swizzle:[WXTableView class] Method:NSSelectorFromString(originSelector) withMethod:@selector(fixedFlickerSelector)];
    });
}

- (void)fixedFlickerSelector
{
    // DO NOT delete this method.
}


@end
