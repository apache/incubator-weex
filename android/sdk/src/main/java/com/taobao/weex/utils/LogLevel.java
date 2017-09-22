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

import android.util.Log;

/**
 * Created by lixinke on 16/5/11.
 */
public enum LogLevel {
  ERROR("error", 0, Log.ERROR), WARN("warn", 1,Log.WARN), INFO("info", 2,Log.INFO),
  DEBUG("debug", 3,Log.DEBUG), VERBOSE("verbose", 4, Log.VERBOSE), ALL("debug", 5,Log.DEBUG),OFF("off",6,Log.DEBUG);
  String name;
  int value;
  int priority;

  LogLevel(String name, int value,int priority) {
    this.name = name;
    this.value = value;
    this.priority = priority;
  }
  public String getName(){
    return name;
  }
  public int getValue(){
    return value;
  }
  public int getPriority(){
    return priority;
  }
  public int compare(LogLevel level){
    return value-level.value;
  }
}
