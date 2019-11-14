/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
package org.apache.weex.ui.component.helper;

import android.os.Handler;
import android.os.Looper;

import org.apache.weex.common.Constants;
import org.apache.weex.common.OnWXScrollListener;
import org.apache.weex.ui.component.WXComponent;
import org.apache.weex.ui.component.WXScroller;
import org.apache.weex.ui.component.list.BasicListComponent;
import org.apache.weex.ui.component.list.ListComponentView;
import org.apache.weex.ui.component.list.template.WXRecyclerTemplateList;
import org.apache.weex.utils.WXUtils;

import java.util.Map;


/**
 * Created by furture on 2017/11/13.
 */

public class ScrollStartEndHelper implements Runnable{

    private Handler handler;
    private WXComponent component;
    private boolean  hasStart;
    private boolean  canStart = false;

    private long  minInterval;
    private int oldState = OnWXScrollListener.IDLE;

    private int x;
    private int y;
    private boolean hasScrollEnd;


    public ScrollStartEndHelper(WXComponent component) {
        this.component = component;
        this.handler = new Handler(Looper.getMainLooper());
        this.minInterval = WXUtils.getNumberInt(component.getAttrs().get("minscrolldelayinterval"), 32);
    }

    /**
     * @param  x scroll offset or dx, which is not accurate
     * @param  y scroll offset or dy, which is not accurate
     * */
    public void  onScrolled(int x, int y){
        if((component.getEvents().contains(Constants.Event.SCROLL_START)
                || component.getEvents().contains(Constants.Event.SCROLL_END))){
            this.x = x;
            this.y = y;
            if(!hasStart && canStart){
                if(component.getEvents().contains(Constants.Event.SCROLL_START)){
                    Map<String, Object> event = getScrollEvent(x,y);
                    if (null !=event && !event.isEmpty()){
                        component.fireEvent(Constants.Event.SCROLL_START,event);

                    }
                }
                hasStart = true;
                canStart = false;
            }
            handler.removeCallbacks(this);
            handler.postDelayed(this, minInterval);
        }
    }


    @Override
    public void run() {
        if(component.isDestoryed()){
            return;
        }
        if(!hasScrollEnd){
            return;
        }
        if(canStart){
            component.fireEvent(Constants.Event.SCROLL_START, getScrollEvent(this.x, this.y));
            canStart = false;
        }
        if(component.getEvents().contains(Constants.Event.SCROLL_END)){
            component.fireEvent(Constants.Event.SCROLL_END, getScrollEvent(this.x, this.y));

        }
        hasStart = false;
        hasScrollEnd = false;

    }

    private Map<String, Object> getScrollEvent(int offsetX, int offsetY){
        if(component instanceof BasicListComponent){
            BasicListComponent basicListComponent = (BasicListComponent) component;
            if(basicListComponent.getHostView() instanceof ListComponentView){
                ListComponentView componentView = (ListComponentView) basicListComponent.getHostView();
                if(componentView != null){
                    return basicListComponent.getScrollEvent(componentView.getInnerView(), offsetX, offsetY);
                }
            }
        }else if(component instanceof WXRecyclerTemplateList){
            WXRecyclerTemplateList templateList = (WXRecyclerTemplateList) component;
            return templateList.getScrollEvent(templateList.getHostView().getInnerView(), offsetX, offsetY);
        }else if(component instanceof WXScroller){
            WXScroller scroller = (WXScroller) component;
            return scroller.getScrollEvent(offsetX, offsetY);
        }
        return null;
    }

    public void onScrollStateChanged(int newState){

        if(oldState == OnWXScrollListener.IDLE){
            canStart = true;
        }

        if(newState == OnWXScrollListener.IDLE){
            hasScrollEnd = true;
            handler.removeCallbacks(this);
            handler.postDelayed(this, minInterval);
        }

        oldState = newState;
    }


    public static boolean isScrollEvent(String type){
        if(Constants.Event.SCROLL.equals(type)){
            return  true;
        }else if(Constants.Event.SCROLL_START.equals(type)){
            return  true;
        }else if(Constants.Event.SCROLL_END.equals(type)){
            return  true;
        }
        return  false;
    }
}
