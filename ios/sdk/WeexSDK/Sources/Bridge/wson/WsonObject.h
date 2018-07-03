//
//  WsonObject.h
//  Pods-WeexDemo
//
//  Created by shenmo on 2018/7/3.
//

#import <Foundation/Foundation.h>
#import "WsonParser.h"

@interface WsonObject : NSObject

@property (nonatomic, readonly, assign) wson_buffer* buffer;

+ (instancetype)fromObject:(id)object;

- (const char*)data;

@end
