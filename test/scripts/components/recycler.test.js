'use strict';

var _ = require('macaca-utils');
var assert = require('chai').assert
var wd = require('weex-wd')
var path = require('path');
var os = require('os');
var util = require("../util.js");


const platform = process.env.platform.toLowerCase() || 'android';
const isAndroid = platform === 'android';

let cell1Path = isAndroid ? '//recycler[1]/div[3]/' : '//recycler[1]/cell[1]/'
let cell2Path = isAndroid ? '//recycler[1]/div[4]/' : '//recycler[1]/cell[2]/'
let cell3Path = isAndroid ? '//recycler[1]/div[5]/' : '//recycler[1]/cell[3]/'
let cell4Path = isAndroid ? '//recycler[1]/div[6]/' : '//recycler[1]/cell[4]/'

const isApproximate = (x, y) =>  {
  return Math.abs(x - y) <= 1
}


describe('recycler', function () {
  this.timeout(util.getTimeoutMills())
  const driver = util.createDriver(wd)

  before(function () {
    console.log('wxpage://' + util.getDeviceHost() +'/components/recycler.js')
    return util.init(driver)
      .get('wxpage://' + util.getDeviceHost() +'/components/recycler.js')
      .waitForElementByXPath('//recycler[1]',util.getGETActionWaitTimeMills(),1000)
  });

  after(function () {
      // return util.quit(driver);
  })

  let scaleFactor = 0
  let screenHeight = 0
  let recyclerWidth = 0
  let navBarHeight = 0
  let cell1Height = 0
  let cell2Height = 0
  it('#1 test recyler layout', () => {
    return driver
    .getWindowSize()
    .then(size=>{
      scaleFactor = size.width / 750
      screenHeight = size.height
      recyclerWidth = isAndroid ? (size.width + 12) : 750 * scaleFactor
      console.log(`screen size:${JSON.stringify(size)}`)
      console.log(`scale factor:${scaleFactor}`)
    })
    .sleep(2000)
    .elementByXPath('//recycler[1]')
    .getRect()
    .then((rect)=>{
      console.log(`recycler rect:${JSON.stringify(rect)}`)
      navBarHeight = rect.y
      assert.isOk(isApproximate(rect.x, 0))
      assert.isOk(isApproximate(rect.width, recyclerWidth))
      assert.isOk(isApproximate(rect.height, screenHeight - navBarHeight))
    })
    .elementByXPath('//recycler[1]/div[1]/')
    .getRect()
    .then((rect)=>{
      console.log(`header 1 rect:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 0))
      assert.isOk(isApproximate(rect.y, navBarHeight))
      assert.isOk(isApproximate(rect.width, recyclerWidth))
      assert.isOk(isApproximate(rect.height, 377 * scaleFactor))
    })
    .elementByXPath('//recycler[1]/div[2]/')
    .getRect()
    .then((rect)=>{
      console.log(`header 2 rect:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 0))
      assert.isOk(isApproximate(rect.y, navBarHeight + 377 * scaleFactor))
      assert.isOk(isApproximate(rect.width, recyclerWidth))
      assert.isOk(isApproximate(rect.height, 94 * scaleFactor))
    })
    .elementByXPath(cell1Path)
    .getRect()
    .then((rect)=>{
      console.log(`cell 1 rect:${JSON.stringify(rect)}`)
      cell1Height = rect.height
      assert.isOk(isApproximate(rect.x, 0))
      assert.isOk(isApproximate(rect.y, navBarHeight + 471 * scaleFactor))
      assert.isOk(isApproximate(rect.width, 369 * scaleFactor))
    })
    .elementByXPath(cell2Path)
    .getRect()
    .then((rect)=>{
      console.log(`cell 2 rect:${JSON.stringify(rect)}`)
      cell2Height = rect.height
      assert.isOk(isApproximate(rect.x, 381 * scaleFactor), 'x not approximate:' + 381 * scaleFactor)
      assert.isOk(isApproximate(rect.y, navBarHeight + 471 * scaleFactor), 'y not approximate:' + (navBarHeight + 471 * scaleFactor))
      assert.isOk(isApproximate(rect.width, 369 * scaleFactor))
    })
    .elementByXPath(cell3Path)
    .getRect()
    .then((rect)=>{
      console.log(`cell 3 rect:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 381 * scaleFactor))
      assert.isOk(isApproximate(rect.y, navBarHeight + 471 * scaleFactor + cell2Height))
      assert.isOk(isApproximate(rect.width, 369 * scaleFactor))
    })
    .elementByXPath(cell4Path)
    .getRect()
    .then((rect)=>{
      console.log(`cell 4 rect:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 0 * scaleFactor))
      assert.isOk(isApproximate(rect.y, navBarHeight + 471 * scaleFactor + cell1Height))
      assert.isOk(isApproximate(rect.width, 369 * scaleFactor))
    })
    .elementByXPath('//div[1]')
    .getRect()
    .then((rect)=>{
      console.log(`fixed rect:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 640 * scaleFactor))
      assert.isOk(isApproximate(rect.y, screenHeight - 110 * scaleFactor))
      assert.isOk(isApproximate(rect.width, 78 * scaleFactor))
      assert.isOk(isApproximate(rect.height, 78 * scaleFactor))
    })
  })

});


