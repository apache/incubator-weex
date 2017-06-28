/**
 * Copyright (c) 2014, Facebook, Inc. All rights reserved. <p/> This source code is licensed under
 * the BSD-style license found in the LICENSE file in the root directory of this source tree. An
 * additional grant of patent rights can be found in the PATENTS file in the same directory.
 */
package com.taobao.weex.dom.flex;

/**
 * CSSLayout with additional information about the conditions under which it was generated.
 * {@link #requestedWidth} and {@link #requestedHeight} are the width and height the parent set on
 * this node before calling layout visited us.
 */
@Deprecated
public class CachedCSSLayout extends CSSLayout {

  public float requestedWidth = CSSConstants.UNDEFINED;
  public float requestedHeight = CSSConstants.UNDEFINED;
  public float parentMaxWidth = CSSConstants.UNDEFINED;
}
