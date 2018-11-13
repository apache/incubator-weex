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
#ifndef CORE_RENDER_NODE_FACTORY_RENDER_CREATOR_H_
#define CORE_RENDER_NODE_FACTORY_RENDER_CREATOR_H_

#include <string>
#include <map>

namespace WeexCore {

class IRenderObject;
class IRenderFactory;

class RenderCreator {
 private:
  RenderCreator() {}

  ~RenderCreator() {}

  // just to release singleton object
  class Garbo {
   public:
    ~Garbo() {
      if (RenderCreator::g_pInstance) {
        delete RenderCreator::g_pInstance;
      }
    }
  };

  static Garbo garbo;

 public:
  static RenderCreator *GetInstance() {
    if (!g_pInstance) {
      g_pInstance = new RenderCreator();
    }
    return g_pInstance;
  }

  IRenderFactory *CreateFactory(const std::string &type);
  IRenderObject *CreateRender(const std::string &type, const std::string &ref);
  
  void RegisterAffineType(const std::string &type, const std::string& asType);
  bool IsAffineType(const std::string &type, const std::string& asType);

 private:
  static RenderCreator *g_pInstance;
  
  std::map<std::string, std::string> affineTypes_; // affineTypes_[A] = B means A is rendered like B
};
}  // namespace WeexCore

#endif  // CORE_RENDER_NODE_FACTORY_RENDER_CREATOR_H_
