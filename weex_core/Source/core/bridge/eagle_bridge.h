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

#include <string>
#include <functional>
#include "WeexApiHeader.h"

namespace WeexCore {
    class RenderObject;
    class DataRenderHandler;
    
    class EagleRenderObject {
    public:
        EagleRenderObject(RenderObject* render_object);
        void AddAttr(std::string key, std::string value);
        void AddStyle(std::string key, std::string value);
        void AddEvent(std::string event);
        std::set<std::string> *events();
        RenderObject* render_object_impl() const {return render_object_impl_;}
        
        void set_page_id(const std::string& page_id);
        void ApplyDefaultStyle();
        void ApplyDefaultAttr();
        int AddRenderObject(int index, RenderObject *child);

    private:
        RenderObject* render_object_impl_;
    };
    

    class EagleBridge {
    public:
        class WeexCoreHandler {
        public:
            EagleRenderObject GetEagleRenderObject(const std::string &type, const std::string &ref);
            bool CreatePage(const std::string& page_id, RenderObject *root);
            bool CreateFinish(const std::string &page_id);
            bool ClosePage(const std::string &page_id);
            void ReportException(const char* page_id, const char* func, const char* exception_string);
            void Send(const char* instance_id, const char* url, std::function<void(const std::string&)> callback);
            int RefreshFinish(const char* page_id, const char* task, const char* callback);
            std::unique_ptr<ValueWithType> CallNativeModule (const char* page_id, const char* module, const char* method,const char* arguments, int arguments_length, const char* options, int options_length);
            void NativeLog(const char* str_array);
            bool RemoveRenderObject(const std::string &page_id, const std::string &ref);
            bool AddRenderObject(const std::string &page_id, const std::string &parent_ref, int index,RenderObject *root);
            bool MoveRenderObject(const std::string &page_id, const std::string &ref,const std::string &parent_ref, int index);
            bool RemoveEvent(const std::string &page_id, const std::string &ref,
                             const std::string &event);
            bool AddEvent(const std::string &page_id, const std::string &ref,
                          const std::string &event);
            bool UpdateAttr(const std::string &page_id, const std::string &ref,
                            const char *data);
            
            bool UpdateAttr(const std::string &page_id, const std::string &ref,
                            std::vector<std::pair<std::string, std::string>> *attrPair);
            
            bool UpdateStyle(const std::string &page_id, const std::string &ref,
                             const char *data);
            
            bool UpdateStyle(const std::string &page_id, const std::string &ref,
                             std::vector<std::pair<std::string, std::string>> *stylePair);
        };
        
        class DataRenderHandler {
        public:
            virtual int DestroyInstance(const char *instanceId);
            virtual void CreatePage(const std::string &input, const std::string &page_id, const std::string &options, const std::string &init_data, std::function<void(const char*)> exec_js) {}
            
            virtual void CreatePage(const char *contents, size_t length, const std::string& page_id, const std::string& options, const std::string& init_data, std::function<void(const char*)> exec_js) {}
            virtual bool RefreshPage(const std::string &page_id, const std::string &init_data) {return false;}
            virtual void UpdateComponentData(const std::string& page_id, const char* cid, const std::string& json_data) {}
        };
        
        static EagleBridge* GetInstance() {
            if (!g_instance) {
                g_instance = new EagleBridge();
            }
            return g_instance;
        }
        DataRenderHandler* data_render_handler() const {return data_render_handler_;}
        void set_data_render_handler(DataRenderHandler* data_render_handler) {data_render_handler_ = data_render_handler;}
        WeexCoreHandler* weex_core_handler() const {return weex_core_handler_;}
        

    private:
        EagleBridge() {}
        static EagleBridge *g_instance;
        DataRenderHandler* data_render_handler_;
        WeexCoreHandler* weex_core_handler_;
    };

}

#endif
