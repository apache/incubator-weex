/*
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
package org.apache.weex;

import android.app.Application;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.support.v4.content.LocalBroadcastManager;
import android.text.TextUtils;
import org.apache.weex.adapter.IDrawableLoader;
import org.apache.weex.adapter.IWXHttpAdapter;
import org.apache.weex.adapter.IWXImgLoaderAdapter;
import org.apache.weex.adapter.IWXJSExceptionAdapter;
import org.apache.weex.adapter.IWXJsFileLoaderAdapter;
import org.apache.weex.adapter.IWXUserTrackAdapter;
import org.apache.weex.appfram.clipboard.WXClipboardModule;
import org.apache.weex.appfram.navigator.IActivityNavBarSetter;
import org.apache.weex.appfram.navigator.INavigator;
import org.apache.weex.appfram.navigator.WXNavigatorModule;
import org.apache.weex.appfram.pickers.WXPickersModule;
import org.apache.weex.appfram.storage.IWXStorageAdapter;
import org.apache.weex.appfram.storage.WXStorageModule;
import org.apache.weex.appfram.websocket.WebSocketModule;
import org.apache.weex.bridge.ModuleFactory;
import org.apache.weex.bridge.WXBridgeManager;
import org.apache.weex.bridge.WXModuleManager;
import org.apache.weex.bridge.WXServiceManager;
import org.apache.weex.common.Destroyable;
import org.apache.weex.common.TypeModuleFactory;
import org.apache.weex.common.WXErrorCode;
import org.apache.weex.common.WXException;
import org.apache.weex.common.WXInstanceWrap;
import org.apache.weex.common.WXModule;
import org.apache.weex.http.WXStreamModule;
import org.apache.weex.performance.WXStateRecord;
import org.apache.weex.ui.ExternalLoaderComponentHolder;
import org.apache.weex.ui.IExternalComponentGetter;
import org.apache.weex.ui.IExternalModuleGetter;
import org.apache.weex.ui.IFComponentHolder;
import org.apache.weex.ui.SimpleComponentHolder;
import org.apache.weex.ui.WXComponentRegistry;
import org.apache.weex.ui.animation.WXAnimationModule;
import org.apache.weex.ui.component.Textarea;
import org.apache.weex.ui.component.WXA;
import org.apache.weex.ui.component.WXBasicComponentType;
import org.apache.weex.ui.component.WXComponent;
import org.apache.weex.ui.component.WXDiv;
import org.apache.weex.ui.component.WXEmbed;
import org.apache.weex.ui.component.WXHeader;
import org.apache.weex.ui.component.WXImage;
import org.apache.weex.ui.component.WXIndicator;
import org.apache.weex.ui.component.WXInput;
import org.apache.weex.ui.component.WXLoading;
import org.apache.weex.ui.component.WXLoadingIndicator;
import org.apache.weex.ui.component.WXRefresh;
import org.apache.weex.ui.component.WXScroller;
import org.apache.weex.ui.component.WXSlider;
import org.apache.weex.ui.component.WXSliderNeighbor;
import org.apache.weex.ui.component.WXSwitch;
import org.apache.weex.ui.component.WXText;
import org.apache.weex.ui.component.WXVideo;
import org.apache.weex.ui.component.WXWeb;
import org.apache.weex.ui.component.list.HorizontalListComponent;
import org.apache.weex.ui.component.list.SimpleListComponent;
import org.apache.weex.ui.component.list.WXCell;
import org.apache.weex.ui.component.list.WXListComponent;
import org.apache.weex.ui.component.list.template.WXRecyclerTemplateList;
import org.apache.weex.ui.component.richtext.WXRichText;
import org.apache.weex.ui.config.AutoScanConfigRegister;
import org.apache.weex.ui.module.WXDeviceInfoModule;
import org.apache.weex.ui.module.ConsoleLogModule;
import org.apache.weex.ui.module.WXLocaleModule;
import org.apache.weex.ui.module.WXMetaModule;
import org.apache.weex.ui.module.WXModalUIModule;
import org.apache.weex.ui.module.WXTimerModule;
import org.apache.weex.ui.module.WXWebViewModule;
import org.apache.weex.utils.LogLevel;
import org.apache.weex.utils.WXExceptionUtils;
import org.apache.weex.utils.WXLogUtils;
import org.apache.weex.utils.WXSoInstallMgrSdk;
import org.apache.weex.utils.batch.BatchOperationHelper;
import org.apache.weex.utils.cache.RegisterCache;
import java.io.Serializable;
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
