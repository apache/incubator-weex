require('./setTimeout')
require('./promise')
require('./console')
require('./objectAssign')

const {
  extend,
  def,
  remove,
  hasOwn,
  bind,
  toArray,
  isObject,
  isPlainObject
} = require('./utils')

exports.extend = extend
exports.def = def
exports.remove = remove
exports.hasOwn = hasOwn
exports.bind = bind
exports.toArray = toArray
exports.isObject = isObject
exports.isPlainObject = isPlainObject
