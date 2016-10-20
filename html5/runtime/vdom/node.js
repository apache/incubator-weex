import { getDoc, uniqueId } from './operation'

export default function Node () {
  this.nodeId = uniqueId()
  this.ref = this.nodeId
  this.children = []
  this.pureChildren = []
  this.parentNode = null
  this.nextSibling = null
  this.previousSibling = null
}

Node.prototype.destroy = function () {
  const doc = getDoc(this.docId)
  if (doc) {
    delete this.docId
    delete doc.nodeMap[this.nodeId]
  }
  this.children.forEach(child => {
    child.destroy()
  })
}
