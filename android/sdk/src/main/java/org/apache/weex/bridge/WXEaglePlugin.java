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
import android.support.annotation.NonNull;
import com.alibaba.fastjson.JSONObject;
import org.apache.weex.InitConfig;
import org.apache.weex.ui.IFComponentHolder;
import java.util.Map;
public interface WXEaglePlugin {
  int NOT_SUPPORT  = 0;
  int EAGLE_ONLY = 1;
  int EAGLE_AND_SCRIPT  = 2;
  /**
   * Called during WXSDKEngine init.
   *
   * @return so name of the plugin, which should be loading into {@link InitConfig.Builder#addNativeLibrary}
   */
  String getSoLibName();
  /**
   * If this kind of plugin support skip js framework on instanceId.
   */
  boolean isSkipFrameworkInit(String instanceId);
  /**
   * @return If this plugin support lazy load Components & Modules.
   */
  boolean isLazyCompAndModuleSupport();
  /**
   * @return Name of this plugin.
   */
  @NonNull
  String getPluginName();
  /**
   * A filter to intercept url in {@link com.taobao.weex.WXSDKInstance#renderByUrl}.
   *
   * @param url Origin weex url in {@link com.taobao.weex.WXSDKInstance#renderByUrl}.
   *
   * @return {@code null} if plugin don't support such url.
   *     <p>
   *     {@code String} if plugin support such url. Url could be modified. Following process should
   *     use this value to overwrite old one.
   */
  String isSupportedUrl(String url);
  /**
   * Called in arbitrary thread
   *
   * @param instanceId instanceId of {@code WXSDKInstance}
   *
   * @return {@code 0} if not supported.
   *     {@code 1} if supported and no further call into JsFramework required.
   *     {@code 2} if supported and further call into JsFramework required.
   */
  int isSupportJSCallback(String instanceId);
  /**
   * Must called in js-thread.
   */
  void invokeJSCallback(String instanceId,
                        String callbackId,
                        String data,
                        boolean keep_alive);
  /**
   * Called in arbitrary thread
   *
   * @param instanceId instanceId of {@code WXSDKInstance}
   *
   * @return {@code 0} if not supported.
   *     {@code 1} if supported and no further call into JsFramework required.
   *     {@code 2} if supported and further call into JsFramework required.
   */
  int isSupportInvokeExecJS(String instanceId);
  /**
   * Must called in js-thread.
   */
  void invokeExecJS(final String instanceId, final String namespace, final String function,
                    final WXJSObject[] args);
  /**
   * Called in arbitrary thread
   *
   * @param instanceId instanceId of {@code WXSDKInstance}
   *
   * @return {@code 0} if not supported.
   *     {@code 1} if supported and no further call into JsFramework required.
   *     {@code 2} if supported and further call into JsFramework required.
   */
  int isSupportFireEvent(String instanceId);
  /**
   * Must called in js-thread.
   */
  void fireEvent(String instanceId, String ref, String type, String data, String domChanges);
  /**
   * Called in any thread. This method should be thread-safe
   */
  void registerModules(final String moduleName, final ModuleFactory factory, final boolean global);
  /**
   * Called in any thread. This method should be thread-safe
   */
  void registerComponent(final String type, final IFComponentHolder holder, final Map<String, Object> componentInfo);
  /**
   * @param task   task name;
   * @param option options, a JSONObject
   *
   * @return true if this task is supported. false else-wise.
   */
  boolean callEagleTaskFromWeex(String task, JSONObject option);
}