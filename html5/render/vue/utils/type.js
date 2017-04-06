// @flow

const toString = Object.prototype.toString

/**
 * Strict object type check. Only returns true
 * for plain JavaScript objects.
 *
 * @param {*} obj
 * @return {Boolean}
 */
const OBJECT_STRING = '[object Object]'
export function isPlainObject (obj?: mixed): boolean {
  return toString.call(obj) === OBJECT_STRING
}

const ARRAY_STRING = '[object Array]'
export function isArray (arr?: mixed): boolean {
  return toString.call(arr) === ARRAY_STRING
}
