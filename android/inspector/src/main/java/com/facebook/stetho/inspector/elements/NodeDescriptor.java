/*
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

package com.facebook.stetho.inspector.elements;

import com.facebook.stetho.common.Accumulator;
import com.facebook.stetho.common.ThreadBound;

import javax.annotation.Nullable;

public interface NodeDescriptor extends ThreadBound {
  void hook(Object element);

  void unhook(Object element);

  NodeType getNodeType(Object element);

  String getNodeName(Object element);

  String getLocalName(Object element);

  @Nullable
  String getNodeValue(Object element);

  void getChildren(Object element, Accumulator<Object> children);

  void getAttributes(Object element, AttributeAccumulator attributes);

  void setAttributesAsText(Object element, String text);

  void getStyles(Object element, StyleAccumulator accumulator);
}
