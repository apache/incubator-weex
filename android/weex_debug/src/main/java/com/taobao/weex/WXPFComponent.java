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
package com.taobao.weex;

import android.app.Application;
import android.content.Context;
import android.widget.Toast;

import com.taobao.prettyfish.core.comp.AbstractComponent;
import com.taobao.prettyfish_base.R;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;


public class WXPFComponent extends AbstractComponent {

  public WXPFComponent(Application application) {
    super(application);
  }

  @Override
  public boolean isDeviceSupport() {
    return true;
  }

  @Override
  public int iconRes() {
    return R.drawable.ic_action_3d;
  }

  @Override
  public String title() {
    return "3D";
  }

  @Override
  public boolean onClick(Context context) {

    try {

      if(!WXEnvironment.sShow3DLayer){
        Toast.makeText(context,"Turn on 3D level switch!use this code WXSDKEngine.show3DLayer(true)",Toast.LENGTH_SHORT).show();
      }

      Class cls = Class.forName("com.taobao.weex.WXDebugTool");
      Method m = cls.getMethod("switchLayerInteractionEnabled", new Class[0]);
      m.invoke(null, new Object[0]);
    } catch (ClassNotFoundException e) {
      e.printStackTrace();
    } catch (NoSuchMethodException e) {
      e.printStackTrace();
    } catch (IllegalAccessException e) {
      e.printStackTrace();
    } catch (InvocationTargetException e) {
      e.printStackTrace();
    }

//    WXDebugTool.switchLayerInteractionEnabled();
    return false;
  }

  @Override
  public void onClose() {
  }
}
