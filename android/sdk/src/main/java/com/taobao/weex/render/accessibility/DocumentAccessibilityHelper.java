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
package com.taobao.weex.render.accessibility;

import android.graphics.Rect;
import android.os.Bundle;
import android.support.v4.view.accessibility.AccessibilityNodeInfoCompat;
import android.support.v4.widget.ExploreByTouchHelper;
import android.text.TextUtils;
import android.view.accessibility.AccessibilityNodeInfo;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

import com.taobao.weex.render.view.DocumentTextureView;

import java.util.List;

/**
 * Created by furture on 2018/8/21.
 */

public class DocumentAccessibilityHelper extends ExploreByTouchHelper {

    private DocumentTextureView documentTextureView;
    private DocumentAccessibilityTask documentAccessibilityTask;

    public DocumentAccessibilityHelper(DocumentTextureView documentTextureView) {
        super(documentTextureView);
        this.documentTextureView = documentTextureView;
    }

    @Override
    protected int getVirtualViewAt(float x, float y) {
        if(!documentTextureView.hasDocumentView()){
            return ExploreByTouchHelper.HOST_ID;
        }
        this.documentAccessibilityTask = new DocumentAccessibilityTask(documentTextureView.getDocumentView(),0, 0);
        if(!documentAccessibilityTask.updatePoint((int)x, (int)y)){
            return documentAccessibilityTask.getViewId();
        }
        documentAccessibilityTask.resetCountDown();
        documentTextureView.getTaskQueue().addTask(documentAccessibilityTask);
        documentAccessibilityTask.waitComplete();
        return documentAccessibilityTask.getViewId();
    }

    @Override
    protected void getVisibleVirtualViews(List<Integer> virtualViewIds) {
        if(documentAccessibilityTask != null){
            virtualViewIds.add(documentAccessibilityTask.getViewId());
        }
    }


    @Override
    protected void onPopulateNodeForVirtualView(int virtualViewId, AccessibilityNodeInfoCompat node) {
        node.setClassName(TextView.class.getName());
        if(documentAccessibilityTask != null
                && virtualViewId == documentAccessibilityTask.getViewId()){
            String type = documentAccessibilityTask.getType();
            if(AccessibilityConstants.TAG_IMAGE.equals(type) || AccessibilityConstants.TAG_IMG.equals(type)){
                node.setClassName(ImageView.class.getName());
            }else if(AccessibilityConstants.TAG_A.equals(type)){
                node.setClassName(Button.class.getName());
            }else {
                node.setClassName(TextView.class.getName());
            }
            if(!TextUtils.isEmpty(documentAccessibilityTask.getRole())){
                node.setRoleDescription(documentAccessibilityTask.getRole());
            }
            node.setText( documentAccessibilityTask.getContentDescription());
            node.setContentDescription( documentAccessibilityTask.getContentDescription());
            node.setBoundsInParent(documentAccessibilityTask.getBounds());
            if(documentAccessibilityTask.hasClick()){
                node.addAction(AccessibilityNodeInfo.ACTION_CLICK);
                node.setClassName(Button.class.getName());
            }
        }else{
            node.setBoundsInParent(new Rect(0,0, documentTextureView.getDocumentView().getDocumentWidth(),
                     documentTextureView.getDocumentView().getDocumentHeight()));
            node.setContentDescription("");
        }
        node.addAction(AccessibilityNodeInfo.ACTION_FOCUS);
    }


    @Override
    protected boolean onPerformActionForVirtualView(int virtualViewId, int action, Bundle arguments) {
        return false;
    }
}
