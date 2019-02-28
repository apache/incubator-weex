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


import java.util.concurrent.ConcurrentHashMap;

/**
 * Created by furture on 2018/8/10.
 */

public abstract class FrameImageAdapter {

    private ConcurrentHashMap<Integer, FrameImage> mRequestImages;

    public FrameImageAdapter() {
        this.mRequestImages = new ConcurrentHashMap<Integer, FrameImage>();
    }

    public abstract FrameImage requestFrameImage(String url, int width, int height, int callbackId);


    public  FrameImage getFrameImage(int callbackId){
        return mRequestImages.get(callbackId);
    }

    public void putFrameImage(int callbackId, FrameImage loadImageTarget){
        mRequestImages.put(callbackId, loadImageTarget);
    }

    public void removeFrameImage(int callbackId){
        mRequestImages.remove(callbackId);
    }
}
