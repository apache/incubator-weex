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

define('@weex-component/bar1', function(require, exports, module) {;;
  module.exports.template = {
    type: 'container',
    children: [{
      type: 'text',
      attr: {
        value: 'bar1'
      }
    }, {
      type: 'bar2',
      id: 'bar2',
      component: true
    }]
  }

  ;
  module.exports.methods = {
    ready: function(isRefresh) {
      if (isRefresh) {
        this.$vm('bar2').ready(true)
      }
    }
  }

})

define('@weex-component/bar2', function(require, exports, module) {;;
  module.exports.template = {
    type: 'container',
    children: [{
      type: 'text',
      attr: {
        value: function() {
          return this.text
        }
      }
    }]
  }

  ;
  module.exports.data = function() {
    return {
      text: 'bar2'
    }
  }

  ;
  module.exports.methods = {
    ready: function(isRefresh) {
      if (isRefresh) {
        this.text = 'bar2-refresh'
      }
    }
  }


})

define('@weex-component/foo', function(require, exports, module) {

  ;
  module.exports.style = {}

  ;
  module.exports.template = {
    type: 'container',
    children: [{
      type: 'bar1',
      shown: function() {
        return this.showbar1
      },
      id: 'bar1',
      component: true
    }]
  }

  ;
  module.exports.data = function() {
    return {
      showbar1: false,
      ext: {
        showbar1: false
      }
    }
  }

  ;module.exports.methods = {
    refreshData: function(data) {
      for (var key in data) {
        this[key] = data[key]
      }
      this.ready(true)
    },
    ready: function(isRefresh) {
      var self = this
      this.showbar1 = this.ext.showbar1

      if (isRefresh) {
        if (this.showbar1) {
          this.$renderThen(() => {
            this.$vm('bar1').$on('hook:ready', function() {
              self.$vm('bar1').ready(true)
            })
          })
        }
      }
    }
  }

})

// require module

bootstrap('@weex-component/foo')
