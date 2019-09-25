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
package org.apache.weex.ui.component;

import android.content.Context;
import android.support.annotation.NonNull;
import android.view.View;
import android.widget.CompoundButton;

import org.apache.weex.WXSDKInstance;
import org.apache.weex.annotation.Component;
import org.apache.weex.common.Constants;
import org.apache.weex.layout.ContentBoxMeasurement;
import org.apache.weex.ui.action.BasicComponentData;
import org.apache.weex.ui.view.WXSwitchView;
import org.apache.weex.utils.WXLogUtils;
import org.apache.weex.utils.WXUtils;

import java.util.HashMap;
import java.util.Map;

@Component(lazyload = false)

public class WXSwitch extends WXComponent<WXSwitchView> {

  private CompoundButton.OnCheckedChangeListener mListener;

  @Deprecated
  public WXSwitch(WXSDKInstance instance, WXVContainer parent, String instanceId, boolean isLazy, BasicComponentData basicComponentData) {
    this(instance, parent, isLazy, basicComponentData);
  }

  public WXSwitch(final WXSDKInstance instance, WXVContainer parent, boolean isLazy, BasicComponentData basicComponentData) {
    super(instance, parent, isLazy, basicComponentData);
    setContentBoxMeasurement(new ContentBoxMeasurement() {
      /** uiThread = false **/
      @Override
      public void measureInternal(float width, float height, int widthMeasureMode, int heightMeasureMode) {
        mMeasureWidth = 0;
        mMeasureHeight = 0;
        try {
          WXSwitchView wxSwitchView = new WXSwitchView(instance.getContext());
          int widthSpec, heightSpec;
          heightSpec = View.MeasureSpec.makeMeasureSpec(0, View.MeasureSpec.UNSPECIFIED);
          if (Float.isNaN(width)) {
            widthSpec = View.MeasureSpec.makeMeasureSpec(0, View.MeasureSpec.UNSPECIFIED);
          } else {
            widthSpec = View.MeasureSpec.makeMeasureSpec((int) width, View.MeasureSpec.AT_MOST);
          }

          wxSwitchView.measure(widthSpec, heightSpec);
          mMeasureWidth = wxSwitchView.getMeasuredWidth();
          mMeasureHeight = wxSwitchView.getMeasuredHeight();
        } catch (RuntimeException e) {
          WXLogUtils.e(WXLogUtils.getStackTrace(e));
        }
      }

      /** uiThread = false **/
      @Override
      public void layoutBefore() {
      }

      /** uiThread = false **/
      @Override
      public void layoutAfter(float computedWidth, float computedHeight) {
      }
    });
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
