# Weex

A framework for building Mobile cross-platform UI.

[![Build Status](https://travis-ci.org/apache/incubator-weex.svg?branch=master)](https://travis-ci.org/apache/incubator-weex/)

## Convenience Distribution 
**Since 0.28.0, Weex would publish two convince binary in each release for Android, please [read the documentation about the detail](major_change.html).**

**Please take the above link seriously, otherwise you would be able to use the latest version of Weex.**

| platform | status |
| -------- | ------ |
| Android | [sdk ![Download](https://api.bintray.com/packages/weex/Android/sdk/images/download.svg)](https://bintray.com/weex/Android/sdk/_latestVersion) Or [sdk_legacy ![Download](https://api.bintray.com/packages/weex/Android/sdk_legacy/images/download.svg)](https://bintray.com/weex/Android/sdk_legacy/_latestVersion) |
| iOS | [![Pod version](https://badge.fury.io/co/WeexSDK.svg)](https://cocoapods.org/pods/WeexSDK) [![Carthage compatible](https://img.shields.io/badge/Carthage-compatible-4BC51D.svg?style=flat)](https://github.com/Carthage/Carthage) |
| [Mobile Web](https://github.com/weexteam/weex-vue-render) | [![npm version](https://badge.fury.io/js/weex-html5.svg)](https://www.npmjs.com/package/weex-vue-render) |

Support Android 4.1 (API 16), iOS 8.0+ and WebKit 534.30+.

## For Windows
First of all, compiling or building Weex from Windows is not support officially.

You could install [Git for Windows](https://git-scm.com/download/win) and run all the following commands in git-bash.

Good Luck.

## Meet Weex

* Install [Weex Playground App](https://weex.apache.org/tools/playground.html) to see examples we already written.
* If you want to write a demo, install [weex-toolkit](https://www.npmjs.com/package/weex-toolkit) in [Node.js](http://nodejs.org/) 8.0+ and
* Run `weex init` to generate & start a simple project in an empty folder.
* Follow the instructions in the project README.
* Enjoy it.

## Use Weex

* [Tutorial](http://weex.apache.org/guide/)
* [Documents](http://weex.apache.org/references)

### Android
**You should install [android environment](./HOW-TO-BUILD.md#android) before building.**

You can either build Weex from IDE (*Android Studio*) or command line.

#### Build From Android Studio
1. Open `android` directory in Android Studio.
2. Run `git submodule update --init --remote` in `android` directory if this is the first time you try to run Weex.

#### Build From Command Line
Please read [How To Build](./HOW-TO-BUILD.md) for detail.

### iOS
**You should install [iOS environment](./HOW-TO-BUILD.md#ios) before building.**

You can either build Weex from IDE (*XCode*) or command line.

#### Build From XCode
* Run playground
  * `cd ios/playground`
  * `pod install`
  * Open `WeexDemo.xcworkspace` in Xcode
  * Click <img src="http://img1.tbcdn.cn/L1/461/1/5470b677a2f2eaaecf412cc55eeae062dbc275f9" height="16" > (`Run` button) or use default shortcut `cmd + r` in Xcode
  * If you want to run the demo on your device, don't need to modify `CURRENT_IP` manually. ~~In `DemoDefine.h`(you can search this file by Xcode default shortcut `cmd + shift + o`), modify `CURRENT_IP` to your local IP~~
* integrate to your application

  - **[CocoaPods](https://cocoapods.org)**

     Add the following line to your Podfile:
   ```
     pod 'WeexSDK'
   ```
    run `pod install`

  - **[Carthage](https://github.com/carthage/carthage)**

    Add the following line to your Cartfile:
   ```
     github "apache/incubator-weex"
   ```
   Run `carthage update`, and you should now have the latest version of   `WeexSDK` in your `Carthage` folder.

#### Build From Command Line
Please read [How To Build](./HOW-TO-BUILD.md) for detail.

### Mobile Web

see [weex-vue-render](https://github.com/weexteam/weex-vue-render).

## Scripts

See [SCRIPTS.md](./SCRIPTS.md) for more information.

## IDE Plugin & Syntax Highlight & DevTool

Weex team have developed a [DevTool](https://github.com/weexteam/weex-devtool) to help you to improve the debugging efficiency.

See more stuff on [this wiki page](https://github.com/alibaba/weex/wiki/Weex-Community)

## Weex Community
* [Mailing List](https://weex.apache.org/guide/contribute/how-to-contribute.html#mailing-list) Weex Mailing List, where most discussion happens.
* [StackOverflow](http://stackoverflow.com/questions/tagged/weex): Ask questions about Weex.
* [SegmentFault (cn)](https://segmentfault.com/t/weex): 中文交流与讨论
* [Telegram Russian Community Group](https://telegram.me/weex_ru)

### Contributing

See [Weex Contributing Guide](./CONTRIBUTING.md) for more information.
