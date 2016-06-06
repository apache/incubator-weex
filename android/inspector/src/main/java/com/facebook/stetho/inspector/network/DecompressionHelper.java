/*
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

package com.facebook.stetho.inspector.network;

import com.facebook.stetho.inspector.console.CLog;
import com.facebook.stetho.inspector.protocol.module.Console;

import javax.annotation.Nullable;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.zip.InflaterOutputStream;

// @VisibleForTest
public class DecompressionHelper {
  static final String GZIP_ENCODING = "gzip";
  static final String DEFLATE_ENCODING = "deflate";

  public static InputStream teeInputWithDecompression(
      NetworkPeerManager peerManager,
      String requestId,
      InputStream availableInputStream,
      OutputStream decompressedOutput,
      @Nullable String contentEncoding,
      ResponseHandler responseHandler) throws IOException {
    OutputStream output = decompressedOutput;
    CountingOutputStream decompressedCounter = null;

    if (contentEncoding != null) {
      boolean gzipEncoding = GZIP_ENCODING.equals(contentEncoding);
      boolean deflateEncoding = DEFLATE_ENCODING.equals(contentEncoding);

      if (gzipEncoding || deflateEncoding) {
        decompressedCounter = new CountingOutputStream(decompressedOutput);
        if (gzipEncoding) {
          output = GunzippingOutputStream.create(decompressedCounter);
        } else if (deflateEncoding) {
          output = new InflaterOutputStream(decompressedCounter);
        }
      } else {
        CLog.writeToConsole(
            peerManager,
            Console.MessageLevel.WARNING,
            Console.MessageSource.NETWORK,
            "Unsupported Content-Encoding in response for request #" + requestId +
                ": " + contentEncoding);
      }
    }

    return new ResponseHandlingInputStream(
        availableInputStream,
        requestId,
        output,
        decompressedCounter,
        peerManager,
        responseHandler);
  }
}
