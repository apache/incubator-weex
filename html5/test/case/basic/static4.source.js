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
function otherNameFunction () {
  var module = {}
  var exports = {}
  module.exports = exports

  ;module.exports.style = {}

  ;module.exports.template = {
    "type": "div",
	  "children": [
	    {
	      "type": "div",
	      "repeat": {
	        "expression": function () {return this.outer},
	        "value": "x"
	      },
	      "children": [
	        {
	          "type": "text",
	          "attr": {
	            "value": function () {return this.x}
	          }
	        }
	      ]
	    }
	  ]
  }

  ;module.exports.data = function() {return {
    outer: null
  }}

  return module.exports
}

// module

define('@weex-component/static4', function (require, exports, module) {

;
  module.exports = {
    "components": {
      innertpl: otherNameFunction()
    },
    ready: function() {
      this.arr.$set(2, 3)
    },
    data: function () {return {
      arr: [1, 2]
    }}
  }


;module.exports.style = {}

;module.exports.template = {
  "type": "div",
  "attr": {
    "static": ""
  },
  "children": [
    {
      "type": "innertpl",
      "attr": {
        "outer": function () {return this.arr}
      }
    }
  ]
}

;})

// require module

bootstrap('@weex-component/static4')
