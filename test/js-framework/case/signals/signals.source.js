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
define('@weex-component/signals-sub', function (require, exports, module) {
  module.exports = {
    data: function () {return {
      thumb: '', title: ''
    }},
    style: {},
    template: {
      type: 'div',
      children: [
        {type: 'image', attr: {src: function () {return this.thumb}}},
        {type: 'text', attr: {value: function () {return this.title}}}
      ]
    }
  }
})

define('@weex-component/signals', function (require, exports, module) {
  module.exports = {
    data: function () {return {
      list: [
        {thumb: '<some image url 1>', title: 'Hello World 1'},
        {thumb: '<some image url 2>', title: 'Hello World 2'},
        {thumb: '<some image url 3>', title: 'Hello World 3'},
        {thumb: '<some image url 4>', title: 'Hello World 4'},
        {thumb: '<some image url 5>', title: 'Hello World 5'},
        {thumb: '<some image url 6>', title: 'Hello World 6'},
        {thumb: '<some image url 7>', title: 'Hello World 7'},
        {thumb: '<some image url 8>', title: 'Hello World 8'},
        {thumb: '<some image url 9>', title: 'Hello World 9'},
        {thumb: '<some image url 10>', title: 'Hello World 10'},
        {thumb: '<some image url 11>', title: 'Hello World 11'},
        {thumb: '<some image url 12>', title: 'Hello World 12'},
        {thumb: '<some image url 13>', title: 'Hello World 13'},
        {thumb: '<some image url 14>', title: 'Hello World 14'},
        {thumb: '<some image url 15>', title: 'Hello World 15'},
        {thumb: '<some image url 16>', title: 'Hello World 16'},
        {thumb: '<some image url 17>', title: 'Hello World 17'},
        {thumb: '<some image url 18>', title: 'Hello World 18'},
        {thumb: '<some image url 19>', title: 'Hello World 19'},
        {thumb: '<some image url 20>', title: 'Hello World 20'}
      ]
    }},
    style: {},
    template: {
      type: 'div',
      children: [
        {
          type: 'signals-sub',
          repeat: {
            expression: function () {return this.list},
            key: 'i',
            value: 'item'
          },
          attr: {
            thumb: function () {return this.item.thumb},
            title: function () {return this.item.title}
          }
        }
      ]
    }
  }
})

// // require module

bootstrap('@weex-component/signals')
