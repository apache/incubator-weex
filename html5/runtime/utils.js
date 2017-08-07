
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

export function bufferToBase64 (buffer) {
  if (typeof btoa !== 'function') {
    return ''
  }
  const string = Array.prototype.map.call(
    new Uint8Array(buffer),
    code => String.fromCharCode(code)
  ).join('')
  return btoa(string) // eslint-disable-line no-undef
}

export function base64ToBuffer (base64) {
  if (typeof atob !== 'function') {
    return new ArrayBuffer(0)
  }
  const string = atob(base64) // eslint-disable-line no-undef
  const array = new Uint8Array(string.length)
  Array.prototype.forEach.call(string, (ch, i) => {
    array[i] = ch.charCodeAt(0)
  })
  return array.buffer
}
