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
package com.taobao.weex.common;

import android.content.Intent;
import android.text.TextUtils;
import android.view.Menu;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.annotation.JSMethod;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.utils.WXUtils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * All modules must extend this class
 */
public abstract class WXModule implements IWXObject {

  public static final String ACTION_ACTIVITY_RESULT = "actionActivityResult";
  public static final String ACTION_REQUEST_PERMISSIONS_RESULT = "actionRequestPermissionsResult";
  public static final String REQUEST_CODE = "requestCode";
  public static final String RESULT_CODE = "resultCode";
  public static final String PERMISSIONS = "permissions";
  public static final String GRANT_RESULTS = "grantResults";


  public WXSDKInstance mWXSDKInstance;
  private String mModuleName;


  protected final WXComponent findComponent(String ref){
    if(mWXSDKInstance != null && ref != null){
      return WXSDKManager.getInstance()
          .getWXRenderManager()
          .getWXComponent(mWXSDKInstance.getInstanceId(), ref);
    }
    return null;
  }

  /** hook the Activity life cycle to Instance module**/
  public void onActivityResult(int requestCode, int resultCode, Intent data){}

  public void onActivityCreate(){}

  public void onActivityStart(){}

  public void onActivityPause(){}

  public void onActivityResume(){}

  public void onActivityStop(){}

  public void onActivityDestroy(){}

  public boolean onActivityBack() {return false;}

  public boolean onCreateOptionsMenu(Menu menu){return false;}

  public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {}

    /** end **/

  private Map<String, List<String>> mEvents = new HashMap<>();
  private Map<String, Boolean> mKeepAlives = new HashMap<>();



  @JSMethod
  public void addEventListener(String eventName, String callback, Map<String, Object> options) {
    if (TextUtils.isEmpty(eventName) || TextUtils.isEmpty(callback)) {
      return;
    }
    boolean isOnce = false;
    if (options != null && options.size() > 0 && options.containsKey("once")) {
      Object temp = options.get("once");
      if (WXUtils.getBoolean(temp,false)) {
        isOnce = true;
      }
    }
    mKeepAlives.put(callback, isOnce);
    if(mEvents.get(eventName)==null){
      mEvents.put(eventName,new ArrayList<String>());
    }
    mEvents.get(eventName).add(callback);
  }

  @JSMethod
  public void removeAllEventListeners(String eventName) {
    if (mEvents.containsKey(eventName)) {
      List<String> callbacks = mEvents.remove(eventName);
      for(String callback:callbacks){
        mKeepAlives.remove(callback);
      }
    }
  }

  /**
   * Check whether the EventName has been registered
   */
  public List<String> getEventCallbacks(String eventName) {
    return mEvents.get(eventName);
  }

  public boolean isOnce(String callback){
    return mKeepAlives.get(callback);
  }

  public String getModuleName() {
    return mModuleName;
  }

  public void setModuleName(String moduleName) {
    mModuleName = moduleName;
  }
}
