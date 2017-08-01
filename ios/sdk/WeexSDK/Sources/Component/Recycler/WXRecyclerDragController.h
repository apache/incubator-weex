//
//  WXRecyclerDragController.h
//  Pods
//
//  Created by ximu on 2017/7/31.
//
//

#import <Foundation/Foundation.h>
#import "WXSectionDataController.h"
#import "WXComponent.h"

@class WXRecyclerDragController;

typedef enum : NSUInteger {
    WXRecyclerDragTriggerNormal,
    WXRecyclerDragTriggerPan
} WXRecyclerDragTriggerType;


@protocol WXRecyclerDragControllerDelegate <NSObject>
- (void)dragFireEvent:(NSString *)eventName params:(NSDictionary *)params;
- (void)updateDataSource;

@end

@interface WXRecyclerDragController : NSObject
@property (nonatomic,weak) id<WXRecyclerDragControllerDelegate> delegate;
@property (nonatomic,weak) UICollectionView *collectionView;
@property (nonatomic, strong) UILongPressGestureRecognizer *currentLongPress;
@property (nonatomic, strong) NSIndexPath  *startIndexPath;
@property (nonatomic, strong) NSIndexPath  *dragingIndexPath;
@property (nonatomic, strong) NSIndexPath  *targetIndexPath;
@property (nonatomic, strong) NSMutableArray *excludedAry;
@property (nonatomic, strong) UICollectionViewCell *dragingCell;
@property (nonatomic, assign) BOOL isDragable;
@property (nonatomic, assign) BOOL isDragAnchor;
@property (nonatomic, assign) WXRecyclerDragTriggerType dragTriggerType;

- (void)goThroughAnchor:(WXComponent *)wxComponent indexPath:(NSIndexPath *)indexPath;
@end
