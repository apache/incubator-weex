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
//
// Created by Darin on 29/05/2018.
//

#ifndef WEEXV8_INITFRAMEWORKARGS_H
#define WEEXV8_INITFRAMEWORKARGS_H

#include <vector>
#include "include/WeexApiHeader.h"
#include "include/wtf/text/WTFString.h"
#include "third_party/IPC/IPCArguments.h"

class InitFrameworkArgs {
public:
    std::vector<INIT_FRAMEWORK_PARAMS *> params;

    explicit InitFrameworkArgs(std::vector<INIT_FRAMEWORK_PARAMS *> &params);

    explicit InitFrameworkArgs(IPCArguments *arguments, size_t startCount);

    ~InitFrameworkArgs();

};


#endif //WEEXV8_INITFRAMEWORKARGS_H
