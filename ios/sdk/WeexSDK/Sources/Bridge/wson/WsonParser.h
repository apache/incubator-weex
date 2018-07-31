//
//  WsonParser.h
//  WsonParser
//
//  Created by furture on 2018/5/30.
//  Copyright © 2018年 furture. All rights reserved.
//

#import <Foundation/Foundation.h>
#include "wson.h"

@interface WsonParser : NSObject

+(wson_buffer*) toWson:(id)val;

+(id) toVal:(wson_buffer*)buffer;


@end
