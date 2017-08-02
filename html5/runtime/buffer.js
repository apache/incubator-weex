export function bufferToString (buffer) {
  return String.fromCharCode.apply(null, new Uint16Array(buffer))
}

export function stringToBuffer (string) {
  const N = string.length
  const buffer = new ArrayBuffer(N * 2)
  const view = new Uint16Array(buffer)
  for (let i = 0; i < N; i++) {
    view[i] = string.charCodeAt(i)
  }
  return buffer
}
