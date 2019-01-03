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

#include "Watchpoint.h"

namespace JSC {

class JSObject;
class PropertyName;

class VariableWriteFireDetail : public FireDetail {
public:
    VariableWriteFireDetail(JSObject* object, const PropertyName& name)
        : m_object(object)
        , m_name(name)
    {
    }
    
    JS_EXPORT_PRIVATE void dump(PrintStream&) const override;
    
    JS_EXPORT_PRIVATE static void touch(VM&, WatchpointSet*, JSObject*, const PropertyName&);

private:
    JSObject* m_object;
    const PropertyName& m_name;
};

} // namespace JSC
