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

// @flow
/**
 * @fileOverview: perf data recorder.
 */

export const perf: {
  time: {}
} = window._weex_perf = {
  time: {}
}

const tmp = {}

function getNow (): number {
  const performance = window.performance
  return performance && performance.now ? performance.now() : new Date().getTime()
}

export function tagBegin (name: string): void {
  tmp[name] = getNow()
}

export function tagEnd (name: string): void {
  let pre = perf.time[name]
  if (!pre) {
    pre = 0
  }
  perf.time[name] = pre + getNow() - tmp[name]
}
