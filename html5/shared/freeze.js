/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
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
