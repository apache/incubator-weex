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

#include "JsonRenderManager.h"

namespace WeexCore{

    JsonRenderManager* JsonRenderManager::g_instance = nullptr;

    JsonRenderManager::JsonRenderManager() {

    }


    void JsonRenderManager::CreatePage(const std::string &script, const std::string &instanceId,
                                   const std::string &render_strategy) {
        JsonPage* replayPage = new JsonPage(instanceId);
        replayPage->parseJson(script);
        mPages[instanceId] = replayPage;
    }


    bool JsonRenderManager::RefreshPage(const std::string &page_id, const std::string &init_data) {
        if(mPages.size() == 0){
            return false;
        }
        auto it = mPages.find(page_id);
        if(it == mPages.end()){
            return false;
        }
        return true;
    }

    bool JsonRenderManager::ClosePage(const std::string &page_id) {
        if(mPages.size() == 0){
            return false;
        }
        auto it = mPages.find(page_id);
        if(it == mPages.end()){
            return false;
        }
        delete  it->second;
        mPages.erase(page_id);
        return true;
    }



}