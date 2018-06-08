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
#ifndef BASE_COMMON_H_
#define BASE_COMMON_H_

// Should be used in the private: declarations to define a class to be
// uncopyable.
#define DISALLOW_COPY(TypeName) TypeName(const TypeName&)

// Should be used in the private: declarations to define a class to be
// unssignable.
#define DISALLOW_ASSIGN(TypeName) void operator=(const TypeName&)

// Should be used in the private: declarations to define a class to be
// uncopyable and unssignable.
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

#endif  // BASE_COMMON_H_
