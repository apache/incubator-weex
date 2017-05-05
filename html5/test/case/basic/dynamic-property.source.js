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
define('@weex-component/dynamic-property', function (require, exports, module) {

  ;module.exports.style = {}

  ;module.exports = {
    data: function () {return {
      A: {},
      B: {},
      C: {
        message: 'initial'
      },
      D: {
        message: 'initial'
      }
    }},

    created: function() {
      this.B.message = 'created'
      this.C.message = 'created'
    },

    ready: function() {
      this.A.message = 'ready'
      this.B.message = 'ready'
      this.C.message = 'ready'
    }
  }

  ;module.exports.template = {
    "type": "div",
	  "children": [
	    {
	      "type": "text",
	      "attr": {
	        "value": function () {return 'A: ' + (this.A.message)}
	      }
	    },
	    {
	      "type": "text",
	      "attr": {
	        "value": function () {return 'B: ' + (this.B.message)}
	      }
	    },
	    {
	      "type": "text",
	      "attr": {
	        "value": function () {return 'C: ' + (this.C.message)}
	      }
	    },
	    {
	      "type": "text",
	      "attr": {
	        "value": function () {return 'D: ' + (this.D.message)}
	      }
	    }
	  ]
  }

;})

bootstrap('@weex-component/dynamic-property')
