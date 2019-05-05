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

#ifndef WEEX_PROJECT_REPLAYPAGE_H
#define WEEX_PROJECT_REPLAYPAGE_H
#include "third_party/json11/json11.hpp"
#include <map>
#include <string>
#include "core/render/page/render_page_custom.h"

namespace WeexCore {

    /**
     * Parse Json and send command to render manager.
     * for qucik replay online state
     * */
    class JsonPage {

        public:
            JsonPage(const std::string& pageId);

        public:
            void parseJson(const std::string &data);

         private:
            void sendChildren(const json11::Json node, const std::string &parent_ref, int index, bool body=false);

        protected:
            std::string mPageId;
            RenderPageCustom* mRenderPageCustom;

    };
}

#endif //WEEX_PROJECT_REPLAYPAGE_H
