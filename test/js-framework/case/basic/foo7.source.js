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
define('@weex-component/foo7', function (require, exports, module) {

;
  module.exports = {
    methods: {
      gotoDetail: function () {
        this.$openURL('https://item.taobao.com/item.htm?id=520421163634')
      }
    }
  }


;module.exports.style = {
  "thumb": {
    "width": 200,
    "height": 200
  },
  "title": {
    "flex": 1,
    "color": "#ff0000",
    "fontSize": 48,
    "fontWeight": "bold",
    "backgroundColor": "#eeeeee"
  }
}

;module.exports.template = {
  "type": "container",
  "events": {
    "click": "gotoDetail"
  },
  "children": [
    {
      "type": "image",
      "classList": [
        "thumb"
      ],
      "attr": {
        "src": "https://gd2.alicdn.com/bao/uploaded/i2/T14H1LFwBcXXXXXXXX_!!0-item_pic.jpg"
      }
    },
    {
      "type": "text",
      "classList": [
        "title"
      ],
      "attr": {
        "value": "一个超赞的宝贝标题"
      }
    }
  ]
}

;})

// require module

bootstrap('@weex-component/foo7')
