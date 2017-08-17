#ifndef _WEEX_JSC_H_
#define _WEEX_JSC_H_

namespace WEEXJSC {
extern "C" void soLoad(JavaVM *vm,
                       jclass *jBridgeClazz,
                       jclass *jWXJSObject,
                       jclass *jWXLogUtils);

}

#endif //_WEEX_JSC_H_