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
package com.taobao.weex.dom;

import com.taobao.weex.common.Constants;

/**
 * Created by sospartan on 7/12/16.
 */
public class TextAreaEditTextDomObject extends BasicEditTextDomObject {

  public static final int DEFAULT_ROWS = 2;
  private int mNumberOfLines = DEFAULT_ROWS;

  @Override
  protected float getMeasureHeight(){
    return getMeasuredLineHeight() * mNumberOfLines;
  }


  @Override
  protected void updateStyleAndAttrs() {
    super.updateStyleAndAttrs();
    Object raw = getAttrs().get(Constants.Name.ROWS);
    if (raw == null) {
      return;
    } else if (raw instanceof String) {
      String rowsStr = (String) raw;
      try {
        int lines = Integer.parseInt(rowsStr);
        if (lines > 0) {
          mNumberOfLines = lines;
        }
      } catch (NumberFormatException e) {
        e.printStackTrace();
      }
    } else if (raw instanceof Integer) {
      mNumberOfLines = (Integer) raw;
    }
  }

}
