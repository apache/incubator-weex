/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import "WXCSSDomainController.h"
#import "WXDOMDomainController.h"
#import "WXCSSTypes.h"
#import <WeexSDK/WXComponent.h>
#import <WeexSDK/WXSDKInstance.h>
#import <WeexSDK/WXUtility.h>

@implementation WXCSSDomainController
@dynamic domain;

+ (WXCSSDomainController *)defaultInstance {
    static WXCSSDomainController *defaultInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        defaultInstance = [[WXCSSDomainController alloc] init];
    });
    return defaultInstance;
}

+ (Class)domainClass {
    return [WXCSSDomain class];
}

#pragma mark - private method
- (WXDOMNode *)p_getNodeFromNodeId:(NSNumber *)nodeId rootNode:(WXDOMNode *)rootNode{
    if (!rootNode) {
        return nil;
    }
    if ([nodeId longValue] == [rootNode.nodeId longValue]) {
        return rootNode;
    }
    if (rootNode.children.count > 0) {
        for (WXDOMNode *node in rootNode.children) {
            if ([node.nodeId longValue] == [nodeId longValue]) {
                return node;
            }else {
                WXDOMNode *returnNode = [self p_getNodeFromNodeId:nodeId rootNode:node];
                if (returnNode) {
                    return returnNode;
                }
            }
        }
    }
    return nil;
}

- (NSArray *)p_formateFrame:(NSString *)frameStr {
    NSCharacterSet *characterSet1 = [NSCharacterSet characterSetWithCharactersInString:@"{},"];
    NSArray *array = [frameStr componentsSeparatedByCharactersInSet:characterSet1];
    NSMutableArray *position = [NSMutableArray array];
    for(NSString *string in array)
    {
        NSString *removeFrame = [string stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
        if ([removeFrame length] > 0) {
            [position addObject:removeFrame];
        }
    }
    return position;
}

#pragma mark - WXCSSCommandDelegate
- (void)domain:(WXCSSDomain *)domain getMatchedStylesForNodeWithNodeId:(NSNumber *)nodeId includePseudo:(NSNumber *)includePseudo includeInherited:(NSNumber *)includeInherited callback:(void (^)(NSArray *matchedCSSRules, NSArray *pseudoElements, NSArray *inherited, id error))callback {
    
    NSArray *inherited = @[];
    NSArray *pseudoElements = @[];
    /*
    //assembling object of rule
    WXDOMNode *rootDomNode = [WXDOMDomainController defaultInstance].rootComponentNode;
    WXDOMNode *node = [self p_getNodeFromNodeId:nodeId rootNode:rootDomNode];
    if (!node) {
        rootDomNode = [WXDOMDomainController defaultInstance].rootComponentNode;
        node = [self p_getNodeFromNodeId:nodeId rootNode:rootDomNode];
        if (!node) {
            callback(nil,pseudoElements,inherited,nil);
            return;
        }
    }
     */
    
    NSInteger transformNodeId = nodeId.integerValue;
    if (transformNodeId > 2) {
        transformNodeId -= 2;
    }
    NSString *nodeKey = [NSString stringWithFormat:@"%ld",transformNodeId];
    NSString *nodeName = nil;
    
    /********actual********/
    UIView *selectedView = [[[WXDOMDomainController defaultInstance] getObjectsForComponentRefs] objectForKey:[NSString stringWithFormat:@"%ld",nodeId.integerValue]];
    NSArray *actualAttrs = [[WXDOMDomainController defaultInstance] attributesArrayForObject:selectedView];
    
    WXCSSSelectorListData *selectorData = [[WXCSSSelectorListData alloc] init];
    //assembling object of cssStyle
    WXCSSCSSStyle *style = [[WXCSSCSSStyle alloc] init];
    NSMutableArray *cssProperties = [NSMutableArray array];
    NSMutableString *cssText = [[NSMutableString alloc] init];
    for (int i = 0; i < actualAttrs.count; i++) {
        if (i & 1) {
            if (![actualAttrs[i-1] isEqualToString:@"frame"]) {
                WXCSSCSSProperty *cssProperty = [[WXCSSCSSProperty alloc] init];
                [cssText appendFormat:@"%@;",actualAttrs[i]];
                cssProperty.name = actualAttrs[i-1];
                cssProperty.value = actualAttrs[i];
                [cssProperties addObject:[cssProperty WX_JSONObject]];
            }else {
                NSArray *names = @[@"width",@"height",@"top",@"left"];
                NSArray *position = [self p_formateFrame:actualAttrs[i]];
                if (position.count == 4) {
                    NSDictionary *property = @{@"left":position[0],
                                               @"top":position[1],
                                               @"width":position[2],
                                               @"height":position[3]};
                    for (int i = 0; i < property.count; i++) {
                        WXCSSCSSProperty *cssProperty = [[WXCSSCSSProperty alloc] init];
                        cssProperty.name = names[i];
                        cssProperty.value = property[names[i]];
                        [cssText appendString:[NSString stringWithFormat:@"%@:%@;",cssProperty.name,cssProperty.value]];
                        [cssProperties addObject:[cssProperty WX_JSONObject]];
                    }
                }
            }
        }else {
            if (![actualAttrs[i] isEqualToString:@"frame"]) {
                [cssText appendFormat:@"%@:",actualAttrs[i]];
                if ([actualAttrs[i] isEqualToString:@"class"]) {
                    nodeName = actualAttrs[i+1];
                }
            }
        }
    }
    selectorData.text = nodeName ? : @"";
    selectorData.selectors = @[@{@"text":nodeName ? : @""},@{@"text":@"actual value"}];
    style.shorthandEntries = @[];
    style.cssText = cssText;
    style.cssProperties = [NSArray arrayWithArray:cssProperties];
    WXCSSCSSRule *rule = [[WXCSSCSSRule alloc] init];
    rule.media = @[];
    rule.origin = @"inspector";
    rule.selectorList = selectorData;
    rule.style = style;
    /********vdom********/
    WXCSSSelectorListData *vdomSelectorData = [[WXCSSSelectorListData alloc] init];
    vdomSelectorData.text = nodeName ? : @"";
    vdomSelectorData.selectors = @[@{@"text":nodeName ? : @""},@{@"text":@"vdom value"}];
    //assembling object of cssStyle
    WXCSSCSSStyle *vdomStyle = [[WXCSSCSSStyle alloc] init];
    NSMutableArray *vdomCssProperties = [NSMutableArray array];
    NSMutableString *vdomCssText = [[NSMutableString alloc] init];
    WXComponent *component = [[WXDOMDomainController defaultInstance] _getComponentFromRef:nodeKey];
    if (component) {
        NSDictionary *vdomStyles = component.styles;
        if (vdomStyles.allKeys > 0) {
            for (NSString *key in vdomStyles.allKeys) {
                WXCSSCSSProperty *cssProperty = [[WXCSSCSSProperty alloc] init];
                cssProperty.name = key;
                if ([[vdomStyles objectForKey:key] isKindOfClass:[NSString class]]) {
                    cssProperty.value = [vdomStyles objectForKey:key];
                }else {
                    cssProperty.value = [NSString stringWithFormat:@"%@",[vdomStyles objectForKey:key]];
                }
                [vdomCssText appendString:[NSString stringWithFormat:@"%@:%@;",cssProperty.name,cssProperty.value]];
                [vdomCssProperties addObject:[cssProperty WX_JSONObject]];
            }
        }
    }
    
    vdomStyle.shorthandEntries = @[];
    vdomStyle.cssText = vdomCssText;
    vdomStyle.cssProperties = [NSArray arrayWithArray:vdomCssProperties];
    
    WXCSSCSSRule *vdomRule = [[WXCSSCSSRule alloc] init];
    vdomRule.media = @[];
    vdomRule.origin = @"inspector";
    vdomRule.selectorList = vdomSelectorData;
    vdomRule.style = vdomStyle;
    
    
    if ([rule WX_JSONObject] && [vdomRule WX_JSONObject]) {
        NSDictionary *ruleMatch = @{@"matchingSelectors":@[[NSNumber numberWithInteger:0]],@"rule":[rule WX_JSONObject]};
        NSDictionary *vdomRuleMatch = @{@"matchingSelectors":@[[NSNumber numberWithInteger:0]],@"rule":[vdomRule WX_JSONObject]};
        NSArray *matchCSSRules = @[ruleMatch, vdomRuleMatch];
        callback(matchCSSRules,pseudoElements,inherited,nil);
    }else {
        callback(nil,pseudoElements,inherited,nil);
    }
}

- (void)domain:(WXCSSDomain *)domain getInlineStylesForNodeWithNodeId:(NSNumber *)nodeId callback:(void (^)(WXCSSCSSStyle *inlineStyle, WXCSSCSSStyle *attributesStyle, id error))callback {
    WXCSSCSSStyle *inlineStyle = [[WXCSSCSSStyle alloc] init];
    inlineStyle.styleSheetId = @"22222.2";
    inlineStyle.cssProperties = @[];
    inlineStyle.cssText = @"";
    inlineStyle.shorthandEntries = @[];
    WXCSSSourceRange *range = [[WXCSSSourceRange alloc] init];
    range.startLine = [NSNumber numberWithInt:0];
    range.endLine = [NSNumber numberWithInt:0];
    range.startColumn = [NSNumber numberWithInt:0];
    range.endColumn = [NSNumber numberWithInt:0];
    inlineStyle.range = range;
    callback(inlineStyle,nil,nil);
}

- (void)domain:(WXCSSDomain *)domain getComputedStyleForNodeWithNodeId:(NSNumber *)nodeId callback:(void (^)(NSArray *computedStyle, id error))callback {
    /*
    WXDOMNode *rootDomNode = [WXDOMDomainController defaultInstance].rootDomNode;
    WXDOMNode *node = [self p_getNodeFromNodeId:nodeId rootNode:rootDomNode];
    
    __block NSArray *position;
    [node.attributes enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        if ([obj isEqualToString:@"frame"]) {
            *stop = YES;
            position = [self p_formateFrame:node.attributes[idx + 1]];
        }
    }];
     */
    
    UIView *selectedView = [[[WXDOMDomainController defaultInstance] getObjectsForComponentRefs] objectForKey:[NSString stringWithFormat:@"%ld",nodeId.integerValue]];
    NSArray *actualAttrs = [[WXDOMDomainController defaultInstance] attributesArrayForObject:selectedView];
    __block NSArray *position;
    [actualAttrs enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        if ([obj isEqualToString:@"frame"]) {
            *stop = YES;
            position = [self p_formateFrame:actualAttrs[idx + 1]];
        }
    }];
    
    NSString *width = @"";
    NSString *height = @"";
    NSString *top = @"";
    NSString *left = @"";
    if (position.count == 4) {
        width = position[2];
        height = position[3];
        top = position[1];
        left = position[0];
        width = [NSString stringWithFormat:@"%lf",[width floatValue]/WXScreenResizeRadio()];
        height = [NSString stringWithFormat:@"%lf",[height floatValue]/WXScreenResizeRadio()];
    }
    NSMutableArray *computedStyles = [[NSMutableArray alloc] init];
    NSArray *layout = @[@{@"name":@"width",@"value":width},
                        @{@"name":@"height",@"value":height},
                        @{@"name":@"padding-top",@"value":@"0px"},
                        @{@"name":@"padding-left",@"value":@"0px"},
                        @{@"name":@"padding-right",@"value":@"0px"},
                        @{@"name":@"padding-bottom",@"value":@""},
                        @{@"name":@"border-top-width",@"value":@""},
                        @{@"name":@"border-left-width",@"value":@""},
                        @{@"name":@"border-right-width",@"value":@""},
                        @{@"name":@"border-bottom-width",@"value":@""},
                        @{@"name":@"margin-bottom",@"value":@"0px"},
                        @{@"name":@"margin-left",@"value":@"0px"},
                        @{@"name":@"margin-right",@"value":@"0px"},
                        @{@"name":@"margin-top",@"value":@"0px"},
                        @{@"name":@"top",@"value":top},
                        @{@"name":@"right",@"value":@"0px"},
                        @{@"name":@"left",@"value":left},
                        @{@"name":@"bottom",@"value":@"0px"}];
    for (int i = 0; i < layout.count; i++) {
        WXCSSCSSComputedStyleProperty *computedStyleProperty = [[WXCSSCSSComputedStyleProperty alloc] init];
        computedStyleProperty.name = layout[i][@"name"];
        computedStyleProperty.value = layout[i][@"value"];
        [computedStyles addObject:[computedStyleProperty WX_JSONObject]];
    }
    
    callback(computedStyles,nil);
}

- (void)domain:(WXCSSDomain *)domain getSupportedCSSPropertiesWithCallback:(void (^)(NSArray *cssProperties, id error))callback {
    WXCSSCSSPropertyInfo *cssPropertyInfo = [[WXCSSCSSPropertyInfo alloc] init];
    cssPropertyInfo.name = @"width";
    cssPropertyInfo.longhands = @[];
    NSArray *cssProperties = @[cssPropertyInfo];
    callback(cssProperties,nil);
}


@end
