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

import com.taobao.weex.render.view.DocumentView;

/**
 * Created by furture on 2018/8/10.
 */

public abstract class RenderImageAdapter {


    public abstract BitmapTarget requestImageTarget(DocumentView documentView, String ref, String url, int width, int height, boolean isPlaceholder);



    public String genImageKey(DocumentView documentView, String ref, String url, int width, int height, boolean isPlaceholder){
        if(isPlaceholder){
            return url;
        }
        StringBuilder stringBuilder = new StringBuilder(url);
        stringBuilder.append('@');
        stringBuilder.append(ref);
        return stringBuilder.toString();
    }


}
