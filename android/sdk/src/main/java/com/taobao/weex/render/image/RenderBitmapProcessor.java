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
package com.taobao.weex.render.image;

import android.graphics.Bitmap;
import android.util.Log;

import com.taobao.weex.render.manager.RenderManager;

/**
 * Created by furture on 2018/8/27.
 */

public class RenderBitmapProcessor {


    public static void toRenderSupportBitmap(final BitmapTarget target){
        if(isSupportedBitmap(target.getBitmap())){
            target.onSupportedBitmap(target.getBitmap());
            return;
        }
        RenderManager.getInstance().getIoHandler().post(new Runnable() {
            @Override
            public void run() {
               final Bitmap bitmap =  target.getBitmap().copy(Bitmap.Config.ARGB_8888, target.getBitmap().isMutable());
               RenderManager.getInstance().getUiHandler().post(new Runnable() {
                   @Override
                   public void run() {
                       target.onSupportedBitmap(bitmap);
                   }
               });
            }
        });
    }


    private static boolean isSupportedBitmap(Bitmap bitmap){
        if(bitmap == null){
            return true;
        }
        Bitmap.Config config = bitmap.getConfig();
        if(config == null){
            return false;
        }
        if(config == Bitmap.Config.ALPHA_8
                || config == Bitmap.Config.RGB_565
                || config == Bitmap.Config.ARGB_4444
                || config == Bitmap.Config.ARGB_8888){
            return true;
        }
        return false;
    }

}
