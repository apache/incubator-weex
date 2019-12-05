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
package org.apache.weex.ui.component.list;

import android.support.v7.widget.RecyclerView;

import org.apache.weex.common.Constants;
import org.apache.weex.ui.view.listview.adapter.TransformItemDecoration;
import org.apache.weex.utils.WXLogUtils;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * recycler item decoration transform
 * Created by jianbai.gbj on 2017/9/14.
 */
public class RecyclerTransform {

    public static final String TRANSFORM = Constants.Name.TRANSFORM;
    private static final Pattern transformPattern = Pattern.compile("([a-z]+)\\(([0-9\\.]+),?([0-9\\.]+)?\\)");
    private static final String TAG = "RecyclerTransform";

    /**
     * These transform functions are supported:
     * - `scale(x,y)`: scale item, x and y should be a positive float number.
     * - `translate(x,y)`: translate item, `x` and `y` shoule be integer numbers.
     * - `opacity(n)`: change the transparency of item, `n` must in `[0,1.0]`.
     * - `rotate(n)`: rotate item, n is integer number.
     *
     * @param raw
     * @return
     */
    public static RecyclerView.ItemDecoration parseTransforms(int orientation, String raw) {
        if (raw == null) {
            return null;
        }
        float scaleX = 0f, scaleY = 0f;
        int translateX = 0, translateY = 0;
        float opacity = 0f;
        int rotate = 0;
        //public TransformItemDecoration(boolean isVertical,float alpha,int translateX,int translateY,int rotation,float scale)
        Matcher matcher = transformPattern.matcher(raw);
        while (matcher.find()) {
            String match = matcher.group();
            String name = matcher.group(1);
            try {
                switch (name) {
                    case "scale":
                        scaleX = Float.parseFloat(matcher.group(2));
                        scaleY = Float.parseFloat(matcher.group(3));
                        break;
                    case "translate":
                        translateX = Integer.parseInt(matcher.group(2));
                        translateY = Integer.parseInt(matcher.group(3));
                        break;
                    case "opacity":
                        opacity = Float.parseFloat(matcher.group(2));
                        break;
                    case "rotate":
                        rotate = Integer.parseInt(matcher.group(2));
                        break;
                    default:
                        WXLogUtils.e(TAG, "Invaild transform expression:" + match);
                        break;
                }
            } catch (NumberFormatException e) {
                WXLogUtils.e("", e);
                WXLogUtils.e(TAG, "Invaild transform expression:" + match);
            }
        }
        return new TransformItemDecoration(orientation == Constants.Orientation.VERTICAL, opacity, translateX, translateY, rotate, scaleX, scaleY);
    }
}
