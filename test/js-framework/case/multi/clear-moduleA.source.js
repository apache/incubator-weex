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

define('@weex-component/moduleA', function(require, exports, module) {

  define('sample', function(require, exports, module) {
    module.exports = { name: 'sample' }
  })

  ;module.exports = {
    data: {
      text: ''
    },
    methods: {
      update: function() {
        var outer = require('sample')
        this.text = outer ? outer.name : 'nothing'
      }
    }
  }


  ;module.exports.style = {}

  ;module.exports.template = {
	  "type": "div",
	  "children": [
	    {
	      "type": "text",
    	  "events": {
    	    "click": "update"
    	  },
	      "attr": {
	        "value": function() { return this.text }
	      }
	    }
	  ]
  }

})

// require module
bootstrap('@weex-component/moduleA', {
    "transformerVersion": "0.3.1"
})
