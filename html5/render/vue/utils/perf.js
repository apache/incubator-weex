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
  createTime: [],
  updateTime: []
}

let earliestBeforeUpdateTime
let earliestBeforeCreateTime

if (!performance.now) {
  performance.now = function () { return new Date().getTime() }
}

/**
 * get first screen time.
 */
export function tagImg () {
  perf.latestImgLoaded = performance.now()
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
    dur: now - earliestBeforeCreateTime
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
    dur: now - earliestBeforeUpdateTime
  })
}, 25)

export function tagUpdated () {
  debouncedTagUpdated()
}
