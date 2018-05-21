//
// Created by 陈佩翰 on 2018/5/21.
//

#ifndef WEEXV8_WX_CORE_MANAGER_H
#define WEEXV8_WX_CORE_MANAGER_H


#include <core/bridge/bridge.h>
#include <core/bridge/js_bridge.h>


namespace WeexCore {
    class WXCoreManager {
    protected:
        WXCoreManager() {};

        ~WXCoreManager() {};
    public:
        void init(Bridge *platformBridge, JSBridge *jsBridge);

        void destroy();

        WXCoreManager *getInstance();

        Bridge *getPlatformBridge();

        JSBridge *getJSBridge();


    private:
        static WXCoreManager *s_instance;
        Bridge *platform_bridge;
        JSBridge *js_bridge;
        //todo core_bridge
    };
}


#endif //WEEXV8_WX_CORE_MANAGER_H
