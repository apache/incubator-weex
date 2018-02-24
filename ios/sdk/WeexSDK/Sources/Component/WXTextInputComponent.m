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

#import "WXTextInputComponent.h"
#import "WXConvert.h"

@implementation WXTextInputView

- (instancetype)init
{
    self = [super init];
    if (self) {
        _padding = UIEdgeInsetsZero;
        _border = UIEdgeInsetsZero;
    }
    return self;
}

- (CGRect)textRectForBounds:(CGRect)bounds
{
    bounds.size.width -= self.padding.left + self.border.left;
    bounds.origin.x += self.padding.left + self.border.left;
    
    bounds.size.height -= self.padding.top + self.border.top;
    bounds.origin.y += self.padding.top + self.border.top;
    
    bounds.size.width -= self.padding.right + self.border.right;
    
    bounds.size.height -= self.padding.bottom + self.border.bottom;
    
    return bounds;
}

- (BOOL)canPerformAction:(SEL)action withSender:(id)sender
{
    // this behavior will hide the action like copy, cut, paste, selectAll and so on.
    return [[self.wx_component valueForKey:@"allowCopyPaste"] boolValue];
}

- (CGRect)editingRectForBounds:(CGRect)bounds
{
    return [self textRectForBounds:bounds];
}
@end

@interface WXTextInputComponent()

@property (nonatomic, strong) WXTextInputView *inputView;
@property (nonatomic, assign) BOOL allowCopyPaste;

@end

@implementation WXTextInputComponent

- (instancetype)initWithRef:(NSString *)ref type:(NSString *)type styles:(NSDictionary *)styles attributes:(NSDictionary *)attributes events:(NSArray *)events weexInstance:(WXSDKInstance *)weexInstance
{
    if (self = [super initWithRef:ref type:type styles:styles attributes:attributes events:events weexInstance:weexInstance]) {
        _allowCopyPaste = YES;
        if (attributes[@"allowCopyPaste"]) {
            _allowCopyPaste = [WXConvert BOOL:attributes[@"allowCopyPaste"]];
        }
    }
    return self;
}

- (UIView *)loadView
{
    _inputView = [[WXTextInputView alloc] init];
    return _inputView;
}

- (void)updateAttributes:(NSDictionary *)attributes {
    [super updateAttributes:attributes];
    if (attributes[@"allowCopyPaste"]) {
        _allowCopyPaste = [WXConvert BOOL:attributes[@"allowCopyPaste"]];
    }
}

-(void)viewDidLoad
{
    [super viewDidLoad];
    _inputView.delegate = self;
}

# pragma mark - overwrite method
-(NSString *)text
{
    return _inputView.text;
}
- (void)setText:(NSString *)text
{
    _inputView.text = text;
}
-(void)setTextColor:(UIColor *)color
{
    _inputView.textColor = color;
}

-(void)setTextAlignment:(NSTextAlignment)textAlignForStyle
{
    _inputView.textAlignment = textAlignForStyle;
}
-(void)setUserInteractionEnabled:(BOOL)userInteractionEnabled
{
    _inputView.userInteractionEnabled = userInteractionEnabled;
}
-(void)setEnabled:(BOOL)enabled
{
    _inputView.enabled=enabled;
}
-(void)setReturnKeyType:(UIReturnKeyType)returnKeyType
{
    _inputView.returnKeyType = returnKeyType;
}
-(void)setInputAccessoryView:(UIView *)inputAccessoryView
{
    _inputView.inputAccessoryView = inputAccessoryView;
}
-(void)setKeyboardType:(UIKeyboardType)keyboardType
{
    _inputView.keyboardType = keyboardType;
}
-(void)setSecureTextEntry:(BOOL)secureTextEntry
{
    _inputView.secureTextEntry = secureTextEntry;
}
-(void)setEditPadding:(UIEdgeInsets)padding
{
    [_inputView setPadding:padding];
}
-(void)setEditBorder:(UIEdgeInsets)border
{
    [_inputView setBorder:border];
}

-(void)setAttributedPlaceholder:(NSMutableAttributedString *)attributedString font:font
{
    [_inputView setAttributedPlaceholder:attributedString];
}

-(void)setFont:(UIFont *)font
{
    [_inputView setFont:font];
}

-(void)setEditSelectionRange:(NSInteger)selectionStart selectionEnd:(NSInteger)selectionEnd
{
    UITextPosition *startPos =  [self.inputView positionFromPosition:self.inputView.beginningOfDocument offset:selectionStart];
    UITextPosition *endPos = [self.inputView positionFromPosition:self.inputView.beginningOfDocument offset:selectionEnd];
    UITextRange *textRange = [self.inputView textRangeFromPosition:startPos
                                                        toPosition:endPos];
    self.inputView.selectedTextRange = textRange;
}

-(NSDictionary *)getEditSelectionRange
{
    NSInteger selectionStart = [self.inputView offsetFromPosition:self.inputView.beginningOfDocument toPosition:self.inputView.selectedTextRange.start];
    NSInteger selectionEnd = [self.inputView offsetFromPosition:self.inputView.beginningOfDocument toPosition:self.inputView.selectedTextRange.end];
    NSDictionary *res = @{@"selectionStart":@(selectionStart),@"selectionEnd":@(selectionEnd)};
    return res;
}

@end
