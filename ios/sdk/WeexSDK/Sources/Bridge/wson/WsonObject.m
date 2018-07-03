//
//  WsonObject.m
//  Pods-WeexDemo
//
//  Created by shenmo on 2018/7/3.
//

#import "WsonObject.h"

@implementation WsonObject

+ (instancetype)fromObject:(id)object
{
    return [[WsonObject alloc] initWithObject:object];
}

- (instancetype)initWithObject:(id)object
{
    if (object == nil) {
        return nil;
    }
    
    if (self = [super init]) {
        @try {
            _buffer = [WsonParser toWson:object];
        }
        @catch (NSException* exception) {
        }
    }
    
    return self;
}

- (void)dealloc
{
    if (_buffer) {
        wson_buffer_free(_buffer);
    }
}

- (const char*)data
{
    if (_buffer != NULL) {
        return (const char*)(_buffer->data);
    }
    return NULL;
}

@end
