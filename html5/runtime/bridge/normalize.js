import { typof } from '../utils'

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

/**
 * Normalize a primitive value.
 * @param  {any}        v
 * @return {primitive}
 */
export function normalizePrimitive (v) {
  const type = typof(v)

  switch (type) {
    case 'Undefined':
    case 'Null':
      return ''

    case 'RegExp':
      return v.toString()
    case 'Date':
      return v.toISOString()

    case 'Number':
    case 'String':
    case 'Boolean':
    case 'Array':
    case 'Object':
      return v

    case 'ArrayBuffer':
      return {
        '@type': 'binary',
        dataType: type,
        base64: bufferToBase64(v)
      }

    case 'Int8Array':
    case 'Uint8Array':
    case 'Uint8ClampedArray':
    case 'Int16Array':
    case 'Uint16Array':
    case 'Int32Array':
    case 'Uint32Array':
    case 'Float32Array':
    case 'Float64Array':
      return {
        '@type': 'binary',
        dataType: type,
        base64: bufferToBase64(v.buffer)
      }

    default:
      return JSON.stringify(v)
  }
}

export function decodePrimitive (data) {
  if (typof(data) === 'Object') {
    // decode base64 into binary
    if (data['@type'] && data['@type'] === 'binary') {
      return base64ToBuffer(data.base64 || '')
    }

    const realData = {}
    for (const key in data) {
      realData[key] = decodePrimitive(data[key])
    }
    return realData
  }
  if (typof(data) === 'Array') {
    return data.map(decodePrimitive)
  }
  return data
}
