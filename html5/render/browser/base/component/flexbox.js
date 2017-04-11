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
'use strict'

const boxAlignMap = {
  stretch: 'stretch',
  'flex-start': 'start',
  'flex-end': 'end',
  center: 'center'
}

const boxOrientMap = {
  row: 'horizontal',
  column: 'vertical'
}

const boxPackMap = {
  'flex-start': 'start',
  'flex-end': 'end',
  center: 'center',
  'space-between': 'justify',
  'space-around': 'justify' // Just same as `space-between`
}

export default {
  style: {
    flex (value) {
      this.node.style.webkitBoxFlex = value
      this.node.style.webkitFlex = value
      this.node.style.flex = value
    },

    alignItems (value) {
      this.node.style.webkitBoxAlign = boxAlignMap[value]
      this.node.style.webkitAlignItems = value
      this.node.style.alignItems = value
    },

    alignSelf (value) {
      this.node.style.webkitAlignSelf = value
      this.node.style.alignSelf = value
    },

    flexDirection (value) {
      this.node.style.webkitBoxOrient = boxOrientMap[value]
      this.node.style.webkitFlexDirection = value
      this.node.style.flexDirection = value
    },

    justifyContent (value) {
      this.node.style.webkitBoxPack = boxPackMap[value]
      this.node.style.webkitJustifyContent = value
      this.node.style.justifyContent = value
    }
  }
}

/* eslint-disable */
;
/* eslint-enable */
