package com.taobao.weex.render.frame;

import android.view.MotionEvent;

public class TouchHelper {


    //see agil MotionEvent Enum
    public static int converActionMaskToRender(int actionMask){
        int enumAction = -1;
        if(actionMask == MotionEvent.ACTION_DOWN || actionMask == MotionEvent.ACTION_POINTER_DOWN){
            enumAction = 0;
        }else if(actionMask == MotionEvent.ACTION_CANCEL
                || actionMask == MotionEvent.ACTION_UP
                || actionMask == MotionEvent.ACTION_POINTER_UP){
            enumAction = 1;
        }else if(actionMask == MotionEvent.ACTION_MOVE || actionMask == MotionEvent.ACTION_HOVER_MOVE){
            enumAction = 2;
        }
        return enumAction;
    }


    public static int convertRenderToNative(int actionRender){
        if(actionRender == 0){
            return  MotionEvent.ACTION_DOWN;
        }else if(actionRender == 1){
            return  MotionEvent.ACTION_UP;
        }if(actionRender == 2){
            return  MotionEvent.ACTION_MOVE;
        }
        return  MotionEvent.ACTION_CANCEL;
    }
}
