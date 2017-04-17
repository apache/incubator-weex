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
define('@weex-component/id', function (require, exports, module) {

;
  module.exports = {
    ready: function () {
      if (this.$el('x')) {
        this.$el('x').setAttr('a', 1)
      }
      if (this.$el('y')) {
        this.$el('y').setAttr('a', 1)
      }
      if (this.$el('')) {
        this.$el('').setAttr('a', 1)
      }
      if (this.$el('0')) {
        this.$el('0').setAttr('a', 1)
      }
      if (this.$el(0)) {
        this.$el(0).setAttr('b', 1)
      }
    }
  }

;module.exports.template = {
  "type": "div",
  "children": [
    {
      "type": "text",
      "id": function () { return "x" },
      "attr": {
        "value": "Hello"
      }
    },
    {
      "type": "text",
      "id": function () { return 0 },
      "attr": {
        "value": "Hello"
      }
    },
    {
      "type": "text",
      "id": function () { return '' },
      "attr": {
        "value": "Hello"
      }
    },
    {
      "type": "text",
      "id": function () { return null },
      "attr": {
        "value": "Hello"
      }
    },
    {
      "type": "text",
      "id": function () { return NaN },
      "attr": {
        "value": "Hello"
      }
    },
    {
      "type": "text",
      "id": function () { return },
      "attr": {
        "value": "Hello"
      }
    }
  ]
}

;})

// require module

bootstrap('@weex-component/id')
