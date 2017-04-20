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
define('@weex-component/04f1048c50eca3454bc70de93d3213d4', function(require, exports, module) {

    ;
    module.exports = {
        data: function() {
            return {
                list: [{
                    text: 1
                }, {
                    text: 2
                }],
                more: [{
                    text: 3
                }]
            }
        },
        ready: function() {
            for (var i = 0; i < this.more.length; i++) {
                this.list.push(this.more[i]);
            }
        }
    }


    ;
    module.exports.style = {}

    ;
    module.exports.template = {
        "type": "list",
        "children": [{
            "type": "cell",
            "append": "tree",
            "repeat": function() {
                return this.list
            },
            "children": [{
                "type": "text",
                "style": {
                    "height": 600
                },
                "attr": {
                    "value": function() {
                        return this.text
                    }
                }
            }]
        }]
    }

    ;
})

// require module
bootstrap('@weex-component/04f1048c50eca3454bc70de93d3213d4', {
    "transformerVersion": "0.3.1"
})
