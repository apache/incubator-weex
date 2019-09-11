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

#ifndef CORE_BRIDGE_SCRIPT_BRIDGE_H
#define CORE_BRIDGE_SCRIPT_BRIDGE_H

#include <memory>
#include "base/common.h"
#include "include/WeexApiHeader.h"
#include "base/log_defines.h"

namespace WeexCore {
class ScriptBridge {
 public:
  class CoreSide {
   public:
    explicit CoreSide() {}
    virtual ~CoreSide() {}
    virtual void CallNative(const char *page_id, const char *task,
                            const char *callback) = 0;
    virtual std::unique_ptr<ValueWithType> CallNativeModule(
        const char *page_id, const char *module, const char *method,
        const char *arguments, int arguments_length, const char *options,
        int options_length) = 0;
    virtual void CallNativeComponent(const char *page_id, const char *ref,
                                     const char *method, const char *arguments,
                                     int arguments_length, const char *options,
                                     int options_length) = 0;
    virtual void AddElement(const char *page_id, const char *parent_ref,
                            const char *dom_str, int dom_str_length,
                            const char *index_str) = 0;
    virtual void SetTimeout(const char *callback_id, const char *time) = 0;
    virtual void NativeLog(const char *str_array) = 0;
    virtual void CreateBody(const char *page_id, const char *dom_str,
                            int dom_str_length) = 0;
    virtual int UpdateFinish(const char *page_id, const char *task,
                             int task_length, const char *callback,
                             int callback_length) = 0;
    virtual void CreateFinish(const char *page_id) = 0;
    virtual int RefreshFinish(const char *page_id, const char *task,
                              const char *callback) = 0;
    virtual void UpdateAttrs(const char *page_id, const char *ref,
                             const char *data, int data_length) = 0;
    virtual void UpdateStyle(const char *page_id, const char *ref,
                             const char *data, int data_length) = 0;
    virtual void RemoveElement(const char *page_id, const char *ref) = 0;
    virtual void MoveElement(const char *page_id, const char *ref,
                             const char *parent_ref, int index) = 0;
    virtual void AddEvent(const char *page_id, const char *ref,
                          const char *event) = 0;
    virtual void RemoveEvent(const char *page_id, const char *ref,
                             const char *event) = 0;
    virtual const char *CallGCanvasLinkNative(const char *context_id, int type,
                                              const char *arg) = 0;
    virtual int SetInterval(const char *page_id, const char *callback_id,
                            const char *time) = 0;
    virtual void ClearInterval(const char *page_id,
                               const char *callback_id) = 0;
    virtual const char *CallT3DLinkNative(int type, const char *arg) = 0;
    virtual void PostMessage(const char *vim_id, const char *data, int dataLength) = 0;
    virtual void DispatchMessage(const char *client_id, const char *data, int dataLength,
                                 const char *callback, const char *vm_id) = 0;
    virtual std::unique_ptr<WeexJSResult> DispatchMessageSync(
        const char *client_id, const char *data, int dataLength,
        const char *vm_id) = 0;
    virtual void ReportException(const char *page_id, const char *func,
                                 const char *exception_string) = 0;
    virtual void SetJSVersion(const char *js_version) = 0;
    virtual void OnReceivedResult(long callback_id,
                                  std::unique_ptr<WeexJSResult> &result) = 0;

    virtual void UpdateComponentData(const char* page_id,
                                     const char* cid,
                                     const char* json_data) = 0;


    virtual bool Log(int level, const char *tag,
                     const char *file,
                     unsigned long line,
                     const char *log) = 0;

    inline ScriptBridge *bridge() { return bridge_; }

   private:
    ScriptBridge *bridge_;
    friend class ScriptBridge;
    inline void set_bridge(ScriptBridge *bridge) { bridge_ = bridge; }
    DISALLOW_COPY_AND_ASSIGN(CoreSide);
  };

  class ScriptSide {
   public:
    explicit ScriptSide() {}
    virtual ~ScriptSide() {}

    virtual int InitFramework(const char *script,
                              std::vector<INIT_FRAMEWORK_PARAMS *> &params) = 0;

    virtual int InitAppFramework(
        const char *instanceId, const char *appFramework,
        std::vector<INIT_FRAMEWORK_PARAMS *> &params) = 0;

    virtual int CreateAppContext(const char *instanceId,
                                 const char *jsBundle) = 0;

    virtual std::unique_ptr<WeexJSResult>  ExecJSOnAppWithResult(const char *instanceId,
                                        const char *jsBundle) = 0;

    virtual int CallJSOnAppContext(const char *instanceId, const char *func,
                                   std::vector<VALUE_WITH_TYPE *> &params) = 0;

    virtual int DestroyAppContext(const char *instanceId) = 0;

    virtual int ExecJsService(const char *source) = 0;

    virtual int ExecTimeCallback(const char *source) = 0;

    virtual int ExecJS(const char *instanceId, const char *nameSpace,
                       const char *func,
                       std::vector<VALUE_WITH_TYPE *> &params) = 0;

    virtual std::unique_ptr<WeexJSResult>  ExecJSWithResult(
        const char *instanceId, const char *nameSpace, const char *func,
        std::vector<VALUE_WITH_TYPE *> &params) = 0;

    virtual void ExecJSWithCallback(const char *instanceId,
                                    const char *nameSpace, const char *func,
                                    std::vector<VALUE_WITH_TYPE *> &params,
                                    long callback_id) = 0;

    virtual int CreateInstance(const char *instanceId, const char *func,
                               const char *script, const char *opts,
                               const char *initData,
                               const char *extendsApi, std::vector<INIT_FRAMEWORK_PARAMS*>& params) = 0;

    virtual std::unique_ptr<WeexJSResult>  ExecJSOnInstance(const char *instanceId,
                                   const char *script,int type) = 0;

    virtual int DestroyInstance(const char *instanceId) = 0;

    virtual int UpdateGlobalConfig(const char *config) = 0;

    virtual int UpdateInitFrameworkParams(const std::string& key, const std::string& value, const std::string& desc) = 0;

    virtual void SetLogType(const int logLevel, const bool isPerf) = 0;

    inline ScriptBridge *bridge() { return bridge_; }



   private:
    ScriptBridge *bridge_;
    friend class ScriptBridge;
    inline void set_bridge(ScriptBridge *bridge) { bridge_ = bridge; }
    DISALLOW_COPY_AND_ASSIGN(ScriptSide);
  };

  inline CoreSide *core_side() { return core_side_.get(); }
  inline ScriptSide *script_side() { return script_side_.get(); }

  inline void set_core_side(CoreSide *core_side) {
    core_side_.reset(core_side);
    core_side_->set_bridge(this);
  }
  inline void set_script_side(ScriptSide *script_side) {
    script_side_.reset(script_side);
    script_side_->set_bridge(this);
  }

  ScriptBridge() : is_passable_(true) {}
  virtual ~ScriptBridge() {}

  inline bool is_passable() { return is_passable_; }

 protected:
  inline void set_is_passable(bool passable) { is_passable_ = passable; }

 private:
  std::unique_ptr<CoreSide> core_side_;
  std::unique_ptr<ScriptSide> script_side_;
  bool is_passable_;
  DISALLOW_COPY_AND_ASSIGN(ScriptBridge);
};
}  // namespace WeexCore

#endif  // CORE_BRIDGE_SCRIPT_BRIDGE_H
