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
package org.apache.weex.appfram.clipboard;

import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.Intent;
import android.content.res.AssetFileDescriptor;
import android.net.Uri;
import android.support.annotation.Nullable;

import org.apache.weex.bridge.JSCallback;
import org.apache.weex.annotation.JSMethod;
import org.apache.weex.common.WXModule;
import org.apache.weex.utils.WXLogUtils;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.HashMap;
import java.util.Map;


public class WXClipboardModule extends WXModule implements IWXClipboard {

    private final String CLIP_KEY = "WEEX_CLIP_KEY_MAIN";

    private static final String RESULT = "result";
    private static final String DATA = "data";

    private static final String RESULT_OK = "success";
    private static final String RESULT_FAILED = "failed";

    @Override
    @JSMethod
    public void setString(String text) {
        if(null == text) {
            return;
        }

        Context context = mWXSDKInstance.getContext();
        ClipboardManager clipboard = (ClipboardManager) context.getSystemService(context.CLIPBOARD_SERVICE);
        ClipData clip = ClipData.newPlainText(CLIP_KEY, text);
        clipboard.setPrimaryClip(clip);
    }

    @Override
    @JSMethod
    public void getString(@Nullable JSCallback callback) {
        Context context = mWXSDKInstance.getContext();
        ClipboardManager clipboard = (ClipboardManager) context.getSystemService(Context.CLIPBOARD_SERVICE);

        Map<String, Object> map = new HashMap<>(2);
        ClipData clip = clipboard.getPrimaryClip();
        if (clip != null && clip.getItemCount() > 0) {
            ClipData.Item item = clip.getItemAt(0);
            CharSequence text = coerceToText(context, item);

            map.put(RESULT, text != null ? RESULT_OK : RESULT_FAILED);
            map.put(DATA, text != null ? text : "");
        } else {
            map.put(RESULT, RESULT_FAILED);
            map.put(DATA, "");
        }

        if (null != callback) {
            callback.invoke(map);
        }
    }

    @Nullable
    private CharSequence coerceToText(Context context, ClipData.Item item) {
        // Condition 1. just a simple text
        CharSequence text = item.getText();
        if (text != null) {
            return text;
        }

        // Condition 2. a URI value
        Uri uri = item.getUri();
        if (uri != null) {
            InputStreamReader reader = null;
            FileInputStream stream = null;
            try {
                AssetFileDescriptor assetFileDescriptor = context.getContentResolver().openTypedAssetFileDescriptor(uri, "text/*", null);
                stream = assetFileDescriptor.createInputStream();
                reader = new InputStreamReader(stream, "UTF-8");

                StringBuilder builder = new StringBuilder(128);
                char[] buffer = new char[8192];
                int len;
                while ((len = reader.read(buffer)) > 0) {
                    builder.append(buffer, 0, len);
                }
                return builder.toString();

            } catch (FileNotFoundException e) {
                //  ignore.
            } catch (IOException e) {
                WXLogUtils.w("ClippedData Failure loading text.", e);
            } finally {
                if (reader != null) {
                    try {
                        reader.close();
                    } catch (IOException e) {
                        // ignore
                    }
                }
                if (stream != null) {
                    try {
                        stream.close();
                    } catch (IOException e) {
                        // ignore
                    }
                }
            }

            return uri.toString();
        }

        // Condition 3.  an intent.
        Intent intent = item.getIntent();
        if (intent != null) {
            return intent.toUri(Intent.URI_INTENT_SCHEME);
        }

        // else case
        return null;
    }
}
