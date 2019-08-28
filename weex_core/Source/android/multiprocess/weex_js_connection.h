/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#ifndef WEEXJSCONNECTION_H
#define WEEXJSCONNECTION_H

#include <memory>
#include <unistd.h>
#include <sys/mman.h>

class IPCSender;
class IPCHandler;


class WeexConnInfo {
 public:
  std::unique_ptr<IPCHandler> handler;
  int ipcFd;

  WeexConnInfo(std::unique_ptr<IPCHandler> handler, bool isClient) {
    this->handler = std::move(handler);
    ipcFd = -1;
    is_client = isClient;
  }

  ~WeexConnInfo() {
    closeFd();
  }

  void *mmap_for_ipc();

  void closeFd() {
    if(ipcFd == -1) {
      return;
    }

    if(hasBeenClosed)
      return;
    hasBeenClosed = true;
    close(ipcFd);
  }

 private:
  bool hasBeenClosed = false;
  bool is_client = false;
};

class WeexJSConnection {
public:
    WeexJSConnection(WeexConnInfo* client, WeexConnInfo *server);

    ~WeexJSConnection();

    IPCSender *start(bool reinit);

    void end();

    IPCSender* sender();

  std::unique_ptr<WeexConnInfo> client_;
  std::unique_ptr<WeexConnInfo> server_;

private:
    struct WeexJSConnectionImpl;

    std::unique_ptr<WeexJSConnectionImpl> m_impl;
};

#endif /* WEEXJSCONNECTION_H */
