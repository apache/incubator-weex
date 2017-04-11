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
 * @deprecated after native register these modules
 */

/**
 * @module dom
 * @type {Array}
 */
export const dom = [
  /**
   * @method scrollToElement
   * @param {string} ref
   * @param {object} options
   * @param {number} options.offset
   */
  {
    name: 'scrollToElement',
    args: ['string', 'object']
  }
]

/**
 * @module stream
 */
export const stream = [
  /**
   * @method sendHttp
   * @param  {object} params
   * @param  {string} params.url
   * @param  {string} [params.method]
   * @param  {object} [params.header]
   * @param  {object} params.body
   * @param  {function} callback
   */
  {
    name: 'sendHttp',
    args: ['object', 'function']
  }
]

/**
 * @module event
 */
export const event = [
  /**
   * @method openURL
   * @param {string} url
   */
  {
    name: 'openURL',
    args: ['string']
  }
]

/**
 * @module pageInfo
 */
export const pageInfo = [
  /**
   * @method setTitle
   * @param {string} text
   */
  {
    name: 'setTitle',
    args: ['string']
  }
]

/**
 * @module animation
 */
export const animation = [

  {
    name: 'transition',
    args: ['string', 'object', 'function']
  }
]
