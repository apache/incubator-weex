//
//  WXObject.h
//  PonyDebugger
//
//  Created by Mike Lewis on 11/6/11.
//
//  Licensed to Square, Inc. under one or more contributor license agreements.
//  See the LICENSE file distributed with this work for the terms under
//  which Square, Inc. licenses this file to you.
//

#import <Foundation/Foundation.h>
#import "WXDefinitions.h"


@interface NSObject (WX_JSONObject)

- (id)WX_JSONObjectCopy;
- (id)WX_JSONObject;

@end


/**
 * Subclasses must implement the copying and mutable copying protocols
 */
@interface WXObject : NSObject <NSCopying>

@property (nonatomic, copy, readonly) NSDictionary *store;

+ (NSDictionary *)keysToEncode;

- (id)WX_JSONObject;

- (BOOL)validate:(NSError **)error;
- (id)valueForKey:(NSString *)key;
- (void)setValue:(id)value forKey:(NSString *)key;

@end


