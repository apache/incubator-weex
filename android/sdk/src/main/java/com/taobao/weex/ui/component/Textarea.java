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

import android.text.TextUtils;
import android.view.Gravity;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.common.Constants;
import com.taobao.weex.dom.TextAreaEditTextDomObject;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.ui.view.WXEditText;
import com.taobao.weex.utils.WXUtils;

/**
 * Created by sospartan on 7/11/16.
 */
public class Textarea extends AbstractEditComponent {
  public Textarea(WXSDKInstance instance, WXDomObject dom, WXVContainer parent, boolean isLazy) {
    super(instance, dom, parent, isLazy);
  }

  @Override
  protected void onHostViewInitialized(WXEditText host) {
    host.setAllowDisableMovement(false);
    super.onHostViewInitialized(host);
  }

  @Override
  protected void appleStyleAfterCreated(WXEditText editText) {
    super.appleStyleAfterCreated(editText);
    String rowsStr = (String) getDomObject().getStyles().get(Constants.Name.ROWS);

    int rows = TextAreaEditTextDomObject.DEFAULT_ROWS;
    try{
      if(!TextUtils.isEmpty(rowsStr)) {
        rows = Integer.parseInt(rowsStr);
      }
    }catch (NumberFormatException e){
      //ignore
      e.printStackTrace();
    }

    editText.setLines(rows);
    editText.setMinLines(rows);
  }

  @Override
  protected int getVerticalGravity() {
    return Gravity.TOP;
  }

  @Override
  protected boolean setProperty(String key, Object param) {
    switch (key) {
      case Constants.Name.ROWS:
        Integer rows = WXUtils.getInteger(param,null);
        if (rows != null)
          setRows(rows);
        return true;
    }
    return super.setProperty(key, param);
  }

  @WXComponentProp(name = Constants.Name.ROWS)
  public void setRows(int rows){
    WXEditText text = getHostView();
    if(text == null||rows <=0 ){
      return;
    }

    text.setLines(rows);
  }
}
