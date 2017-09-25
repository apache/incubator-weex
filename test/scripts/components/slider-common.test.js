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
'use strict';

var _ = require('macaca-utils');
var assert = require('chai').assert
var wd = require('weex-wd')
var path = require('path');
var os = require('os');
var util = require("../util.js");

describe('slider common', function () {
  this.timeout(util.getTimeoutMills());
  var driver = util.createDriver(wd);

  beforeEach(function () {
    return util.init(driver)
      .get(util.getPage('/components/slider-common.js'))
  });

  afterEach(function () {
    return util.quit(driver);
  });


  it('#1 OnScroll & onChange Event', () => {
    return driver
      .sleep(3000)
      .waitForElementById('slider_1', util.getGETActionWaitTimeMills(), 1000)
      .elementById('s1_page1')
      .click()
      .elementById('s1_p')
      .text()
      .then(t => {
        assert.equal(t, '233')
      })
      .elementById('s1_i')
      .text()
      .then(t => {
        assert.equal(t, '1')
      })
      .elementById('slider_1')
      .swipeLeft(1, 0.4)
      .elementById('s1_f')
      .text()
      .then(t => {
        assert.equal('true', t)
      })
      .elementById('s1_d')
      .text()
      .then(t => {
        assert.equal('left', t)
      })
      .elementById('s1_i')
      .text()
      .then(t => {
        assert.equal('2', t)
      })
  })

  it('#2 Infinite scroll = true', () => {
    return driver.sleep(3000)
      .waitForElementById('slider_1', util.getGETActionWaitTimeMills(), 1000)
      .elementById('s2_switch')
      .click()
      .sleep(5000)
      .elementById('s2_count')
      .text()
      .then(t => {
        assert.notEqual(1, parseInt(t))
      })
  })

  it('#3 Infinite scroll = false', () => {
    return driver.sleep(3000)
      .waitForElementById('slider_1', util.getGETActionWaitTimeMills(), 1000)
      .elementById('s3_switch')
      .click()
      .sleep(5000)
      .elementById('s3_count')
      .text()
      .then(t => {
        assert.equal(1, parseInt(t))
      })
  })
});