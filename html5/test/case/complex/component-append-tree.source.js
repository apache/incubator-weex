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

define('@weex-component/sub', function(require, exports, module) {
  
      ;
      module.exports = {
          data: function() {
              return {
                  str: '-----'
              }
          },
          ready: function() {
              this.str = 'ready'
              setTimeout(() => {
                  this.str = 'timeout'
              }, 100)
          }
      }
  
  
      ;
      module.exports.style = {}
  
      ;
      module.exports.template = {
          "type": "text",
          "attr": {
              "value": function() {
                  return this.str
              }
          }
      }
  
      ;
  })
  
  // module
  
  define('@weex-component/ad164d40f42747f598b56eccfd0eae8e', function(require, exports, module) {
  
      ;
  
      ;
      module.exports.style = {}
  
      ;
      module.exports.template = {
          "type": "scroller",
          "children": [{
              "type": "div",
              "append": "node",
              "children": [{
                  "type": "sub"
              }]
          }, {
              "type": "div",
              "append": "tree",
              "children": [{
                  "type": "sub"
              }]
          }]
      }
  
      ;
  })
  
  // require module
  bootstrap('@weex-component/ad164d40f42747f598b56eccfd0eae8e', {
      "transformerVersion": "0.4.3"
  })