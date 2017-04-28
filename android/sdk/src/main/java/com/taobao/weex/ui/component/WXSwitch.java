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
package com.taobao.weex.ui.component;

import android.content.Context;
import android.support.annotation.NonNull;
import android.widget.CompoundButton;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.annotation.Component;
import com.taobao.weex.common.Constants;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.ui.view.WXSwitchView;
import com.taobao.weex.utils.WXUtils;

import java.util.HashMap;
import java.util.Map;

@Component(lazyload = false)

public class WXSwitch extends WXComponent<WXSwitchView> {

  private CompoundButton.OnCheckedChangeListener mListener;

  @Deprecated
  public WXSwitch(WXSDKInstance instance, WXDomObject dom, WXVContainer parent, String instanceId, boolean isLazy) {
    this(instance, dom, parent, isLazy);
  }

  public WXSwitch(WXSDKInstance instance, WXDomObject dom, WXVContainer parent, boolean isLazy) {
    super(instance, dom, parent, isLazy);
  }

  @Override
  protected WXSwitchView initComponentHostView(@NonNull Context context) {
    return new WXSwitchView(context);
  }


  @Override
  public void addEvent(String type) {
    super.addEvent(type);
    if (type != null && type.equals(Constants.Event.CHANGE) && getHostView() != null) {
      if (mListener == null) {
        mListener = new CompoundButton.OnCheckedChangeListener() {
          @Override
          public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            Map<String, Object> params = new HashMap<>(2);
            params.put("value", isChecked);

            Map<String, Object> domChanges = new HashMap<>();
            Map<String, Object> attrsChanges = new HashMap<>();
            attrsChanges.put("checked",Boolean.toString(isChecked));
            domChanges.put("attrs",attrsChanges);
            fireEvent(Constants.Event.CHANGE, params,domChanges);
          }
        };
      }
      getHostView().setOnCheckedChangeListener(mListener);
    }
  }

  @Override
  protected void removeEventFromView(String type) {
    super.removeEventFromView(type);
    if (getHostView() != null && Constants.Event.CHANGE.equals(type)) {
      getHostView().setOnCheckedChangeListener(null);
    }
  }

  @Override
  protected boolean setProperty(String key, Object param) {
    switch (key) {
      case Constants.Name.CHECKED:
        Boolean result = WXUtils.getBoolean(param, null);
        if (result != null) {
          setChecked(result);
        }
        return true;
    }
    return super.setProperty(key, param);
  }

  @WXComponentProp(name = Constants.Name.CHECKED)
  public void setChecked(boolean checked) {
    getHostView().setOnCheckedChangeListener(null);
    getHostView().setChecked(checked);
    getHostView().setOnCheckedChangeListener(mListener);
  }
}
