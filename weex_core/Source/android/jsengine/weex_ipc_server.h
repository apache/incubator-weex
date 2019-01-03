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
#ifndef WEEXJSSERVER_H
#define WEEXJSSERVER_H
#include <memory>
#include <string>
class IPCSender;
class IPCSerializer;
class IPCHandler;
class WeexJSServer {
public:
    WeexJSServer(int serverFd, int clientFd, bool enableTrace, std::string crashFileName);
    ~WeexJSServer();
    void loop();
    IPCSender* getSender();
    IPCSerializer* getSerializer();
    IPCHandler* getHandler();
private:
    struct WeexJSServerImpl;
    std::unique_ptr<WeexJSServerImpl> m_impl;
};
#endif /* WEEXJSSERVER_H */
