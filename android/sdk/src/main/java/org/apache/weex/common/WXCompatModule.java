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
package org.apache.weex.common;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.support.v4.content.LocalBroadcastManager;

import org.apache.weex.WXEnvironment;

/**
 * Created by lixinke on 16/10/10.
 */

public abstract class WXCompatModule extends WXModule implements Destroyable {

  private ModuleReceive mModuleReceive;

  public WXCompatModule() {
    mModuleReceive = new ModuleReceive(this);
    LocalBroadcastManager.getInstance(WXEnvironment.getApplication())
        .registerReceiver(mModuleReceive, new IntentFilter(WXModule.ACTION_ACTIVITY_RESULT));
    LocalBroadcastManager.getInstance(WXEnvironment.getApplication())
        .registerReceiver(mModuleReceive, new IntentFilter(WXModule.ACTION_REQUEST_PERMISSIONS_RESULT));
  }

  public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
  }

  public void onActivityResult(int requestCode, int resultCode, Intent data) {
  }

  @Override
  public void destroy() {
    LocalBroadcastManager.getInstance(WXEnvironment.getApplication())
        .unregisterReceiver(mModuleReceive);
  }

  static class ModuleReceive extends BroadcastReceiver {

    private WXCompatModule mWXCompatModule;

    ModuleReceive(WXCompatModule module) {
      mWXCompatModule = module;
    }

    @Override
    public void onReceive(Context context, Intent intent) {
      String action = intent.getAction();
      switch (action) {
        case WXModule.ACTION_ACTIVITY_RESULT:
          int requestCode = intent.getIntExtra(WXModule.REQUEST_CODE, -1);
          int resultCode = intent.getIntExtra(WXModule.RESULT_CODE, Activity.RESULT_OK);
          mWXCompatModule.onActivityResult(requestCode, resultCode, intent);
          break;
        case WXModule.ACTION_REQUEST_PERMISSIONS_RESULT:
          requestCode = intent.getIntExtra(WXModule.REQUEST_CODE, -1);
          String[] permissions = intent.getStringArrayExtra(WXModule.PERMISSIONS);
          int[] grantResults = intent.getIntArrayExtra(WXModule.GRANT_RESULTS);
          mWXCompatModule.onRequestPermissionsResult(requestCode, permissions, grantResults);
          break;
      }
    }
  }
}
