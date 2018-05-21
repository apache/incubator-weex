//
// Created by 陈佩翰 on 2018/5/21.
//

#ifndef WEEXV8_JSBRIDGE_H
#define WEEXV8_JSBRIDGE_H


#include "wx_type_define.h"
#include <map>


namespace WeexCore {

    //typedef void(*JSExceptionHandler)(int32_t runTimeId,int32_t contextId,char* exception,std::map<char*,char*> extInfos);


    class JSBridge{

    protected:
        JSBridge(){}
    public:
        virtual bool createJSRunTime        (uint32_t runTimeId, std::map<std::string,std::string> &params)=0;
        virtual bool destroyJSRunTime       (uint32_t runTimeId)=0;

        virtual bool createJSContext        (uint32_t runTimeId,uint32_t contextId) =0;
        virtual bool destoryJSContext       (uint32_t runTimeId,uint32_t contextId) =0;

        virtual WXValue execJSMethod            (uint32_t runTimeId, uint32_t contextId,char *methodName,WXValue args[],uint8_t argsLength)=0;
        virtual WXValue executeJavascript   (uint32_t runTimeId, uint32_t contextId,char *script)=0;

        virtual void    reigsterJSVale          (uint32_t runTimeId,uint32_t contextId,char* name,WXValue value)=0;
        virtual WXValue getJSVale               (uint32_t runTimeId,uint32_t contextId,char* name)=0;
        virtual void    reigsterJSFunc          (uint32_t runTimeId,uint32_t contextId,WXFuncSignature func)=0;

        WXValue callExecNative          (uint32_t runTimeId, uint32_t contextId,WXValue args[],uint8_t argsLength);
        void    callReportJSException   (int32_t runTimeId,int32_t contextId,char* exception,std::map<std::string,std::string> &extInfos);
        void    callSetJSVersion        (const char *jsVersion);
    };
}

#endif //WEEXV8_JSBRIDGE_H
