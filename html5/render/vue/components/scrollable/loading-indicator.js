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
  width: 1.0rem;
  height: 1.0rem;
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
  width: 1em;
  height: 1em;
  left: -60%;
  top: 40%;
  border-radius: 50%;
  position: relative;
  text-indent: -9999em;
  -webkit-animation: weex-spinner 1.1s infinite ease;
          animation: weex-spinner 1.1s infinite ease;
  -webkit-transform: translate3d(1.0rem, 0, 0);
          transform: translate3d(1.0rem, 0, 0);
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
