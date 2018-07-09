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

#import "WXScrollerComponent+Layout.h"
#import "WXLog.h"

@implementation WXScrollerComponent (FlexLayout)

- (WeexCore::WXCoreLayoutNode *)flexScrollerCSSNode
{
#ifdef WX_IMPORT_WEEXCORE
    return nullptr;
#else
    return _flexScrollerCSSNode;
#endif
}

- (void)_insertChildCssNode:(WXComponent *)subcomponent atIndex:(NSInteger)index
{
#ifdef WX_IMPORT_WEEXCORE
#else
    self.flexScrollerCSSNode->addChildAt(subcomponent.flexCssNode, (uint32_t) index);
#ifdef DEBUG
    WXLogDebug(@"flexLayout -> ref:%@,subNodeIndex:%ld,childCount:%ld",self.ref,(long)index,_flexScrollerCSSNode->getChildCount());
#endif
#endif
}

- (void)_rmChildCssNode:(WXComponent *)subcomponent
{
#ifdef WX_IMPORT_WEEXCORE
#else
    self.flexScrollerCSSNode->removeChild(subcomponent->_flexCssNode);
#ifdef DEBUG
    WXLogDebug(@"flexLayout -> ref:%@ ,scrollerCSSNode->removeChild ,childRef:%@",self.ref,subcomponent.ref);
#endif
#endif
}

@end
