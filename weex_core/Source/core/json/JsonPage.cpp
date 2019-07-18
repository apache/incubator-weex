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
// Created by furture on 2019/4/29.
//

#include "JsonPage.h"
#include "base/log_defines.h"
#include "core/render/node/render_object.h"
#include "core/render/manager/render_manager.h"
#include "core/render/node/factory/render_creator.h"
#include <string>
#include <map>
#include "core/render/target/render_target.h"

namespace WeexCore {


    static std::string getString(const json11::Json& json) {
        if (json.is_string()) {
            return json.string_value();
        }
        else if (json.is_bool()) {
            return std::to_string(json.bool_value());
        }
        else if (json.is_number()) {
            return std::to_string(json.number_value());
        }
        else if (json.is_null()) {
            return "";
        }
        return json.dump();
    }

    static std::string getString(const json11::Json& json, const std::string& key) {
        return getString(json[key]);
    }

    static std::map<std::string, std::string> getMap(const json11::Json& json, const std::string& key){
        json11::Json kvs  = json[key];
        std::map<std::string, std::string> map;
        if(!kvs.is_object()){
            return map;
        }
        std::map<std::string, json11::Json> items = kvs.object_items();
        for(auto it = items.begin(); it != items.end(); it++) {
            map.insert({it->first, getString(it->second)});
        }
        return map;
    }

//    static std::vector<std::string> getVector(const json11::Json& json, const std::string& key){
//        json11::Json kvs  = json[key];
//        std::vector<std::string> values;
//        if(!kvs.is_array()){
//            return values;
//        }
//        const std::vector<json11::Json>& items = kvs.array_items();
//        for(auto it = items.begin(); it != items.end(); it++) {
//            values.push_back(getString(*it));
//        }
//        return values;
//    }

    static std::set<std::string> getSet(const json11::Json& json, const std::string& key){
        json11::Json kvs  = json[key];
        std::set<std::string> values;
        if(!kvs.is_array()){
            return values;
        }
        const std::vector<json11::Json>& items = kvs.array_items();
        for(auto it = items.begin(); it != items.end(); it++) {
            values.insert(getString(*it));
        }
        return values;
    }


    static RenderObject* toRenderObject(const std::string& pageId,
                                        const std::string& renderType,
                                        const std::string& ref,
                                        const std::map<std::string, std::string>& style,
                                        const std::map<std::string, std::string>& attr,
                                        const std::set<std::string>& event){
        RenderObject* render = (RenderObject*) RenderCreator::GetInstance()->CreateRender(renderType, ref);
        render->set_page_id(pageId);

        for(auto it=style.begin(); it != style.end(); it++){
            render->AddStyle(it->first, it->second, false);
        }

        for(auto it=attr.begin(); it != attr.end(); it++){
            render->AddAttr(it->first, it->second);
        }

        for(auto it=event.begin(); it != event.end(); it++){
            render->AddEvent(*it);
        }

        render->ApplyDefaultStyle(false);
        render->ApplyDefaultAttr();
        return render;
    }


    JsonPage::JsonPage(const std::string &pageId) :mPageId(pageId), mRenderPageCustom(nullptr) {
        std::string targetName = RenderTargetManager::getRenderTargetName(pageId);
        if (!targetName.empty()) {
            if (RenderTargetManager::sharedInstance()->getAvailableTargetNames().count(targetName) == 0) {
                // cannot find the target, degress
                targetName = "";
            }
        }
        if (!targetName.empty()) {
            mRenderPageCustom = RenderManager::GetInstance()->CreateCustomPage(pageId, targetName);
        }else{
            mRenderPageCustom = nullptr;
        }
    }


    void JsonPage::parseJson(const std::string &data) {
        std::string error;
        json11::Json json = json11::Json::parse(data, error);
        if(error.length() > 0){
            LOGE("JsonPage::parseJson %s", error.data());
        }
        LOGE("JsonPage::parseJson success");
        sendChildren(json, "", 0, true);
        RenderManager::GetInstance()->CreateFinish(mPageId);
    }


    void JsonPage::sendChildren(const json11::Json json, const std::string &parent_ref, int index, bool body) {
        const std::string ref = getString(json, "ref");
        const std::string type = getString(json, "type");
        std::map<std::string, std::string> attr = getMap(json, "attr");
        std::map<std::string, std::string> style = getMap(json, "style");
        std::set<std::string> event = getSet(json, "event");
        if(mRenderPageCustom == nullptr){
            RenderObject* renderObject = toRenderObject(mPageId, type, ref, style, attr, event);
            if(body){
                RenderManager::GetInstance()->CreatePage(mPageId, renderObject);
            }else{
                RenderManager::GetInstance()->AddRenderObject(mPageId, parent_ref, index, renderObject);
            }
        }else{
            std::map<std::string, std::string>* attrPtr = new std::map<std::string,std::string>(std::move(attr));
            std::map<std::string, std::string>* stylePtr = new std::map<std::string,std::string>(std::move(style));
            std::set<std::string>* eventPtr = new std::set<std::string>(std::move(event));
            if(body){
                mRenderPageCustom->CreateBody(ref,type, stylePtr, attrPtr, eventPtr);
            }else{
                mRenderPageCustom->AddRenderObject(ref, type, parent_ref, index, stylePtr, attrPtr, eventPtr);
            }
            //delete attrPtr;
            //delete stylePtr;
            //delete eventPtr;
        }
        const json11::Json children = json["children"];
        if(children.is_array()){
            const std::vector<json11::Json>& items = children.array_items();
            int  itemIndex = 0;
            for(const json11::Json& item : items){
                sendChildren(item, ref, itemIndex, false);
                itemIndex++;
            }
        }
    }






}
