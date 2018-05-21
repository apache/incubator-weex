//
// Created by 陈佩翰 on 2018/5/21.
//

#include "wx_core_manager.h"


void WeexCore::WXCoreManager::init(Bridge *platformBridge, JSBridge *jsBridge) {
    s_instance = new WXCoreManager();
}

void WeexCore::WXCoreManager::destroy() {
    delete s_instance;
}

WeexCore::WXCoreManager *WeexCore::WXCoreManager::getInstance() {
    return s_instance;
}

WeexCore::JSBridge *WeexCore::WXCoreManager::getJSBridge() {
    return s_instance->js_bridge;
}

WeexCore::Bridge *WeexCore::WXCoreManager::getPlatformBridge() {
    return s_instance->platform_bridge;
}