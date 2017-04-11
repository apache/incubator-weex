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

// const funcIdToTimerIdMap = {}
// const funcIdToIntervalIdMap = {}

// const timer = {

//   setTimeout (funcId, delay) {
//     const sender = this.sender
//     delay < 0 && (delay = 0)
//     const timerId = setTimeout(function () {
//       delete funcIdToTimerIdMap[funcId]
//       sender.performCallback(funcId)
//     }, delay)
//     funcIdToTimerIdMap[funcId] = timerId
//     return funcId
//   },

//   clearTimeout (funcId) {
//     clearTimeout(funcIdToTimerIdMap[funcId])
//     delete funcIdToTimerIdMap[funcId]
//   },

//   setInterval (funcId, interval) {
//     const sender = this.sender
//     interval < 0 && (interval = 0)
//     const timerId = setInterval(function () {
//       delete funcIdToIntervalIdMap[funcId]
//       sender.performCallback(funcId, null, true)
//     })
//     funcIdToIntervalIdMap[funcId] = timerId
//     return funcId
//   },

//   clearInterval (funcId) {
//     clearInterval(funcIdToIntervalIdMap[funcId])
//     delete funcIdToIntervalIdMap[funcId]
//   }

// }

// const meta = {
//   timer: [{
//     name: 'setTimeout',
//     args: ['function', 'number']
//   }, {
//     name: 'clearTimeout',
//     args: ['number']
//   }, {
//     name: 'setInterval',
//     args: ['function', 'number']
//   }, {
//     name: 'clearInterval',
//     args: ['number']
//   }]
// }

// export default {
//   init: function (Weex) {
//     Weex.registerApiModule('timer', timer, meta)
//   }
// }
