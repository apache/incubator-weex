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

package com.taobao.weex.utils;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.support.v4.content.res.ResourcesCompat;

import java.util.List;

public class ImgURIUtil {

  public static Drawable getDrawableFromLoaclSrc(Context context, Uri rewrited) {
    Resources resources = context.getResources();
    List<String> segments = rewrited.getPathSegments();
    if (segments.size() != 1) {
      WXLogUtils.e("Local src format is invalid.");
      return null;
    }
    int id = resources.getIdentifier(segments.get(0), "drawable", context.getPackageName());
    return id == 0 ? null : ResourcesCompat.getDrawable(resources, id, null);
  }
}
