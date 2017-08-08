/* 'Weex Vanilla Framework 0.21.2, Build 2017-08-08 11:30. */

(function (global, factory) {
  typeof exports === 'object' && typeof module !== 'undefined' ? factory(exports) :
  typeof define === 'function' && define.amd ? define(['exports'], factory) :
  (factory((global.WeexVanillaFramework = global.WeexVanillaFramework || {})));
}(this, (function (exports) { 'use strict';

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
var config = {};

var instanceMap = {};

function init (cfg) {
  config.Document = cfg.Document;
  config.Element = cfg.Element;
  config.Comment = cfg.Comment;
  config.sendTasks = cfg.sendTasks;
}

function createInstance (id, code, options, data, serviceObjects) {
  if ( options === void 0 ) options = {};
  if ( data === void 0 ) data = {};
  if ( serviceObjects === void 0 ) serviceObjects = {};

  var document = new config.Document(id, options.bundleUrl);
  var callbacks = {};
  var instance = { id: id, data: data, document: document, callbacks: callbacks };

  var lastCallbackId = 0;
  document.addCallback = function (func) {
    lastCallbackId++;
    callbacks[lastCallbackId] = func;
    return lastCallbackId
  };
  document.handleCallback = function (funcId, data, ifLast) {
    var callback = callbacks[funcId];
    if (ifLast) {
      delete callbacks[funcId];
    }
    return callback(data)
  };
  instanceMap[id] = instance;

  var globalObjects = Object.assign({
    Document: config.Document,
    Element: config.Element,
    Comment: config.Comment,
    sendTasks: function (tasks) { return config.sendTasks(id, tasks, -1); },
    options: options,
    document: document
  }, serviceObjects);

  var globalKeys = [];
  var globalValues = [];
  for (var key in globalObjects) {
    globalKeys.push(key);
    globalValues.push(globalObjects[key]);
  }
  globalKeys.push(code);

  var result = new (Function.prototype.bind.apply( Function, [ null ].concat( globalKeys) ));
  result.apply(void 0, globalValues);

  config.sendTasks(id, [{ module: 'dom', method: 'createFinish', args: [] }], -1);

  return instance
}

function destroyInstance (id) {
  delete instanceMap[id];
}

function getRoot (id) {
  return instanceMap[id].document.body.toJSON()
}

function receiveTasks (id, tasks) {
  var jsHandlers = {
    fireEvent: function (id, ref, type, data, domChanges) {
      var ref$1 = instanceMap[id];
      var document = ref$1.document;
      var el = document.getRef(ref);
      return document.fireEvent(el, type, data, domChanges)
    },

    callback: function (id, funcId, data, ifLast) {
      var ref = instanceMap[id];
      var document = ref.document;
      return document.handleCallback(funcId, data, ifLast)
    }
  };

  var ref = instanceMap[id] || {};
  var document = ref.document;
  if (document && Array.isArray(tasks)) {
    var results = [];
    tasks.forEach(function (task) {
      var handler = jsHandlers[task.method];
      var args = [].concat( task.args );
      if (typeof handler === 'function') {
        args.unshift(id);
        results.push(handler.apply(void 0, args));
      }
    });
    return results
  }
}

exports.init = init;
exports.createInstance = createInstance;
exports.destroyInstance = destroyInstance;
exports.getRoot = getRoot;
exports.receiveTasks = receiveTasks;

Object.defineProperty(exports, '__esModule', { value: true });

})));
