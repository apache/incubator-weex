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
package org.apache.weex;

import android.text.TextUtils;

public class Script {
    private String mContent;
    private byte[] mBinary;

    public Script(String content) {
        mContent = content;
    }

    public Script(byte[] binary) {
        mBinary = binary;
    }

    public String getContent() {
        return mContent;
    }

    public byte[] getBinary() {
        return mBinary;
    }

    public int length() {
        if (mContent != null) {
            return mContent.length();
        } else if (mBinary != null){
            return mBinary.length;
        }
        return 0;
    }

    public boolean isEmpty() {
        return TextUtils.isEmpty(mContent) && (mBinary == null || mBinary.length == 0);
    }
}
