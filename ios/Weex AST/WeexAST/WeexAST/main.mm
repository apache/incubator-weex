//
//  main.m
//  WeexAST
//
//  Created by Warmness on 12/7/2018.
//  Copyright © 2018 Warmness. All rights reserved.
//

#import <Foundation/Foundation.h>
#include "core/data_render/json/json11.hpp"
#include "core/data_render/parser.h"

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        // insert code here...
        char pstr_alinn_weight[1024] = { 0 };
        const char *pstr_current_path = strstr(argv[0], "Debug/");
        ((char *)pstr_current_path + strlen("Debug/"))[0] = '\0';
        sprintf(pstr_alinn_weight, "%s%s", argv[0], "normal_layout.json");
        NSString *content = [NSString stringWithContentsOfFile:[NSString stringWithUTF8String:pstr_alinn_weight] encoding:NSUTF8StringEncoding error:nil];
        std::string error;
        weex::core::data_render::Parser::parse(content.UTF8String, error);

        //std::string
        NSLog(@"Hello, World!");
    }
    return 0;
}
