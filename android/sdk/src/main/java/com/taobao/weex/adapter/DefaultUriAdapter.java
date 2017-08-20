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
package com.taobao.weex.adapter;

import android.net.Uri;
import android.support.annotation.NonNull;
import android.text.TextUtils;

import com.taobao.weex.WXSDKInstance;

import java.util.List;


/**
 * Default Uri adapter. Provide basic capability to handle relative path, local file path etc.
 * Created by sospartan on 21/11/2016.
 */
public class DefaultUriAdapter implements URIAdapter {


  @NonNull
  @Override
  public Uri rewrite(WXSDKInstance instance, String type, Uri uri) {
    if (TextUtils.isEmpty(instance.getBundleUrl())) {
      return uri;
    }

    Uri base = Uri.parse(instance.getBundleUrl());
    Uri.Builder resultBuilder = uri.buildUpon();
    
   if (uri.isRelative()) {
      //When uri is empty, means use the base url instead. Web broswer behave this way.
      if(uri.getEncodedPath().length() == 0){
        return base;
      } else {
        resultBuilder = buildRelativeURI(resultBuilder, base, uri);
        return resultBuilder.build();
      }
    }
    return uri;
  }

  private Uri.Builder buildRelativeURI(Uri.Builder resultBuilder, Uri base, Uri uri) {
    if (uri.getAuthority() != null) {
      return resultBuilder.scheme(base.getScheme());
    } else {
      resultBuilder
          .encodedAuthority(base.getEncodedAuthority())
          .scheme(base.getScheme())
          .path(null);

      if (uri.getPath().startsWith("/")) {
        //relative to root
        resultBuilder.appendEncodedPath(uri.getEncodedPath().substring(1));
      } else {
        List<String> segments = base.getPathSegments();
        //ignore last segment if not end with /
        int ignoreLast = 1;
        if (base.getPath().endsWith("/")) {
          ignoreLast = 0;
        }
        for (int i = 0, len = segments.size() - ignoreLast; i < len; i++) {
          resultBuilder.appendEncodedPath(segments.get(i));
        }
        resultBuilder.appendEncodedPath(uri.getEncodedPath());
      }
      return resultBuilder;
    }
  }

}
