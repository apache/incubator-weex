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
package org.apache.weex.ui.component.richtext.node;

import static org.apache.weex.utils.WXViewUtils.getRealPxByWidth;

import android.content.Context;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.support.annotation.NonNull;
import android.text.SpannableStringBuilder;
import org.apache.weex.WXSDKEngine;
import org.apache.weex.WXSDKInstance;
import org.apache.weex.WXSDKManager;
import org.apache.weex.adapter.DrawableStrategy;
import org.apache.weex.adapter.URIAdapter;
import org.apache.weex.common.Constants;
import org.apache.weex.ui.component.richtext.span.ImgSpan;
import org.apache.weex.ui.component.richtext.span.ItemClickSpan;
import org.apache.weex.utils.ImgURIUtil;
import org.apache.weex.utils.WXUtils;

import java.util.LinkedList;
import java.util.List;
import java.util.Map;

class ImgNode extends RichTextNode {

  static class ImgNodeCreator implements RichTextNodeCreator<ImgNode> {

    @Override
    public ImgNode createRichTextNode(Context context, String instanceId, String componentRef) {
      return new ImgNode(context, instanceId, componentRef);
    }
    @Override
    public ImgNode createRichTextNode(Context context,String instanceId,String componentRef,String ref,
                                       Map<String,Object> styles, Map<String,Object> attrs){
      return new ImgNode(context,instanceId,componentRef,ref,styles,attrs);
    }
  }

  public static final String NODE_TYPE = "image";

  private ImgNode(Context context, String instanceId, String componentRef) {
    super(context, instanceId, componentRef);
  }
  private ImgNode(Context context, String instanceId, String componentRef, String ref, Map<String,Object> styles, Map<String,Object> attrs) {
    super(context, instanceId, componentRef, ref, styles, attrs);
  }

  @Override
  public String toString() {
    return "\uFEFF";
  }

  @Override
  protected boolean isInternalNode() {
    return false;
  }

  @Override
  protected void updateSpans(SpannableStringBuilder spannableStringBuilder, int level) {
    WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(mInstanceId);
    if (WXSDKEngine.getDrawableLoader() != null &&
        style.containsKey(Constants.Name.WIDTH) &&
        style.containsKey(Constants.Name.HEIGHT) &&
        attr.containsKey(Constants.Name.SRC) &&
        instance != null) {
      List<Object> spans = new LinkedList<>();
      spans.add(createImgSpan(instance));

      if (attr.containsKey(PSEUDO_REF)) {
        spans.add(new ItemClickSpan(mInstanceId, mComponentRef,
            attr.get(PSEUDO_REF).toString()));
      }

      for (Object span : spans) {
        spannableStringBuilder.setSpan(
            span, 0, spannableStringBuilder.length(), createSpanFlag(level));
      }
    }
  }

  @NonNull
  private ImgSpan createImgSpan(WXSDKInstance instance) {
    int width = (int) getRealPxByWidth(WXUtils.getFloat(style.get(Constants.Name.WIDTH)),
        instance.getInstanceViewPortWidth());
    int height = (int) getRealPxByWidth(WXUtils.getFloat(style.get(Constants.Name.HEIGHT)),
        instance.getInstanceViewPortWidth());
    ImgSpan imageSpan = new ImgSpan(width, height);

    String url = attr.get(Constants.Name.SRC).toString();
    Uri rewrited = instance.rewriteUri(Uri.parse(url), URIAdapter.IMAGE);
    if (Constants.Scheme.LOCAL.equals(rewrited.getScheme())) {
      Drawable localDrawable = ImgURIUtil.getDrawableFromLoaclSrc(mContext, rewrited);
      imageSpan.setDrawable(localDrawable, false);
    } else {
      DrawableStrategy drawableStrategy = new DrawableStrategy();
      drawableStrategy.width = width;
      drawableStrategy.height = height;
      WXSDKEngine.getDrawableLoader().setDrawable(rewrited.toString(), imageSpan, drawableStrategy);
    }
    return imageSpan;
  }
}
