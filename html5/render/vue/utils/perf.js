// @flow
/**
 * @fileOverview: perf data recorder.
 */

import { debounce, depress } from './func'

// performance tracker for weex.
const perf: {
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
  firstAllMountedTime: ?number
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
  firstAllMountedTime: null
}

const tmp = {}

const IMG_REC_INDENT: number = 500  // record loading events after 500ms towards last recording.

let earliestBeforeUpdateTime: number = 0
let earliestBeforeCreateTime: number = 0

function getNow (): number {
  return performance.now ? performance.now() : new Date().getTime()
}

function getEntries (): Array<any> {
  return performance.getEntries
    ? performance.getEntries()
    : [{ responseEnd: getNow() - IMG_REC_INDENT }]
}

/**
 * get first screen time.
 */
const debouncedTagImg = debounce(function () {
  const entries = getEntries()
  const len = entries.length
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
  perf.latestRenderFinishes.push(end)
  const start = Math.max(earliestBeforeCreateTime, earliestBeforeUpdateTime)
  perf.renderTime.push({
    start,
    end,
    duration: end - start
  })

  const num = perf.renderTime.length
  perf[`screenTime${num}`] = end
  window.weex.emit('renderfinish', end)
  if (process.env.NODE_ENV === 'development') {
    console.log(`screenTime[${num}]: ${end} ms.`)
    console.log('_weex_perf:', window._weex_perf)
  }
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

/**
 * recording the latest 'mounted' time.
 */
const debouncedTagMounted = debounce(function () {
  const now = getNow()
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
const debouncedTagUpdated = debounce(function () {
  const now = getNow()
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
