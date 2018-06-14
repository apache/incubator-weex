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


#ifndef WEEXCORE_JS_BRIDGE_IMPL_IOS_H
#define WEEXCORE_JS_BRIDGE_IMPL_IOS_H


#include <core/bridge/js_bridge.h>


namespace WeexCore {
    
    class JSBridgeIOS : public JSBridge {
    public:
      //  JSBridgeIOS();
        ~JSBridgeIOS();
    public:
        
        /** new api  **/

        bool createJSRunTime(uint32_t runTimeId, std::map<std::string, std::string> *params);

        void destroyJSRunTime(uint32_t runTimeId);

        bool createJSContext(uint32_t runTimeId, uint32_t contextId) ;

        void destroyJSContext(uint32_t runTimeId, uint32_t contextId) ;

        void execJSMethod(uint32_t runTimeId, uint32_t contextId, char *methodName,wson_buffer* args);
        
        wson_buffer* execJSMethodWithResult(uint32_t runTimeId, uint32_t contextId, char *methodName, wson_buffer* args);
        
        bool executeJavascript(uint32_t runTimeId, uint32_t contextId, char *script);
        
        void reigsterJSVale(uint32_t runTimeId, uint32_t contextId, char *name, wson_buffer* valuse);
        
        wson_buffer* getJSVale(uint32_t runTimeId, uint32_t contextId, char *name);
        
        void reigsterJSFunc(uint32_t runTimeId, uint32_t contextId, wson_buffer* func);
    };
}


#endif /* WEEXCORE_JS_BRIDGE_IMPL_IOS_H */
