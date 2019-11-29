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
package org.apache.weex.bridge;

import static org.apache.weex.http.WXHttpUtil.KEY_USER_AGENT;

import android.net.Uri;
import android.support.annotation.Keep;
import android.text.TextUtils;
import org.apache.weex.WXEnvironment;
import org.apache.weex.WXHttpListener;
import org.apache.weex.WXSDKInstance;
import org.apache.weex.WXSDKManager;
import org.apache.weex.adapter.IWXHttpAdapter;
import org.apache.weex.adapter.URIAdapter;
import org.apache.weex.base.CalledByNative;
import org.apache.weex.bridge.WXBridgeManager.BundType;
import org.apache.weex.common.WXErrorCode;
import org.apache.weex.common.WXRequest;
import org.apache.weex.common.WXResponse;
import org.apache.weex.http.WXHttpUtil;
import org.apache.weex.utils.WXExceptionUtils;

import org.apache.weex.utils.WXLogUtils;
import java.util.HashMap;
import java.util.Locale;

public class RequestHandler {

  @Keep
  native void nativeInvokeOnSuccess(long callback, String script, String bundleType);
  native void nativeInvokeOnFailed(long callback);

  @CalledByNative
  public static RequestHandler create() {
    return new RequestHandler();
  }

  @CalledByNative
  public void send(String instanceId, String url, long nativeCallback) {
    if (TextUtils.isEmpty(instanceId) || TextUtils.isEmpty(url) ||
        nativeCallback == 0 ||
        !WXSDKManager.getInstance().getAllInstanceMap().containsKey(
            instanceId)) {
      return;
    }

    WXSDKManager manager = WXSDKManager.getInstance();
    WXSDKInstance instance =
        WXSDKManager.getInstance().getSDKInstance(instanceId);
    if (instance == null)
      return;
    IWXHttpAdapter adapter = WXSDKManager.getInstance().getIWXHttpAdapter();

    WXRequest wxRequest = new WXRequest();
    wxRequest.url = manager.getURIAdapter()
                        .rewrite(instance, URIAdapter.BUNDLE, Uri.parse(url))
                        .toString();

    if (wxRequest.paramMap == null) {
      wxRequest.paramMap = new HashMap<>();
    }
    wxRequest.paramMap.put(
        KEY_USER_AGENT, WXHttpUtil.assembleUserAgent(
                            instance.getContext(), WXEnvironment.getConfig()));
    wxRequest.paramMap.put("isBundleRequest", "true");
    WXLogUtils.i("Eagle", String.format(Locale.ENGLISH, "Weex eagle is going to download script from %s", url));
    adapter.sendRequest(wxRequest, new OnHttpListenerInner(instance, nativeCallback, url));
  }

  @Keep
  @CalledByNative
  public void getBundleType(String instanceId, final String content, final long nativeCallback){
    BundType bundleType = WXBridgeManager.getInstance().getBundleType("", content);
    final String bundleTypeStr = bundleType == null ? "Others" : bundleType.toString();
    WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
    if ("Others".equalsIgnoreCase(bundleTypeStr) && null != instance){
      WXExceptionUtils.commitCriticalExceptionRT(
          instanceId,
          WXErrorCode.WX_KEY_EXCEPTION_NO_BUNDLE_TYPE,
          "RequestHandler.onSuccess",
          "eagle ->" +WXErrorCode.WX_KEY_EXCEPTION_NO_BUNDLE_TYPE.getErrorMsg(),
          null
      );
    }
    WXBridgeManager.getInstance().post(new Runnable() {
      @Override
      public void run() {
        if(WXBridgeManager.getInstance().isJSFrameworkInit()) {
          nativeInvokeOnSuccess(nativeCallback, content, bundleTypeStr);
        }
        else {
          nativeInvokeOnFailed(nativeCallback);
        }
      }
    });
  }

  class OnHttpListenerInner extends WXHttpListener {
    private long sNativeCallback;

    OnHttpListenerInner(WXSDKInstance instance, long nativeCallback, String bundlUrl) {
      super(instance, bundlUrl);
      sNativeCallback = nativeCallback;
    }

    @Override
    public void onSuccess(WXResponse response) {
        final String script = new String(response.originalData);
        BundType bundleType = WXBridgeManager.getInstance().getBundleType("", script);
        final String bundleTypeStr = bundleType == null ? "Others" : bundleType.toString();
        if ("Others".equalsIgnoreCase(bundleTypeStr) && null != getInstance()){
          WXExceptionUtils.commitCriticalExceptionRT(
              getInstance().getInstanceId(),
              WXErrorCode.WX_KEY_EXCEPTION_NO_BUNDLE_TYPE,
              "RequestHandler.onSuccess",
              "eagle ->" +WXErrorCode.WX_KEY_EXCEPTION_NO_BUNDLE_TYPE.getErrorMsg(),
              null
          );
        }
        WXBridgeManager.getInstance().post(new Runnable() {
          @Override
          public void run() {
            if(WXBridgeManager.getInstance().isJSFrameworkInit()) {
              nativeInvokeOnSuccess(sNativeCallback, script, bundleTypeStr);
            }
            else{
              nativeInvokeOnFailed(sNativeCallback);
            }
          }
        });
    }

    @Override
    public void onFail(WXResponse response) {
      nativeInvokeOnFailed(sNativeCallback);
    }
  }
}
