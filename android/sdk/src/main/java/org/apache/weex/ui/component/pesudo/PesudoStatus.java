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
package org.apache.weex.ui.component.pesudo;

import android.support.annotation.Nullable;
import android.support.v4.util.ArrayMap;

import org.apache.weex.common.Constants;

import java.util.Map;
import org.apache.weex.common.Constants.PSEUDO;

/**
 * Created by sospartan on 05/01/2017.
 */

public class PesudoStatus {

  /**
   * See {@link PSEUDO}
   */
  private int[] mStatuses = new int[4];

  private static final int UNSET = 0;
  private static final int SET = 1;

  static final int CLASS_ACTIVE = 0;
  static final int CLASS_FOCUS = 1;
  static final int CLASS_ENABLED = 2;
  static final int CLASS_DISABLED = 3;

  public PesudoStatus(){
    for (int i = 0; i < mStatuses.length; i++) {
        mStatuses[i] = UNSET;
    }
  }

  /**
   *
   * @param clzName See {@link PSEUDO}
   * @param status
   */
  public void setStatus(String clzName,boolean status){
    switch (clzName){
      case Constants.PSEUDO.ACTIVE:
        setStatus(PesudoStatus.CLASS_ACTIVE,status);
        break;
      case Constants.PSEUDO.DISABLED:
        setStatus(PesudoStatus.CLASS_DISABLED,status);
        break;
      case Constants.PSEUDO.ENABLED:
        setStatus(PesudoStatus.CLASS_ENABLED,status);
        break;
      case Constants.PSEUDO.FOCUS:
        setStatus(PesudoStatus.CLASS_FOCUS,status);
        break;
    }
  }

  void setStatus(int clz,boolean status){
    mStatuses[clz] = status?SET:UNSET;
  }

  public boolean isSet(int clz){
    return mStatuses[clz] == SET;
  }

  public @Nullable String getStatuses(){
    StringBuilder sb = new StringBuilder();
    if(isSet(CLASS_ACTIVE)){
      sb.append(Constants.PSEUDO.ACTIVE);
    }
    if(isSet(CLASS_DISABLED)){
      sb.append(Constants.PSEUDO.DISABLED);
    }
    //enabled is ignored

    if(isSet(CLASS_FOCUS) && !isSet(CLASS_DISABLED)){
      sb.append(Constants.PSEUDO.FOCUS);
    }
    return sb.length()==0?null:sb.toString();
  }

  public Map<String,Object> updateStatusAndGetUpdateStyles(String clzName,
                                                           boolean status,
                                                           Map<String, Map<String,Object>> pesudoStyles,
                                                           Map<String,Object> originalStyles){
    String prevStatusesStr = getStatuses();//before change
    setStatus(clzName,status);
    String statusesStr = getStatuses();//after change

    Map<String,Object> updateStyles = pesudoStyles.get(statusesStr);
    Map<String,Object> prevUpdateStyles = pesudoStyles.get(prevStatusesStr);

    /**
     * NEW INSTANCE, DO NOT USE MAP OBJECT FROM pesudoStyles
     */
    Map<String,Object> resultStyles = new ArrayMap<>();
    if(prevUpdateStyles != null){
      resultStyles.putAll(prevUpdateStyles);
    }

    //reset
    for (String key : resultStyles.keySet()) {
      resultStyles.put(key, originalStyles.containsKey(key) ? originalStyles.get(key) : "");
    }

    //apply new update
    if(updateStyles != null) {
      for (Map.Entry<String, Object> entry : updateStyles.entrySet()) {
        resultStyles.put(entry.getKey(), entry.getValue());
      }
    }
    return resultStyles;
  }
}
