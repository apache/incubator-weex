/**
 * @fileOverview utils for tests.
 */

export function toArray (list) {
  if (!list) return []
  return Array.prototype.slice.call(list)
}
