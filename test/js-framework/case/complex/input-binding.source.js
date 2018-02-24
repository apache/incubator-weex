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
define('@weex-component/d6b20355da35c77d07f0a11ec5256f42', function(require, exports, module) {

    ;
    module.exports = {
        data: function() {
            return {
                x: 'abcde',
                changes: []
            }
        },
        methods: {
            change: function(e) {
                var value = e.target.attr.value
                this.x = value
                this.changes.push(value)
            }
        }
    }


    ;
    module.exports.style = {}

    ;
    module.exports.template = {
        "type": "div",
        "children": [{
            "type": "input",
            "attr": {
                "value": function() {
                    return this.x
                }
            },
            "events": {
                "change": "change"
            }
        }, {
            "type": "text",
            "repeat": {
                "expression": function() {
                    return this.changes
                },
                "key": "k",
                "value": "v"
            },
            "attr": {
                "value": function() {
                    return (this.k + 1) + ' - ' + (this.v)
                }
            }
        }]
    }

    ;
})

// require module
bootstrap('@weex-component/d6b20355da35c77d07f0a11ec5256f42', {
    "transformerVersion": "0.3.1"
})
