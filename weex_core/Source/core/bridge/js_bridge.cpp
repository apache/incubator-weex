//
// Created by 陈佩翰 on 2018/5/21.
//

#include "js_bridge.h"


WeexCore::WXValue WeexCore::JSBridge::callExecNative(uint32_t runTimeId, uint32_t contextId, WXValue *args,
                                                     uint8_t argsLength) {

    //

}


void WeexCore::JSBridge::callReportJSException(int32_t runTimeId, int32_t contextId, char *exception,
                                               std::map<std::string, std::string> &extInfos) {

}

void WeexCore::JSBridge::callSetJSVersion(const char *jsVersion) {

}