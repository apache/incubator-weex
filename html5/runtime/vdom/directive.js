function typof (v) {
  const s = Object.prototype.toString.call(v)
  return s.substring(8, s.length - 1)
}

// match the binding delimiter
const delimiterRE = /\[\[((?:.|\n)+?)\]\]/g

export function generateBinding (text) {
  if (typof(text) === 'String') {
    return { '@binding': text }
  }
  return text
}

export function parseString (string) {
  const tokens = []
  let lastIndex = delimiterRE.lastIndex = 0
  let match, index
  while ((match = delimiterRE.exec(string))) {
    index = match.index
    if (index > lastIndex) {
      tokens.push(string.slice(lastIndex, index))
    }
    const binding = generateBinding(match[1].trim())
    tokens.push(binding)
    lastIndex = index + match[0].length
  }
  if (lastIndex < string.length) {
    tokens.push(string.slice(lastIndex))
  }
  if (tokens.length === 1) {
    return tokens[0]
  }
  return tokens
}

export function filterDirective (value) {
  if (typof(value) === 'String' && delimiterRE.test(value)) {
    return parseString(value)
  }
  if (typof(value) === 'Object') {
    const realData = {}
    for (const key in value) {
      realData[key] = filterDirective(value[key])
    }
    return realData
  }
  if (typof(value) === 'Array') {
    return value.map(filterDirective)
  }
  return value
}
