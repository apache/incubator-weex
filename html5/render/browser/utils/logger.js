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

// let _initialized = false

// export default logger = {
//   log: function () {},
//   warn: function () {},
//   error: function () {}
// }

// function hijack (k) {
//   if (utils.isArray(k)) {
//     k.forEach(function (key) {
//       hijack(key)
//     })
//   }
//   else {
//     if (console[k]) {
//       logger[k] = function () {
//         console[k].apply(
//           console,
//           ['[h5-render]'].concat(Array.prototype.slice.call(arguments, 0))
//         )
//       }
//     }
//   }
// }

// logger.init = function () {
//   if (_initialized) {
//     return
//   }
//   _initialized = true
//   if (config.debug && console) {
//     hijack(['log', 'warn', 'error'])
//   }
// }
