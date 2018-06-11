#include "base_js_context.h"
#include <core/manager/weex_core_manager.h>


namespace WeexCore {
    bool BaseJSContext::onInit() {
        return WeexCoreManager::getInstance()->getJSBridge()->createJSContext(
                this->jsRunTime->runTimeId,
                this->contextId
        );
    }

    void BaseJSContext::onDestroy() {
        WeexCoreManager::getInstance()->getJSBridge()->destroyJSContext(
                this->jsRunTime->runTimeId,
                this->contextId
        );
    }

    void BaseJSContext::execJSMethod(char *methodName, wson_buffer *args) {
        WeexCoreManager::getInstance()->getJSBridge()->execJSMethod(
                this->jsRunTime->runTimeId,
                this->contextId,
                methodName,
                args
        );
    }

    wson_buffer *BaseJSContext::execJSMethodWithResult(char *methodName, wson_buffer *args) {
        return WeexCoreManager::getInstance()->getJSBridge()->execJSMethodWithResult(
                this->jsRunTime->runTimeId,
                this->contextId,
                methodName,
                args
        );
    }

    bool BaseJSContext::executeJavascript(char *script) {
        return WeexCoreManager::getInstance()->getJSBridge()->executeJavascript(
                this->jsRunTime->runTimeId,
                this->contextId,
                script
        );
    }

    void BaseJSContext::reigsterJSVale(char *name, wson_buffer *value) {
        WeexCoreManager::getInstance()->getJSBridge()->reigsterJSVale(
                this->jsRunTime->runTimeId,
                this->contextId,
                name,
                value
        );
    }

    wson_buffer *BaseJSContext::getJSVale(char *name) {
        return WeexCoreManager::getInstance()->getJSBridge()->getJSVale(
                this->jsRunTime->runTimeId,
                this->contextId,
                name
        );

    }

    void BaseJSContext::reigsterJSFunc(wson_buffer *func) {
        WeexCoreManager::getInstance()->getJSBridge()->reigsterJSFunc(
                this->jsRunTime->runTimeId,
                this->contextId,
                func
        );
    }
}