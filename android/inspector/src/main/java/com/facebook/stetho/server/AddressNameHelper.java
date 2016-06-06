/*
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

package com.facebook.stetho.server;

import com.facebook.stetho.common.ProcessUtil;

public class AddressNameHelper {
  private static final String PREFIX = "stetho_";

  public static String createCustomAddress(String suffix) {
    return
        PREFIX +
        ProcessUtil.getProcessName() +
        suffix;
  }
}
