/*
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

package com.facebook.stetho.inspector.elements.android;

import android.app.Application;

import android.os.Handler;
import android.os.Looper;
import com.facebook.stetho.common.ThreadBound;
import com.facebook.stetho.common.UncheckedCallable;
import com.facebook.stetho.common.Util;
import com.facebook.stetho.common.android.HandlerUtil;
import com.facebook.stetho.inspector.elements.DocumentProvider;
import com.facebook.stetho.inspector.elements.DocumentProviderFactory;

public final class AndroidDocumentProviderFactory
    implements DocumentProviderFactory, ThreadBound {
  private final Application mApplication;
  private final Handler mHandler;

  public AndroidDocumentProviderFactory(Application application) {
    mApplication = Util.throwIfNull(application);
    mHandler = new Handler(Looper.getMainLooper());
  }

  @Override
  public DocumentProvider create() {
    return new AndroidDocumentProvider(mApplication, this);
  }

  // ThreadBound implementation
  @Override
  public boolean checkThreadAccess() {
    return HandlerUtil.checkThreadAccess(mHandler);
  }

  @Override
  public void verifyThreadAccess() {
    HandlerUtil.verifyThreadAccess(mHandler);
  }

  @Override
  public <V> V postAndWait(UncheckedCallable<V> c) {
    return HandlerUtil.postAndWait(mHandler, c);
  }

  @Override
  public void postAndWait(Runnable r) {
    HandlerUtil.postAndWait(mHandler, r);
  }

  @Override
  public void postDelayed(Runnable r, long delayMillis) {
    if (!mHandler.postDelayed(r, delayMillis)) {
      throw new RuntimeException("Handler.postDelayed() returned false");
    }
  }

  @Override
  public void removeCallbacks(Runnable r) {
    mHandler.removeCallbacks(r);
  }
}
