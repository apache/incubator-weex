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
package org.apache.weex.ui.view;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.drawable.Drawable;
import android.support.annotation.NonNull;
import android.widget.FrameLayout;
import org.apache.weex.ui.flat.widget.Widget;
import org.apache.weex.utils.WXLogUtils;
import org.apache.weex.utils.WXViewUtils;
import java.util.List;

public class BaseFrameLayout extends FrameLayout{
    private List<Widget> mWidgets;

    public BaseFrameLayout(Context context){
        super(context);
    }

    @Override
    protected void dispatchDraw(Canvas canvas) {
        try {
            dispatchDrawInterval(canvas);
        } catch (Throwable e) {
            WXLogUtils.e(WXLogUtils.getStackTrace(e));
        }
    }

    protected void dispatchDrawInterval(Canvas canvas) {
        if (mWidgets != null) {
            canvas.save();
            canvas.translate(getPaddingLeft(), getPaddingTop());
            for (Widget widget : mWidgets) {
                widget.draw(canvas);
            }
            canvas.restore();
        } else {
            WXViewUtils.clipCanvasWithinBorderBox(this, canvas);
            super.dispatchDraw(canvas);
        }
    }
    public void mountFlatGUI(List<Widget> widgets){
        this.mWidgets = widgets;
        if (mWidgets != null) {
            setWillNotDraw(true);
        }
        invalidate();
    }

    public void unmountFlatGUI(){
        mWidgets = null;
        setWillNotDraw(false);
        invalidate();
    }

    @Override
    protected boolean verifyDrawable(@NonNull Drawable who) {
        return mWidgets != null || super.verifyDrawable(who);
    }

}
