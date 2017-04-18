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

#import "WXConfigDefaultImpl.h"
#import "WXThreadSafeMutableDictionary.h"
#import "WXUtility.h"

@interface WXConfigDefaultImpl()
@property(nonatomic, strong) WXThreadSafeMutableDictionary* configDic;
@end
@implementation WXConfigDefaultImpl

- (instancetype)init
{
    if (self = [super init]) {
        _configDic = [WXThreadSafeMutableDictionary new];
    }
    
    return self;
}

- (void)setClassWithName:(Class)className name:(NSString *)name
{
    if (![WXUtility isBlankString:name] && className) {
        [_configDic setObject:className forKey:name];
    }
}

- (Class)classWithName:(NSString *)name
{
    if (![WXUtility isBlankString:name]) {
        return [_configDic objectForKey:name];
    }
    
    return nil;
}

@end
