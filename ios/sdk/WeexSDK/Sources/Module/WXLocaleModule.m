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

#import "WXLocaleModule.h"

@implementation WXLocaleModule

WX_EXPORT_METHOD_SYNC(@selector(getLanguage:))
WX_EXPORT_METHOD_SYNC(@selector(getLanguages))

/**
 Get preferred language of the user
 */
- (NSString *)getLanguage:(WXKeepAliveCallback)callback {
    if (callback) {
        callback([NSLocale preferredLanguages][0], NO);
    }
	return [NSLocale preferredLanguages][0];
}

/**
 Get an array of strings representing the user's preferred languages
 */
- (NSArray<NSString *> *)getLanguages {
	return [NSLocale preferredLanguages];
}

@end
