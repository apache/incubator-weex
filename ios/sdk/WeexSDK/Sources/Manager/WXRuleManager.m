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

#import "WXRuleManager.h"
#import "WXThreadSafeMutableDictionary.h"
#import "WXUtility.h"
#import "WXConvert.h"
#import "WXHandlerFactory.h"
#import "WXURLRewriteProtocol.h"
#import "WXComponentManager.h"
#import "WXDefine.h"
#import "WXSDKEngine.h"

@interface WXRuleManager()
@property (nonatomic, strong) WXThreadSafeMutableDictionary *fontStorage;
@end

@implementation WXRuleManager

static WXRuleManager *_sharedInstance = nil;

+ (WXRuleManager *)sharedInstance
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        if (!_sharedInstance) {
            _sharedInstance = [[self alloc] init];
            _sharedInstance.fontStorage = [[WXThreadSafeMutableDictionary alloc] init];
        }
    });
    return _sharedInstance;
}

- (void)removeRule:(NSString *)type rule:(NSDictionary *)rule
{
    if ([type isEqualToString:@"fontFace"]) {
        if (rule[@"fontFamily"]) {
            [_fontStorage removeObjectForKey:rule[@"fontFamily"]];
        } else {
            [_fontStorage removeAllObjects];
        }
    }
}

- (void)addRule:(NSString*)type rule:(NSDictionary *)rule
{
    if ([type isEqualToString:@"fontFace"] && [rule[@"src"] isKindOfClass:[NSString class]]) {
        if (rule[@"src"] && rule[@"fontFamily"] && ![WXUtility isBlankString:rule[@"src"]]) {
            NSString *ruleSrc = [WXConvert NSString:rule[@"src"]];
            if (!ruleSrc) {
                WXLogError(@"%@ is illegal for font src",rule[@"src"]);
                return;
            }
            NSUInteger start = [rule[@"src"] rangeOfString:@"url('"].location + @"url('".length;
            NSUInteger end  = [rule[@"src"] rangeOfString:@"')" options:NSBackwardsSearch].location;
            if (end <= start || start == NSNotFound || end == NSNotFound) {
                WXLogWarning(@"font url is not specified");
                return;
            }
            
            NSString *fontSrc = [rule[@"src"] substringWithRange:NSMakeRange(start, end-start)];
            NSString *newURL = [fontSrc copy];
            WX_REWRITE_URL(fontSrc, WXResourceTypeFont, self.instance)
            
            if (!newURL) {
                return;
            }
            
            fontSrc = newURL;
            WXThreadSafeMutableDictionary * fontFamily = [self.fontStorage objectForKey:rule[@"fontFamily"]];
            if (fontFamily && [fontFamily[@"tempSrc"] isEqualToString:fontSrc]) {
                // if the new src is same as src in dictionary , ignore it, or update it
                return;
            }
            if (!fontFamily) {
                fontFamily = [[WXThreadSafeMutableDictionary alloc] init];
            }
            NSURL *fontURL = [NSURL URLWithString:fontSrc];
            if (!fontURL) {
                // if the fontSrc string is illegal, the fontURL will be nil
                return;
            }
            if([fontURL isFileURL]){
                [fontFamily setObject:fontSrc forKey:@"src"];
            }else {
                [fontFamily setObject:fontSrc forKey:@"tempSrc"];
            }
            
            [_fontStorage setObject:fontFamily forKey:rule[@"fontFamily"]];
            // remote font file
            NSString *fontfile = [NSString stringWithFormat:@"%@/%@",WX_FONT_DOWNLOAD_DIR,[WXUtility md5:[fontURL absoluteString]]];
            if ([WXUtility isFileExist:fontfile]) {
                // if has been cached, load directly
                [fontFamily setObject:[NSURL fileURLWithPath:fontfile] forKey:@"localSrc"];
                return;
            }
            __weak typeof(self) weakSelf = self;
            [WXUtility getIconfont:fontURL completion:^(NSURL * _Nonnull url, NSError * _Nullable error) {
                if (!error && url) {
                    // load success
                    WXThreadSafeMutableDictionary * dictForFontFamily = [weakSelf.fontStorage objectForKey:rule[@"fontFamily"]];
                    NSString *fontSrc = [dictForFontFamily objectForKey:@"tempSrc"];
                    if (fontSrc) {
                        // only remote font will be mark as tempSrc
                        [dictForFontFamily setObject:fontSrc forKey:@"src"];
                    }
                    [dictForFontFamily setObject:url forKey:@"localSrc"];
                    
                    [[NSNotificationCenter defaultCenter] postNotificationName:WX_ICONFONT_DOWNLOAD_NOTIFICATION object:nil userInfo:@{@"fontFamily":rule[@"fontFamily"]}];
                } else {
                    //there was some errors during loading
                    WXLogError(@"load font failed %@",error.description);
                }
            }];
        }
    }
}

- (WXThreadSafeMutableDictionary *)getRule:(NSString *)type
{
    if ([type isEqualToString:@"fontFace"]) {
        return _fontStorage;
    }
    
    return nil;
}

@end
