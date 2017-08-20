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

import { debounce, depress, extend } from './func'
// performance tracker for weex.
export const perf: {
  earliestBeforeCreates: Array<number>,
  latestMounts: Array<number>,
  earliestBeforeUpdates: Array<number>,
  latestUpdates: Array<number>,
  latestRenderFinishes: Array<number>,
  // createTime: earliest beforeCreate -> latest mounted.
  createTime: Array<{ start: number, end: number, duration: number }>,
  // updateTime: earliest beforeUpdate -> latest updated.
  updateTime: Array<{ start: number, end: number, duration: number }>,
  // renderTime: earliest beforeCreate/beforeUpdate -> latest img loaded.
  renderTime: Array<{ start: number, end: number, duration: number }>,
  entries: Array<{ requestStart: number, responseEnd: number }>,
  time: {},
  firstAllMountedTime: ?number,
  stat: {
    // info stat.
    connectionType: string,
    requestType: string,
    pageName: string,
    templateUrl: string,
    WXSDKVersion: string,
    bizType: string,
    JSLibVersion: string,

    // performance stat.
    screenRenderTime: number,
    JSTemplateSize: number,
    componentCount: number,
    networkTime: number,
    JSLibInitTime: number,
    totalTime: number
  }
} = window._weex_perf = {
  earliestBeforeCreates: [],
  latestMounts: [],
  earliestBeforeUpdates: [],
  latestUpdates: [],
  latestRenderFinishes: [],
  createTime: [],
  updateTime: [],
  renderTime: [],
  entries: [],
  time: {},
  firstAllMountedTime: null,
  stat: {
    // info stat.
    connectionType: '',
    requestType: '',
    pageName: '',
    templateUrl: '',
    WXSDKVersion: '',
    bizType: '',
    JSLibVersion: '',
    // performance stat.
    screenRenderTime: 0,
    JSTemplateSize: 0,
    componentCount: 0,
    networkTime: 0,
    JSLibInitTime: 0,
    totalTime: 0
  }
}

const tmp = {}

const IMG_REC_INDENT: number = 500  // record loading events after 500ms towards last recording.
const REGEXP_IMG_TYPE = /(.png|.jpg|.jpeg|.webp|.gif)$/  // image file type regexp.

let earliestBeforeUpdateTime: number = 0
let earliestBeforeCreateTime: number = 0

let statCollected = false
function collectStat (): void {
  if (statCollected) {
    return
  }
  statCollected = true
  const version = window.weex.config.env.weexVersion
  const protocol = location.protocol
  extend(perf.stat, {
    connectionType: protocol.substring(0, protocol.length - 1),
    requestType: 'network',
    pageName: location.href,
    WXSDKVersion: version,
    bizType: 'weex',
    JSLibVersion: version
  })
  const performance = window.performance
  // get templateUrl && JSTemplateSize
  const entries = performance && performance.getEntries && performance.getEntries()
  if (entries) {
    const params = window.lib.httpurl(location.href).params
    const bundle = params.page || params._wx_tpl
    if (bundle) {
      const len = entries.length
      let i = 0
      while (i < len) {
        const entry = entries[i]
        if (entry.entryType === 'resource'
          && entry.name.indexOf(bundle.replace(/^[./]+/, '')) > -1) {
          const res: any = extend({}, entry)
          extend(perf.stat, {
            templateUrl: res.name,
            JSTemplateSize: (res.transferSize / 1024).toFixed(2),
            networkTime: parseInt(res.duration)
          })
          break
        }
        i++
      }
    }
  }
}

export function collectStatPerf (name: string, val: string | number): void {
  if (!name) { return }
  perf.stat[name] = val
}

function getNow (): number {
  const performance = window.performance
  return performance && performance.now ? performance.now() : new Date().getTime()
}

function getEntries (): Array<any> {
  const performance = window.performance
  return performance && performance.getEntries
    ? performance.getEntries()
      .filter(function (entry) {
        return REGEXP_IMG_TYPE.test(entry.name)
      })
    : [{ responseEnd: getNow() - IMG_REC_INDENT }]
}

function _d (func: Function, wait: number) {
  let timerId
  let now
  function later (now) {
    timerId = null
    func(now)
  }
  return function () {
    now = getNow()
    clearTimeout(timerId)
    timerId = setTimeout(later.bind(null, now), wait)
  }
}

export function tagFirstScreen (time?: number): void {
  if (!time) {
    time = getNow()
  }
  perf.latestRenderFinishes.push(time)
  const start = Math.max(earliestBeforeCreateTime, earliestBeforeUpdateTime)
  perf.renderTime.push({
    start,
    end: time,
    duration: time - start
  })

  const num = perf.renderTime.length
  perf[`screenTime${num}`] = time

  if (!window._first_screen_detected) {
    window._first_screen_detected = true
    collectStat()
    extend(perf.stat, {
      screenRenderTime: parseInt(time),
      componentCount: window._component_count
    })
    window.weex.emit('firstscreenfinish', time)
  }

  if (process.env.NODE_ENV === 'development') {
    console.log(`screenTime[${num}]: ${time} ms.`)
    console.log('_weex_perf:', window._weex_perf)
  }

  window.weex.emit('renderfinish', time)
}

/**
 * get first screen time.
 */
const debouncedTagImg = debounce(function () {
  const entries = getEntries()
  const len = entries.length
  if (!len) {
    /**
     * no image loaded. This probably happened because of the disabling of images' loading
     * events. So just tag now as the first screen time.
     */
    return tagFirstScreen()
  }
  let i = 0
  let end = 0
  while (i < len) {
    const responseEnd = entries[i].responseEnd
    end = end < responseEnd ? responseEnd : end
    perf.entries.push({
      requestStart: entries[i].requestStart,
      responseEnd
    })
    i++
  }
  tagFirstScreen(end)
}, IMG_REC_INDENT)

export function tagImg (): void {
  debouncedTagImg()
}

/**
 * recording the earliest 'beforeCreate' time.
 */
const depressedTagBeforeCreate = depress(function () {
  const now = getNow()
  earliestBeforeCreateTime = now
  perf.earliestBeforeCreates.push(now)
}, 25)

export function tagBeforeCreate (): void {
  depressedTagBeforeCreate()
}

export function tagRootMounted (): void {
  const now = getNow()
  perf.latestMounts.push(now)
  if (!perf.firstAllMountedTime) {
    perf.firstAllMountedTime = now
    if (process.env.NODE_ENV === 'development') {
      console.log(`first all mounted time: ${now} ms.`)
    }
  }
  collectStatPerf('totalTime', parseInt(now))
}

/**
 * recording the latest 'mounted' time.
 */
const debouncedTagMounted = _d(function (now) {
  perf.latestMounts.push(now)
  perf.createTime.push({
    start: earliestBeforeCreateTime,
    end: now,
    duration: now - earliestBeforeCreateTime
  })

  if (!perf.firstAllMountedTime) {
    perf.firstAllMountedTime = now
    if (process.env.NODE_ENV === 'development') {
      console.log(`first all mounted time: ${now} ms.`)
    }
  }
}, 25)

export function tagMounted (): void {
  debouncedTagMounted()
}

/**
 * recording the earliest 'beforeUpdate' time.
 */
const depressedTagBeforeUpdate = depress(function () {
  const now = getNow()
  earliestBeforeUpdateTime = now
  perf.earliestBeforeUpdates.push(now)
}, 25)

export function tagBeforeUpdate (): void {
  depressedTagBeforeUpdate()
}

/**
 * recording the latest 'updated' time.
 */
const debouncedTagUpdated = _d(function (now) {
  perf.latestUpdates.push(now)
  perf.updateTime.push({
    start: earliestBeforeUpdateTime,
    end: now,
    duration: now - earliestBeforeUpdateTime
  })
}, 25)

export function tagUpdated (): void {
  debouncedTagUpdated()
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
