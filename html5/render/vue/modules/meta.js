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

const meta = {
  /**
   * setViewport
   * Changing viewport width at runtime is not supported. Please use weex-viewport meta
   * tag to specify your viewport in your html file.
   */
  setViewport (options) {
    console.warn(`[vue-render] meta.setViewport doesn't works as expected in web platform.`
    + ` Please use <meta name="weex-viewport" content="xxx"> to specify your viewport width.`)
  }
}

export default {
  init (weex) {
    weex.registerModule('meta', meta)
  }
}
