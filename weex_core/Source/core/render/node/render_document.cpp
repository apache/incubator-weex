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
// Created by furture on 2018/8/7.
//

#include <core/css/constants_name.h>
#include "render_document.h"
namespace WeexCore {

    /**
     * should set on flag on page, when have the flag fillter the page
     * */
    bool isRenderDocumentChild(RenderObject *node){
        while(node != nullptr){
            if(node->isSegmentChild()){
                return true;
            }
            if(node->type() == kRenderDocument){
                return true;
            }
            if(node->type() == kRenderCell || node->type() == kRenderList){
                return false;
            }
            if(node->is_root_render()){
                return false;
            }
            node = (RenderObject *) node->getParent();
        }
        return false;
    }

    RenderDocument::~RenderDocument() {
        if (this->documentChilds.size() > 0) {
            for (auto it = this->documentChilds.begin();
                 it != this->documentChilds.end(); ++it) {
                RenderObject *child = *it;
                if (child) {
                    delete child;
                    child = nullptr;
                }
            }
            this->documentChilds.clear();
        }
    }

    int RenderDocument::AddRenderObject(int index, RenderObject *child){
        if (child == nullptr || index < -1) {
            return index;
        }

        Index count = documentChilds.size();
        index = index >= count ? -1 : index;
        if(index < 0){
            index = count;
        }
        documentChilds.insert(documentChilds.begin() + count, child);
        child->setParent(this, child);
        child->set_parent_render(this);
        markDirty();
        return index;
    }

    StyleType RenderDocument::ApplyStyle(const std::string &key, const std::string &value,
                                         const bool updating) {
        if (key == BORDER_WIDTH) {
            MapInsertOrAssign(styles(), key, value);
            return kTypeBorder;
        } else if (key == BORDER_TOP_WIDTH) {
            MapInsertOrAssign(styles(), key, value);
            return kTypeBorder;
        } else if (key == BORDER_RIGHT_WIDTH) {
            MapInsertOrAssign(styles(), key, value);
            return kTypeBorder;
        } else if (key == BORDER_BOTTOM_WIDTH) {
            MapInsertOrAssign(styles(), key, value);
            return kTypeBorder;
        } else if (key == BORDER_LEFT_WIDTH) {
            MapInsertOrAssign(styles(), key, value);
            return kTypeBorder;
        } else if (key == PADDING) {
            MapInsertOrAssign(styles(), key, value);
            return kTypePadding;
        } else if (key == PADDING_LEFT) {
            MapInsertOrAssign(styles(), key, value);
            return kTypePadding;
        } else if (key == PADDING_TOP) {
            MapInsertOrAssign(styles(), key, value);
            return kTypePadding;
        } else if (key == PADDING_RIGHT) {
            MapInsertOrAssign(styles(), key, value);
            return kTypePadding;
        } else if (key == PADDING_BOTTOM) {
            MapInsertOrAssign(styles(), key, value);
            return kTypePadding;
        }
        return RenderObject::ApplyStyle(key, value, updating);
    }

    RenderObject* RenderDocument::GetChild(const Index &index) {
        return documentChilds[index];
    }

    void RenderDocument::RemoveRenderObject(RenderObject *child) {
        for (int index = 0; index < documentChilds.size(); index++) {
            if (child == documentChilds[index]) {
                documentChilds.erase(documentChilds.begin() + index);
                break;
            }
        }
        markDirty();
    }
}
