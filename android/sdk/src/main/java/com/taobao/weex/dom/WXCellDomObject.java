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
package com.taobao.weex.dom;

import com.taobao.weex.dom.flex.CSSNode;
import com.taobao.weex.dom.flex.MeasureOutput;
import com.taobao.weex.ui.component.WXBasicComponentType;
import com.taobao.weex.utils.WXLogUtils;

/**
 * Created by zhengshihan on 2017/4/11.
 */

public class WXCellDomObject extends WXDomObject {

  /** package **/ static final CSSNode.MeasureFunction CELL_MEASURE_FUNCTION = new MeasureFunction() {
    @Override
    public void measure(CSSNode node, float width, MeasureOutput measureOutput) {
      if (node != null) {
        CSSNode parent = node.getParent();
        if (parent != null && parent instanceof WXRecyclerDomObject) {
          WXRecyclerDomObject parentDom = ((WXRecyclerDomObject) parent);
          parentDom.preCalculateCellWidth();
          WXDomObject domObject = (WXDomObject) node;
          if (WXBasicComponentType.CELL.equals(domObject.getType())) {
            float w = ((WXRecyclerDomObject) parent).getColumnWidth();
            node.setLayoutWidth(w);
          } else if (WXBasicComponentType.HEADER.equals(domObject.getType())){
            float w = parentDom.getAvailableWidth();
            WXLogUtils.d("getAvailableWidth:"+w);
            node.setLayoutWidth(w);
          }
        }
      }
    }
  };

  public WXCellDomObject() {
    setMeasureFunction(CELL_MEASURE_FUNCTION);
  }
}
