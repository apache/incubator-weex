'use strict';

var _ = require('macaca-utils');
var assert = require('chai').assert
var wd = require('weex-wd')
var path = require('path');
var os = require('os');
var util = require("../util.js");


const platform = process.env.platform.toLowerCase() || 'android';
const isAndroid = platform === 'android';

const fixedPath = '//div[1]'
const header1Path = '//recycler[1]/div[1]/'
const header2Path = '//recycler[1]/div[2]/'
const cell1Path = isAndroid ? '//recycler[1]/div[3]/' : '//recycler[1]/cell[1]/'
const cell2Path = isAndroid ? '//recycler[1]/div[4]/' : '//recycler[1]/cell[2]/'
const cell3Path = isAndroid ? '//recycler[1]/div[5]/' : '//recycler[1]/cell[3]/'
const cell4Path = isAndroid ? '//recycler[1]/div[6]/' : '//recycler[1]/cell[4]/'
const cell27Path = isAndroid ? '//recycler[1]/div[29]/' : '//recycler[1]/cell[27]/'
const cell28Path = isAndroid ? '//recycler[1]/div[30]/' : '//recycler[1]/cell[28]/'
const cell29Path = isAndroid ? '//recycler[1]/div[31]/' : '//recycler[1]/cell[29]/'
const cell30Path = isAndroid ? '//recycler[1]/div[32]/' : '//recycler[1]/cell[30]/'
const footerPath = '//recycler[1]/div[1]'

const isApproximate = (x, y) =>  {
  return Math.abs(x - y) <= 1
}

if (isAndroid) {
  return;
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
      return util.quit(driver); 
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
    .elementByXPath(header1Path)
    .getRect()
    .then((rect)=>{
      console.log(`header 1 rect:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 0))
      assert.isOk(isApproximate(rect.y, navBarHeight))
      assert.isOk(isApproximate(rect.width, recyclerWidth))
      assert.isOk(isApproximate(rect.height, 377 * scaleFactor))
    })
    .elementByXPath(header2Path)
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
      assert.isOk(isApproximate(rect.x, 381 * scaleFactor))
      assert.isOk(isApproximate(rect.y, navBarHeight + 471 * scaleFactor))
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
  })

  it('#2 test column count', () => {
    return driver
    .elementByXPath(cell3Path)
    .click()
    .elementByXPath(cell1Path)
    .getRect()
    .then((rect)=>{
      console.log(`cell 1 rect after changing column count to 3:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 0))
      assert.isOk(isApproximate(rect.y, navBarHeight + 471 * scaleFactor))
      assert.isOk(isApproximate(rect.width, 242 * scaleFactor))
    })
    .elementByXPath(cell2Path)
    .getRect()
    .then((rect)=>{
      console.log(`cell 2 rect after changing column count to 3:${JSON.stringify(rect)}`)
      cell2Height = rect.height
      assert.isOk(isApproximate(rect.x, 254 * scaleFactor))
      assert.isOk(isApproximate(rect.y, navBarHeight + 471 * scaleFactor))
      assert.isOk(isApproximate(rect.width, 242 * scaleFactor))
    })
    .elementByXPath(cell3Path)
    .getRect()
    .then((rect)=>{
      console.log(`cell 3 rect after changing column count to 3:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 508 * scaleFactor))
      assert.isOk(isApproximate(rect.y, navBarHeight + 471 * scaleFactor))
      assert.isOk(isApproximate(rect.width, 242 * scaleFactor))
    })
    .elementByXPath(cell4Path)
    .getRect()
    .then((rect)=>{
      console.log(`cell 4 rect after changing column count to 3:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 254 * scaleFactor))
      assert.isOk(isApproximate(rect.y, navBarHeight + 471 * scaleFactor + cell2Height))
      assert.isOk(isApproximate(rect.width, 242 * scaleFactor))
    })
    .elementByXPath(cell3Path)
    .click()
  })

  it('#3 test column gap', () => {
    return driver
    .elementByXPath(cell2Path)
    .click()
    .elementByXPath(cell1Path)
    .getRect()
    .then((rect)=>{
      console.log(`cell 1 rect after changing column gap to normal:${JSON.stringify(rect)}`)
      cell1Height = rect.height
      assert.isOk(isApproximate(rect.x, 0))
      assert.isOk(isApproximate(rect.y, navBarHeight + 471 * scaleFactor))
      assert.isOk(isApproximate(rect.width, 357 * scaleFactor))
    })
    .elementByXPath(cell2Path)
    .getRect()
    .then((rect)=>{
      console.log(`cell 2 rect after changing column gap to normal:${JSON.stringify(rect)}`)
      cell2Height = rect.height
      assert.isOk(isApproximate(rect.x, 393 * scaleFactor))
      assert.isOk(isApproximate(rect.y, navBarHeight + 471 * scaleFactor))
      assert.isOk(isApproximate(rect.width, 357 * scaleFactor))
    })
    .elementByXPath(cell3Path)
    .getRect()
    .then((rect)=>{
      console.log(`cell 3 rect after changing column gap to normal:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 393 * scaleFactor))
      assert.isOk(isApproximate(rect.y, navBarHeight + 471 * scaleFactor + cell2Height))
      assert.isOk(isApproximate(rect.width, 357 * scaleFactor))
    })
    .elementByXPath(cell4Path)
    .getRect()
    .then((rect)=>{
      console.log(`cell 4 rect after changing column gap to normal:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 0))
      assert.isOk(isApproximate(rect.y, navBarHeight + 471 * scaleFactor + cell1Height))
      assert.isOk(isApproximate(rect.width, 357 * scaleFactor))
    })
    .elementByXPath(cell2Path)
    .click()
  })

  it('#4 test column width', () => {
    return driver
    .elementByXPath(cell1Path)
    .click()
    .elementByXPath(cell1Path)
    .getRect()
    .then((rect)=>{
      console.log(`cell 1 rect after changing column width to 600:${JSON.stringify(rect)}`)
      cell1Height = rect.height
      assert.isOk(isApproximate(rect.x, 0))
      assert.isOk(isApproximate(rect.y, navBarHeight + 471 * scaleFactor))
      assert.isOk(isApproximate(rect.width, recyclerWidth))
    })
    .elementByXPath(cell2Path)
    .getRect()
    .then((rect)=>{
      console.log(`cell 2 rect after changing column width to 600:${JSON.stringify(rect)}`)
      cell2Height = rect.height
      assert.isOk(isApproximate(rect.x, 0))
      assert.isOk(isApproximate(rect.y, navBarHeight + 471 * scaleFactor + cell1Height))
      assert.isOk(isApproximate(rect.width, recyclerWidth))
    })
    .elementByXPath(cell1Path)
    .click()
  })

  it('#5 test deleting header', () => {
    return driver
    .elementByXPath(header2Path)
    .click()
    .elementByXPath(header1Path)
    .getRect()
    .then((rect)=>{
      console.log(`header 2 rect after deleting header 1:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 0))
      assert.isOk(isApproximate(rect.y, navBarHeight))
      assert.isOk(isApproximate(rect.width, recyclerWidth))
    })
    .elementByXPath(header2Path)
    .click()
  })

  it('#6 test footer', () => {
    return driver
    .elementByXPath(fixedPath)
    .click()
    .sleep(500)
    .elementByXPath(footerPath)
    .getRect()
    .then((rect)=>{
      console.log(`footer rect:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 0))
      assert.isOk(isApproximate(rect.y, screenHeight - 94 * scaleFactor))
      assert.isOk(isApproximate(rect.width, recyclerWidth))
      assert.isOk(isApproximate(rect.height, 94 * scaleFactor))
    })
  })

  it('#7 test sticky and fixed', () => {
    return driver
    .elementByXPath(fixedPath)
    .getRect()
    .then((rect)=>{
      console.log(`fixed rect:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 640 * scaleFactor))
      assert.isOk(isApproximate(rect.y, screenHeight - 110 * scaleFactor))
      assert.isOk(isApproximate(rect.width, 78 * scaleFactor))
      assert.isOk(isApproximate(rect.height, 78 * scaleFactor))
    })
    .elementByXPath(header2Path)
    .getRect()
    .then((rect)=>{
      console.log(`sticky header rect:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 0))
      assert.isOk(isApproximate(rect.y, navBarHeight))
      assert.isOk(isApproximate(rect.width, recyclerWidth))
      assert.isOk(isApproximate(rect.height, 94 * scaleFactor))
    })
  })

  it('#8 test removing cell', () => {
    return driver
    .elementByXPath(cell29Path)
    .click()
    .elementByXPath(cell28Path)
    .getRect()
    .then((rect)=>{
      console.log(`cell 28 rect after removing cell 29:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 0))
      assert.isOk(isApproximate(rect.y, screenHeight - 94 * scaleFactor - rect.height))
      assert.isOk(isApproximate(rect.width, 369 * scaleFactor))
    })
  })

  it('#9 test moving cell', () => {
    return driver
    .elementByXPath(cell29Path)
    .click()
    .elementByXPath(fixedPath)
    .click()
    .elementByXPath(cell29Path)
    .getRect()
    .then((rect)=>{
      console.log(`cell 28 rect after moving cell 29 to 1:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 381 * scaleFactor))
      assert.isOk(isApproximate(rect.y, screenHeight - 94 * scaleFactor - rect.height))
      assert.isOk(isApproximate(rect.width, 369 * scaleFactor))
    })
  })

  it('#10 test recycler padding', () => {
    return driver
    .elementByXPath(cell28Path)
    .click()
    .elementByXPath(fixedPath)
    .click()
    .elementByXPath(header2Path)
    .getRect()
    .then((rect)=>{
      console.log(`sticking header rect after setting padding to 12:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 12 * scaleFactor))
      assert.isOk(isApproximate(rect.y, navBarHeight))
      assert.isOk(isApproximate(rect.width, recyclerWidth - 24 * scaleFactor))
      assert.isOk(isApproximate(rect.height, 94 * scaleFactor))
    })
    .elementByXPath(footerPath)
    .getRect()
    .then((rect)=>{
      console.log(`footer rect after setting padding to 12:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 12 * scaleFactor))
      assert.isOk(isApproximate(rect.y, screenHeight - 106 * scaleFactor))
      assert.isOk(isApproximate(rect.width, recyclerWidth - 24 * scaleFactor))
      assert.isOk(isApproximate(rect.height, 94 * scaleFactor))
    })
    .elementByXPath(cell28Path)
    .getRect()
    .then((rect)=>{
      console.log(`cell 28 rect after setting padding to 12:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 12 * scaleFactor))
      assert.isOk(isApproximate(rect.width, 357 * scaleFactor))
    })
    .elementByXPath(cell29Path)
    .getRect()
    .then((rect)=>{
      console.log(`cell 29 rect after setting padding to 12:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 381 * scaleFactor))
      assert.isOk(isApproximate(rect.width, 357 * scaleFactor))
    })
    .elementByXPath(cell28Path)
    .click()
  })

  it('#11 test onscroll', () => {
    let originContentOffset = 0
    return driver
    .elementByXPath(cell29Path)
    .click()
    .elementByXPath(header2Path + '/div[1]/div[1]/text[1]')
    .text()
    .then(text => {
      console.log(text)
      originContentOffset = Number.parseInt(text.replace('Content Offset:-',''))
    })
    .touch('drag', {fromX:recyclerWidth / 2, fromY:screenHeight / 5, toX:recyclerWidth / 2, toY: screenHeight * 4 / 5})
    .sleep(1000)
    .touch('drag', {fromX:recyclerWidth / 2, fromY:screenHeight / 5, toX:recyclerWidth / 2, toY: screenHeight * 4 / 5})
    .elementByXPath(header2Path + '/div[1]/div[1]/text[1]')
    .text()
    .then(text => {
      console.log(text)
      const contentOffset = Number.parseInt(text.replace('Content Offset:-',''))
      assert.isOk(originContentOffset - contentOffset > screenHeight / scaleFactor)
    })
    .elementByXPath(fixedPath)
    .click()
  })

  it('#12 test scrollable', () => {
    let originContentOffset = 0
    return driver
    .elementByXPath(cell27Path)
    .click()
    .elementByXPath(header2Path + '/div[1]/div[1]/text[1]')
    .text()
    .then(text => {
      console.log(text)
      originContentOffset = Number.parseInt(text.replace('Content Offset:-',''))
    })
    .touch('drag', {fromX:recyclerWidth / 2, fromY:screenHeight / 5, toX:recyclerWidth / 2, toY: screenHeight * 4 / 5})
    .elementByXPath(header2Path + '/div[1]/div[1]/text[1]')
    .text()
    .then(text => {
      console.log(text)
      const contentOffset = Number.parseInt(text.replace('Content Offset:-',''))
      assert.equal(contentOffset, originContentOffset)
    })
  })
});


