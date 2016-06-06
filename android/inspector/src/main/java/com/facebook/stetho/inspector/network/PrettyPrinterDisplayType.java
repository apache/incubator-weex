/*
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

package com.facebook.stetho.inspector.network;

import com.facebook.stetho.inspector.protocol.module.Page;

public enum PrettyPrinterDisplayType {
  JSON(Page.ResourceType.XHR),
  HTML(Page.ResourceType.DOCUMENT),
  TEXT(Page.ResourceType.DOCUMENT);

  private final Page.ResourceType mResourceType;

  private PrettyPrinterDisplayType(Page.ResourceType resourceType) {
    mResourceType = resourceType;
  }

  /**
   * Converts PrettyPrinterDisplayType values to the appropriate
   *  {@link Page.ResourceType} values that Stetho understands
   */
  public Page.ResourceType getResourceType() {
    return mResourceType;
  }
}
