/*
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

package com.facebook.stetho.inspector.jsonrpc;

import com.facebook.stetho.inspector.jsonrpc.protocol.JsonRpcError;
import com.facebook.stetho.common.Util;

public class JsonRpcException extends Exception {
  private final JsonRpcError mErrorMessage;

  public JsonRpcException(JsonRpcError errorMessage) {
    super(errorMessage.code + ": " + errorMessage.message);
    mErrorMessage = Util.throwIfNull(errorMessage);
  }

  public JsonRpcError getErrorMessage() {
    return mErrorMessage;
  }
}
