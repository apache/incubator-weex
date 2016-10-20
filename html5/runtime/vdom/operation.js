
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

export function nextElement (node) {
  while (node) {
    if (node.nodeType === 1) {
      return node
    }
    node = node.nextSibling
  }
}

export function previousElement (node) {
  while (node) {
    if (node.nodeType === 1) {
      return node
    }
    node = node.previousSibling
  }
}

export function insertIndex (target, list, newIndex, changeSibling) {
  if (newIndex < 0) {
    newIndex = 0
  }
  const before = list[newIndex - 1]
  const after = list[newIndex]
  list.splice(newIndex, 0, target)
  if (changeSibling) {
    before && (before.nextSibling = target)
    target.previousSibling = before
    target.nextSibling = after
    after && (after.previousSibling = target)
  }
  return newIndex
}

export function moveIndex (target, list, newIndex, changeSibling) {
  const index = list.indexOf(target)
  if (index < 0) {
    return -1
  }
  if (changeSibling) {
    const before = list[index - 1]
    const after = list[index + 1]
    before && (before.nextSibling = after)
    after && (after.previousSibling = before)
  }
  list.splice(index, 1)
  let newIndexAfter = newIndex
  if (index <= newIndex) {
    newIndexAfter = newIndex - 1
  }
  const beforeNew = list[newIndexAfter - 1]
  const afterNew = list[newIndexAfter]
  list.splice(newIndexAfter, 0, target)
  if (changeSibling) {
    beforeNew && (beforeNew.nextSibling = target)
    target.previousSibling = beforeNew
    target.nextSibling = afterNew
    afterNew && (afterNew.previousSibling = target)
  }
  if (index === newIndexAfter) {
    return -1
  }
  return newIndex
}

export function removeIndex (target, list, changeSibling) {
  const index = list.indexOf(target)
  if (index < 0) {
    return
  }
  if (changeSibling) {
    const before = list[index - 1]
    const after = list[index + 1]
    before && (before.nextSibling = after)
    after && (after.previousSibling = before)
  }
  list.splice(index, 1)
}
