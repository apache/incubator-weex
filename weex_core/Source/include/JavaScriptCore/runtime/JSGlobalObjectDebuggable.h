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

#if ENABLE(REMOTE_INSPECTOR)

#include "JSGlobalObjectInspectorController.h"
#include "RemoteInspectionTarget.h"
#include <wtf/Noncopyable.h>

namespace Inspector {
class FrontendChannel;
enum class DisconnectReason;
}

namespace JSC {

class JSGlobalObject;

class JSGlobalObjectDebuggable final : public Inspector::RemoteInspectionTarget {
    WTF_MAKE_FAST_ALLOCATED;
    WTF_MAKE_NONCOPYABLE(JSGlobalObjectDebuggable);
public:
    JSGlobalObjectDebuggable(JSGlobalObject&);
    ~JSGlobalObjectDebuggable() { }

    Inspector::RemoteControllableTarget::Type type() const override { return Inspector::RemoteControllableTarget::Type::JavaScript; }

    String name() const override;
    bool hasLocalDebugger() const override { return false; }

    void connect(Inspector::FrontendChannel*, bool automaticInspection) override;
    void disconnect(Inspector::FrontendChannel*) override;
    void dispatchMessageFromRemote(const String& message) override;
    void pause() override;

    bool automaticInspectionAllowed() const override { return true; }
    void pauseWaitingForAutomaticInspection() override;

private:
    JSGlobalObject& m_globalObject;
};

} // namespace JSC

SPECIALIZE_TYPE_TRAITS_CONTROLLABLE_TARGET(JSC::JSGlobalObjectDebuggable, JavaScript);

#endif // ENABLE(REMOTE_INSPECTOR)
