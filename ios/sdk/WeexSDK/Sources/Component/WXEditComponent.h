/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import <Foundation/Foundation.h>
#import "WXComponent.h"
#import "WXTextComponentProtocol.h"
#import "WXDatePickerManager.h"

@interface WXEditComponent : WXComponent<WXDatePickerManagerDelegate,UITextViewDelegate,UITextFieldDelegate>

//attribute
@property (nonatomic, strong) UIColor *placeholderColor;
@property (nonatomic, strong) NSString *placeholderString;
@property (nonatomic, strong) UILabel *placeHolderLabel;

@end
