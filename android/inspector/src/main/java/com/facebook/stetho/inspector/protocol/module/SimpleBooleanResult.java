/*
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

package com.facebook.stetho.inspector.protocol.module;

import com.facebook.stetho.inspector.jsonrpc.JsonRpcResult;
import com.facebook.stetho.json.annotation.JsonProperty;

public class SimpleBooleanResult implements JsonRpcResult {
  @JsonProperty(required = true)
  public boolean result;

  public SimpleBooleanResult() {
  }

  public SimpleBooleanResult(boolean result) {
    this.result = result;
  }
}
