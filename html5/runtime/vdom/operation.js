
const docMap = {}

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

let nextNodeRef = 1
export function uniqueId () {
  return (nextNodeRef++).toString()
}

export function appendBody (doc, node, before) {
  const { documentElement } = doc

  if (documentElement.pureChildren.length > 0 || node.parentNode) {
    return
  }
  const children = documentElement.children
  const beforeIndex = children.indexOf(before)
  if (beforeIndex < 0) {
    children.push(node)
  }
  else {
    children.splice(beforeIndex, 0, node)
  }

  if (node.nodeType === 1) {
    if (node.role === 'body') {
      node.docId = doc.id
      node.ownerDocument = doc
      node.parentNode = documentElement
      linkParent(node, documentElement)
    }
    else {
      node.children.forEach(child => {
        child.parentNode = node
      })
      setBody(doc, node)
      node.docId = doc.id
      node.ownerDocument = doc
      linkParent(node, documentElement)
      delete doc.nodeMap[node.nodeId]
    }
    documentElement.pureChildren.push(node)
    doc.listener.createBody(node)
  }
  else {
    node.parentNode = documentElement
    doc.nodeMap[node.ref] = node
  }
}

export function setBody (doc, el) {
  el.role = 'body'
  el.depth = 1
  delete doc.nodeMap[el.nodeId]
  el.ref = '_root'
  doc.nodeMap._root = el
  doc.body = el
}

export function linkParent (node, parent) {
  node.parentNode = parent
  if (parent.docId) {
    node.docId = parent.docId
    node.ownerDocument = parent.ownerDocument
    node.ownerDocument.nodeMap[node.nodeId] = node
    node.depth = parent.depth + 1
  }
  node.children.forEach(child => {
    linkParent(child, node)
  })
}
