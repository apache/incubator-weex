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
#ifndef CORE_RENDER_PAGE_REACTOR_PAGE_H_
#define CORE_RENDER_PAGE_REACTOR_PAGE_H_

#if defined __cplusplus

#include <string>
#include <map>
#include <functional>

namespace WeexCore {

void PostTaskOnComponentThread(const std::function<void()>& callback);

class RenderObject;

class ReactorPage {
public:
    explicit ReactorPage(const std::string& page_id)
    : page_id_(page_id) {}

    void CreateBody(const std::string& ref,
                    const std::string& type,
                    const std::map<std::string, std::string>& styles,
                    const std::map<std::string, std::string>& attrs,
                    const std::vector<std::string>& events);

    void AddElement(const std::string& ref,
                    const std::string& type,
                    const std::map<std::string, std::string>& styles,
                    const std::map<std::string, std::string>& attrs,
                    const std::vector<std::string>& events,
                    const std::string parent_ref, int index=-1);

    void UpdateStyles(const std::string& ref, std::vector<std::pair<std::string, std::string>> styles);

    void UpdateAttrs(const std::string& ref, std::vector<std::pair<std::string, std::string>> attrs);

    void RemoveElement(const std::string& ref);

    void CreateFinish();

    const std::string& page_id() const {return page_id_;}

    std::string CallNativeModule(const std::string& module,
                          const std::string& method,
                          const std::string& arguments,
                          size_t arguments_length,
                          const std::string& options,
                          int options_length);

    void CallNativeComponent(const std::string& ref,
                             const std::string& method,
                             const std::string& arguments,
                             size_t arguments_length,
                             const std::string& options,
                             int options_length);

private:
    RenderObject* CreateRenderObject(const std::string& ref, const std::string& type, unsigned index, const std::map<std::string, std::string>& styles, const std::map<std::string, std::string>& attrs, const std::vector<std::string>& events, bool reserve_styles, WeexCore::RenderObject* parent);

    const std::string page_id_;
};

}  // namespace WeexCore

#endif

#endif  // CORE_RENDER_PAGE_REACTOR_PAGE_H_
