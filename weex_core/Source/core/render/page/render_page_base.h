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

#include <string>
#include <map>
#include <set>
#include <vector>

namespace WeexCore {
    
class RenderObject;
class RenderPerformance;
    
class RenderPageBase {
public:
    explicit RenderPageBase(const std::string& page_id, const std::string& page_type);
    virtual ~RenderPageBase();
    
    std::string page_id() { return page_id_; }
    const std::string& page_type() const { return page_type_; }
    bool is_platform_page() const { return is_platform_page_; }
    
    // DOM
    virtual bool CreateBody(const std::string& ref, const std::string& type,
                            std::map<std::string, std::string>* styles,
                            std::map<std::string, std::string>* attrs,
                            std::set<std::string>* events) { return false; }
    
    virtual bool AddRenderObject(const std::string& ref, const std::string& type,
                                 const std::string &parent_ref, int index,
                                 std::map<std::string, std::string>* styles,
                                 std::map<std::string, std::string>* attrs,
                                 std::set<std::string>* events) { return false; }
    
    virtual bool RemoveRenderObject(const std::string &ref) = 0;
    
    virtual bool MoveRenderObject(const std::string &ref, const std::string &parent_ref, int index) = 0;
    
    virtual bool UpdateStyle(const std::string &ref, std::vector<std::pair<std::string, std::string>> *src) = 0;
    
    virtual bool UpdateAttr(const std::string &ref, std::vector<std::pair<std::string, std::string>> *attrs) = 0;
    
    virtual bool AddEvent(const std::string &ref, const std::string &event) = 0;
    
    virtual bool RemoveEvent(const std::string &ref, const std::string &event) = 0;
    
    virtual bool CreateFinish() = 0;
    
    // Performance
    void CssLayoutTime(const int64_t &time);
    void ParseJsonTime(const int64_t &time);
    void CallBridgeTime(const int64_t &time);
    std::vector<int64_t> PrintFirstScreenLog();
    std::vector<int64_t> PrintRenderSuccessLog();
    
    // Life cycle
    virtual RenderObject *GetRenderObject(const std::string &ref) { return nullptr; };
    virtual void SetDefaultHeightAndWidthIntoRootRender(const float default_width, const float default_height,
                                                        const bool is_width_wrap_content, const bool is_height_wrap_content) {};
    
    virtual void OnRenderPageClose() = 0;
    
    virtual float GetViewportWidth() = 0;
    virtual void SetViewportWidth(float value) = 0;
    virtual bool GetRoundOffDeviation() = 0;
    virtual void SetRoundOffDeviation(bool value) = 0;
    
protected:
    bool is_platform_page_;
    std::string page_id_;
    std::string page_type_;
    
    RenderPerformance *render_performance_;
};
    
}  // namespace WeexCore
