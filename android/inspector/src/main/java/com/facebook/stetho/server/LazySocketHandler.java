/*
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

package com.facebook.stetho.server;

import android.net.LocalSocket;

import javax.annotation.Nonnull;
import javax.annotation.Nullable;

import java.io.IOException;

/**
 * Optimization designed to allow us to lazily construct/configure the true Stetho server
 * only after the first caller connects.  This gives us much more wiggle room to have performance
 * impact in the set up path that only applies when Stetho is _used_, not simply enabled.
 */
public class LazySocketHandler implements SocketHandler {
  private final SocketHandlerFactory mSocketHandlerFactory;

  @Nullable
  private SocketHandler mSocketHandler;

  public LazySocketHandler(SocketHandlerFactory socketHandlerFactory) {
    mSocketHandlerFactory = socketHandlerFactory;
  }

  @Override
  public void onAccepted(LocalSocket socket) throws IOException {
    getSocketHandler().onAccepted(socket);
  }

  @Nonnull
  private synchronized SocketHandler getSocketHandler() {
    if (mSocketHandler == null) {
      mSocketHandler = mSocketHandlerFactory.create();
    }
    return mSocketHandler;
  }
}
