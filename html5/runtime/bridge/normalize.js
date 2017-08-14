import { typof, bufferToBase64, base64ToBuffer } from '../utils'

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
