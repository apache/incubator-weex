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
#include "core/network/http_module.h"
#include "core/network/request_handler.h"

namespace weex {
namespace core {
namespace network {

HttpModule::HttpModule()
    : request_handler_(RequestHandler::CreateDefaultHandler()) {}

HttpModule::HttpModule(RequestHandler* request_handler)
    : request_handler_(request_handler) {}

void HttpModule::Send(const char* instance_id, const char* url,
                      Callback callback) {
  request_handler_->Send(instance_id, url, callback);
}

void HttpModule::GetBundleType(const char *instance_id, const char *content, Callback callback){
  request_handler_->GetBundleType(instance_id, content, callback);
}

}  // namespace network
}  // namespace core
}  // namespace weex
