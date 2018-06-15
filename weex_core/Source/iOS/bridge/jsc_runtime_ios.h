#ifndef jsc_runtime_ios_hpp
#define jsc_runtime_ios_hpp

#include <core/js_engin/base_js_runtime.h>

namespace WeexCore {
    class JSCRunTimeIOS: public BaseJSRunTime{
    public:
        ~JSCRunTimeIOS();
        
    public:
        bool initRunTime();
        
        void destroy();
        
        BaseJSContext *createContext();
        
        void destroyContext(BaseJSContext *context);
        
        uint32_t getRunTimeId();
    };
}

#endif /* jsc_runtime_ios_hpp */
