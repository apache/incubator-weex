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
import * as styleValidator from './style'
import * as propValidator from './prop'
import { hyphenate, camelize, isPlainObject } from '../utils'
import { isSupportedStyle } from './check'

let onfail = function nope () {}
let showConsole = true

function warn (...args) {
  const message = args.join(' ')
  showConsole && console.warn(message)
  onfail(message)
  return message
}

/**
 * Configure the validator.
 * @param {Object} configs
 */
export function configure (configs = {}) {
  showConsole = !configs.silent
  if (typeof configs.onfail === 'function') {
    onfail = configs.onfail
  }
}

/**
 * Validate the styles of the component.
 * @param {String} type
 * @param {Object} styles
 */
export function validateStyles (type, styles = {}) {
  let isValid = true

  if (isPlainObject(type)) {
    styles = type
    type = '*'
  }

  for (const key in styles) {
    if (!isSupportedStyle(type, hyphenate(key))) {
      isValid = false
      warn((type === '*')
        ? `[Style Validator] Not support to use the "${key}" style property.`
        : `[Style Validator] <${type}> is not support to use the "${key}" style property.`
      )
    }
    else {
      const validator = styleValidator[camelize(key)] || styleValidator.common
      if (!validator(styles[key], hyphenate(key))) {
        isValid = false
        warn(`[Style Validator] The style "${key}" is not support the "${styles[key]}" value.`)
      }
    }
  }
  return isValid
}

/**
 * Validate the properties of the component.
 * @param {String} type
 * @param {Object} props
 */
export function validateProps (type, props = {}) {
  let isValid = true
  for (const key in props) {
    const validator = propValidator[camelize(key)]
    if (validator && !validator(props[key])) {
      isValid = false
      warn(`[Property Validator] The property "${key}" is not support the "${props[key]}" value.`)
    }
  }
  return isValid
}
