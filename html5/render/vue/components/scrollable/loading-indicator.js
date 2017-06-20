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
import { extractComponentStyle } from '../../core'

const _css = `
.weex-refresh-indicator,
.weex-loading-indicator {
  width: 1rem !important;
  height: 1rem !important;
  -webkit-box-align: center;
  -webkit-align-items: center;
      -ms-flex-align: center;
          align-items: center;
  -webkit-box-pack: center;
  -webkit-justify-content: center;
      -ms-flex-pack: center;
          justify-content: center;
  overflow: visible;
  background: none;
}
.weex-refresh-indicator:before,
.weex-loading-indicator:before {
  display: block;
  content: '';
  font-size: 0.16rem;
  width: 0.5em;
  height: 0.5em;
  left: 0;
  top: 0;
  border-radius: 50%;
  position: relative;
  text-indent: -9999em;
  -webkit-animation: weex-spinner 1.1s infinite ease;
          animation: weex-spinner 1.1s infinite ease;
}
`

export default {
  render (createElement) {
    this.weexType = 'loading-indicator'
    this._renderHook()
    return createElement('mark', {
      attrs: { 'weex-type': 'loading-indicator' },
      staticClass: 'weex-loading-indicator weex-ct',
      staticStyle: extractComponentStyle(this)
    })
  },
  _css
}
