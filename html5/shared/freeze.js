/**
 * Freeze the prototype of javascript build-in objects.
 */
/* istanbul ignore next */
export function freezePrototype () {
  Object.freeze(Object)
  Object.freeze(Array)

  // Object.freeze(Object.prototype)
  freezeObjectProto()
  Object.freeze(Array.prototype)
  Object.freeze(String.prototype)
  Object.freeze(Number.prototype)
  Object.freeze(Boolean.prototype)

  // Object.freeze(Error.prototype)
  freezeErrorProto()
  Object.freeze(Date.prototype)
  Object.freeze(RegExp.prototype)
}

function freezeObjectProto () {
  const proto = Object.prototype
  const protoName = 'Object.prototype'
  freezeProtoProperty(proto, '__defineGetter__', protoName)
  freezeProtoProperty(proto, '__defineSetter__', protoName)
  freezeProtoProperty(proto, '__lookupGetter__', protoName)
  freezeProtoProperty(proto, '__lookupSetter__', protoName)
  freezeProtoProperty(proto, 'constructor', protoName)
  freezeProtoProperty(proto, 'hasOwnProperty', protoName)
  freezeProtoProperty(proto, 'isPrototypeOf', protoName)
  freezeProtoProperty(proto, 'propertyIsEnumerable', protoName)
  freezeProtoProperty(proto, 'toLocaleString', protoName)
  freezeProtoProperty(proto, 'toString', protoName)
  freezeProtoProperty(proto, 'valueOf', protoName)
  Object.seal(proto)
}

function freezeErrorProto () {
  const proto = Error.prototype
  const protoName = 'Error.prototype'
  freezeProtoProperty(proto, 'name', protoName)
  freezeProtoProperty(proto, 'message', protoName)
  freezeProtoProperty(proto, 'toString', protoName)
  freezeProtoProperty(proto, 'constructor', protoName)
  Object.seal(proto)
}

function freezeProtoProperty (proto, propertyName, protoName) {
  if (!proto.hasOwnProperty(propertyName)) {
    return
  }

  const origin = proto[propertyName]
  Object.defineProperty(proto, propertyName, {
    get: function () {
      return origin
    },
    set: function (value) {
      if (this === proto) {
        throw Error(`Cannot assign to read only property ${propertyName} of ${protoName}`)
      }

      Object.defineProperty(this, propertyName, {
        value: value,
        writable: true
      })

      return value
    }
  })
}
