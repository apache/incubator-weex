//
//  measure_func_adapter_impl_ios.cpp
//  Pods-WeexDemo
//
//  Created by 陈佩翰 on 2018/6/7.
//

#include "measure_func_adapter_impl_ios.h"

namespace WeexCore {
    void MeasureFunctionAdapterImplIOS::LayoutBefore(WeexCore::WXCoreLayoutNode *node){
        
    }
    
    void MeasureFunctionAdapterImplIOS::LayoutAfter(WeexCore::WXCoreLayoutNode *node, float width, float height){
        
    }
    WXCoreSize MeasureFunctionAdapterImplIOS::Measure(WXCoreLayoutNode *node, float width,MeasureMode widthMeasureMode,
                                                      float height,MeasureMode heightMeasureMode) {
        WXCoreSize size;
        size.width=0;
        size.height=0;
        return size;
    }
}
