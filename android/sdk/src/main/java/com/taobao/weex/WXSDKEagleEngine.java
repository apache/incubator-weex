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
package com.taobao.weex;

import android.app.Application;
import android.content.res.Resources;

import com.taobao.weex.appfram.clipboard.WXClipboardModule;
import com.taobao.weex.appfram.navigator.WXNavigatorModule;
import com.taobao.weex.appfram.pickers.WXPickersModule;
import com.taobao.weex.appfram.storage.WXStorageModule;
import com.taobao.weex.appfram.websocket.WebSocketModule;
import com.taobao.weex.bridge.WXBridgeManager;
import com.taobao.weex.bridge.WXModuleManager;
import com.taobao.weex.common.TypeModuleFactory;
import com.taobao.weex.common.WXException;
import com.taobao.weex.common.WXInstanceWrap;
import com.taobao.weex.common.WXModule;
import com.taobao.weex.http.WXStreamModule;
import com.taobao.weex.ui.IFComponentHolder;
import com.taobao.weex.ui.SimpleComponentHolder;
import com.taobao.weex.ui.WXComponentRegistry;
import com.taobao.weex.ui.animation.WXAnimationModule;
import com.taobao.weex.ui.component.*;
import com.taobao.weex.ui.component.list.HorizontalListComponent;
import com.taobao.weex.ui.component.list.SimpleListComponent;
import com.taobao.weex.ui.component.list.WXCell;
import com.taobao.weex.ui.component.list.WXListComponent;
import com.taobao.weex.ui.component.list.template.WXRecyclerTemplateList;
import com.taobao.weex.ui.component.richtext.WXRichText;
import com.taobao.weex.ui.module.ConsoleLogModule;
import com.taobao.weex.ui.module.WXDeviceInfoModule;
import com.taobao.weex.ui.module.WXLocaleModule;
import com.taobao.weex.ui.module.WXMetaModule;
import com.taobao.weex.ui.module.WXModalUIModule;
import com.taobao.weex.ui.module.WXTimerModule;
import com.taobao.weex.ui.module.WXWebViewModule;
import com.taobao.weex.utils.WXLogUtils;

import java.util.HashMap;
import java.util.Map;

public class WXSDKEagleEngine {
  private static final String TAG = "WXSDKEagleEngine";

  static void registerApplicationOptionsEagle(final Application application) {
    if (application == null) {
      WXLogUtils.e(TAG, "RegisterApplicationOptions application is null");
      return;
    }

    Resources resources = application.getResources();
    registerCoreEnvEagle("screen_width_pixels", String.valueOf(resources.getDisplayMetrics().widthPixels));
    registerCoreEnvEagle("screen_height_pixels", String.valueOf(resources.getDisplayMetrics().heightPixels));

    int resourceId = resources.getIdentifier("status_bar_height", "dimen", "android");
    if (resourceId > 0) {
      int statusBarHeight = resources.getDimensionPixelSize(resourceId);
      registerCoreEnvEagle("status_bar_height", String.valueOf(statusBarHeight));
    }
  }


  /**
   *
   * Register component to eagle. The registration is singleton in  WXSDKEngine level
   * @param type name of component. Same as type field in the JS.
   * @param clazz the class of the WXComponent to be registered.
   * @param appendTree true for appendTree flag
   * @return true for registration success, false for otherwise.
   * @throws WXException Throws exception if type conflicts.
   */
  public static boolean registerEagleComponent(String type, Class<? extends WXComponent> clazz, boolean appendTree) throws WXException {
    return registerEagleComponent(clazz, appendTree,type);
  }

  public static boolean registerEagleComponent(IFComponentHolder holder, boolean appendTree, String ... names) throws WXException {
    boolean result =  true;
    try {
      for (String name : names) {
        Map<String, Object> componentInfo = new HashMap<>();
        if (appendTree) {
          componentInfo.put("append", "tree");
        }
        result = result && WXComponentRegistry.registerEagleOnlyComponent(name, holder, componentInfo);
      }
      return result;
    } catch (Throwable e) {
      e.printStackTrace();
      return result;
    }
  }

  /**
   *
   * Register component to eagle. The registration is singleton in WXSDKEngine level
   * @param clazz the class of the WXComponent to be registered.
   * @param appendTree true for appendTree flag
   * @return true for registration success, false for otherwise.
   * @param names names(alias) of component. Same as type field in the JS.
   * @throws WXException Throws exception if type conflicts.
   */
  public static boolean registerEagleComponent(Class<? extends WXComponent> clazz, boolean appendTree,String ... names) throws WXException {
    if(clazz == null){
      return false;
    }
    SimpleComponentHolder holder = new SimpleComponentHolder(clazz);
    return registerEagleComponent(holder,appendTree,names);
  }

  public static boolean registerEagleComponent(String type, Class<? extends WXComponent> clazz) throws WXException {
    return WXComponentRegistry.registerEagleOnlyComponent(type, new SimpleComponentHolder(clazz),new HashMap<String, Object>());
  }

  public static void registerCoreEnvEagle(String key, String value) {
    WXBridgeManager.getInstance().registerCoreEnvEagle(key, value);
  }


  public static boolean registerEagleModule(String moduleName, Class<? extends WXModule> moduleClass) throws WXException {
    return moduleClass != null && WXModuleManager.registerEagleModule(moduleName, new TypeModuleFactory<>(moduleClass), false);
  }

  static void registerEagle() {
    try {
      registerEagleComponent(
          new SimpleComponentHolder(
              WXText.class,
              new WXText.Creator()
          ),
          false,
          WXBasicComponentType.TEXT
      );
      registerEagleComponent(
          new SimpleComponentHolder(
              WXDiv.class,
              new WXDiv.Ceator()
          ),
          false,
          WXBasicComponentType.CONTAINER,
          WXBasicComponentType.DIV,
          WXBasicComponentType.HEADER,
          WXBasicComponentType.FOOTER
      );
      registerEagleComponent(
          new SimpleComponentHolder(
              WXImage.class,
              new WXImage.Creator()
          ),
          false,
          WXBasicComponentType.IMAGE,
          WXBasicComponentType.IMG
      );
      registerEagleComponent(
          new SimpleComponentHolder(
              WXScroller.class,
              new WXScroller.Creator()
          ),
          false,
          WXBasicComponentType.SCROLLER
      );
      registerEagleComponent(
          new SimpleComponentHolder(
              WXSlider.class,
              new WXSlider.Creator()
          ),
          true,
          WXBasicComponentType.SLIDER,
          WXBasicComponentType.CYCLE_SLIDER
      );
      registerEagleComponent(
          new SimpleComponentHolder(
              WXSliderNeighbor.class,
              new WXSliderNeighbor.Creator()
          ),
          true,
          WXBasicComponentType.SLIDER_NEIGHBOR
      );
      registerEagleComponent(
          new SimpleComponentHolder(
              WXCell.class,
              new WXCell.Creator()
          ),
          true,
          WXBasicComponentType.CELL);
      registerEagleComponent(
          new SimpleComponentHolder(
              WXListComponent.class,
              new WXListComponent.Creator()
          ),
          true,
          WXBasicComponentType.LIST,
          WXBasicComponentType.VLIST,
          WXBasicComponentType.RECYCLER,
          WXBasicComponentType.WATERFALL);

      registerEagleComponent(
          new SimpleComponentHolder(
              WXRichText.class,
              new WXRichText.Creator()
          ),
          false,
          WXBasicComponentType.RICHTEXT
      );

      String simpleList = "simplelist";
      registerEagleComponent(SimpleListComponent.class,false,simpleList);
      registerEagleComponent(WXRecyclerTemplateList.class, false,WXBasicComponentType.RECYCLE_LIST);
      registerEagleComponent(HorizontalListComponent.class,false,WXBasicComponentType.HLIST);
      registerEagleComponent(WXBasicComponentType.CELL_SLOT, WXCell.class, true);
      registerEagleComponent(WXBasicComponentType.INDICATOR, WXIndicator.class, true);
      registerEagleComponent(WXBasicComponentType.VIDEO, WXVideo.class, false);
      registerEagleComponent(WXBasicComponentType.INPUT, WXInput.class, false);
      registerEagleComponent(WXBasicComponentType.TEXTAREA, Textarea.class,false);
      registerEagleComponent(WXBasicComponentType.SWITCH, WXSwitch.class, false);
      registerEagleComponent(WXBasicComponentType.A, WXA.class, false);
      registerEagleComponent(WXBasicComponentType.EMBED, WXEmbed.class, true);
      registerEagleComponent(WXBasicComponentType.WEB, WXWeb.class);
      registerEagleComponent(WXBasicComponentType.REFRESH, WXRefresh.class);
      registerEagleComponent(WXBasicComponentType.LOADING, WXLoading.class);
      registerEagleComponent(WXBasicComponentType.LOADING_INDICATOR, WXLoadingIndicator.class);
      registerEagleComponent(WXBasicComponentType.HEADER, WXHeader.class);

      registerEagleModule("modal", WXModalUIModule.class);
      registerEagleModule("instanceWrap", WXInstanceWrap.class);
      registerEagleModule("animation", WXAnimationModule.class);
      registerEagleModule("webview", WXWebViewModule.class);
      registerEagleModule("navigator", WXNavigatorModule.class);
      registerEagleModule("stream", WXStreamModule.class);
      registerEagleModule("timer", WXTimerModule.class);
      registerEagleModule("storage", WXStorageModule.class);
      registerEagleModule("clipboard", WXClipboardModule.class);
      registerEagleModule("globalEvent",WXGlobalEventModule.class);
      registerEagleModule("picker", WXPickersModule.class);
      registerEagleModule("meta", WXMetaModule.class);
      registerEagleModule("webSocket", WebSocketModule.class);
      registerEagleModule("locale", WXLocaleModule.class);
      registerEagleModule("deviceInfo", WXDeviceInfoModule.class);
      registerEagleModule("sdk-console-log", ConsoleLogModule.class);
    } catch (WXException e) {
      WXLogUtils.e("[WXSDKEngine] register:", e);
    }
  }
}
