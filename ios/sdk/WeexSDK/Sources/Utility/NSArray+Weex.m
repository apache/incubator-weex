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

#import "NSArray+Weex.h"

@implementation NSArray (Weex)

- (id)wx_safeObjectAtIndex:(NSUInteger)index
{
    if (index >= self.count) {
        return nil;
    }
    
    return [self objectAtIndex:index];
}

@end

@implementation NSMutableArray (Weex)

// It's quite ingenius, using a Category to allow the creation of a mutable array that does no retain/release by backing it with a CFArray with proper callbacks.http://stackoverflow.com/questions/4692161/non-retaining-array-for-delegates
+ (id)wx_mutableArrayUsingWeakReferences {
    return [self wx_mutableArrayUsingWeakReferencesWithCapacity:0];
}

+ (id)wx_mutableArrayUsingWeakReferencesWithCapacity:(NSUInteger)capacity {
    CFArrayCallBacks callbacks = {0, NULL, NULL, CFCopyDescription, CFEqual};
    // We create a weak reference array
    return (__bridge id)(CFArrayCreateMutable(0, capacity, &callbacks));
}

+ (void)wx_releaseArray:(id)array {
    
    CFBridgingRelease((__bridge CFArrayRef _Nullable)(array));
}

@end
