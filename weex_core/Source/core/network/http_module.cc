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
#include "core/network/default_request_handler.h"

namespace weex {
namespace core {
namespace data_render {

HttpModule::HttpModule() : request_handler_(new DefaultRequestHandler) {
}

HttpModule::HttpModule(RequestHandler* request_handler) {
    if (!request_handler) {
        request_handler_.reset(new DefaultRequestHandler);
    } else {
        request_handler_.reset(request_handler);
    }
}

void HttpModule::Send(const char* url, FUNC callback) {
    request_handler_->Send(url, callback);
}

}
}
}
