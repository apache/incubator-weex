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
/* global lib: true */

'use strict'

/**
 * transfer Quadratic Bezier Curve to Cubic Bezier Curve
 *
 * @param  {number} a abscissa of p1
 * @param  {number} b ordinate of p1
 * @return {Array} parameter matrix for cubic bezier curve
 *   like [[p1x, p1y], [p2x, p2y]]
 */
function quadratic2cubicBezier (a, b) {
  return [
    [
      (a / 3 + (a + b) / 3 - a) / (b - a),
      (a * a / 3 + a * b * 2 / 3 - a * a) / (b * b - a * a)
    ], [
      (b / 3 + (a + b) / 3 - a) / (b - a),
      (b * b / 3 + a * b * 2 / 3 - a * a) / (b * b - a * a)
    ]
  ]
}

/**
 * derive position data from knowing motion parameters
 * base on Newton's second law: s = vt + at^2/2
 *
 * @param {object} config object of { v, a, s, t }
 *   - v: initial velocity
 *   - a: accelerate speed
 *   - t: time
 *   - s: shifting
 */
function Motion (config) {
  this.v = config.v || 0
  this.a = config.a || 0

  if (typeof config.t !== 'undefined') {
    this.t = config.t
  }

  if (typeof config.s !== 'undefined') {
    this.s = config.s
  }

  // derive time from shifting
  if (typeof this.t === 'undefined') {
    if (typeof this.s === 'undefined') {
      this.t = -this.v / this.a
    }
    else {
      const t1 = (Math.sqrt(this.v * this.v + 2 * this.a * this.s) - this.v)
        / this.a
      const t2 = (-Math.sqrt(this.v * this.v + 2 * this.a * this.s) - this.v)
        / this.a
      this.t = Math.min(t1, t2)
    }
  }

  // derive shifting from time
  if (typeof this.s === 'undefined') {
    this.s = this.a * this.t * this.t / 2 + this.v * this.t
  }
}

/**
 * derive cubic bezier parameters from motion parameters
 * @return {Array} parameter matrix for cubic bezier curve
 *   like [[p1x, p1y], [p2x, p2y]]
 */
Motion.prototype.generateCubicBezier = function () {
  return quadratic2cubicBezier(
    this.v / this.a, this.t + this.v / this.a
  )
}

!lib && (lib = {})
lib.motion = Motion

module.exports = Motion
