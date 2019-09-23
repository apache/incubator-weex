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
package org.apache.weex.ui.component.list.template;

import android.support.v4.view.ViewCompat;
import android.view.View;

import org.apache.weex.WXSDKManager;
import org.apache.weex.dom.WXAttr;
import org.apache.weex.ui.component.WXComponent;
import org.apache.weex.ui.component.WXVContainer;
import org.apache.weex.ui.component.list.WXCell;
import org.apache.weex.ui.view.listview.WXRecyclerView;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Created by furture on 2018/6/27.
 */

public class TemplateDom {

    public static final String KEY_RESET_ANIMATION = "resetAnimation";
    public static final String KEY_ATTRS = "attrs";
    public static final String KEY_TYPE = "type";
    public static final String KEY_VIRTUAL_DOM_REF = "ref";
    public static final String VIRTUAL_DOM_IDENTIFY = "[[VirtualElement]]";


    public static final String ATTRS_KEY_REF = "ref";

    public static final String ATTACH_CELL_SLOT = "_attach_slot";
    public static final String DETACH_CELL_SLOT = "_detach_slot";

    public static final char SEPARATOR = '@';


    public static String genKeyVirtualDomRef(String listRef, int position, String key){
        return listRef + SEPARATOR + position + SEPARATOR  +  key;
    }

    public static WXComponent findVirtualComponentByVRef(String pageId, String virtualRef) {
        try{

            String[]  segments = virtualRef.split(SEPARATOR + "");
            String listRef = segments[0]; // list ref
            WXComponent component = WXSDKManager
                .getInstance().getWXRenderManager().getWXComponent(pageId, listRef);
            if(!(component instanceof WXRecyclerTemplateList)){
                return null;
            }
            WXRecyclerTemplateList templateList = (WXRecyclerTemplateList) component;
            if(templateList.getHostView() == null || templateList.getHostView().getInnerView() == null){
                return null;
            }
            int position = Integer.parseInt(segments[1]); // position
            WXRecyclerView recyclerView = templateList.getHostView().getInnerView();
            TemplateViewHolder itemHolder = (TemplateViewHolder) recyclerView.findViewHolderForAdapterPosition(position);
            if(itemHolder == null){
                return null;
            }

            WXCell cell = itemHolder.getTemplate();
            String viewKey = segments[2]; //viewkey
            WXComponent target = findComponentByViewTreeKey(cell, viewKey);
            return  target;
        }catch (Exception e){
            return  null;
        }
    }

    /**
     * find all component that contains ref attr
     * */
    public static Map<String,Object> findAllComponentRefs(String listRef, int position , WXComponent component){
        Map<String,Object> refs = new HashMap<>();
        findAllComponentRefs(listRef, position, component, refs);
        Map<String,Object> refsMap = new HashMap<>();
        refsMap.put("refs", refs);
        refsMap.put("position", position);
        refsMap.put("listRef", listRef);
        return refsMap;
    }

    private static void findAllComponentRefs(String listRef, int position, WXComponent component, Map<String,Object> refs){
        if(component.isWaste()){
            return;
        }
        if(component instanceof WXVContainer){
            WXVContainer container = (WXVContainer) component;
            for(int i=0; i<container.getChildCount(); i++){
                WXComponent child = container.getChild(i);
                findAllComponentRefs(listRef, position, child, refs);
            }
        }
        WXAttr attrs = component.getAttrs();
        if(attrs != null
                && attrs.get(TemplateDom.ATTRS_KEY_REF) == null){
            return;
        }
        String ref  = attrs.get(TemplateDom.ATTRS_KEY_REF).toString();
        List<Object> refList = (List<Object>) refs.get(ref);
        if(refList == null){
            refList = new ArrayList<>();
            refs.put(ref, refList);
        }
        Map map = toMap(listRef, position, component);
        refList.add(map);
    }


    public static Map toMap(String listRef, int position, WXComponent component){
        Map map = new HashMap();
        map.put(TemplateDom.KEY_ATTRS, component.getAttrs());
        map.put(TemplateDom.KEY_TYPE, component.getComponentType());
        map.put(TemplateDom.KEY_VIRTUAL_DOM_REF, TemplateDom.genKeyVirtualDomRef(listRef, position, component.getViewTreeKey()));
        map.put(VIRTUAL_DOM_IDENTIFY, true);
        return map;
    }


    public static boolean isVirtualDomRef(String ref){
        if(ref != null){
            return  ref.indexOf(SEPARATOR) > 0;
        }
        return  false;
    }


    public static void resetAnimaiton(View view){
        if(view == null){
            return;
        }
        if(ViewCompat.getTranslationX(view) != 0){
            ViewCompat.setTranslationX(view, 0);
        }

        if(ViewCompat.getTranslationY(view) != 0){
            ViewCompat.setTranslationY(view, 0);
        }

        if(ViewCompat.getTranslationZ(view) != 0){
            ViewCompat.setTranslationZ(view, 0);
        }

        if(ViewCompat.getScaleX(view) != 1.0f){
            ViewCompat.setScaleX(view, 1.0f);
        }

        if(ViewCompat.getScaleY(view) != 1.0f){
            ViewCompat.setScaleY(view, 1.0f);
        }

        if(ViewCompat.getRotationX(view) != 0){
            ViewCompat.setRotationX(view, 0);
        }

        if(ViewCompat.getRotationY(view) != 0){
            ViewCompat.setRotationY(view, 0);
        }

        if(ViewCompat.getElevation(view) != 0){
            ViewCompat.setElevation(view, 0);
        }
    }


    /**
     * find child list, has same ref
     * */
    public static final  WXComponent findComponentByViewTreeKey(WXComponent component, String viewKey){
        if(component.getViewTreeKey().equals(viewKey)){
            return component;
        }
        if(component instanceof WXVContainer){
            WXVContainer container = (WXVContainer) component;
            for(int i=0; i<container.getChildCount(); i++){
                WXComponent element = container.getChild(i);
                if(findComponentByViewTreeKey(element, viewKey) != null){
                    return element;
                }
            }

        }
        return null;
    }

    /**
     * find child by ref
     * */
    private  static WXComponent findChildByAttrsRef(WXComponent component, String ref){
        if(component.getAttrs() != null && ref.equals(component.getAttrs().get(TemplateDom.ATTRS_KEY_REF))){
            return component;
        }
        if(component instanceof WXVContainer){
            WXVContainer container = (WXVContainer) component;
            for(int i=0; i<container.getChildCount(); i++){
                WXComponent child = findChildByAttrsRef(container.getChild(i), ref);
                if(child != null){
                    return  child;
                }
            }
        }
        return  null;
    }


}
