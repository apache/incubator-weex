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
#import <pthread/pthread.h>
#import <CoreText/CoreText.h>

// WXText is a non-public is not permitted
@interface WXTextView : UIView
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

- (void)setTextStorage:(NSTextStorage *)textStorage
{
    if (_textStorage != textStorage) {
        _textStorage = textStorage;
        [self.wx_component setNeedsDisplay];
    }
}

- (NSString *)description
{
    NSString *superDescription = super.description;
    NSRange semicolonRange = [superDescription rangeOfString:@";"];
    NSString * content = _textStorage.string;
    if ([(WXTextComponent*)self.wx_component useCoreText]) {
        content = [(WXTextComponent*)self.wx_component valueForKey:@"_text"];
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
    return [(WXTextComponent*)self.wx_component valueForKey:@"_text"];
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

static BOOL textRenderUsingCoreText = YES;

NSString *const WXTextTruncationToken = @"\u2026";
CGFloat WXTextDefaultLineThroughWidth = 1.2;

@interface WXTextComponent()
@property (nonatomic, strong) NSString *useCoreTextAttr;
@end

@implementation WXTextComponent
{
    UIEdgeInsets _border;
    UIEdgeInsets _padding;
    NSTextStorage *_textStorage;
    CGFloat _textStorageWidth;
    
    NSString *_text;
    UIColor *_color;
    NSString *_fontFamily;
    CGFloat _fontSize;
    CGFloat _fontWeight;
    WXTextStyle _fontStyle;
    NSUInteger _lines;
    NSTextAlignment _textAlign;
    WXTextDecoration _textDecoration;
    NSString *_textOverflow;
    CGFloat _lineHeight;
    CGFloat _letterSpacing;
    BOOL _truncationLine; // support trunk tail
    
    BOOL _needsRemoveObserver;
    NSAttributedString * _ctAttributedString;
    
    pthread_mutex_t _ctAttributedStringMutex;
    pthread_mutexattr_t _propertMutexAttr;
}

+ (void)setRenderUsingCoreText:(BOOL)usingCoreText
{
    textRenderUsingCoreText = usingCoreText;
}

+ (BOOL)textRenderUsingCoreText
{
    return textRenderUsingCoreText;
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
        _needsRemoveObserver = NO;
        pthread_mutexattr_init(&(_propertMutexAttr));
        pthread_mutexattr_settype(&(_propertMutexAttr), PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&(_ctAttributedStringMutex), &(_propertMutexAttr));
        
        if ([attributes objectForKey:@"coretext"]) {
            _useCoreTextAttr = [WXConvert NSString:attributes[@"coretext"]];
        } else {
            _useCoreTextAttr = nil;
        }
        
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
    
    if ([WXTextComponent textRenderUsingCoreText]) {
        return YES;
    }
    return NO;
}

- (void)dealloc
{
    if (_needsRemoveObserver) {
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
    WX_STYLE_FILL_TEXT(color, color, UIColor, NO)
    WX_STYLE_FILL_TEXT(fontFamily, fontFamily, NSString, YES)
    WX_STYLE_FILL_TEXT_PIXEL(fontSize, fontSize, YES)
    WX_STYLE_FILL_TEXT(fontWeight, fontWeight, WXTextWeight, YES)
    WX_STYLE_FILL_TEXT(fontStyle, fontStyle, WXTextStyle, YES)
    WX_STYLE_FILL_TEXT(lines, lines, NSUInteger, YES)
    WX_STYLE_FILL_TEXT(textAlign, textAlign, NSTextAlignment, NO)
    WX_STYLE_FILL_TEXT(textDecoration, textDecoration, WXTextDecoration, YES)
    WX_STYLE_FILL_TEXT(textOverflow, textOverflow, NSString, NO)
    WX_STYLE_FILL_TEXT_PIXEL(lineHeight, lineHeight, YES)
    WX_STYLE_FILL_TEXT_PIXEL(letterSpacing, letterSpacing, YES)
    
    UIEdgeInsets padding = {
        WXFloorPixelValue(self.cssNode->style.padding[CSS_TOP] + self.cssNode->style.border[CSS_TOP]),
        WXFloorPixelValue(self.cssNode->style.padding[CSS_LEFT] + self.cssNode->style.border[CSS_LEFT]),
        WXFloorPixelValue(self.cssNode->style.padding[CSS_BOTTOM] + self.cssNode->style.border[CSS_BOTTOM]),
        WXFloorPixelValue(self.cssNode->style.padding[CSS_RIGHT] + self.cssNode->style.border[CSS_RIGHT])
    };
    
    if (!UIEdgeInsetsEqualToEdgeInsets(padding, _padding)) {
        _padding = padding;
        [self setNeedsRepaint];
    }
}

- (void)fillAttributes:(NSDictionary *)attributes
{
    id text = attributes[@"value"];
    if (text) {
        _text = [WXConvert NSString:text];
        [self setNeedsRepaint];
        [self setNeedsLayout];
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
    
    if (!_testId) {
        self.view.isAccessibilityElement = YES;
    }
    
    [self setNeedsDisplay];
}

- (UIView *)loadView
{
    return [[WXTextView alloc] init];
}

- (BOOL)needsDrawRect
{
    return YES;
}

- (UIImage *)drawRect:(CGRect)rect;
{
    CGContextRef context = UIGraphicsGetCurrentContext();
    if (_isCompositingChild) {
        [self drawTextWithContext:context bounds:rect padding:_padding view:nil];
    } else {
        WXTextView *textView = (WXTextView *)_view;
        [self drawTextWithContext:context bounds:rect padding:_padding view:textView];
    }
    
    return nil;
}

- (CGSize (^)(CGSize))measureBlock
{
    __weak typeof(self) weakSelf = self;
    return ^CGSize (CGSize constrainedSize) {
        CGSize computedSize = CGSizeZero;
        NSTextStorage *textStorage = nil;
        if (![self useCoreText]) {
            textStorage = [weakSelf textStorageWithWidth:constrainedSize.width];
            NSLayoutManager *layoutManager = textStorage.layoutManagers.firstObject;
            NSTextContainer *textContainer = layoutManager.textContainers.firstObject;
            computedSize = [layoutManager usedRectForTextContainer:textContainer].size;
        } else {
            computedSize = [weakSelf calculateTextHeightWithWidth:constrainedSize.width];
        }
    
        //TODO:more elegant way to use max and min constrained size
        if (!isnan(weakSelf.cssNode->style.minDimensions[CSS_WIDTH])) {
            computedSize.width = MAX(computedSize.width, weakSelf.cssNode->style.minDimensions[CSS_WIDTH]);
        }
        
        if (!isnan(weakSelf.cssNode->style.maxDimensions[CSS_WIDTH])) {
            computedSize.width = MIN(computedSize.width, weakSelf.cssNode->style.maxDimensions[CSS_WIDTH]);
        }
        
        if (!isnan(weakSelf.cssNode->style.minDimensions[CSS_HEIGHT])) {
            computedSize.height = MAX(computedSize.height, weakSelf.cssNode->style.minDimensions[CSS_HEIGHT]);
        }
        
        if (!isnan(weakSelf.cssNode->style.maxDimensions[CSS_HEIGHT])) {
            computedSize.height = MIN(computedSize.height, weakSelf.cssNode->style.maxDimensions[CSS_HEIGHT]);
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

- (NSString *)text
{
    return _text;
}

- (NSAttributedString *)ctAttributedString
{
    pthread_mutex_lock(&(_ctAttributedStringMutex));
    if (!_ctAttributedString) {
        _ctAttributedString = [[self buildCTAttributeString] copy];
    }
    pthread_mutex_unlock(&(_ctAttributedStringMutex));
    return [_ctAttributedString copy];
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
    NSString *string = [NSString stringWithFormat:@"%@", [self text] ?: @""];
    NSMutableAttributedString *attributedString = [[NSMutableAttributedString alloc] initWithString:string];
    if (_color) {
        [attributedString addAttribute:NSForegroundColorAttributeName value:_color range:NSMakeRange(0, string.length)];
    }
    
    if (_fontFamily) {
        NSString * keyPath = [NSString stringWithFormat:@"%@.tempSrc", _fontFamily];
        NSString * fontSrc = [[[WXRuleManager sharedInstance] getRule:@"fontFace"] valueForKeyPath:keyPath];
        keyPath = [NSString stringWithFormat:@"%@.localSrc", _fontFamily];
        NSString * fontLocalSrc = [[[WXRuleManager sharedInstance] getRule:@"fontFace"] valueForKeyPath:keyPath];
        //custom localSrc is cached
        if (!fontLocalSrc && fontSrc) {
            // if use custom font, when the custom font download finish, refresh text.
            _needsRemoveObserver = YES;
            [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(repaintText:) name:WX_ICONFONT_DOWNLOAD_NOTIFICATION object:nil];
        }
    }
    
    // set font
    UIFont *font = [WXUtility fontWithSize:_fontSize textWeight:_fontWeight textStyle:_fontStyle fontFamily:_fontFamily scaleFactor:self.weexInstance.pixelScaleFactor useCoreText:[self useCoreText]];
    CTFontRef ctFont = CTFontCreateWithName((__bridge CFStringRef)font.fontName,
                                           font.pointSize,
                                           NULL);
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
    
    if (_textAlign) {
        paragraphStyle.alignment = _textAlign;
    }
    
    // set default lineBreakMode
    paragraphStyle.lineBreakMode = NSLineBreakByCharWrapping;
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
    
    if ([self adjustLineHeight]) {
        if (_lineHeight > font.lineHeight) {
            [attributedString addAttribute:NSBaselineOffsetAttributeName
                                     value:@((_lineHeight - font.lineHeight)/ 2)
                                     range:(NSRange){0, attributedString.length}];
        }
    }
    
    return attributedString;
}

- (NSAttributedString *)buildAttributeString
{
    NSString *string = [self text] ?: @"";
    
    NSMutableAttributedString *attributedString = [[NSMutableAttributedString alloc] initWithString:string];
    
    // set textColor
    if(_color) {
        [attributedString addAttribute:NSForegroundColorAttributeName value:_color range:NSMakeRange(0, string.length)];
    }
    
    if (_fontFamily) {
        NSString * keyPath = [NSString stringWithFormat:@"%@.tempSrc", _fontFamily];
        NSString * fontSrc = [[[WXRuleManager sharedInstance] getRule:@"fontFace"] valueForKeyPath:keyPath];
        keyPath = [NSString stringWithFormat:@"%@.localSrc", _fontFamily];
        NSString * fontLocalSrc = [[[WXRuleManager sharedInstance] getRule:@"fontFace"] valueForKeyPath:keyPath];
        //custom localSrc is cached
        if (!fontLocalSrc && fontSrc) {
            // if use custom font, when the custom font download finish, refresh text.
            _needsRemoveObserver = YES;
            [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(repaintText:) name:WX_ICONFONT_DOWNLOAD_NOTIFICATION object:nil];
        }
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

    if (_textAlign) {
        paragraphStyle.alignment = _textAlign;
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
    if ([self adjustLineHeight]) {
        if (_lineHeight > font.lineHeight) {
            [attributedString addAttribute:NSBaselineOffsetAttributeName
                                     value:@((_lineHeight - font.lineHeight)/ 2)
                                     range:(NSRange){0, attributedString.length}];
        }
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
    
    textContainer.lineBreakMode = NSLineBreakByClipping;
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
    
    [self fillCSSStyles:styles];
    
    [self syncTextStorageForView];
}

- (void)_updateAttributesOnComponentThread:(NSDictionary *)attributes
{
    [super _updateAttributesOnComponentThread:attributes];
    
    [self fillAttributes:attributes];
    
    [self syncTextStorageForView];
}

- (void)drawTextWithContext:(CGContextRef)context bounds:(CGRect)bounds padding:(UIEdgeInsets)padding view:(WXTextView *)view
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
        //add path
        CGPathRef cgPath = NULL;
        cgPath = CGPathCreateWithRect(textFrame, NULL);
        CTFrameRef _coreTextFrameRef = NULL;
        if (_coreTextFrameRef) {
            CFRelease(_coreTextFrameRef);
            _coreTextFrameRef = NULL;
        }
        if(!attributedStringCopy) {
            return;
        }
        CTFramesetterRef ctframesetterRef = CTFramesetterCreateWithAttributedString((__bridge CFAttributedStringRef)(attributedStringCopy));
        _coreTextFrameRef = CTFramesetterCreateFrame(ctframesetterRef, CFRangeMake(0, attributedStringCopy.length), cgPath, NULL);
        CFArrayRef ctLines = NULL;
        if (NULL == _coreTextFrameRef) {
            // try to protect crash from frame is NULL
            return;
        }
        CFRelease(ctframesetterRef);
        ctframesetterRef = NULL;
        ctLines = CTFrameGetLines(_coreTextFrameRef);
        CFIndex lineCount = CFArrayGetCount(ctLines);
        NSMutableArray * mutableLines = [NSMutableArray new];
        CGPoint lineOrigins[lineCount];
        NSUInteger rowCount = 0;
        BOOL needTruncation = NO;
        CTLineRef ctTruncatedLine = NULL;
        CTFrameGetLineOrigins(_coreTextFrameRef, CFRangeMake(0, 0), lineOrigins);
        for (CFIndex lineIndex = 0;(!_lines || _lines > lineIndex) && lineIndex < lineCount; lineIndex ++) {
            CTLineRef lineRef = NULL;
            lineRef = CFArrayGetValueAtIndex(ctLines, lineIndex);
            if (!lineRef) {
                break;
            }
            CGPoint lineOrigin = lineOrigins[lineIndex];
            lineOrigin.x += padding.left;
            lineOrigin.y -= padding.top;
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
            }else {
                [self drawTextWithRuns:runs context:context lineOrigin:lineOrigin];
            }
        }
        
        [mutableLines removeAllObjects];
        CGPathRelease(cgPath);
        CFRelease(_coreTextFrameRef);
        _coreTextFrameRef = NULL;
        cgPath = NULL;
        CGContextRestoreGState(context);
    }
}

- (void)drawTextWithRuns:(CFArrayRef)runs context:(CGContextRef)context lineOrigin:(CGPoint)lineOrigin
{
    for (CFIndex runIndex = 0; runIndex < CFArrayGetCount(runs); runIndex ++) {
        CTRunRef run = NULL;
        run = CFArrayGetValueAtIndex(runs, runIndex);
        CFDictionaryRef attr = NULL;
        attr = CTRunGetAttributes(run);
        if (0 == runIndex) {
            NSNumber *baselineOffset = (NSNumber*)CFDictionaryGetValue(attr, NSBaselineOffsetAttributeName);
            if (baselineOffset) {
                lineOrigin.y += [baselineOffset doubleValue];
            }
        }
        CGContextSetTextPosition(context, lineOrigin.x, lineOrigin.y);
        CTRunDraw(run, context, CFRangeMake(0, 0));
        CFIndex glyphCount = CTRunGetGlyphCount(run);
        if (glyphCount <= 0) continue;
        
        NSUnderlineStyle strikethrough = (NSUnderlineStyle)CFDictionaryGetValue(attr, NSStrikethroughStyleAttributeName);
        
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
        CTRunRef run = CFArrayGetValueAtIndex(runs, lastLineRunCount - 1);
        attrs = (id)CTRunGetAttributes(run);
        attrs = attrs ? attrs.mutableCopy : [NSMutableDictionary new];
        CTFontRef font = (__bridge CTFontRef)(attrs[(id)kCTFontAttributeName]);
        CGFloat fontSize = font ? CTFontGetSize(font):32 * self.weexInstance.pixelScaleFactor;
        UIFont * uiFont = [UIFont systemFontOfSize:fontSize];
        if (uiFont) {
            font = CTFontCreateWithName((__bridge CFStringRef)uiFont.fontName, uiFont.pointSize, NULL);
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
    CTRunRef run = CFArrayGetValueAtIndex(runs, runIndex);
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
    CTFramesetterRef ctframesetterRef = CTFramesetterCreateWithAttributedString((__bridge CFAttributedStringRef)(attributedStringCpy));
    suggestSize = CTFramesetterSuggestFrameSizeWithConstraints(ctframesetterRef, CFRangeMake(0, 0), NULL, CGSizeMake(aWidth, MAXFLOAT), NULL);
    
    CGMutablePathRef path = NULL;
    path = CGPathCreateMutable();
        // sufficient height to draw text
    CGPathAddRect(path, NULL, CGRectMake(0, 0, aWidth, suggestSize.height * 10));
        
    CTFrameRef frameRef = NULL;
    frameRef = CTFramesetterCreateFrame(ctframesetterRef, CFRangeMake(0, attributedStringCpy.length), path, NULL);
    CGPathRelease(path);
    
    CFArrayRef lines = NULL;
    if (NULL == frameRef) {
        //try to protect unexpected crash.
        return suggestSize;
    }
    CFRelease(ctframesetterRef);
    ctframesetterRef = NULL;
    lines = CTFrameGetLines(frameRef);
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
        lineRef = CFArrayGetValueAtIndex(lines, lineIndex);
        CTLineGetTypographicBounds(lineRef, &ascent, &descent, &leading);
        totalHeight += ascent + descent;
        actualLineCount ++;
    }
    
    totalHeight = totalHeight + actualLineCount * leading;
    CFRelease(frameRef);
    frameRef = NULL;
    
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
        CTRunRef run = CFArrayGetValueAtIndex(runs, index);
        CFDictionaryRef attrs = CTRunGetAttributes(run);
        if (attrs) {
            CTFontRef font = CFDictionaryGetValue(attrs, kCTFontAttributeName);
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
        _color = [UIColor blackColor];
        [self setNeedsRepaint];
    }
    if ([styles containsObject:@"fontSize"]) {
        _fontSize = WX_TEXT_FONT_SIZE;
        [self setNeedsRepaint];
        [self setNeedsLayout];
    }
}

@end

