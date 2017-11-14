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
/* global localStorage */

'use strict'

let supportLocalStorage = false
try {
  supportLocalStorage = typeof localStorage !== 'undefined'
}
catch (err) {
  // not support.
}

const SUCCESS = 'success'
const FAILED = 'failed'
const INVALID_PARAM = 'invalid_param'
const UNDEFINED = 'undefined'

function callFail (sender, callbackId, errorMsg) {
  sender.performCallback(callbackId, {
    result: FAILED,
    data: errorMsg || UNDEFINED
  })
}

function callNotSupportFail (sender, callbackId) {
  sender.performCallback(callbackId, {
    result: FAILED,
    data: 'localStorage is disabled or not supported.'
  })
}

const storage = {

  /**
   * When passed a key name and value, will add that key to the storage,
   * or update that key's value if it already exists.
   * @param {string} key
   * @param {string} value not null nor undifined，but 0 works.
   * @param {function} callbackId
   */
  setItem: function (key, value, callbackId) {
    const sender = this.sender
    if (!supportLocalStorage) {
      return callNotSupportFail(sender, callbackId)
    }
    if (!key || (!value && value !== 0)) {
      sender.performCallback(callbackId, {
        result: 'failed',
        data: INVALID_PARAM
      })
      return
    }
    try {
      localStorage.setItem(key, value)
      sender.performCallback(callbackId, {
        result: SUCCESS,
        data: UNDEFINED
      })
    }
    catch (e) {
      // accept any exception thrown during a storage attempt as a quota error
      callFail(sender, callbackId)
    }
  },

  /**
   * When passed a key name, will return that key's value.
   * @param {string} key
   * @param {function} callbackId
   */
  getItem: function (key, callbackId) {
    const sender = this.sender
    if (!supportLocalStorage) {
      return callNotSupportFail(sender, callbackId)
    }
    if (!key) {
      sender.performCallback(callbackId, {
        result: FAILED,
        data: INVALID_PARAM
      })
      return
    }
    try {
      const val = localStorage.getItem(key)
      sender.performCallback(callbackId, {
        result: val ? SUCCESS : FAILED,
        data: val || UNDEFINED
      })
    }
    catch (e) {
      // accept any exception thrown during a storage attempt as a quota error
      callFail(sender, callbackId)
    }
  },

  /**
   *When passed a key name, will remove that key from the storage.
   * @param {string} key
   * @param {function} callbackId
   */
  removeItem: function (key, callbackId) {
    const sender = this.sender
    if (!supportLocalStorage) {
      return callNotSupportFail(sender, callbackId)
    }
    if (!key) {
      sender.performCallback(callbackId, {
        result: FAILED,
        data: INVALID_PARAM
      })
      return
    }
    try {
      localStorage.removeItem(key)
      sender.performCallback(callbackId, {
        result: SUCCESS,
        data: UNDEFINED
      })
    }
    catch (e) {
      // accept any exception thrown during a storage attempt as a quota error
      callFail(sender, callbackId)
    }
  },

  /**
   * Returns an integer representing the number of data items stored in the Storage object.
   * @param {function} callbackId
   */
  length: function (callbackId) {
    const sender = this.sender
    if (!supportLocalStorage) {
      return callNotSupportFail(sender, callbackId)
    }
    try {
      const len = localStorage.length
      sender.performCallback(callbackId, {
        result: SUCCESS,
        data: len
      })
    }
    catch (e) {
      // accept any exception thrown during a storage attempt as a quota error
      callFail(sender, callbackId)
    }
  },

  /**
   * Returns an array that contains all keys stored in Storage object.
   * @param {function} callbackId
   */
  getAllKeys: function (callbackId) {
    const sender = this.sender
    if (!supportLocalStorage) {
      return callNotSupportFail(sender, callbackId)
    }
    try {
      const _arr = []
      for (let i = 0; i < localStorage.length; i++) {
        _arr.push(localStorage.key(i))
      }
      sender.performCallback(callbackId, {
        result: SUCCESS,
        data: _arr
      })
    }
    catch (e) {
      // accept any exception thrown during a storage attempt as a quota error
      callFail(sender, callbackId)
    }
  }
}

const meta = {
  storage: [{
    name: 'setItem',
    args: ['string', 'string', 'function']
  }, {
    name: 'getItem',
    args: ['string', 'function']
  }, {
    name: 'removeItem',
    args: ['string', 'function']
  }, {
    name: 'length',
    args: ['function']
  }, {
    name: 'getAllKeys',
    args: ['function']
  }]
}

export default {
  init: function (Weex) {
    Weex.registerApiModule('storage', storage, meta)
  }
}
