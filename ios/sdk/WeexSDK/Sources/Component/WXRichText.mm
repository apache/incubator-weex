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

#import "WXRichText.h"
#import "WXSDKManager.h"
#import "WXSDKEngine.h"
#import "WXConvert.h"
#import "WXSDKInstance.h"
#import "WXComponent+Layout.h"
#import "WXNavigationProtocol.h"
#import "WXImgLoaderProtocol.h"
#import "WXComponentManager.h"
#import "WXLog.h"
#include <pthread/pthread.h>

@interface WXRichNode : NSObject

@property (nonatomic, strong) NSString  *type;
@property (nonatomic, strong) NSString  *ref;
@property (nonatomic, strong) NSString  *text;
@property (nonatomic, strong) UIColor   *color;
@property (nonatomic, strong) UIColor   *backgroundColor;
@property (nonatomic, strong) NSString  *fontFamily;
@property (nonatomic, assign) CGFloat   fontSize;
@property (nonatomic, assign) CGFloat   fontWeight;
@property (nonatomic, assign) WXTextStyle  fontStyle;
@property (nonatomic, assign) WXTextDecoration textDecoration;
@property (nonatomic, strong) NSString  *pseudoRef;
@property (nonatomic, assign) CGFloat width;
@property (nonatomic, assign) CGFloat height;
@property (nonatomic, strong) NSURL *href;
@property (nonatomic, strong) NSURL *src;
@property (nonatomic, assign) NSRange range;
@property (nonatomic, strong) NSMutableArray *childNodes;

@end

@implementation WXRichNode
- (instancetype)init
{
    if (self = [super init]) {
        _childNodes = [[NSMutableArray alloc] init];
    }
    return self;
}

@end

@interface WXRichTextView : UITextView

@end

@implementation WXRichTextView

- (instancetype)initWithFrame:(CGRect)frame
{
    if ((self = [super initWithFrame:frame])) {
        self.isAccessibilityElement = YES;
        self.accessibilityTraits |= UIAccessibilityTraitStaticText;
        self.opaque = NO;
        self.editable = NO;
        self.selectable = YES;
        self.contentMode = UIViewContentModeRedraw;
        self.textContainerInset = UIEdgeInsetsZero;
        self.textContainer.lineFragmentPadding = 0.0f;
        self.textContainer.lineBreakMode = NSLineBreakByClipping;
    }
    return self;
}

@end

#define WX_STYLE_FILL_RICHTEXT(key, type)\
do {\
    id value = styles[@#key]; \
    if (value) { \
        node.key = [WXConvert type:value];\
    } else if (!([@#key isEqualToString:@"backgroundColor"] || [@#key isEqualToString:@"textDecoration"]) && superNode.key ) { \
        node.key = superNode.key; \
    } \
} while(0);

#define WX_STYLE_FILL_RICHTEXT_PIXEL(key)\
do {\
    id value = styles[@#key];\
    if (value) {\
        node.key = [WXConvert WXPixelType:value scaleFactor:self.weexInstance.pixelScaleFactor];\
    } else if (superNode.key ) { \
        node.key = superNode.key; \
    } \
} while(0);


@implementation WXRichText
{
    WXRichTextView *textView;
    NSMutableArray *_richNodes;
    NSMutableDictionary *_nodeRanges;
    NSMutableDictionary *_styles;
    NSMutableDictionary *_attributes;
    NSTextAlignment _textAlign;
    UIColor *_backgroundColor;
    pthread_mutex_t _attributedStringMutex;
    pthread_mutexattr_t _propertMutexAttr;
    CGFloat _lineHeight;
}

- (void)dealloc
{
    pthread_mutex_destroy(&_attributedStringMutex);
    pthread_mutexattr_destroy(&_propertMutexAttr);
}

- (WXRichTextView *)textView
{
    if (!textView) {
        textView = [[WXRichTextView alloc]init];
        textView.delegate = self;
        textView.scrollEnabled = NO;
    }
    return textView;
}

- (instancetype)initWithRef:(NSString *)ref
                       type:(NSString *)type
                     styles:(NSDictionary *)styles
                 attributes:(NSDictionary *)attributes
                     events:(NSArray *)events
               weexInstance:(WXSDKInstance *)weexInstance
{
    self = [super initWithRef:ref type:type styles:styles attributes:attributes events:events weexInstance:weexInstance];
    if (self) {
        _richNodes = [NSMutableArray new];
        _nodeRanges = [NSMutableDictionary new];
        _styles = [NSMutableDictionary dictionaryWithDictionary:styles];
        _attributes = [NSMutableDictionary dictionaryWithDictionary:attributes];
        _textAlign = styles[@"textAlign"] ? [WXConvert NSTextAlignment:styles[@"textAlign"]] : NSTextAlignmentLeft;
        _lineHeight = styles[@"lineHeight"] ? [WXConvert CGFloat:styles[@"lineHeight"]] / 2: 0;
        pthread_mutexattr_init(&(_propertMutexAttr));
        pthread_mutexattr_settype(&(_propertMutexAttr), PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&(_attributedStringMutex), &(_propertMutexAttr));
    }
    return self;
}

- (void)fillAttributes:(NSDictionary *)attributes
{
    id value = attributes[@"value"];
    if (!value) {
        return;
    }
    if ([value isKindOfClass:[NSString class]]) {
        value = [WXUtility objectFromJSON:value];
    }
    if ([value isKindOfClass: [NSArray class]]) {
        [_richNodes removeAllObjects];
        
        WXRichNode *rootNode = [[WXRichNode alloc]init];
        [_richNodes addObject:rootNode];
        
        rootNode.type = @"root";
        if (_styles) {
            [self fillCSSStyles:_styles toNode:rootNode superNode:nil];
        }
        
        for (NSDictionary *dict in value) {
            [self recursivelyAddChildNode:dict toSuperNode:rootNode];
        }
        
        _backgroundColor = rootNode.backgroundColor?:[UIColor whiteColor];
    }
}

- (void)fillCSSStyles:(NSDictionary *)styles toNode:(WXRichNode *)node superNode:(WXRichNode *)superNode
{
    WX_STYLE_FILL_RICHTEXT(color, UIColor)
    WX_STYLE_FILL_RICHTEXT(backgroundColor, UIColor)
    WX_STYLE_FILL_RICHTEXT(fontFamily, NSString)
    WX_STYLE_FILL_RICHTEXT_PIXEL(fontSize)
    WX_STYLE_FILL_RICHTEXT(fontWeight, WXTextWeight)
    WX_STYLE_FILL_RICHTEXT(fontStyle, WXTextStyle)
    WX_STYLE_FILL_RICHTEXT(textDecoration, WXTextDecoration)
    WX_STYLE_FILL_RICHTEXT_PIXEL(width)
    WX_STYLE_FILL_RICHTEXT_PIXEL(height)
}

- (void)fillAttributes:(NSDictionary *)attributes toNode:(WXRichNode *)node superNode:(WXRichNode *)superNode
{
    if (attributes[@"pseudoRef"]) {
        node.pseudoRef = attributes[@"pseudoRef"];
        node.href = [NSURL URLWithString:@"click://"];
    }
    
    if (attributes[@"href"]) {
        node.href = [NSURL URLWithString:attributes[@"href"]];
    }
    else if (superNode.href) {
        node.href = superNode.href;
        if (!(node.pseudoRef.length) && superNode.pseudoRef.length) {
            node.pseudoRef = superNode.pseudoRef;
        }
    }
    
    if (attributes[@"src"]) {
        node.src = [NSURL URLWithString:attributes[@"src"]];
    }
    
    if (attributes[@"value"] ) {
        id value = attributes[@"value"];
        if ([value isKindOfClass:[NSString class]]) {
            node.text = (NSString *)value;
        }
    }
}

- (void)recursivelyAddChildNode:(NSDictionary *)nodeValue toSuperNode:(WXRichNode *)superNode
{
    if (![nodeValue isKindOfClass:[NSDictionary class]]) {
        WXLogError(@"Invalid rich text structure.");
        return;
    }
    
    if (![nodeValue[@"type"] isKindOfClass:[NSString class]]) {
        WXLogError(@"Invalid rich text structure.");
        return;
    }
    
    WXRichNode *node = [[WXRichNode alloc]init];
    [_richNodes addObject:node];
    
    node.type = nodeValue[@"type"];

    [self fillCSSStyles:nodeValue[@"style"] toNode:node superNode:superNode];

    if (nodeValue[@"attr"]) {
        [self fillAttributes:nodeValue[@"attr"] toNode:node superNode:superNode];
    }
    
    if (nodeValue[@"children"]) {
        id value = nodeValue[@"children"];
        if ([value isKindOfClass:[NSArray class]]) {
            NSArray *children = (NSArray *)value;
            for(NSDictionary *childValue in children){
                [self recursivelyAddChildNode:childValue toSuperNode:node];
            }
        }
    }
}

- (WXRichNode*)findRichNode:(NSString*)ref {
    NSMutableArray *array = [NSMutableArray arrayWithArray:_richNodes];
    for (WXRichNode* node in array) {
        if ([node.ref isEqualToString:ref]) {
            return node;
        }
    }
    return nil;
}

- (NSInteger)indexOfRichNode:(WXRichNode*)node {
    NSInteger index = -1;
    NSMutableArray *array = [NSMutableArray arrayWithArray:_richNodes];
    for (WXRichNode* item in array) {
        if ([item.ref isEqualToString:node.ref]) {
            return index+1;
        }
        index++;
    }
    return index;
}

- (void)removeChildNode:(NSString*)ref superNodeRef:(NSString *)superNodeRef {
    WXRichNode* node = [self findRichNode:ref];
    WXRichNode* superNode = [self findRichNode:@"_root"];
    if (superNodeRef.length > 0) {
        superNode = [self findRichNode:superNodeRef];
    }
    if (superNode) {
         [superNode.childNodes removeObject:node];
    }
    [_richNodes removeObject:node];
    [self setNeedsLayout];
    [self innerLayout];
}

- (void)addChildNode:(NSString *)type ref:(NSString*)ref styles:(NSDictionary*)styles attributes:(NSDictionary*)attributes  toSuperNodeRef:(NSString *)superNodeRef {
    if ([_richNodes count] == 0) {
        WXRichNode *rootNode = [[WXRichNode alloc]init];
        [_richNodes addObject:rootNode];
        rootNode.type = @"root";
        rootNode.ref = @"_root";
        if (_styles) {
            [self fillCSSStyles:_styles toNode:rootNode superNode:nil];
        }
        _backgroundColor = rootNode.backgroundColor?:[UIColor whiteColor];
    }

    WXRichNode* superNode = [self findRichNode:@"_root"];
    if (superNodeRef.length > 0) {
        superNode = [self findRichNode:superNodeRef];
    }

    WXRichNode *node = [[WXRichNode alloc]init];
    node.ref = ref;
    NSInteger index = [self indexOfRichNode:superNode];
    if (index < 0) {
        return;
    }
    if (index == 0) {
        [_richNodes addObject:node];
    } else {
        [_richNodes insertObject:node atIndex:(index + superNode.childNodes.count + 1)];
    }
    [superNode.childNodes addObject:node];
    node.type = type;

    [self fillCSSStyles:styles toNode:node superNode:superNode];
    [self fillAttributes:attributes toNode:node superNode:superNode];
    [self setNeedsLayout];
    [self innerLayout];
}

#pragma mark - Subclass

- (UIView *)loadView
{
    return  [self textView];
}

- (void)viewDidUnload
{
    textView = nil;
}

- (void)viewDidLoad
{
    [self innerLayout];
}

- (void)layoutDidFinish
{
    [self innerLayout];
}

- (void)innerLayout
{
    __weak typeof(self) wself = self;
    WXPerformBlockOnComponentThread(^{
        __strong typeof(wself) sself = wself;
        if (sself) {
            if (sself.flexCssNode == nullptr) {
                return;
            }
            UIEdgeInsets padding = {
                WXFloorPixelValue(sself.flexCssNode->getPaddingTop()+sself.flexCssNode->getBorderWidthTop()),
                WXFloorPixelValue(sself.flexCssNode->getPaddingLeft()+sself.flexCssNode->getBorderWidthLeft()),
                WXFloorPixelValue(sself.flexCssNode->getPaddingBottom()+sself.flexCssNode->getBorderWidthBottom()),
                WXFloorPixelValue(sself.flexCssNode->getPaddingRight()+sself.flexCssNode->getBorderWidthRight())
            };
            
            NSMutableAttributedString* attrString = [sself buildAttributeString];
            WXPerformBlockOnMainThread(^{
                WXRichTextView* view = [sself textView];
                view.attributedText = attrString;
                view.textContainerInset = padding;
                view.backgroundColor = [UIColor clearColor];
            });
        }
    });
}

- (CGSize (^)(CGSize))measureBlock
{
    __weak typeof(self) weakSelf = self;
    
    return ^CGSize (CGSize constrainedSize) {
        
        NSMutableAttributedString *attributedString = [weakSelf buildAttributeString];
        
        CGFloat width = constrainedSize.width;
        if (isnan(width)) {
            width = CGFLOAT_MAX;
        }
        
        CGRect rect = [attributedString boundingRectWithSize:CGSizeMake(width, CGFLOAT_MAX) options:NSStringDrawingUsesLineFragmentOrigin | NSStringDrawingUsesFontLeading context:nil];
        CGSize computedSize = rect.size;
        if(weakSelf.flexCssNode != nullptr){
            if (!isnan(weakSelf.flexCssNode->getMinWidth())) {
                computedSize.width = MAX(computedSize.width, weakSelf.flexCssNode->getMinWidth());
            }
            if (!isnan(weakSelf.flexCssNode->getMaxWidth())) {
                computedSize.width = MIN(computedSize.width, weakSelf.flexCssNode->getMaxWidth());
            }
            if (!isnan(weakSelf.flexCssNode->getMinHeight())) {
                computedSize.width = MAX(computedSize.height, weakSelf.flexCssNode->getMinHeight());
            }
            if (!isnan(weakSelf.flexCssNode->getMaxHeight())) {
                computedSize.width = MIN(computedSize.height, weakSelf.flexCssNode->getMaxHeight());
            }
        }
        return (CGSize) {
            WXCeilPixelValue(computedSize.width),
            WXCeilPixelValue(computedSize.height)
        };
    };
}

#pragma mark Text Building

- (NSMutableAttributedString *)buildAttributeString
{
    pthread_mutex_lock(&(_attributedStringMutex));
    [self fillAttributes:_attributes];
    NSMutableArray *array = [NSMutableArray arrayWithArray:_richNodes];
    pthread_mutex_unlock(&(_attributedStringMutex));
    
    NSMutableDictionary *nodeRange = [NSMutableDictionary dictionary];
    NSMutableAttributedString *attrStr = [[NSMutableAttributedString alloc] init];
    NSUInteger location;
    
    __weak typeof(self) weakSelf = self;
    for (WXRichNode *node in array) {
        location = attrStr.length;
        
        if ([node.type isEqualToString:@"span"]) {
            if (node.text && [node.text length] > 0) {
                NSString *text = node.text;
                [attrStr.mutableString appendString:text];
                
                NSRange range = NSMakeRange(location, text.length);
                [attrStr addAttribute:NSForegroundColorAttributeName value:node.color ?: [UIColor blackColor] range:range];
                [attrStr addAttribute:NSBackgroundColorAttributeName value:node.backgroundColor ?: _backgroundColor range:range];
                
                UIFont *font = [WXUtility fontWithSize:node.fontSize textWeight:node.fontWeight textStyle:WXTextStyleNormal fontFamily:node.fontFamily scaleFactor:self.weexInstance.pixelScaleFactor];
                if (font) {
                    [attrStr addAttribute:NSFontAttributeName value:font range:range];
                }
                if (node.fontStyle == WXTextStyleItalic) {
                    [attrStr addAttribute:NSObliquenessAttributeName value:@0.3 range:range];
                }
                else
                {
                    [attrStr addAttribute:NSObliquenessAttributeName value:@0 range:range];
                }
                [attrStr addAttribute:NSUnderlineStyleAttributeName value:[NSNumber numberWithInteger:WXTextDecorationNone] range:range];
                [attrStr addAttribute:NSStrikethroughStyleAttributeName value:[NSNumber numberWithInteger:WXTextDecorationNone] range:range];
                
                if (node.textDecoration == WXTextDecorationUnderline) {
                    [attrStr addAttribute:NSUnderlineStyleAttributeName value:[NSNumber numberWithInteger:WXTextDecorationUnderline] range:range];
                }
                else if (node.textDecoration == WXTextDecorationLineThrough) {
                    [attrStr addAttribute:NSStrikethroughStyleAttributeName value:[NSNumber numberWithInteger:WXTextDecorationLineThrough] range:range];
                }
                
                if (node.href) {
                    [attrStr addAttribute:NSLinkAttributeName value:node.href range:range];
                }
                else {
                    [attrStr removeAttribute:NSLinkAttributeName range:range];
                }
                
                NSMutableParagraphStyle *paragraphStyle = [[NSMutableParagraphStyle alloc] init];
                paragraphStyle.alignment = _textAlign;
                if(_lineHeight != 0 )
                {
                    paragraphStyle.minimumLineHeight = _lineHeight;
                    paragraphStyle.maximumLineHeight = _lineHeight;
                    [attrStr addAttribute:NSBaselineOffsetAttributeName value:@((_lineHeight - font.lineHeight)/2) range:range];
                }
                [attrStr addAttribute:NSParagraphStyleAttributeName value:paragraphStyle range:range];
                
                [nodeRange setObject:node forKey:NSStringFromRange(range)];
            }
        }
        else if ([node.type isEqualToString:@"image"]) {
            NSTextAttachment *imgAttachment = [[NSTextAttachment alloc]init];
            imgAttachment.bounds = CGRectMake(0, 0, node.width, node.height);
            
            NSAttributedString *attachAttriStr = [NSAttributedString attributedStringWithAttachment:imgAttachment];
            [attrStr appendAttributedString:attachAttriStr];
            
            NSRange range = NSMakeRange(location, attachAttriStr.length);
            [attrStr addAttribute:NSFontAttributeName value: [UIFont systemFontOfSize:node.height] range:range];
            
            if (node.href) {
                [attrStr addAttribute:NSLinkAttributeName value:node.href range:range];
            }
            else {
                [attrStr removeAttribute:NSLinkAttributeName range:range];
            }
            
            [nodeRange setObject:node forKey:NSStringFromRange(range)];
            
            if (node.src) {
                [[self imageLoader] downloadImageWithURL:node.src.absoluteString imageFrame:imgAttachment.bounds userInfo:nil completed:^(UIImage *image, NSError *error, BOOL finished) {
                    dispatch_async(dispatch_get_main_queue(), ^{
                        imgAttachment.image = image;
                        [[weakSelf textView].layoutManager invalidateDisplayForCharacterRange:range];
                    });
                }];
            }
        }
    }
    
    pthread_mutex_lock(&(_attributedStringMutex));
    [_nodeRanges removeAllObjects];
    _nodeRanges = [NSMutableDictionary dictionaryWithDictionary:nodeRange];
    pthread_mutex_unlock(&(_attributedStringMutex));
    
    return attrStr;
}

- (void)updateStyles:(NSDictionary *)styles {
    
    if (styles[@"textAlign"]) {
        _textAlign = [WXConvert NSTextAlignment:styles[@"textAlign"]];
    }
    if (styles[@"lineHeight"]) {
        _lineHeight = [WXConvert CGFloat:styles[@"lineHeight"]] / 2;
    }
    
    WXPerformBlockOnComponentThread(^{
        [_styles addEntriesFromDictionary:styles];
        [self syncTextStorageForView];
    });
}

- (void)updateChildNodeStyles:(NSDictionary *)styles ref:(NSString*)ref parentRef:(NSString*)parentRef {
    WXPerformBlockOnComponentThread(^{
        WXRichNode* node =  [self findRichNode:ref];
        if (node) {
            WXRichNode* superNode = [self findRichNode:@"_root"];
            if (parentRef.length > 0) {
                superNode = [self findRichNode:parentRef];
            }
            if (superNode) {
                [self fillCSSStyles:styles toNode:node superNode:superNode];
                [self syncTextStorageForView];
            }
        }
    });
}

- (void)updateAttributes:(NSDictionary *)attributes {
    WXPerformBlockOnComponentThread(^{
        _attributes = [NSMutableDictionary dictionaryWithDictionary:attributes];
        [self syncTextStorageForView];
    });
}

- (void)updateChildNodeAttributes:(NSDictionary *)attributes ref:(NSString*)ref parentRef:(NSString*)parentRef {
    WXPerformBlockOnComponentThread(^{
        WXRichNode* node =  [self findRichNode:ref];
        if (node) {
            WXRichNode* superNode = [self findRichNode:@"_root"];
            if (parentRef.length > 0) {
                superNode = [self findRichNode:parentRef];
            }
            if (superNode) {
                [self fillAttributes:attributes toNode:node superNode:superNode];
                [self syncTextStorageForView];
            }
        }
    });
}

- (void)syncTextStorageForView {
    pthread_mutex_lock(&(_attributedStringMutex));
    [self fillAttributes:_attributes];
    pthread_mutex_unlock(&(_attributedStringMutex));
    
    if (_styles[@"height"]) {
       [self innerLayout];
    }
    else {
       [self setNeedsLayout];
    }
}

#pragma mark - UITextView Delegate

- (BOOL)textView:(UITextView *)textView shouldInteractWithTextAttachment:(NSTextAttachment *)textAttachment inRange:(NSRange)characterRange {
    return NO;
}

- (BOOL)textView:(UITextView *)textView shouldInteractWithURL:(NSURL *)URL inRange:(NSRange)characterRange {
    if (!URL) {
        return NO;
    }
    
    NSString *rangeStr = NSStringFromRange(characterRange);
    WXRichNode *node = [_nodeRanges objectForKey:rangeStr];
    
    if (![[node.href absoluteString] isEqualToString:@"click://"]) {
        id<WXNavigationProtocol> navigationHandler = [self navigationHandler];
        if ([navigationHandler respondsToSelector:@selector(pushViewControllerWithParam:
                                                            completion:
                                                            withContainer:)]) {
            [navigationHandler pushViewControllerWithParam:@{@"url":URL.absoluteString} completion:^(NSString *code, NSDictionary *responseData) {
            } withContainer:self.weexInstance.viewController];
        } else {
            WXLogError(@"Event handler of class %@ does not respond to pushViewControllerWithParam", NSStringFromClass([navigationHandler class]));
        }
    }
    else if (node.pseudoRef) {
        NSMutableDictionary *params = [NSMutableDictionary new];
        [params setObject:node.pseudoRef forKey:@"pseudoRef"];
        [[WXSDKManager bridgeMgr] fireEvent:self.weexInstance.instanceId ref:self.ref type:@"itemclick" params:params domChanges:nil];
    }
    
    return NO;
}

# pragma mark - imageLoader

- (id<WXImgLoaderProtocol>)imageLoader {
    static id<WXImgLoaderProtocol> imageLoader;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        imageLoader = [WXSDKEngine handlerForProtocol:@protocol(WXImgLoaderProtocol)];
    });
    return imageLoader;
}

- (id<WXNavigationProtocol>)navigationHandler {
    static id<WXNavigationProtocol> navigationHandler;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        navigationHandler = [WXSDKEngine handlerForProtocol:@protocol(WXNavigationProtocol)];
    });
    return navigationHandler;
}

@end
