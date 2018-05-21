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
// Created by 陈佩翰 on 2018/5/21.
//

#include "wx_core_manager.h"


void WeexCore::WXCoreManager::init(Bridge *platformBridge, JSBridge *jsBridge) {
    getInstance()->platform_bridge = platformBridge;
    getInstance()->js_bridge = jsBridge;
}

void WeexCore::WXCoreManager::destroy() {
    delete s_instance;
}

WeexCore::WXCoreManager *WeexCore::WXCoreManager::getInstance() {
    if (nullptr == s_instance) {
        s_instance = new WXCoreManager();
    }
    return s_instance;
}

WeexCore::JSBridge *WeexCore::WXCoreManager::getJSBridge() {
    return s_instance->js_bridge;
}

WeexCore::Bridge *WeexCore::WXCoreManager::getPlatformBridge() {
    return platform_bridge;
}