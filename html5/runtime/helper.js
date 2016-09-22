let nextNodeRef = 1

const instanceMap = {}

function getNextNodeRef () {
  return nextNodeRef++
}

exports.instanceMap = instanceMap
exports.getNextNodeRef = getNextNodeRef
