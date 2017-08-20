/*
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
package com.taobao.weex.adapter;

/**
 * Interface for loading library.
 */
public interface IWXSoLoaderAdapter {
  /**
   * A method <code>doLoadLibrary</code> that
   * helps embedders to load a shared library with a short name.
   * <p>
   *
   * Embedders would have a chance to take charge of library loading,
   * they could load libraries with different class loaders,
   * or load libraries from specified library pathes.
   *
   * @param shortName the name of the library
   */
  void doLoadLibrary(String shortName);

  /**
   * A method <code>doLoad</code> that
   * helps embedders to load a shared library.
   * <p>
   *
   * Embedders would have a chance to take charge of library loading,
   * they could load libraries with different class loaders,
   * or load libraries from specified library pathes.
   *
   * @param name the file to load.
   */
  void doLoad(String name);
}
