/*
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

package com.facebook.stetho.inspector.protocol.module;

import java.util.Collections;
import java.util.List;

import com.facebook.stetho.inspector.jsonrpc.JsonRpcPeer;
import com.facebook.stetho.inspector.jsonrpc.JsonRpcResult;
import com.facebook.stetho.inspector.protocol.ChromeDevtoolsDomain;
import com.facebook.stetho.inspector.protocol.ChromeDevtoolsMethod;

import com.facebook.stetho.json.annotation.JsonProperty;
import org.json.JSONObject;

public class HeapProfiler implements ChromeDevtoolsDomain {
  public HeapProfiler() {
  }

  @ChromeDevtoolsMethod
  public JsonRpcResult getProfileHeaders(JsonRpcPeer peer, JSONObject params) {
    ProfileHeaderResponse response = new ProfileHeaderResponse();
    response.headers = Collections.emptyList();
    return response;
  }

  private static class ProfileHeaderResponse implements JsonRpcResult {
    @JsonProperty(required = true)
    public List<ProfileHeader> headers;
  }

  private static class ProfileHeader {
    @JsonProperty(required = true)
    public String title;

    @JsonProperty(required = true)
    public int uid;
  }
}
