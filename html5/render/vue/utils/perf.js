/**
 * @fileOverview: perf data recorder.
 */

import { debounce, depress } from './func'

// performance tracker for weex.
const perf = window._weex_perf = {
  earliestBeforeCreates: [],
  latestMounts: [],
  earliestBeforeUpdates: [],
  latestUpdates: [],
  latestRenderFinishes: [],
  // createTime: earliest beforeCreate -> latest mounted.
  createTime: [],
  // updateTime: earliest beforeUpdate -> latest updated.
  updateTime: [],
  // renderTime: earliest beforeCreate/beforeUpdate -> latest img loaded.
  renderTime: []
}

let earliestBeforeUpdateTime = 0
let earliestBeforeCreateTime = 0

if (!performance.now) {
  performance.now = function () { return new Date().getTime() }
}

/**
 * get first screen time.
 */
const debouncedTagImg = debounce(function () {
  const now = performance.now()
  perf.latestRenderFinishes.push(now)
  const start = Math.max(earliestBeforeCreateTime, earliestBeforeUpdateTime)
  perf.renderTime.push({
    start,
    end: now,
    duration: now - start
  })
}, 500)
export function tagImg () {
  debouncedTagImg()
}

/**
 * recording the earliest 'beforeCreate' time.
 */
const depressedTagBeforeCreate = depress(function () {
  const now = performance.now()
  earliestBeforeCreateTime = now
  perf.earliestBeforeCreates.push(now)
}, 25)

export function tagBeforeCreate () {
  depressedTagBeforeCreate()
}

/**
 * recording the latest 'mounted' time.
 */
const debouncedTagMounted = debounce(function () {
  const now = performance.now()
  perf.latestMounts.push(now)
  perf.createTime.push({
    start: earliestBeforeCreateTime,
    end: now,
    duration: now - earliestBeforeCreateTime
  })
}, 25)

export function tagMounted () {
  debouncedTagMounted()
}

/**
 * recording the earliest 'beforeUpdate' time.
 */
const depressedTagBeforeUpdate = depress(function () {
  const now = performance.now()
  earliestBeforeUpdateTime = now
  perf.earliestBeforeUpdates.push(now)
}, 25)

export function tagBeforeUpdate () {
  depressedTagBeforeUpdate()
}

/**
 * recording the latest 'updated' time.
 */
const debouncedTagUpdated = debounce(function () {
  const now = performance.now()
  perf.latestUpdates.push(now)
  perf.updateTime.push({
    start: earliestBeforeUpdateTime,
    end: now,
    duration: now - earliestBeforeUpdateTime
  })
}, 25)

export function tagUpdated () {
  debouncedTagUpdated()
}
