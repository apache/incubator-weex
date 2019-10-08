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

#import "WXTextComponent.h"
#import "WXSDKInstance_private.h"
#import "WXComponent_internal.h"
#import "WXLayer.h"
#import "WXUtility.h"
#import "WXConvert.h"
#import "WXRuleManager.h"
#import "WXDefine.h"
#import "WXView.h"
#import "WXComponent+Layout.h"
#import <pthread/pthread.h>
#import <CoreText/CoreText.h>

// WXText is a non-public is not permitted
@interface WXTextView : WXView
@property (nonatomic, strong) NSTextStorage *textStorage;
@end

@implementation WXTextView

- (instancetype)initWithFrame:(CGRect)frame
{
    if ((self = [super initWithFrame:frame])) {
        self.accessibilityTraits |= UIAccessibilityTraitStaticText;
        
        self.opaque = NO;
        self.contentMode = UIViewContentModeRedraw;
        self.textStorage = [NSTextStorage new];
    }
    return self;
}

+ (Class)layerClass
{
    return [WXLayer class];
}

- (void)copy:(id)sender
{
    [[UIPasteboard generalPasteboard] setString:((WXTextComponent*)self.wx_component).text];
}

- (void)setTextStorage:(NSTextStorage *)textStorage
{
    if (_textStorage != textStorage) {
        _textStorage = textStorage;
        [self.wx_component setNeedsDisplay];
    }
}

- (BOOL)canBecomeFirstResponder
{
    return YES;
}

- (BOOL)canPerformAction:(SEL)action withSender:(id)sender
{
    if (action == @selector(copy:)) {
        return [[self.wx_component valueForKey:@"_enableCopy"] boolValue];
    }
    return [super canPerformAction:action withSender:sender];
}

- (NSString *)description
{
    NSString *superDescription = super.description;
    NSRange semicolonRange = [superDescription rangeOfString:@";"];
    NSString * content = _textStorage.string;
    if ([(WXTextComponent*)self.wx_component useCoreText]) {
        content = ((WXTextComponent*)self.wx_component).text;
    }
    NSString *replacement = [NSString stringWithFormat:@"; text: %@; frame:%f,%f,%f,%f", content, self.frame.origin.x, self.frame.origin.y, self.frame.size.width, self.frame.size.height];
    return [superDescription stringByReplacingCharactersInRange:semicolonRange withString:replacement];
}

- (NSString *)accessibilityValue
{
    if (self.wx_component && self.wx_component->_ariaLabel) {
        return [super accessibilityValue];
    }
    if (![(WXTextComponent*)self.wx_component useCoreText]) {
        return _textStorage.string;
    }
    return ((WXTextComponent*)self.wx_component).text;
}

- (NSString *)accessibilityLabel
{
    if (self.wx_component) {
        if (self.wx_component->_ariaLabel) {
            return self.wx_component->_ariaLabel;
        }
    }
    return [super accessibilityLabel];
}

@end

static NSString *const WXTextTruncationToken = @"\u2026";
static CGFloat WXTextDefaultLineThroughWidth = 1.2;

@interface WXTextComponent()
@property (nonatomic, strong) NSString *useCoreTextAttr;
@end

@implementation WXTextComponent
{
    UIEdgeInsets _border;
    UIEdgeInsets _padding;
    NSTextStorage *_textStorage;
    float _textStorageWidth;
    float _color[4];
    NSString *_fontFamily;
    float _fontSize;
    float _fontWeight;
    WXTextStyle _fontStyle;
    NSUInteger _lines;
    NSTextAlignment _textAlign;
    WXTextDecoration _textDecoration;
    NSString *_textOverflow;
    float _lineHeight;
    float _letterSpacing;
    float _fontDescender;
    float _fontAscender;
    BOOL _truncationLine; // support trunk tail
    
    NSAttributedString * _ctAttributedString;
    NSString *_wordWrap;
    
    pthread_mutex_t _ctAttributedStringMutex;
    pthread_mutexattr_t _propertMutexAttr;
    BOOL _observerIconfont;
    BOOL _enableCopy;
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
        // just for coretext and textkit render replacement
        pthread_mutexattr_init(&(_propertMutexAttr));
        pthread_mutexattr_settype(&(_propertMutexAttr), PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&(_ctAttributedStringMutex), &(_propertMutexAttr));
        
        _textAlign = NSTextAlignmentNatural;
        
        if ([attributes objectForKey:@"coretext"]) {
            _useCoreTextAttr = [WXConvert NSString:attributes[@"coretext"]];
        } else {
            _useCoreTextAttr = nil;
        }
        
        _color[0] = -1.0;

        [self fillCSSStyles:styles];
        [self fillAttributes:attributes];
        
    }
    return self;
}

- (BOOL)useCoreText
{
    if ([_useCoreTextAttr isEqualToString:@"true"]) {
        return YES;
    }
    if ([_useCoreTextAttr isEqualToString:@"false"]) {
        return NO;
    }
    return YES;
}

- (void)dealloc
{
    if (_fontFamily && _observerIconfont) {
        [[NSNotificationCenter defaultCenter] removeObserver:self name:WX_ICONFONT_DOWNLOAD_NOTIFICATION object:nil];
    }
    pthread_mutex_destroy(&_ctAttributedStringMutex);
    pthread_mutexattr_destroy(&_propertMutexAttr);
}

#define WX_STYLE_FILL_TEXT(key, prop, type, needLayout)\
do {\
    id value = styles[@#key];\
    if (value) {\
        _##prop = [WXConvert type:value];\
        [self setNeedsRepaint];\
        if (needLayout) {\
            [self setNeedsLayout];\
        }\
    }\
} while(0);

#define WX_STYLE_FILL_TEXT_WITH_DEFAULT_VALUE(key, prop, type, defaultValue,needLayout)\
do {\
    id value = styles[@#key];\
    if (value) {\
        if([WXUtility isBlankString:value]){\
            _##prop = defaultValue;\
        }else {\
            _##prop = [WXConvert type:value];\
        }\
        [self setNeedsRepaint];\
        if (needLayout) {\
            [self setNeedsLayout];\
        }\
    }\
} while(0);


#define WX_STYLE_FILL_TEXT_PIXEL(key, prop, needLayout)\
do {\
    id value = styles[@#key];\
    if (value) {\
        _##prop = [WXConvert WXPixelType:value scaleFactor:self.weexInstance.pixelScaleFactor];\
        [self setNeedsRepaint];\
    if (needLayout) {\
        [self setNeedsLayout];\
    }\
}\
} while(0);

- (void)fillCSSStyles:(NSDictionary *)styles
{
    WX_STYLE_FILL_TEXT(fontFamily, fontFamily, NSString, YES) //!OCLint
    WX_STYLE_FILL_TEXT_PIXEL(fontSize, fontSize, YES) //!OCLint
    WX_STYLE_FILL_TEXT(fontWeight, fontWeight, WXTextWeight, YES) //!OCLint
    WX_STYLE_FILL_TEXT(fontStyle, fontStyle, WXTextStyle, YES) //!OCLint
    WX_STYLE_FILL_TEXT(lines, lines, NSUInteger, YES) //!OCLint
    WX_STYLE_FILL_TEXT(textAlign, textAlign, NSTextAlignment, NO) //!OCLint
    WX_STYLE_FILL_TEXT(textDecoration, textDecoration, WXTextDecoration, YES) //!OCLint
    WX_STYLE_FILL_TEXT(textOverflow, textOverflow, NSString, NO) //!OCLint
    WX_STYLE_FILL_TEXT_PIXEL(lineHeight, lineHeight, YES) //!OCLint
    WX_STYLE_FILL_TEXT_PIXEL(letterSpacing, letterSpacing, YES) //!OCLint
    WX_STYLE_FILL_TEXT(wordWrap, wordWrap, NSString, YES); //!OCLint

    UIColor* color = nil;
    id value = styles[@"color"];
    if (value) {
        if([WXUtility isBlankString:value]){
            color = [UIColor blackColor];
        } else {
            color = [WXConvert UIColor:value];
        }
        if (color) {
            [self setNeedsRepaint];
            CGFloat red, green, blue, alpha;
            [color getRed:&red green:&green blue:&blue alpha:&alpha];
            _color[0] = red;
            _color[1] = green;
            _color[2] = blue;
            _color[3] = alpha;
        }
    }

    if (_fontFamily && !_observerIconfont) {
        // notification received when custom icon font file download finish
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(repaintText:) name:WX_ICONFONT_DOWNLOAD_NOTIFICATION object:nil];
        _observerIconfont = YES;
    }
    
    if (self.flexCssNode == nullptr) {
        return;
    }
    
        UIEdgeInsets flex_padding = {
            WXFloorPixelValue(self.flexCssNode->getPaddingTop() + self.flexCssNode->getBorderWidthTop()),
            WXFloorPixelValue(self.flexCssNode->getPaddingLeft() + self.flexCssNode->getBorderWidthLeft()),
            WXFloorPixelValue(self.flexCssNode->getPaddingBottom() + self.flexCssNode->getBorderWidthBottom()),
            WXFloorPixelValue(self.flexCssNode->getPaddingRight() + self.flexCssNode->getBorderWidthRight())
        };
        
        if (!UIEdgeInsetsEqualToEdgeInsets(flex_padding, _padding)) {
            _padding = flex_padding;
            [self setNeedsRepaint];
        }
}

- (void)fillAttributes:(NSDictionary *)attributes
{
    id text = [WXConvert NSString:attributes[@"value"]];
    if (text && ![self.text isEqualToString:text]) {
        self.text = text;
        [self setNeedsRepaint];
        [self setNeedsLayout];
    }
    if (attributes[@"enableCopy"]) {
        _enableCopy = [WXConvert BOOL:attributes[@"enableCopy"]];
    }
}

- (void)setNeedsRepaint
{
    _textStorage = nil;
    
    pthread_mutex_lock(&(_ctAttributedStringMutex));
    _ctAttributedString = nil;
    pthread_mutex_unlock(&(_ctAttributedStringMutex));
}

#pragma mark - Subclass

- (void)setNeedsLayout
{
    [super setNeedsLayout];
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    BOOL useCoreText = NO;
    if ([self.view.wx_component isKindOfClass:NSClassFromString(@"WXTextComponent")] && [self.view.wx_component respondsToSelector:@selector(useCoreText)]) {
        useCoreText = [(WXTextComponent*)self.view.wx_component useCoreText];
    }
    if (!useCoreText) {
        ((WXTextView *)self.view).textStorage = _textStorage;
    }
    if (_enableCopy) {
        UILongPressGestureRecognizer *longPress = [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(displayMenuController:)];
        [self.view addGestureRecognizer:longPress];
    }
    self.view.isAccessibilityElement = YES;
    
    [self setNeedsDisplay];
}

- (void)displayMenuController:(id)sender
{
    if ([self.view becomeFirstResponder] && ((UILongPressGestureRecognizer*)sender).state == UIGestureRecognizerStateBegan) {
        UIMenuController *theMenu = [UIMenuController sharedMenuController];
        CGSize size = [self ctAttributedString].size;
        CGRect selectionRect = CGRectMake(self.view.frame.origin.x, self.view.frame.origin.y, size.width, size.height);
        [theMenu setTargetRect:selectionRect inView:self.view.superview];
        [theMenu setMenuVisible:YES animated:YES];
    }
}

- (UIView *)loadView
{
    return [[WXTextView alloc] init];
}

- (void)layoutDirectionDidChanged:(BOOL)isRTL {
    [self setNeedsRepaint];
}

- (BOOL)needsDrawRect
{
    return YES;
}

- (UIImage *)drawRect:(CGRect)rect;
{
    CGContextRef context = UIGraphicsGetCurrentContext();
    if (_isCompositingChild) {
        [self drawTextWithContext:context bounds:rect padding:_padding];
    } else {
        [self drawTextWithContext:context bounds:rect padding:_padding];
    }
    
    return nil;
}

- (CGSize (^)(CGSize))measureBlock
{
    __weak typeof(self) weakSelf = self;
    return ^CGSize (CGSize constrainedSize) {
        __strong typeof(self) strongSelf = weakSelf;
        if (strongSelf == nil) {
            return CGSizeZero;
        }
        
        if (strongSelf.flexCssNode == nullptr) {
            return CGSizeZero;
        }
        
#ifdef DEBUG
        WXLogDebug(@"flexLayout -> measureblock %@, constrainedSize:%@",
              self.type,
              NSStringFromCGSize(constrainedSize)
              );
#endif
        CGSize computedSize = CGSizeZero;
        NSTextStorage *textStorage = nil;
        
        //TODO:more elegant way to use max and min constrained size
            if (!isnan(strongSelf.flexCssNode->getMinWidth())) {
                constrainedSize.width = MAX(constrainedSize.width, strongSelf.flexCssNode->getMinWidth());
            }
            
            if (!isnan(strongSelf.flexCssNode->getMaxWidth())) {
                constrainedSize.width = MIN(constrainedSize.width, strongSelf.flexCssNode->getMaxWidth());
            }
        
        if (![self useCoreText]) {
            textStorage = [strongSelf textStorageWithWidth:constrainedSize.width];
            NSLayoutManager *layoutManager = textStorage.layoutManagers.firstObject;
            NSTextContainer *textContainer = layoutManager.textContainers.firstObject;
            computedSize = [layoutManager usedRectForTextContainer:textContainer].size;
        } else {
            computedSize = [strongSelf calculateTextHeightWithWidth:constrainedSize.width];
        }
            if (!isnan(strongSelf.flexCssNode->getMinWidth())) {
                computedSize.width = MAX(computedSize.width, strongSelf.flexCssNode->getMinWidth());
            }
            
            if (!isnan(strongSelf.flexCssNode->getMaxWidth())) {
                computedSize.width = MIN(computedSize.width, strongSelf.flexCssNode->getMaxWidth());
            }
            
            if (!isnan(strongSelf.flexCssNode->getMinHeight())) {
                computedSize.height = MAX(computedSize.height, strongSelf.flexCssNode->getMinHeight());
            }
            
            if (!isnan(strongSelf.flexCssNode->getMaxHeight())) {
                computedSize.height = MIN(computedSize.height, strongSelf.flexCssNode->getMaxHeight());
            }
        if (textStorage && [WXUtility isBlankString:textStorage.string]) {
            //  if the text value is empty or nil, then set the height is 0.
            computedSize.height = 0;
        }
        
        return (CGSize) {
            WXCeilPixelValue(computedSize.width),
            WXCeilPixelValue(computedSize.height)
        };
    };
}

#pragma mark Text Building

- (NSAttributedString *)ctAttributedString
{
    if (!self.text) {
        return nil;
    }
    NSAttributedString * attributedString = nil;
    pthread_mutex_lock(&(_ctAttributedStringMutex));
    if (!_ctAttributedString) {
        _ctAttributedString = [self buildCTAttributeString];
        WXPerformBlockOnComponentThread(^{
            [self.weexInstance.componentManager startComponentTasks];
        });
    }
    attributedString = [_ctAttributedString copy];
    pthread_mutex_unlock(&(_ctAttributedStringMutex));
    return attributedString;
}

- (void)repaintText:(NSNotification *)notification
{
    if (![_fontFamily isEqualToString:notification.userInfo[@"fontFamily"]]) {
        return;
    }
    [self setNeedsRepaint];
    WXPerformBlockOnComponentThread(^{
        [self.weexInstance.componentManager startComponentTasks];
        WXPerformBlockOnMainThread(^{
            [self setNeedsLayout];
            [self setNeedsDisplay];
        });
    });
}

- (NSMutableAttributedString *)buildCTAttributeString
{
    NSString * string = self.text;
    if (![string isKindOfClass:[NSString class]]) {
        WXLogError(@"text %@ is invalid", self.text);
        string = @"";
    }
    NSMutableAttributedString *attributedString = [[NSMutableAttributedString alloc] initWithString: string];
    if (_color[0] >= 0) {
        [attributedString addAttribute:NSForegroundColorAttributeName value:[UIColor colorWithRed:_color[0] green:_color[1] blue:_color[2] alpha:_color[3]] range:NSMakeRange(0, string.length)];
    }
    
    // set font
    UIFont *font = [WXUtility fontWithSize:_fontSize textWeight:_fontWeight textStyle:WXTextStyleNormal fontFamily:_fontFamily scaleFactor:self.weexInstance.pixelScaleFactor useCoreText:[self useCoreText]];
    CTFontRef ctFont;
    
    if (_fontStyle == WXTextStyleItalic) {
        CGAffineTransform matrix = CGAffineTransformMake(1, 0, tanf(16 * (CGFloat)M_PI / 180), 1, 0, 0);
        ctFont = CTFontCreateWithFontDescriptor((__bridge CTFontDescriptorRef)font.fontDescriptor, font.pointSize, &matrix);
    }else {
        ctFont = CTFontCreateWithFontDescriptor((__bridge CTFontDescriptorRef)font.fontDescriptor, font.pointSize, NULL);
    }
    
    _fontAscender = font.ascender;
    _fontDescender = font.descender;
    
    if (ctFont) {
        [attributedString addAttribute:(id)kCTFontAttributeName value:(__bridge id)(ctFont) range:NSMakeRange(0, string.length)];
        CFRelease(ctFont);
    }
    
    if(_textDecoration == WXTextDecorationUnderline){
        [attributedString addAttribute:(id)kCTUnderlineStyleAttributeName value:@(kCTUnderlinePatternSolid | kCTUnderlineStyleSingle) range:NSMakeRange(0, string.length)];
    } else if(_textDecoration == WXTextDecorationLineThrough){
        [attributedString addAttribute:NSStrikethroughStyleAttributeName value:@(NSUnderlinePatternSolid | NSUnderlineStyleSingle) range:NSMakeRange(0, string.length)];
    }
    
    NSMutableParagraphStyle *paragraphStyle = [NSMutableParagraphStyle new];
    
    // handle text direction style, default ltr
    NSTextAlignment retAlign = _textAlign;
    BOOL isRtl = [self isDirectionRTL];
    if (isRtl) {
        if (0 == retAlign) {
            //force text right-align if don't specified any align.
            retAlign = NSTextAlignmentRight;
        }
        paragraphStyle.baseWritingDirection = NSWritingDirectionRightToLeft;
    } else {
        //if you specify NSWritingDirectionNaturalDirection, the receiver resolves the writing
        //directionto eitherNSWritingDirectionLeftToRight or NSWritingDirectionRightToLeft,
        //depending on the direction for the user’s language preference setting.
        paragraphStyle.baseWritingDirection =  NSWritingDirectionNatural;
    }
    
    if (retAlign) {
        paragraphStyle.alignment = retAlign;
    }
    
    if ([[_wordWrap lowercaseString] isEqualToString:@"anywhere"]) {
        paragraphStyle.lineBreakMode = NSLineBreakByCharWrapping;
    }
    else {
        paragraphStyle.lineBreakMode = NSLineBreakByWordWrapping;
    }
    
    _truncationLine = NO;
    if (_textOverflow && [_textOverflow length] > 0) {
        if (_lines && [_textOverflow isEqualToString:@"ellipsis"])
            _truncationLine = YES;
    }
    
    if (_lineHeight) {
        paragraphStyle.maximumLineHeight = _lineHeight;
        paragraphStyle.minimumLineHeight = _lineHeight;
    }
    if (_lineHeight || _textAlign || [_textOverflow length] > 0) {
        [attributedString addAttribute:NSParagraphStyleAttributeName
                                 value:paragraphStyle
                                 range:(NSRange){0, attributedString.length}];
    }
    
    if (_letterSpacing) {
        [attributedString addAttribute:NSKernAttributeName value:@(_letterSpacing) range:(NSRange){0, attributedString.length}];
    }
    
    return attributedString;
}

- (NSAttributedString *)buildAttributeString
{
    NSString *string = self.text ?: @"";
    
    NSMutableAttributedString *attributedString = [[NSMutableAttributedString alloc] initWithString:string];
    
    // set textColor
    if (_color[0] >= 0) {
        [attributedString addAttribute:NSForegroundColorAttributeName value:[UIColor colorWithRed:_color[0] green:_color[1] blue:_color[2] alpha:_color[3]] range:NSMakeRange(0, string.length)];
    }
    
    // set font
    UIFont *font = [WXUtility fontWithSize:_fontSize textWeight:_fontWeight textStyle:_fontStyle fontFamily:_fontFamily scaleFactor:self.weexInstance.pixelScaleFactor];
    if (font) {
        [attributedString addAttribute:NSFontAttributeName value:font range:NSMakeRange(0, string.length)];
    }
    
    if(_textDecoration == WXTextDecorationUnderline){
        [attributedString addAttribute:NSUnderlineStyleAttributeName value:@(NSUnderlinePatternSolid | NSUnderlineStyleSingle) range:NSMakeRange(0, string.length)];
    } else if(_textDecoration == WXTextDecorationLineThrough){
        [attributedString addAttribute:NSStrikethroughStyleAttributeName value:@(NSUnderlinePatternSolid | NSUnderlineStyleSingle) range:NSMakeRange(0, string.length)];
    }
    
    NSMutableParagraphStyle *paragraphStyle = [NSMutableParagraphStyle new];

    // handle text direction style, default ltr
    NSTextAlignment retAlign = _textAlign;
    BOOL isRtl = [self isDirectionRTL];
    if (isRtl) {
        if (0 == retAlign) {
            //force text right-align if don't specified any align.
            retAlign = NSTextAlignmentRight;
        }
        paragraphStyle.baseWritingDirection = NSWritingDirectionRightToLeft;
    } else {
        //if you specify NSWritingDirectionNaturalDirection, the receiver resolves the writing
        //directionto eitherNSWritingDirectionLeftToRight or NSWritingDirectionRightToLeft,
        //depending on the direction for the user’s language preference setting.
        paragraphStyle.baseWritingDirection =  NSWritingDirectionNatural;
    }
    
    if (retAlign) {
        paragraphStyle.alignment = retAlign;
    }
    
    if (_lineHeight) {
        paragraphStyle.maximumLineHeight = _lineHeight;
        paragraphStyle.minimumLineHeight = _lineHeight;
    }
    
    if (_lineHeight || _textAlign) {
        [attributedString addAttribute:NSParagraphStyleAttributeName
                                 value:paragraphStyle
                                 range:(NSRange){0, attributedString.length}];
    }

    return attributedString;
}

- (BOOL)adjustLineHeight
{
    if (WX_SYS_VERSION_LESS_THAN(@"10.0")) {
        return true;
    }
    return ![self useCoreText];
}

- (NSTextStorage *)textStorageWithWidth:(CGFloat)width
{
    if (_textStorage && width == _textStorageWidth) {
        return _textStorage;
    }
    
    NSLayoutManager *layoutManager = [NSLayoutManager new];
    
    // build AttributeString
    NSAttributedString *attributedString = [self buildAttributeString];
    
    NSTextStorage *textStorage = [[NSTextStorage alloc] initWithAttributedString:attributedString];
    [textStorage addLayoutManager:layoutManager];
    
    NSTextContainer *textContainer = [NSTextContainer new];
    textContainer.lineFragmentPadding = 0.0;
    
    if ([[_wordWrap lowercaseString] isEqualToString:@"break-word"]) {
        textContainer.lineBreakMode = NSLineBreakByWordWrapping;
    } else if ([[_wordWrap lowercaseString] isEqualToString:@"normal"]){
        textContainer.lineBreakMode = NSLineBreakByClipping;
    } else {
        // set default lineBreakMode
        textContainer.lineBreakMode = NSLineBreakByCharWrapping;
    }
    
    if (_textOverflow && [_textOverflow length] > 0) {
        if ([_textOverflow isEqualToString:@"ellipsis"])
            textContainer.lineBreakMode = NSLineBreakByTruncatingTail;
    }
    textContainer.maximumNumberOfLines = _lines > 0 ? _lines : 0;
    textContainer.size = (CGSize){isnan(width) ? CGFLOAT_MAX : width, CGFLOAT_MAX};

    [layoutManager addTextContainer:textContainer];
    [layoutManager ensureLayoutForTextContainer:textContainer];
    
    _textStorageWidth = width;
    _textStorage = textStorage;
    
    return textStorage;
}

- (void)syncTextStorageForView
{
    CGFloat width = self.calculatedFrame.size.width - (_padding.left + _padding.right);
    NSTextStorage *textStorage = nil;
    if (![self useCoreText]) {
        textStorage = [self textStorageWithWidth:width];
    }
    [self.weexInstance.componentManager  _addUITask:^{
        if ([self isViewLoaded]) {
            if (![self useCoreText]) {
                ((WXTextView *)self.view).textStorage = textStorage;
            }
            [self readyToRender]; // notify super component
            [self setNeedsDisplay];
        }
    }];
}

- (void)_frameDidCalculated:(BOOL)isChanged
{
    [super _frameDidCalculated:isChanged];
    [self syncTextStorageForView];
}

- (void)_updateStylesOnComponentThread:(NSDictionary *)styles resetStyles:(NSMutableArray *)resetStyles isUpdateStyles:(BOOL)isUpdateStyles
{
    [super _updateStylesOnComponentThread:styles resetStyles:(NSMutableArray *)resetStyles isUpdateStyles:isUpdateStyles];
    NSMutableDictionary * newStyles = [styles mutableCopy];
    for (NSString * key in [resetStyles copy]) {
        [newStyles setObject:@"" forKey:key];
    }
    [self fillCSSStyles:newStyles];
    
    [self syncTextStorageForView];
}

- (void)_updateAttributesOnComponentThread:(NSDictionary *)attributes
{
    [super _updateAttributesOnComponentThread:attributes];
    
    [self fillAttributes:attributes];
    
    [self syncTextStorageForView];
}

- (void)drawTextWithContext:(CGContextRef)context bounds:(CGRect)bounds padding:(UIEdgeInsets)padding
{
    if (bounds.size.width <= 0 || bounds.size.height <= 0) {
        return;
    }
    
    if ([self _needsDrawBorder]) {
        [self _drawBorderWithContext:context size:bounds.size];
    } else {
        WXPerformBlockOnMainThread(^{
            [self _resetNativeBorderRadius];
        });
    }
    if (![self useCoreText]) {
        NSLayoutManager *layoutManager = _textStorage.layoutManagers.firstObject;
        NSTextContainer *textContainer = layoutManager.textContainers.firstObject;
        
        CGRect textFrame = UIEdgeInsetsInsetRect(bounds, padding);
        NSRange glyphRange = [layoutManager glyphRangeForTextContainer:textContainer];
        
        [layoutManager drawBackgroundForGlyphRange:glyphRange atPoint:textFrame.origin];
        [layoutManager drawGlyphsForGlyphRange:glyphRange atPoint:textFrame.origin];
    } else {
        CGRect textFrame = UIEdgeInsetsInsetRect(bounds, padding);
        // sufficient height for text to draw, or frame lines will be empty
        textFrame.size.height = bounds.size.height * 2;
        CGContextSaveGState(context);
        //flip the coordinate system
        CGContextSetTextMatrix(context, CGAffineTransformIdentity);
        CGContextTranslateCTM(context, 0, textFrame.size.height);
        CGContextScaleCTM(context, 1.0, -1.0);
        
        NSAttributedString * attributedStringCopy = [self ctAttributedString];
        if (!attributedStringCopy) {
            return;
        }
        //add path
        CGPathRef cgPath = CGPathCreateWithRect(textFrame, NULL);
        CTFramesetterRef ctframesetterRef = CTFramesetterCreateWithAttributedString((__bridge CFAttributedStringRef)(attributedStringCopy));
        CTFrameRef coreTextFrameRef = CTFramesetterCreateFrame(ctframesetterRef, CFRangeMake(0, attributedStringCopy.length), cgPath, NULL);
        if (NULL == coreTextFrameRef) {
            // try to protect crash from frame is NULL
            CFRelease(ctframesetterRef);
            CGPathRelease(cgPath);
            return;
        }
        CFRelease(ctframesetterRef);
        CFArrayRef ctLines = CTFrameGetLines(coreTextFrameRef);
        CFIndex lineCount = CFArrayGetCount(ctLines);
        NSMutableArray * mutableLines = [NSMutableArray new];
        CGPoint lineOrigins[lineCount];
        NSUInteger rowCount = 0;
        BOOL needTruncation = NO;
        CTLineRef ctTruncatedLine = NULL;
        CTFrameGetLineOrigins(coreTextFrameRef, CFRangeMake(0, 0), lineOrigins);
        
        if (lineCount > 0 && _lineHeight && WX_SYS_VERSION_LESS_THAN(@"10.0")) {
            CGFloat ascent, descent, leading;
            CTLineRef line1 = (CTLineRef)CFArrayGetValueAtIndex(ctLines, 0);
            CTLineGetTypographicBounds(line1, &ascent, &descent, &leading);
            lineOrigins[0].y += (_lineHeight-(leading+ascent+descent))/2;
        }
        
        for (CFIndex lineIndex = 0;(!_lines || _lines > lineIndex) && lineIndex < lineCount; lineIndex ++) {
            CTLineRef lineRef = NULL;
            lineRef = (CTLineRef)CFArrayGetValueAtIndex(ctLines, lineIndex);
            if (!lineRef) {
                break;
            }
            CGPoint lineOrigin = lineOrigins[lineIndex];
            lineOrigin.x += padding.left;
            if(_lineHeight && WX_SYS_VERSION_LESS_THAN(@"10.0")){
                lineOrigin.y = lineOrigins[0].y - padding.top - _lineHeight * lineIndex ;
            }else{
                lineOrigin.y = lineOrigin.y - padding.top ;
            }
            
            CFArrayRef runs = CTLineGetGlyphRuns(lineRef);
            [mutableLines addObject:(__bridge id _Nonnull)(lineRef)];
            // lineIndex base 0
            rowCount = lineIndex + 1;
            if (_lines > 0 && _truncationLine) {
                if (_truncationLine && rowCount > _lines) {
                    needTruncation = YES;
                    do {
                        NSUInteger lastRow = [mutableLines count];
                        if (lastRow < rowCount) {
                           break;
                        }
                        [mutableLines removeLastObject];
                    } while (1);

                }
            }
            if (_lines > 0 && _truncationLine) {
                if (rowCount >= _lines &&!needTruncation && (CTLineGetStringRange(lineRef).length + CTLineGetStringRange(lineRef).location) < attributedStringCopy.length) {
                    needTruncation = YES;
                }
            }
            
            if (needTruncation) {
                CGContextSetTextPosition(context, lineOrigin.x, lineOrigin.y);
                ctTruncatedLine = [self buildTruncatedLineWithRuns:runs lines:mutableLines path:cgPath];
                if (ctTruncatedLine) {
                    CFArrayRef truncatedRuns = CTLineGetGlyphRuns(ctTruncatedLine);
                    [self drawTextWithRuns:truncatedRuns context:context lineOrigin:lineOrigin];
                    CFRelease(ctTruncatedLine);
                    ctTruncatedLine = NULL;
                    continue;
                }
            } else {
                [self drawTextWithRuns:runs context:context lineOrigin:lineOrigin];
            }
        }
        
        [mutableLines removeAllObjects];
        CGPathRelease(cgPath);
        CFRelease(coreTextFrameRef);
        CGContextRestoreGState(context);
    }
}

- (void)drawTextWithRuns:(CFArrayRef)runs context:(CGContextRef)context lineOrigin:(CGPoint)lineOrigin
{
    for (CFIndex runIndex = 0; runIndex < CFArrayGetCount(runs); runIndex ++) {
        CTRunRef run = NULL;
        run = (CTRunRef)CFArrayGetValueAtIndex(runs, runIndex);
        CFDictionaryRef attr = NULL;
        attr = CTRunGetAttributes(run);
        //To properly draw the glyphs in a run, the fields tx and ty of the CGAffineTransform returned by CTRunGetTextMatrix should be set to the current text position.
        CGAffineTransform transform = CTRunGetTextMatrix(run);
        transform.tx = lineOrigin.x;
        transform.ty = lineOrigin.y;
        CGContextSetTextMatrix(context, transform);
        
        CGContextSetTextPosition(context, lineOrigin.x, lineOrigin.y);
        CTRunDraw(run, context, CFRangeMake(0, 0));
        CFIndex glyphCount = CTRunGetGlyphCount(run);
        if (glyphCount <= 0) continue;
        
        long longForStrikethroughStyleAttributeName= (long)CFDictionaryGetValue(attr, (__bridge void *)NSStrikethroughStyleAttributeName);
        NSUnderlineStyle strikethrough = (NSUnderlineStyle)longForStrikethroughStyleAttributeName;
        
        if (strikethrough) {
            // draw strikethrough
            [self drawLineThroughWithRun:runs context:context index:runIndex origin:lineOrigin];
        }
    }
}

- (CTLineRef)buildTruncatedLineWithRuns:(CFArrayRef)runs lines:(NSMutableArray*)mutableLines path:(CGPathRef)cgPath
{
    NSAttributedString * truncationToken = nil;
    CTLineRef ctTruncatedLine = NULL;
    CTLineRef lastLine = (__bridge CTLineRef)(mutableLines.lastObject);
   
    CFArrayRef lastLineRuns = CTLineGetGlyphRuns(lastLine);
    NSUInteger lastLineRunCount = CFArrayGetCount(lastLineRuns);
    
    CTLineRef truncationTokenLine = NULL;
    NSMutableDictionary *attrs = nil;
    if (lastLineRunCount > 0) {
        CTRunRef run = (CTRunRef)CFArrayGetValueAtIndex(runs, lastLineRunCount - 1);
        attrs = (id)CTRunGetAttributes(run);
        attrs = attrs ? attrs.mutableCopy : [NSMutableDictionary new];
        CTFontRef font = (__bridge CTFontRef)(attrs[(id)kCTFontAttributeName]);
        CGFloat fontSize = font ? CTFontGetSize(font):32 * self.weexInstance.pixelScaleFactor;
        UIFont * uiFont = [UIFont systemFontOfSize:fontSize];
        if (uiFont) {
            font = CTFontCreateWithFontDescriptor((__bridge CTFontDescriptorRef)uiFont.fontDescriptor, uiFont.pointSize, NULL);
        }
        if (font) {
            attrs[(id)kCTFontAttributeName] = (__bridge id)(font);
            uiFont = nil;
            CFRelease(font);
        }
        CGColorRef color = (__bridge CGColorRef)(attrs[(id)kCTForegroundColorAttributeName]);
        if (color && CFGetTypeID(color) == CGColorGetTypeID() && CGColorGetAlpha(color) == 0) {
            [attrs removeObjectForKey:(id)kCTForegroundColorAttributeName];
        }
        
        attrs = attrs?:[NSMutableDictionary new];
        truncationToken = [[NSAttributedString alloc] initWithString:WXTextTruncationToken attributes:attrs];
        truncationTokenLine = CTLineCreateWithAttributedString((CFAttributedStringRef)truncationToken);
    }
    
    if (truncationTokenLine) {
        // default truncationType is kCTLineTruncationEnd
        CTLineTruncationType truncationType = kCTLineTruncationEnd;
        NSAttributedString *attributedString = [self ctAttributedString];
        NSAttributedString * lastLineText = nil;
        NSRange lastLineTextRange = WXNSRangeFromCFRange(CTLineGetStringRange(lastLine));
        NSRange attributeStringRange = NSMakeRange(0, attributedString.string.length);
        NSRange interSectionRange = NSIntersectionRange(lastLineTextRange, attributeStringRange);
        if (!NSEqualRanges(interSectionRange, lastLineTextRange)) {
            // out of bounds
            lastLineTextRange = interSectionRange;
        }
        lastLineText = [attributedString attributedSubstringFromRange: lastLineTextRange];
        if (!lastLineText) {
            lastLineText = attributedString;
        }
        NSMutableAttributedString *mutableLastLineText = lastLineText.mutableCopy;
        [mutableLastLineText appendAttributedString:truncationToken];
        CTLineRef ctLastLineExtend = CTLineCreateWithAttributedString((__bridge CFAttributedStringRef)[mutableLastLineText copy]);
        if (ctLastLineExtend) {
            CGRect cgPathRect = CGRectZero;
            CGFloat truncatedWidth = 0;
            if (CGPathIsRect(cgPath, &cgPathRect)) {
                truncatedWidth = cgPathRect.size.width;
            }
            ctTruncatedLine = CTLineCreateTruncatedLine(ctLastLineExtend, truncatedWidth, truncationType, truncationTokenLine);
            CFRelease(ctLastLineExtend);
            ctLastLineExtend = NULL;
            CFRelease(truncationTokenLine);
            truncationTokenLine = NULL;
        }
    }
    
    return ctTruncatedLine;
}

- (void)drawLineThroughWithRun:(CFArrayRef)runs context:(CGContextRef)context index:(CFIndex)runIndex origin:(CGPoint)lineOrigin
{
    CFRetain(runs);
    CGContextRetain(context);
    
    CGContextSaveGState(context);
    CGFloat xHeight = 0, underLinePosition = 0, lineThickness = 0;
    CTRunRef run = (CTRunRef)CFArrayGetValueAtIndex(runs, runIndex);
    WXTextGetRunsMaxMetric(runs, &xHeight, &underLinePosition, &lineThickness);
    
    CGPoint strikethroughStart;
    strikethroughStart.x =  lineOrigin.x - underLinePosition;
    strikethroughStart.y = lineOrigin.y + xHeight/2;
    CGPoint runPosition = CGPointZero;
    CTRunGetPositions(run, CFRangeMake(0, 1), &runPosition);
    strikethroughStart.x = lineOrigin.x + runPosition.x;
    CGContextSetLineWidth(context, WXTextDefaultLineThroughWidth);
    double length = CTRunGetTypographicBounds(run, CFRangeMake(0, 0), NULL, NULL, NULL);
    CGContextMoveToPoint(context, strikethroughStart.x, strikethroughStart.y);
    CGContextAddLineToPoint(context, strikethroughStart.x + length, strikethroughStart.y);
    CGContextStrokePath(context);
    
    CGContextRestoreGState(context);
    CFRelease(runs);
    CGContextRelease(context);
}

- (CGSize)calculateTextHeightWithWidth:(CGFloat)aWidth
{
    CGFloat totalHeight = 0;
    CGSize suggestSize = CGSizeZero;
    NSAttributedString * attributedStringCpy = [self ctAttributedString];
    if (!attributedStringCpy) {
        return CGSizeZero;
    }
    if (isnan(aWidth)) {
        aWidth = CGFLOAT_MAX;
    }
    aWidth = [attributedStringCpy boundingRectWithSize:CGSizeMake(aWidth, CGFLOAT_MAX) options:NSStringDrawingUsesLineFragmentOrigin|NSStringDrawingUsesFontLeading context:nil].size.width;
    
    /* Must get ceil of aWidth. Or core text may not return correct bounds.
     Maybe aWidth without ceiling triggered some critical conditions. */
    aWidth = ceil(aWidth);
    CTFramesetterRef ctframesetterRef = CTFramesetterCreateWithAttributedString((__bridge CFAttributedStringRef)(attributedStringCpy));
    suggestSize = CTFramesetterSuggestFrameSizeWithConstraints(ctframesetterRef, CFRangeMake(0, 0), NULL, CGSizeMake(aWidth, MAXFLOAT), NULL);
    
    if (_lines == 0) {
        // If not line limit use suggestSize directly.
        CFRelease(ctframesetterRef);
        return CGSizeMake(aWidth, suggestSize.height);
    }
    
    CGMutablePathRef path = NULL;
    path = CGPathCreateMutable();
        // sufficient height to draw text
    CGPathAddRect(path, NULL, CGRectMake(0, 0, aWidth, suggestSize.height * 10));
        
    CTFrameRef frameRef = NULL;
    frameRef = CTFramesetterCreateFrame(ctframesetterRef, CFRangeMake(0, attributedStringCpy.length), path, NULL);
    CGPathRelease(path);
    CFRelease(ctframesetterRef);
    
    if (NULL == frameRef) {
        //try to protect unexpected crash.
        return suggestSize;
    }
    
    CFArrayRef lines = CTFrameGetLines(frameRef);
    CFIndex lineCount = CFArrayGetCount(lines);
    CGFloat ascent = 0;
    CGFloat descent = 0;
    CGFloat leading = 0;
    
    // height = ascent + descent + lineCount*leading
    // ignore linespaing
    NSUInteger actualLineCount = 0;
    for (CFIndex lineIndex = 0; (!_lines|| lineIndex < _lines) && lineIndex < lineCount; lineIndex ++)
    {
        CTLineRef lineRef = NULL;
        lineRef = (CTLineRef)CFArrayGetValueAtIndex(lines, lineIndex);
        CTLineGetTypographicBounds(lineRef, &ascent, &descent, &leading);
        totalHeight += ascent + descent;
        actualLineCount ++;
    }
    
    totalHeight = totalHeight + actualLineCount * leading;
    CFRelease(frameRef);
    
    if (WX_SYS_VERSION_LESS_THAN(@"10.0")) {
        // there is something wrong with coreText drawing text height, trying to fix this with more efficent way.
        if(actualLineCount && actualLineCount < lineCount) {
            suggestSize.height = suggestSize.height * actualLineCount / lineCount;
        }
        return CGSizeMake(aWidth, suggestSize.height);
    }
    return CGSizeMake(aWidth, totalHeight);
}

static void WXTextGetRunsMaxMetric(CFArrayRef runs, CGFloat *xHeight, CGFloat *underlinePosition, CGFloat *lineThickness)
{
    CFRetain(runs);
    CGFloat maxXHeight = 0;
    CGFloat maxUnderlinePos = 0;
    CGFloat maxLineThickness = 0;
    for (NSUInteger index = 0, runsCount = CFArrayGetCount(runs); index < runsCount; index ++) {
        CTRunRef run = (CTRunRef)CFArrayGetValueAtIndex(runs, index);
        CFDictionaryRef attrs = CTRunGetAttributes(run);
        if (attrs) {
            CTFontRef font = (CTFontRef)CFDictionaryGetValue(attrs, kCTFontAttributeName);
            if (font) {
                CGFloat xHeight = CTFontGetXHeight(font);
                if (xHeight > maxXHeight) {
                    maxXHeight = xHeight;
                }
                
                CGFloat underlinePos = CTFontGetUnderlinePosition(font);
                if (underlinePos < maxUnderlinePos) {
                    maxUnderlinePos = underlinePos;
                }
                
                CGFloat lineThickness = CTFontGetUnderlineThickness(font);
                if (lineThickness > maxLineThickness) {
                    maxLineThickness = lineThickness;
                }
            }
        }
    }
    
    if (xHeight) {
        *xHeight = maxXHeight;
    }
    
    if (underlinePosition) {
        *underlinePosition = maxUnderlinePos;
    }
    
    if (lineThickness) {
        *lineThickness = maxLineThickness;
    }
    
    CFRelease(runs);
}
                                                                                                                                       
NS_INLINE NSRange WXNSRangeFromCFRange(CFRange range) {
    return NSMakeRange(range.location, range.length);
}

#ifdef UITEST
- (NSString *)description
{
    return super.description;
}
#endif
 
- (void)_resetCSSNodeStyles:(NSArray *)styles
{
    [super _resetCSSNodeStyles:styles];
    if ([styles containsObject:@"color"]) {
        _color[0] = 0;
        _color[1] = 0;
        _color[2] = 0;
        _color[3] = 1.0;
        [self setNeedsRepaint];
    }
    if ([styles containsObject:@"fontSize"]) {
        _fontSize = WX_TEXT_FONT_SIZE;
        [self setNeedsRepaint];
        [self setNeedsLayout];
    }
}

@end

