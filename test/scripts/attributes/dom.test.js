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

describe('dom-operation', function () {
  this.timeout(util.getTimeoutMills());
  var driver = util.createDriver(wd);
  var maxW = util.getGETActionWaitTimeMills();
  beforeEach(function () {
    return util.init(driver)
      .get(util.getPage('/attributes/dom-operation.js'))
  });

  afterEach(function () {
      return util.quit(driver)
  })

  it('#1 Repeat', ()=>{
    return driver
    .waitForElementById('status',maxW,1000)
    .elementById("item5")
    .then((elem)=>{
        assert.notEqual(elem ,undefined)
    })
    .elementById("maynotexist")
    .then((noexist)=>{
        assert.equal(noexist,undefined)
    })
  })

  it('#2 Add new item', ()=>{
    return driver
    .waitForElementById('status',maxW,1000)
    .elementById("btn1")
    .click()
    .sleep(2000)
    .elementById("item6")
    .then((elem)=>{
        assert.notEqual(elem ,undefined)
    })
    .elementById("maynotexist")
    .then((existed)=>{
        assert.notEqual(existed,undefined)
    })
  })

  it('#3 Remove item', ()=>{
    return driver
    .waitForElementById('status',maxW,1000)
    .click()
    .sleep(2000)
    .elementById("item6")
    .then((elem)=>{
        assert.equal(elem,undefined)
    })
    .elementById("maynotexist")
    .then((noexist)=>{
        assert.equal(noexist,undefined)
    })
  })

  it('#4 Update DOM attr', ()=>{
      return driver
      .waitForElementById('status',maxW,1000)
      .click()
      .sleep(2000)
      .elementById("status2")
      .text()
      .then((text)=>{
          assert.equal("finished",text);
      })
  })

  
});
