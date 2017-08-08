/* Weex Vanilla Framework 0.21.7, Build 2017-08-08 11:59. */

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
//# sourceMappingURL=data:application/json;charset=utf-8;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjpudWxsLCJzb3VyY2VzIjpbIi4uLy4uL2h0bWw1L2ZyYW1ld29ya3MvdmFuaWxsYS9pbmRleC5qcyJdLCJzb3VyY2VzQ29udGVudCI6WyIvKlxuICogTGljZW5zZWQgdG8gdGhlIEFwYWNoZSBTb2Z0d2FyZSBGb3VuZGF0aW9uIChBU0YpIHVuZGVyIG9uZVxuICogb3IgbW9yZSBjb250cmlidXRvciBsaWNlbnNlIGFncmVlbWVudHMuICBTZWUgdGhlIE5PVElDRSBmaWxlXG4gKiBkaXN0cmlidXRlZCB3aXRoIHRoaXMgd29yayBmb3IgYWRkaXRpb25hbCBpbmZvcm1hdGlvblxuICogcmVnYXJkaW5nIGNvcHlyaWdodCBvd25lcnNoaXAuICBUaGUgQVNGIGxpY2Vuc2VzIHRoaXMgZmlsZVxuICogdG8geW91IHVuZGVyIHRoZSBBcGFjaGUgTGljZW5zZSwgVmVyc2lvbiAyLjAgKHRoZVxuICogXCJMaWNlbnNlXCIpOyB5b3UgbWF5IG5vdCB1c2UgdGhpcyBmaWxlIGV4Y2VwdCBpbiBjb21wbGlhbmNlXG4gKiB3aXRoIHRoZSBMaWNlbnNlLiAgWW91IG1heSBvYnRhaW4gYSBjb3B5IG9mIHRoZSBMaWNlbnNlIGF0XG4gKlxuICogICBodHRwOi8vd3d3LmFwYWNoZS5vcmcvbGljZW5zZXMvTElDRU5TRS0yLjBcbiAqXG4gKiBVbmxlc3MgcmVxdWlyZWQgYnkgYXBwbGljYWJsZSBsYXcgb3IgYWdyZWVkIHRvIGluIHdyaXRpbmcsXG4gKiBzb2Z0d2FyZSBkaXN0cmlidXRlZCB1bmRlciB0aGUgTGljZW5zZSBpcyBkaXN0cmlidXRlZCBvbiBhblxuICogXCJBUyBJU1wiIEJBU0lTLCBXSVRIT1VUIFdBUlJBTlRJRVMgT1IgQ09ORElUSU9OUyBPRiBBTllcbiAqIEtJTkQsIGVpdGhlciBleHByZXNzIG9yIGltcGxpZWQuICBTZWUgdGhlIExpY2Vuc2UgZm9yIHRoZVxuICogc3BlY2lmaWMgbGFuZ3VhZ2UgZ292ZXJuaW5nIHBlcm1pc3Npb25zIGFuZCBsaW1pdGF0aW9uc1xuICogdW5kZXIgdGhlIExpY2Vuc2UuXG4gKi9cbmNvbnN0IGNvbmZpZyA9IHt9XG5cbmNvbnN0IGluc3RhbmNlTWFwID0ge31cblxuZXhwb3J0IGZ1bmN0aW9uIGluaXQgKGNmZykge1xuICBjb25maWcuRG9jdW1lbnQgPSBjZmcuRG9jdW1lbnRcbiAgY29uZmlnLkVsZW1lbnQgPSBjZmcuRWxlbWVudFxuICBjb25maWcuQ29tbWVudCA9IGNmZy5Db21tZW50XG4gIGNvbmZpZy5zZW5kVGFza3MgPSBjZmcuc2VuZFRhc2tzXG59XG5cbmV4cG9ydCBmdW5jdGlvbiBjcmVhdGVJbnN0YW5jZSAoaWQsIGNvZGUsIG9wdGlvbnMgPSB7fSwgZGF0YSA9IHt9LCBzZXJ2aWNlT2JqZWN0cyA9IHt9KSB7XG4gIGNvbnN0IGRvY3VtZW50ID0gbmV3IGNvbmZpZy5Eb2N1bWVudChpZCwgb3B0aW9ucy5idW5kbGVVcmwpXG4gIGNvbnN0IGNhbGxiYWNrcyA9IHt9XG4gIGNvbnN0IGluc3RhbmNlID0geyBpZCwgZGF0YSwgZG9jdW1lbnQsIGNhbGxiYWNrcyB9XG5cbiAgbGV0IGxhc3RDYWxsYmFja0lkID0gMFxuICBkb2N1bWVudC5hZGRDYWxsYmFjayA9IGZ1bmMgPT4ge1xuICAgIGxhc3RDYWxsYmFja0lkKytcbiAgICBjYWxsYmFja3NbbGFzdENhbGxiYWNrSWRdID0gZnVuY1xuICAgIHJldHVybiBsYXN0Q2FsbGJhY2tJZFxuICB9XG4gIGRvY3VtZW50LmhhbmRsZUNhbGxiYWNrID0gKGZ1bmNJZCwgZGF0YSwgaWZMYXN0KSA9PiB7XG4gICAgY29uc3QgY2FsbGJhY2sgPSBjYWxsYmFja3NbZnVuY0lkXVxuICAgIGlmIChpZkxhc3QpIHtcbiAgICAgIGRlbGV0ZSBjYWxsYmFja3NbZnVuY0lkXVxuICAgIH1cbiAgICByZXR1cm4gY2FsbGJhY2soZGF0YSlcbiAgfVxuICBpbnN0YW5jZU1hcFtpZF0gPSBpbnN0YW5jZVxuXG4gIGNvbnN0IGdsb2JhbE9iamVjdHMgPSBPYmplY3QuYXNzaWduKHtcbiAgICBEb2N1bWVudDogY29uZmlnLkRvY3VtZW50LFxuICAgIEVsZW1lbnQ6IGNvbmZpZy5FbGVtZW50LFxuICAgIENvbW1lbnQ6IGNvbmZpZy5Db21tZW50LFxuICAgIHNlbmRUYXNrczogdGFza3MgPT4gY29uZmlnLnNlbmRUYXNrcyhpZCwgdGFza3MsIC0xKSxcbiAgICBvcHRpb25zLFxuICAgIGRvY3VtZW50XG4gIH0sIHNlcnZpY2VPYmplY3RzKVxuXG4gIGNvbnN0IGdsb2JhbEtleXMgPSBbXVxuICBjb25zdCBnbG9iYWxWYWx1ZXMgPSBbXVxuICBmb3IgKGNvbnN0IGtleSBpbiBnbG9iYWxPYmplY3RzKSB7XG4gICAgZ2xvYmFsS2V5cy5wdXNoKGtleSlcbiAgICBnbG9iYWxWYWx1ZXMucHVzaChnbG9iYWxPYmplY3RzW2tleV0pXG4gIH1cbiAgZ2xvYmFsS2V5cy5wdXNoKGNvZGUpXG5cbiAgY29uc3QgcmVzdWx0ID0gbmV3IEZ1bmN0aW9uKC4uLmdsb2JhbEtleXMpXG4gIHJlc3VsdCguLi5nbG9iYWxWYWx1ZXMpXG5cbiAgY29uZmlnLnNlbmRUYXNrcyhpZCwgW3sgbW9kdWxlOiAnZG9tJywgbWV0aG9kOiAnY3JlYXRlRmluaXNoJywgYXJnczogW10gfV0sIC0xKVxuXG4gIHJldHVybiBpbnN0YW5jZVxufVxuXG5leHBvcnQgZnVuY3Rpb24gZGVzdHJveUluc3RhbmNlIChpZCkge1xuICBkZWxldGUgaW5zdGFuY2VNYXBbaWRdXG59XG5cbmV4cG9ydCBmdW5jdGlvbiBnZXRSb290IChpZCkge1xuICByZXR1cm4gaW5zdGFuY2VNYXBbaWRdLmRvY3VtZW50LmJvZHkudG9KU09OKClcbn1cblxuZXhwb3J0IGZ1bmN0aW9uIHJlY2VpdmVUYXNrcyAoaWQsIHRhc2tzKSB7XG4gIGNvbnN0IGpzSGFuZGxlcnMgPSB7XG4gICAgZmlyZUV2ZW50OiAoaWQsIHJlZiwgdHlwZSwgZGF0YSwgZG9tQ2hhbmdlcykgPT4ge1xuICAgICAgY29uc3QgeyBkb2N1bWVudCB9ID0gaW5zdGFuY2VNYXBbaWRdXG4gICAgICBjb25zdCBlbCA9IGRvY3VtZW50LmdldFJlZihyZWYpXG4gICAgICByZXR1cm4gZG9jdW1lbnQuZmlyZUV2ZW50KGVsLCB0eXBlLCBkYXRhLCBkb21DaGFuZ2VzKVxuICAgIH0sXG5cbiAgICBjYWxsYmFjazogKGlkLCBmdW5jSWQsIGRhdGEsIGlmTGFzdCkgPT4ge1xuICAgICAgY29uc3QgeyBkb2N1bWVudCB9ID0gaW5zdGFuY2VNYXBbaWRdXG4gICAgICByZXR1cm4gZG9jdW1lbnQuaGFuZGxlQ2FsbGJhY2soZnVuY0lkLCBkYXRhLCBpZkxhc3QpXG4gICAgfVxuICB9XG5cbiAgY29uc3QgeyBkb2N1bWVudCB9ID0gaW5zdGFuY2VNYXBbaWRdIHx8IHt9XG4gIGlmIChkb2N1bWVudCAmJiBBcnJheS5pc0FycmF5KHRhc2tzKSkge1xuICAgIGNvbnN0IHJlc3VsdHMgPSBbXVxuICAgIHRhc2tzLmZvckVhY2goKHRhc2spID0+IHtcbiAgICAgIGNvbnN0IGhhbmRsZXIgPSBqc0hhbmRsZXJzW3Rhc2subWV0aG9kXVxuICAgICAgY29uc3QgYXJncyA9IFsuLi50YXNrLmFyZ3NdXG4gICAgICBpZiAodHlwZW9mIGhhbmRsZXIgPT09ICdmdW5jdGlvbicpIHtcbiAgICAgICAgYXJncy51bnNoaWZ0KGlkKVxuICAgICAgICByZXN1bHRzLnB1c2goaGFuZGxlciguLi5hcmdzKSlcbiAgICAgIH1cbiAgICB9KVxuICAgIHJldHVybiByZXN1bHRzXG4gIH1cbn1cbiJdLCJuYW1lcyI6WyJjb25zdCIsImxldCJdLCJtYXBwaW5ncyI6Ijs7Ozs7Ozs7QUFBQTs7Ozs7Ozs7Ozs7Ozs7Ozs7O0FBa0JBQSxJQUFNLE1BQU0sR0FBRyxFQUFFLENBQUE7O0FBRWpCQSxJQUFNLFdBQVcsR0FBRyxFQUFFLENBQUE7O0FBRXRCLEFBQU8sU0FBUyxJQUFJLEVBQUUsR0FBRyxFQUFFO0VBQ3pCLE1BQU0sQ0FBQyxRQUFRLEdBQUcsR0FBRyxDQUFDLFFBQVEsQ0FBQTtFQUM5QixNQUFNLENBQUMsT0FBTyxHQUFHLEdBQUcsQ0FB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
