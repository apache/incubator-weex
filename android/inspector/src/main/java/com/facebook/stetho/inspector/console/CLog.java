/*
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

package com.facebook.stetho.inspector.console;

import com.facebook.stetho.common.LogRedirector;
import com.facebook.stetho.inspector.helper.ChromePeerManager;
import com.facebook.stetho.inspector.protocol.module.Console;

import javax.annotation.Nonnull;

/**
 * Utility for reporting an event to the console
 */
public class CLog {
  private static final String TAG = "CLog";

  public static void writeToConsole(
      ChromePeerManager chromePeerManager,
      Console.MessageLevel logLevel,
      Console.MessageSource messageSource,
      String messageText) {
    // Send to logcat to increase the chances that a developer will notice :)
    LogRedirector.d(TAG, messageText);

    Console.ConsoleMessage message = new Console.ConsoleMessage();
    message.source = messageSource;
    message.level = logLevel;
    message.text = messageText;
    Console.MessageAddedRequest messageAddedRequest = new Console.MessageAddedRequest();
    messageAddedRequest.message = message;
    chromePeerManager.sendNotificationToPeers("Console.messageAdded", messageAddedRequest);
  }

  public static void writeToConsole(
      Console.MessageLevel logLevel,
      Console.MessageSource messageSource,
      String messageText
  ) {
    ConsolePeerManager peerManager = ConsolePeerManager.getInstanceOrNull();
    if (peerManager == null) {
      return;
    }

    writeToConsole(peerManager, logLevel, messageSource, messageText);
  }
}
