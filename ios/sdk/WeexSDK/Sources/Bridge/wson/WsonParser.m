//
//  WsonParser.m
//  WsonParser
//
//  Created by furture on 2018/5/30.
//  Copyright © 2018年 furture. All rights reserved.
//

#import "WsonParser.h"
#include <objc/runtime.h>
#include <inttypes.h>
#include <CoreGraphics/CoreGraphics.h>
#import <UIKit/UIKit.h>

@implementation WsonParser


+(wson_buffer*) toWson:(id)val{
    wson_buffer* buffer =  wson_buffer_new();
    [WsonParser toWson:val buffer:buffer];
    return buffer;
}

+(id) toVal:(wson_buffer*)buffer{
    if(buffer->position > 0){
        buffer->position = 0;
    }
    return [WsonParser toObj:buffer];
}


+(id) toObj:(wson_buffer*)buffer{
     uint8_t  type = wson_next_type(buffer);
    switch (type) {
        case WSON_STRING_TYPE:
        case WSON_NUMBER_BIG_INT_TYPE:
        case WSON_NUMBER_BIG_DECIMAL_TYPE:{
                uint32_t length = wson_next_uint(buffer);
                unichar* charSrc = (unichar*)wson_next_bts(buffer, length);
                return [NSString stringWithCharacters:charSrc length:length/sizeof(unichar)];
            }
            break;
        case WSON_ARRAY_TYPE:{
            uint32_t length = wson_next_uint(buffer);
            NSMutableArray* array = [NSMutableArray arrayWithCapacity:length];
            for(uint32_t i=0; i<length; i++){
                if(wson_has_next(buffer)){
                    [array addObject:[WsonParser toObj:buffer]];
                }else{
                    break;
                }
            }
            return array;
        }
            break;
        case WSON_MAP_TYPE:{
             uint32_t mapLength = wson_next_uint(buffer);
             NSMutableDictionary* dic = [NSMutableDictionary dictionaryWithCapacity:mapLength];
             for(int i=0; i<mapLength; i++){
                uint32_t strLength = wson_next_uint(buffer);
                unichar* charSrc = (unichar*)wson_next_bts(buffer, strLength);
                NSString* key= [NSString stringWithCharacters:charSrc length:strLength/sizeof(unichar)];
                [dic setObject:[WsonParser toObj:buffer] forKey:key];
             }
            return dic;
            }
            break;
        case WSON_NUMBER_INT_TYPE:{
            int32_t  num =  wson_next_int(buffer);
            return [NSNumber numberWithInt:num];
            }
            break;
        case WSON_BOOLEAN_TYPE_TRUE:{
             return [NSNumber numberWithBool:true];
            }
            break;
        case WSON_BOOLEAN_TYPE_FALSE:{
             return [NSNumber numberWithBool:false];
            }
            break;
        case WSON_NUMBER_DOUBLE_TYPE:{
            double  num = wson_next_double(buffer);
            return [NSNumber numberWithDouble:num];
            }
            break;
        case WSON_NUMBER_FLOAT_TYPE:{
            float  num = wson_next_float(buffer);
           return [NSNumber numberWithFloat:num];
        }
            break;
        case WSON_NUMBER_LONG_TYPE:{
            int64_t  num = wson_next_long(buffer);
            return [NSNumber numberWithLongLong:num];
        }
            break;
        case WSON_NULL_TYPE:{
            return [NSNull null];
           }
            break;
        default:
            NSLog(@"weex weex wson err  wson_to_js_value  unhandled type %d buffer position  %d length %d", type, buffer->position, buffer->length);
            break;
    }
    return nil;
}



+(void) wson_push_key_string:(NSString*) val buffer:(wson_buffer*)buffer{
    NSString* string = [val description];
    NSUInteger length =  string.length;
    wson_push_uint(buffer, (uint32_t)length*sizeof(unichar));
    wson_buffer_require(buffer, length*sizeof(unichar));\
    unichar* charBuffer = (unichar*)((uint8_t*)buffer->data + buffer->position);
    [string getCharacters:charBuffer];\
    buffer->position +=length*sizeof(unichar);
}


+(void) toWson:(id)val buffer:(wson_buffer*)buffer{
    if(val == nil){
        wson_push_type_null(buffer);
        return;
    }
    if([val isKindOfClass:[NSString class]]){
        NSString* string = val;
        NSUInteger length =  string.length;
        wson_push_type(buffer, WSON_STRING_TYPE);
        wson_push_uint(buffer, (uint32_t)length*sizeof(unichar));
        wson_buffer_require(buffer, length*sizeof(unichar));
        unichar* charBuffer = (unichar*)((uint8_t*)buffer->data + buffer->position);
        [string getCharacters:charBuffer];
        buffer->position +=length*sizeof(unichar);
        return;
    }else if([val isKindOfClass:[NSNumber class]]){
        NSNumber* number = val;
        const char* type = [number objCType];
        switch (type[0]) {
            case _C_DBL:
                wson_push_type_double(buffer, [number doubleValue]);
                return;
            case _C_FLT:
                wson_push_type_float(buffer, [number floatValue]);
                return;
            case _C_INT:
            case _C_UINT:
            case _C_CHR:
            case _C_UCHR:
            case _C_SHT:
            case _C_USHT:
                wson_push_type_int(buffer, [number intValue]);
                return;
            case _C_LNG:
            case _C_ULNG:
            case _C_ULNG_LNG:
            case _C_LNG_LNG:
                 wson_push_type_long(buffer, (int64_t)[number longLongValue]);
                 return;
            case _C_BFLD:
            case _C_BOOL:{
                    if([number boolValue]){
                         wson_push_type_boolean(buffer, 1);
                    }else{
                        wson_push_type_boolean(buffer, 0);
                    }
                    return;
                }
                return;
            default:
                break;
        }
        wson_push_type_long(buffer, (int64_t)[number longLongValue]);
        return;
    }else if([val isKindOfClass:[NSDictionary class]]){
        NSDictionary* dic = val;
        wson_push_type_map(buffer, (uint32_t)[dic count]);
        for(id key in dic){
            [WsonParser wson_push_key_string:key buffer:buffer];
            [WsonParser toWson:[dic objectForKey:key] buffer:buffer];
        }
        return;
    }else if([val isKindOfClass:[NSArray class]]){
        NSArray* array = val;
        wson_push_type_array(buffer, (uint32_t)[array count]);
        for(id ele in array){
            [WsonParser toWson:ele buffer:buffer];
        }
        return;
    }else if([val isKindOfClass:[NSDate class]]){
        NSDate* date = val;
        NSTimeInterval interval = [date timeIntervalSince1970];
        wson_push_type_long(buffer, interval*1000);
    }else if([val isKindOfClass:[NSValue class]]){
        NSValue* value = val;
        const char* type = [value objCType];
        switch (type[0]) {
            case _C_STRUCT_B:{
                type++;
                if(strncmp(type, "CGRect", 6) == 0){
                    CGRect rect = [value CGRectValue];
                    wson_push_type_map(buffer, 4);
                    [WsonParser wson_push_key_string:@"x" buffer:buffer];
                    wson_push_type_float(buffer, rect.origin.x);
                    [WsonParser wson_push_key_string:@"y"buffer:buffer];
                    wson_push_type_float(buffer, rect.origin.y);
                    [WsonParser wson_push_key_string:@"width"buffer:buffer];
                    wson_push_type_float(buffer, rect.size.width);
                    [WsonParser wson_push_key_string:@"height"buffer:buffer];
                    wson_push_type_float(buffer, rect.size.height);
                    return;
                }else if(strncmp(type, "CGPoint", 7) == 0){
                    CGPoint point = [value CGPointValue];
                    wson_push_type_map(buffer, 2);
                    [WsonParser wson_push_key_string:@"width"buffer:buffer];
                    wson_push_type_float(buffer, point.x);
                    [WsonParser wson_push_key_string:@"y"buffer:buffer];
                    wson_push_type_float(buffer, point.y);
                    return;
                }else if(strncmp(type, "CGSize", 6) == 0){
                    CGSize size = [value CGSizeValue];
                    wson_push_type_map(buffer, 2);
                    [WsonParser wson_push_key_string:@"width"buffer:buffer];
                    wson_push_type_float(buffer, size.width);
                    [WsonParser wson_push_key_string:@"height"buffer:buffer];
                    wson_push_type_float(buffer, size.height);
                    return;
                }else if(strncmp(type, "NSRange", 7) == 0){
                    NSRange range = [value rangeValue];
                    wson_push_type_map(buffer, 2);
                    [WsonParser wson_push_key_string:@"location" buffer:buffer];
                    wson_push_type_float(buffer, range.location);
                    [WsonParser wson_push_key_string:@"length" buffer:buffer];
                    wson_push_type_float(buffer,  range.length);
                    return;
                }else if(strncmp(type, "CGVector", 8) == 0){
                    CGVector result = [val CGVectorValue];
                    wson_push_type_map(buffer, 2);
                    [WsonParser wson_push_key_string:@"dx" buffer:buffer];
                    wson_push_type_float(buffer, result.dx);
                    [WsonParser wson_push_key_string:@"dy" buffer:buffer];
                    wson_push_type_float(buffer,  result.dy);
                }else if(strncmp(type, "UIEdgeInsets", 12) == 0){
                     UIEdgeInsets edge = [value UIEdgeInsetsValue];
                     wson_push_type_map(buffer, 4);
                     [WsonParser wson_push_key_string:@"left" buffer:buffer];
                     wson_push_type_float(buffer,  edge.left);
                     [WsonParser wson_push_key_string:@"top" buffer:buffer];
                     wson_push_type_float(buffer, edge.top);
                     [WsonParser wson_push_key_string:@"bottom" buffer:buffer];
                     wson_push_type_float(buffer,  edge.bottom);
                     [WsonParser wson_push_key_string:@"right" buffer:buffer];
                     wson_push_type_float(buffer,  edge.right);
                     return;
                }else{
                   wson_push_type_null(buffer);
                   return;
                }
            }
        }
        wson_push_type_null(buffer);
        return;
    }else{
        if([NSNull null] == val){
             wson_push_type_null(buffer);
        }else{
            [WsonParser toWson:[val description] buffer:buffer];
        }
    }
}
@end
