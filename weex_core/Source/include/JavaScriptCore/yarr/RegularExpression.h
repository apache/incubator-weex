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

#include <wtf/text/WTFString.h>

namespace JSC { namespace Yarr {

enum MultilineMode {
    MultilineDisabled,
    MultilineEnabled
};

class JS_EXPORT_PRIVATE RegularExpression {
    WTF_MAKE_FAST_ALLOCATED;
public:
    RegularExpression(const String&, TextCaseSensitivity, MultilineMode = MultilineDisabled);
    ~RegularExpression();

    RegularExpression(const RegularExpression&);
    RegularExpression& operator=(const RegularExpression&);

    int match(const String&, int startFrom = 0, int* matchLength = 0) const;
    int searchRev(const String&) const;

    int matchedLength() const;
    bool isValid() const;

private:
    class Private;
    RefPtr<Private> d;
};

void JS_EXPORT_PRIVATE replace(String&, const RegularExpression&, const String&);

} } // namespace JSC::Yarr
