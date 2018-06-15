#ifndef jsc_context_ios_h
#define jsc_context_ios_h

#include <core/js_engin/base_js_context.h>
#include "jsc_runtime_ios.h"

namespace WeexCore {
    class JSCContextIOS: public BaseJSContext{
    protected:
        uint32_t contextId;
    public:
        ~JSCContextIOS();
    public:
    
        void onInit(uint32_t contextId,void* impl, JSCRunTimeIOS* runTime);
        
    public:
        void execJSMethod(char *methodName, wson_buffer *args);
        
        wson_buffer *execJSMethodWithResult(char *methodName, wson_buffer *args);
        
        bool executeJavascript(char *script);
        
        void reigsterJSVale(char *name, wson_buffer *valuse);
        
        wson_buffer *getJSVale(char *name);
        
        void reigsterJSFunc(wson_buffer *func);
        
        uint32_t getContextId();
        
        BaseJSRunTime* getJsRunTime();

    };
}


#endif /* jsc_context_ios_h */
