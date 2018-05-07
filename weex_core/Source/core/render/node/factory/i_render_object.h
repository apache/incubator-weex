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
#ifndef WEEX_PROJECT_IRENDEROBJECT_H
#define WEEX_PROJECT_IRENDEROBJECT_H

#include <string>
#include <core/layout/layout.h>

namespace WeexCore {

  class IRenderObject : public WXCoreLayoutNode {
  public:
    inline void SetRef(std::string ref) {
      mRef = ref;
    }

    inline const std::string &Ref() const {
      return mRef;
    }

    inline void SetPageId(std::string pageId) {
      this->mPageId = pageId;
    }

    inline const std::string &PageId() const {
      return mPageId;
    }

    inline void SetType(std::string type) {
      mType = type;
    }

    inline const std::string &Type() const {
      return mType;
    }

    void copyFrom(IRenderObject* src){
        WXCoreLayoutNode::copyFrom(src);
        SetRef(src->Ref());
        SetPageId(src->PageId());
        SetType(src->Type());
    }

  private:
    std::string mPageId = "";
    std::string mRef = "";
    std::string mType = "";
  };
}

#endif //WEEX_PROJECT_IRENDEROBJECT_H
