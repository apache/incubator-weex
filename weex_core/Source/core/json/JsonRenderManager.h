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
// Created by furture on 2019/4/29.
//

#ifndef WEEX_PROJECT_REPLAYMANAGER_H
#define WEEX_PROJECT_REPLAYMANAGER_H
#include "JsonPage.h"
#include <map>
#include <string>

namespace WeexCore{

    class JsonRenderManager {

        public:
            static JsonRenderManager *GetInstance() {
                if (!g_instance) {
                    g_instance = new JsonRenderManager();
                }
                return g_instance;
            }

        public:
           void CreatePage(const std::string& script, const std::string& instanceId, const std::string& render_strategy);
           bool RefreshPage(const std::string &page_id, const std::string &init_data);
           bool ClosePage(const std::string &page_id);

        private:
            JsonRenderManager();

        private:
            static  JsonRenderManager *g_instance;
            std::map<std::string, JsonPage*> mPages;

    };
}



#endif //WEEX_PROJECT_REPLAYMANAGER_H
