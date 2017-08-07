
/**
 * Get a unique id.
 */
let nextNodeRef = 1
export function uniqueId () {
  return (nextNodeRef++).toString()
}

export function typof (v) {
  const s = Object.prototype.toString.call(v)
  return s.substring(8, s.length - 1)
}
