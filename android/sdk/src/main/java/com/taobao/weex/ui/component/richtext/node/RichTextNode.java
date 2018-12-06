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
package com.taobao.weex.ui.component.richtext.node;

import static com.taobao.weex.dom.WXStyle.UNSET;

import android.content.Context;
import android.graphics.Color;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.util.ArrayMap;
import android.text.Spannable;
import android.text.SpannableString;
import android.text.SpannableStringBuilder;
import android.text.Spanned;
import android.text.style.AbsoluteSizeSpan;
import android.text.style.BackgroundColorSpan;
import android.text.style.ForegroundColorSpan;
import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.common.Constants;
import com.taobao.weex.dom.WXCustomStyleSpan;
import com.taobao.weex.dom.WXStyle;
import com.taobao.weex.utils.WXResourceUtils;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

public abstract class RichTextNode {

    public static final String TYPE = "type";
    public static final String STYLE = "style";
    public static final String ATTR = "attr";
    public static final String CHILDREN = "children";
    public static final String VALUE = Constants.Name.VALUE;
    public static final String ITEM_CLICK="itemclick";
    public static final String PSEUDO_REF="pseudoRef";
    private static final int MAX_LEVEL = Spanned.SPAN_PRIORITY >> Spanned.SPAN_PRIORITY_SHIFT;

    protected final Context mContext;
    protected final String mInstanceId;
    protected final String mComponentRef;
    protected Map<String, Object> style;
    protected Map<String, Object> attr;
    protected List<RichTextNode> children;

    protected RichTextNode(Context context, String instanceId, String componentRef) {
        mContext = context;
        mInstanceId = instanceId;
        mComponentRef = componentRef;
    }

    public static
    @NonNull
    Spannable parse(@NonNull Context context, @NonNull String instanceId, @NonNull String componentRef, String json) {
        JSONArray jsonArray = JSON.parseArray(json);
        JSONObject jsonObject;
        List<RichTextNode> nodes;
        RichTextNode node;
        if (jsonArray != null && !jsonArray.isEmpty()) {
            nodes = new ArrayList<>(jsonArray.size());
            for (int i = 0; i < jsonArray.size(); i++) {
                jsonObject = jsonArray.getJSONObject(i);
                if (jsonObject != null) {
                    node = RichTextNodeManager.createRichTextNode(context, instanceId, componentRef, jsonObject);
                    if (node != null) {
                        nodes.add(node);
                    }
                }
            }
            return parse(nodes);
        }
        return new SpannableString("");
    }

    public static int createSpanFlag(int level) {
        return createPriorityFlag(level) | Spanned.SPAN_INCLUSIVE_EXCLUSIVE;
    }

    @Override
    public abstract String toString();

    protected abstract boolean isInternalNode();

    final void parse(@NonNull Context context, @NonNull String instanceId, @NonNull String componentRef, JSONObject jsonObject) {
        JSONObject jsonStyle, jsonAttr, child;
        JSONArray jsonChildren;
        RichTextNode node;
        if ((jsonStyle = jsonObject.getJSONObject(STYLE)) != null) {
            style = new ArrayMap<>();
            style.putAll(jsonStyle);
        } else {
            style = new ArrayMap<>(0);
        }

        if ((jsonAttr = jsonObject.getJSONObject(ATTR)) != null) {
            attr = new ArrayMap<>(jsonAttr.size());
            attr.putAll(jsonAttr);
        } else {
            attr = new ArrayMap<>(0);
        }

        if ((jsonChildren=jsonObject.getJSONArray(CHILDREN))!=null) {
            children = new ArrayList<>(jsonChildren.size());
            for (int i = 0; i < jsonChildren.size(); i++) {
                child = jsonChildren.getJSONObject(i);
                node = RichTextNodeManager.createRichTextNode(context, instanceId, componentRef, child);
                if (node != null) {
                    children.add(node);
                }
            }
        } else {
            children = new ArrayList<>(0);
        }
    }

    protected void updateSpans(SpannableStringBuilder spannableStringBuilder, int level) {
        WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(mInstanceId);
        if (style != null && instance != null) {
            List<Object> spans = new LinkedList<>();

            WXCustomStyleSpan customStyleSpan = createCustomStyleSpan();
            if (customStyleSpan != null) {
                spans.add(customStyleSpan);
            }

            if (style.containsKey(Constants.Name.FONT_SIZE)) {
                spans.add(new AbsoluteSizeSpan(WXStyle.getFontSize(style, instance.getInstanceViewPortWidth())));
            }

            if (style.containsKey(Constants.Name.BACKGROUND_COLOR)) {
                int color = WXResourceUtils.getColor(style.get(Constants.Name.BACKGROUND_COLOR).toString(),
                    Color.TRANSPARENT);
                if (color != Color.TRANSPARENT) {
                    spans.add(new BackgroundColorSpan(color));
                }
            }

            if (style.containsKey(Constants.Name.COLOR)) {
                spans.add(new ForegroundColorSpan(WXResourceUtils.getColor(WXStyle.getTextColor(style))));
            }

            int spanFlag = createSpanFlag(level);
            for (Object span : spans) {
                spannableStringBuilder.setSpan(span, 0, spannableStringBuilder.length(), spanFlag);
            }
        }
    }

    private static int createPriorityFlag(int level) {
        return level <= MAX_LEVEL ?
            (MAX_LEVEL - level) << Spanned.SPAN_PRIORITY_SHIFT :
            MAX_LEVEL << Spanned.SPAN_PRIORITY_SHIFT;
    }

    private static
    @NonNull
    Spannable parse(@NonNull List<RichTextNode> list) {
        SpannableStringBuilder spannableStringBuilder = new SpannableStringBuilder();
        for (RichTextNode richTextNode : list) {
            spannableStringBuilder.append(richTextNode.toSpan(1));
        }
        return spannableStringBuilder;
    }

    private Spannable toSpan(int level) {
        SpannableStringBuilder spannableStringBuilder = new SpannableStringBuilder();
        spannableStringBuilder.append(toString());
        if (isInternalNode() && children != null) {
            for (RichTextNode child : children) {
                spannableStringBuilder.append(child.toSpan(level + 1));
            }
        }
        updateSpans(spannableStringBuilder, level);
        return spannableStringBuilder;
    }

    private
    @Nullable
    WXCustomStyleSpan createCustomStyleSpan() {
        int fontWeight = UNSET, fontStyle = UNSET;
        String fontFamily = null;
        if (style.containsKey(Constants.Name.FONT_WEIGHT)) {
            fontWeight = WXStyle.getFontWeight(style);
        }
        if (style.containsKey(Constants.Name.FONT_STYLE)) {
            fontStyle = WXStyle.getFontStyle(style);
        }
        if (style.containsKey(Constants.Name.FONT_FAMILY)) {
            fontFamily = WXStyle.getFontFamily(style);
        }
        if (fontWeight != UNSET
            || fontStyle != UNSET
            || fontFamily != null) {
            return new WXCustomStyleSpan(fontStyle, fontWeight, fontFamily);
        } else {
            return null;
        }
    }
}
