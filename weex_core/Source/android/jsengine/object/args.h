/**
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
//
// Created by jianbai.gbj on 15/05/2018.
//

#ifdef USE_JS_RUNTIME
#include "js_runtime/weex/object/args.h"
#else

#ifndef ARGS_H
#define ARGS_H

#include <string>
#include <wtf/text/WTFString.h>
#include "wson/wson.h"

/**
 * auto conver for json and wson
 */
namespace WeexCore { 
    static const int ARGS_TYPE_JSON = 0;
    static const int ARGS_TYPE_WSON = 1;
    class Args{
        public:
            Args();
            ~Args();

            int getType(){
                return type;
            };

            void setString(String& string){
                this->json = string;
                this->type = ARGS_TYPE_JSON;
                // if string is not utf8, we convert it
                utf8.assign(json.utf8().data());
            }
            /**object will auto free when args destructor */
            void setWson(wson_buffer* buffer){
                this->wson = buffer;
                this->type = ARGS_TYPE_WSON;
            }

            inline const char* getValue() const {
                if(type == ARGS_TYPE_WSON){
                    if(wson){
                       return (char*)(wson->data);
                    }
                    return nullptr;
                }else{
                    return utf8.c_str();
                }
            }

            inline int getLength() const {
                if(type == ARGS_TYPE_WSON){
                    if(wson){
                       return (wson->position);
                    }
                    return 0;
                }else{
                    return strlen(json.utf8().data());
                }
            }

        private:
            int type; 
        public:
            String json;
            std::string utf8;
            wson_buffer* wson = nullptr;
     };
};

#endif
#endif //USE_JS_RUNTIME
