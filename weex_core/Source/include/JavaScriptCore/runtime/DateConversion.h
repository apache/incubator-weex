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
#pragma once

#include <wtf/GregorianDateTime.h>

namespace WTF {
class String;
} // namespace WTF

namespace JSC {

enum DateTimeFormat {
    DateTimeFormatDate = 1,
    DateTimeFormatTime = 2,
    DateTimeFormatDateAndTime = DateTimeFormatDate | DateTimeFormatTime
};

JS_EXPORT_PRIVATE WTF::String formatDateTime(const GregorianDateTime&, DateTimeFormat, bool asUTCVariant);

} // namespace JSC
