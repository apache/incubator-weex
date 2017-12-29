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
define('@weex-component/computed', function (require, exports, module) {

;
  module.exports = {
    data: function () {
      return {x: 1, y: 2}
    },
    computed: {
      n: function () {
        return this.x + this.y
      },
      m: {
        get: function () {
          return this.x + this.y
        },
        set: function (v) {
          this.y = v
        }
      }
    },
    methods: {
      foo: function () {
        this.m = 10
      }
    }
  }


;module.exports.template = {
  "type": "container",
  "events": {
    "click": "foo"
  },
  "children": [
    {
      "type": "text",
      "attr": {
        "value": function () {return this.n}
      }
    },
    {
      "type": "text",
      "attr": {
        "value": function () {return this.m}
      }
    }
  ]
}

;})

// require module

bootstrap('@weex-component/computed')
