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
define('@weex-component/modal', function (require, exports, module) {

;
  var modal = require('@weex-module/modal')
  module.exports = {
    data: function () {
      return {
        x: 'foo'
      }
    },
    created: function () {
      var message = 'Here is a message for you!'
      var okTitle = 'OK'
      modal.alert({
        message: message,
        okTitle: okTitle
      }, function() {
        this.x = 'bar'
      }.bind(this))
    }
  }


;module.exports.template = {
  "type": "div",
  "children": [
    {
      "type": "text",
      "attr": {
        "value": function () {
          return this.x
        }
      }
    }
  ]
}

;})

// require module

bootstrap('@weex-component/modal')
