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

describe('image onload @ignore-ios', function () {
  this.timeout(util.getTimeoutMills());
  var driver = util.createDriver(wd);

  beforeEach(function () {
    return util.init(driver)
      .get(util.getPage('/image-onload.js'))
      .waitForElementById('imgSize',util.getGETActionWaitTimeMills(),1000)
  });

  afterEach(function () {
      return util.quit(driver);
  })


  it('#1 download image', () => {
    return driver
    .sleep(5000)
    .elementById('imgSize')
    .text()
    .then((text)=>{
        if(text == '-1,-1') {
            return;
        }
        assert.equal(text, '360,388')
    })
  });

  it('#2 test download image failed event', () => {
    return driver
    .sleep(5000)
    .elementById('download')
    .text()
    .then((text)=>{
        assert.equal(text, 'failed')
    })
  })
});


