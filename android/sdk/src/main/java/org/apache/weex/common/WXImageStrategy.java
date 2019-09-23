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

import android.widget.ImageView;

import org.apache.weex.WXSDKInstance;

import java.util.Map;

public class WXImageStrategy {


  /**
   * <strong>Never!</strong>
   * <strong>Never!</strong>
   * <strong>Never!</strong>
   * Never use this flag, ImageView has done all the job of clipping!
   * There is no method to read this flag any more.
   * This field will be removed when it's appropriate.
   */
  @Deprecated
  public boolean isClipping;

  /**
   * Whether to sharp the image. The default is false.
   */
  public boolean isSharpen;

  /**
   * The blur radius of the image. 0 means no blur.
   * */
  public int blurRadius;

  public String placeHolder;

  /**
   * running weex instanceId
   * @see WXSDKInstance#mInstanceId
   */
  public String instanceId;

  public WXImageStrategy()
  {

  }

  public WXImageStrategy(String instanceId)
  {
    this.instanceId = instanceId;
  }


  public ImageListener getImageListener() {
    return imageListener;
  }

  public void setImageListener(ImageListener imageListener) {
    this.imageListener = imageListener;
  }

  ImageListener imageListener;

  public interface ImageListener{
    public void onImageFinish(String url,ImageView imageView,boolean  result,Map extra);
  }
}
