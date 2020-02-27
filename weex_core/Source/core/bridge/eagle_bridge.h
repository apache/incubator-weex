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

#ifndef CORE_BRIDGE_EAGLE_BRIDGE_H
#define CORE_BRIDGE_EAGLE_BRIDGE_H

#if defined __cplusplus

#include <string>
#include <map>
#include <functional>
#include <set>
#include <mutex>

struct ValueWithType;
namespace WeexCore {

enum class EagleModeReturn {
  NOT_EAGLE = 0,
  EAGLE_ONLY = 1,
  EAGLE_AND_SCRIPT = 2
};

class RenderObject;

class EagleRenderObject {
 public:
  friend class EagleBridge;

  EagleRenderObject();
  EagleRenderObject(RenderObject* render_object);
  void AddAttr(const std::string& key, const std::string& value);
  void AddStyle(const std::string& key, const std::string& value);
  void UpdateAttr(const std::string& key, const std::string& value);
  void UpdateStyle(const std::string& key, const std::string& value);
  void AddEvent(const std::string& event);
  void RemoveEvent(const std::string& event);
  void set_is_richtext_child(const bool is_richtext_child);
  std::set<std::string>* events();

  void set_page_id(const std::string& page_id);
  void ApplyDefaultStyle();
  void ApplyDefaultAttr();
  int getChildCount();
  int getChildIndex(EagleRenderObject child);
  EagleRenderObject GetChild(int index);
  EagleRenderObject parent_render();
  int AddRenderObject(int index, EagleRenderObject child);
  void RemoveRenderObject(EagleRenderObject child);
  const std::string& page_id();
  const std::string& ref();

  bool operator==(const EagleRenderObject& object) const {
    return render_object_impl_ == object.render_object_impl_;
  }
  explicit operator bool() const {
    return static_cast<bool>(render_object_impl_);
  }

 private:
  RenderObject* render_object_impl_;
};

class EagleBridge {
 public:
  class WeexCoreHandler {
   public:
    EagleRenderObject GetEagleRenderObject(const std::string& type, const std::string& ref);
    bool CreatePage(const std::string& page_id, EagleRenderObject root);
    bool HavePage(const std::string& page_id);
    bool CreateFinish(const std::string& page_id);
    bool RefreshFinish(const char* page_id, const char* task, const char* callback);
    bool ClosePage(const std::string& page_id);

    void NativeLog(const char* str_array);
    void ReportException(const char* page_id, const char* func, const char* exception_string);

    bool RemoveRenderObject(const std::string& page_id, const std::string& ref);
    bool AddRenderObject(const std::string& page_id,
                         const std::string& parent_ref,
                         int index,
                         EagleRenderObject root);
    bool MoveRenderObject(const std::string& page_id,
                          const std::string& ref,
                          const std::string& parent_ref,
                          int index);
    bool RemoveEvent(const std::string& page_id, const std::string& ref,
                     const std::string& event);
    bool AddEvent(const std::string& page_id, const std::string& ref,
                  const std::string& event);
    bool UpdateAttr(const std::string& page_id, const std::string& ref,
                    const char* data);

    bool UpdateAttr(const std::string& page_id, const std::string& ref,
                    std::vector<std::pair<std::string, std::string>>* attrPair);

    bool UpdateStyle(const std::string& page_id, const std::string& ref,
                     const char* data);

    bool UpdateStyle(const std::string& page_id, const std::string& ref,
                     std::vector<std::pair<std::string, std::string>>* stylePair);

    //vue api
    int ExecJS(const char* instanceId, const char* nameSpace,
               const char* func,
               std::vector<struct ValueWithType*>& params);

    void PostTaskToMsgLoop(const std::function<void()>& closure);

#if  OS_IOS
    void PostTaskOnComponentThread(const std::function<void()>& closure);
#endif
    void Send(const char* instance_id,
              const char* url,
              std::function<void(const std::string&, const std::string&)> callback);

    void GetBundleType(const char* instance_id,
                       const char* url,
                       std::function<void(const std::string&, const std::string&)> callback);

    //rax api
//    void RequrieMoudle(const std::string& class_name);

//    void RequrieComponent(const std::string& class_name);

    std::string CallNativeModule(const char* page_id,
                                 const char* module,
                                 const char* method,
                                 const char* arguments,
                                 int arguments_length,
                                 const char* options,
                                 int options_length);
    void CallNativeComponent(const char* page_id,
                             const char* module,
                             const char* method,
                             const char* arguments,
                             int arguments_length,
                             const char* options,
                             int options_length);

    //扩展方法. type是plugin的name, bool返回值代表switch分支是否包含当前task.
    bool CallWeexTaskFromEagle(const std::string& type,
                               const std::string& task,
                               const std::string& options);
  };

  class DataRenderHandler {
   public:
    //如果plugin不存在则ReportException.
    virtual void CreatePage(const char* instanceId,
                            const char* func,
                            const char* script,
                            int script_length,
                            const char* opts,
                            const char* initData,
                            const char* extendsApi, std::function<void(const char*, const char*)> function) = 0;

    virtual EagleModeReturn RefreshPage(const char*  page_id,
                                        const char*  init_data) = 0;

    virtual EagleModeReturn DestroyInstance(const char* instanceId) = 0;

    virtual void UpdateComponentData(const char* instanceId,
                                     const char* cid,
                                     const char* json_data) = 0;

    // 这些方法应该走Java层, 然后在plugin内部的jni处理.
    // virtual void FireEvent(const std::string &page_id, const std::string &ref, const std::string &event,const std::string &args,const std::string &dom_changes) {}
    // virtual void InvokeCallback(const std::string& page_id,
    //                           const std::string& callback_id,
    //                           const std::string& data,
    //                           bool keep_alive) {}
    // virtual void RegisterModules(const std::string &modules) {}
    // virtual void RegisterComponent(const std::string &str) {};


    //扩展方法, weex call Eagle使用, 如果是instance相关的, 那么options里应该有对应的instance id. 并且只call到对应的plugin上, 如果是全局相关的. 那么每个plugin都被call到.
    //   bool返回值代表switch分支是否包含当前task.
    virtual bool CallEagleTaskFromWeex(const char* instanceId, const char* task, const char* options) = 0;
  };

  static EagleBridge* GetInstance();

  //WeexCore调用这个方法来创建PluginPage.  如果没有对应的Plugin则内部调用ReportException.
  //Call In JSThread
  void CreatePage(const char* plugin_type, const char* instanceId, const char* func,
                  const char* script, int script_length,
                  const char* opts,
                  const char* initData,
                  const char* extendsApi, std::function<void(const char*, const char*)> function);

  //Call In JSThread
  EagleModeReturn DestroyPage(const char* instanceId);

  //Call In JSThread
  EagleModeReturn RefreshPage(const char* instanceId, const char* init_data);

  //Call In JSThread
  void UpdateComponentData(const char* instanceId,
                           const char* cid,
                           const char* json_data);

  //Call In JSThread
  bool CallEagleTaskFromWeex(const char* instanceId, const char* task, const char* options);

  //注册plugin.
  void set_data_render_handler(const std::string& plugin_type,
                               DataRenderHandler* data_render_handler);

  DataRenderHandler* get_data_render_handler(const std::string& plugin_type);

  inline WeexCoreHandler* weex_core_handler() const { return weex_core_handler_.get(); };


 private:
  EagleBridge();

  //支持多plugin. CreatePage时候根据plugin_type确定.
  //Call In JSThread
  DataRenderHandler* data_render_handler(const std::string& instance_id) const;

  static EagleBridge* g_instance;

  std::unique_ptr<WeexCoreHandler> weex_core_handler_;
  std::map<std::string, DataRenderHandler*> plugins_;
  std::map<std::string, DataRenderHandler*> pages_;
  std::mutex plugin_mutex_;
};
}

#endif
#endif
