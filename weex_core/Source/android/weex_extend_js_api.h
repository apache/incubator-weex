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
#ifndef _EXTEND_JSAPI_H_
#define _EXTEND_JSAPI_H_

#include <jni.h>

namespace WeexCore {

  typedef const char *(*FunType)(const char *, int, const char *);

 __attribute__((visibility ("default")))  const char *CallGCanvasFun(const char *conextId, int x, const char *args);

  extern "C" void Inject_GCanvasFunc(FunType fp);

  typedef const char* (*FunTypeT3d)(int, const char*);

  __attribute__((visibility ("default"))) const char* CallT3dFunc(int x, const char* args);

  extern "C" void Inject_T3dFunc(FunTypeT3d fp);

} //WeexCore

#endif // _EXTENSION_JSAPI_H_
