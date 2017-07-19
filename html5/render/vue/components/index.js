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
import _switch from './switch'

import a from './a'
import div from './div'
import image from './image'
import input from './input'
import header from './scrollable/header'
import list from './scrollable/list'
import waterfall from './scrollable/waterfall'
import cell from './scrollable/list/cell'
import scroller from './scrollable/scroller'
import slider from './slider'
import neighbor from './slider/slider-neighbor'
// import indicator from './warning'
import indicator from './slider/indicator'
// import refresh from './warning'
// import loading from './warning'
import refresh from './scrollable/refresh'
import loading from './scrollable/loading'
import loadingIndicator from './scrollable/loading-indicator'
import text from './text'
import textarea from './textarea'
import video from './video'
import web from './web'

export default {
  a,
  div,
  container: div,
  image,
  img: image,
  input,
  switch: _switch,
  header,
  list,
  waterfall,
  cell,
  scroller,
  slider,
  cycleslider: slider,
  'slider-neighbor': neighbor,
  indicator,
  refresh,
  loading,
  'loading-indicator': loadingIndicator,
  text,
  textarea,
  video,
  web
}
