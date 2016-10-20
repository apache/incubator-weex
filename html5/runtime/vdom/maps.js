
const docMap = {}
let nextNodeRef = 1

export function addDoc (id, doc) {
  if (id) {
    docMap[id] = doc
  }
}

export function getDoc (id) {
  return docMap[id]
}

export function removeDoc (id) {
  delete docMap[id]
}

export function getListener (id) {
  const doc = docMap[id]
  if (doc && doc.listener) {
    return doc.listener
  }
  return null
}

export function getRef () {
  return nextNodeRef++
}
