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
package org.apache.weex.base;

public class FloatUtil {

  private static final float EPSILON = .00001f;

  public static boolean floatsEqual(float f1, float f2) {
    if (Float.isNaN(f1) || Float.isNaN(f2)) {
      return Float.isNaN(f1) && Float.isNaN(f2);
    }
    return Math.abs(f2 - f1) < EPSILON;
  }
}
