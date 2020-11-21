# Abstract
## Feature
### WeexCore
* Unified C++ log system of WeexCore.

### Android
* Upgrade targetsdk & compilesdk to 28
* Change Android package name from `com.taobao.weex` to `org.apache.weex`, ref [link](https://github.com/apache/incubator-weex/tree/master/android#build-types)
* Switch to BSD-Licensed [android-jsc](https://www.npmjs.com/package/jsc-android) as the default JavaScript Interpreter.

## Main Bugfix


## 0.29.0 Detail Commit
* [Android] Optimize AndroidR logic (#3291) 98704f862
* [iOS] fix image show error 66b1302e3
* [Feature] Support Android R (#3284) fd558c049
* fix faf65f132
* [iOS] fix input component wakeup UIDatePicker 7d4e96841
* [iOS] fix image can not show on XCode 12 3a8d56107
* [ios] update xcode version in travis ci 691b89046
* Revert "[iOS] set instance frame when relayout" 6eac6a710
* [ios] ios14 set datepicker style 6da2c67d3
* [iOS] set instance frame when relayout 2681e92a4
* [core] support sync call module on reactor ceb3fcfbf
* Revert "[iOS] fix image can not show when use xcode12 build" 223702e7c
* [iOS]fix waterfall crash error 59db21068
* [weex] support call weex component method aa6e96a1a
* [iOS]fix touch event conflict 6b99cedb9
* [iOS] fix image can not show when use xcode12 build e0de1afb1
* [iOS] fix sometimes chooseColor return nil bug 7b82d270d
* [iOS] fix crash when multithread call initSDKEnvironment 3da1499c4
* [iOS] adapt modal and picker modules for iPad 07b6210b3
* [iOS] add air tag for performance log 14aaa9192
* [iOS] support reactor protocol to render c3dda86b2
* [iOS] add log when download image 5fb1c168e
* [iOS] fix eagle error 35c1d1734
* [iOS] fix ci build script 43a9c09d8
* [iOS] fix bugs for iPad 0ad73cf97
* [Android]Fix rows doesn't work in textarea 4fac61cfe
* Refactor: Remove Maven mirror in Aliyun for speed. (#3168) 9ba8676ae
* Feature(Android): Support x86_64 (#3167) a675c3e15
* Refactor: Code structure for eagle (#3161) e3ce66159
* [iOS] adapt for iPad device d06ba620a
* [iOS] support module intercept 0d75d53b8
* T:  delete unused code (#3117) a9689bd93
* [iOS] Do not reserve dark scheme handler. 399e2904b
* [iOS] Do not reserve dark scheme handler. 4091743ae
* [iOS] Let application decide whether to use dark scheme. 7a58092c1
* [iOS] Let application decide whether to use dark scheme. 36d2073e9
* [iOS] fix un catched exception when nan value passed from front end 057cc79a5
* [iOS] Support dark scheme for input controls. 36ccbb1ff
* Fix Lint problems cb8c9791d
* fix bug wkwebview crash 23791716a
* [iOS] Protect timer module for NAN value. 6c122a663
* [iOS] Fix text italic width not sufficient. d5fadaf1f
* [iOS] fix waterfall component repaint bug 81abb4a9b
* Upgrade readme for iOS minimum supported version. 740bdc89e
* [iOS] Inherit auto-invert for embed. a9b5e9b2e
* [iOS] Remove log which may cause infinite loop printing invalid object. 97142138f
* [iOS] Fix dark scheme. 7793f071a
* Updatex86 wtf 821035274
* Let WxInstance know wether Page'sRenderObject is dirty to control vsync sequence 8a0737dfe
* [iOS] Fix problem that richtext background color is not correctly inverted. d893d61ed
* Add WebKit framework to WeexSDKTests target 2d4869e5a
* [iOS] Optimize dark scheme logic. 8d8751bdc
* Make WXWebView inherit from WKWebView instead of UIWebView and make WXWebComponent implement WKNavigationDelegate instead of UIWebViewDelegate cd6e78e96
* [iOS] Add unregisterSchemeChangeListener method. 4471d2400
* [iOS] Add dark scheme module for each instance. Add invertingBehavior for more precise control of each instance for dark scheme auto-inverting. df95e7b9f
* Support Tmall preInstall fbd2f9469
* Optimize for custom render target. 152c0a1f0
* [iOS] Clip module should be used in main thread. 91729bc8f
* [iOS] Do not use queue for iOS toast. 7a6a95488
* [iOS] Fix dark-scheme animation logic. 27d6bb0d1
* Merge Release/20190925 into maser (#3044) 362420665
* Use xcode11 for travis CI. 297c10f6f
* Use xcode11 for travis CI. edea9077d
* [iOS] Dark mode bugfix. f62bf91fb
* [iOS] Add missing files. 5b721ad53
* [iOS] Dark support, change 'theme' to 'scheme' and add '-weex' prefix to styles. 8e93afb6c
* [iOS] Supporting 'light-theme-' prefix. a793023f0
* [iOS] Change theme change event name. 273836dd0
* [iOS] For heron page, do not fire theme change event in Weex. e3c883401
* [iOS] Add support for dark-theme-box-shadow. b491da9c6
* [iOS] Add defaultInvertValueForRootComponent method to decide default invert value for root component. 38c88b6d5
* [iOS] Add scene type for rich text. 9d6f48d7e
* [iOS] Add switch for dark theme support. e9e1dbfe1
* Support dark mode. e0feb8921
* Use xcode11 for travis CI. d27dad8a1
* Use xcode11 for travis CI. 0ac952762
* Use xcode11 for travis CI. 8f33293d6
* [Android] fix cell animation ,same as ios demo:https://jsplayground.taobao.org/raxplayground/073b16df-c0b0-4b13-a04f-64f3bcee13f9?spm=a2o8d.corp_prod_issue_detail_v2.0.0.3c40274fTD2suj 2f6678a09
* [Android] fix mmap leak and zombie process e61d0a45f
* [iOS] Dark mode bugfix. 7ef555379
* [iOS] Add missing files. 339e69709
* [iOS] Dark support, change 'theme' to 'scheme' and add '-weex' prefix to styles. 45fd0f045
* [iOS] Supporting 'light-theme-' prefix. 5fe8c23f8
* Merge Release/20190925 into master (#3031) 0871a1b11
* [Android] support loadMore for horizontalScrollView (#3025) 2fa03e58d
* [iOS] Change theme change event name. 1dad20c22
* [iOS] For heron page, do not fire theme change event in Weex. d95f7fc6d
* fix eagle card render interaction time delay cause by the main thread queue blocked e6739fe6d
* [iOS] Add support for dark-theme-box-shadow. e234001f3
* [Android] fix memleak for apm 5fc6dcf97
* Revert "Fix Poplayer multi Thread Native Crash" 4aa2036a7
* Fix Poplayer multi Thread Native Crash 1d857d3df
* [iOS] Add defaultInvertValueForRootComponent method to decide default invert value for root component. d4d957dae
* [iOS] Add scene type for rich text. 6face6876
* [iOS] Add switch for dark theme support. a1a818d52
* Support dark mode. 6bd805e7b
* Update WXDefine.h (#3012) 0d993b883
* [iOS] check js thread block 15c22d781
* Bump serve from 1.4.0 to 7.1.3 (#3000) c9c55458f
* Bump cached-path-relative from 1.0.1 to 1.0.2 (#3002) 90ed4ce77
* Bump node.extend from 2.0.0 to 2.0.2 (#3001) e932e0d20
* Bump handlebars from 4.0.11 to 4.0.14 (#2998) 947a105b3
* Bump eslint from 2.13.1 to 4.18.2 (#2996) 941f897fd
* build: change getDeviceHost servePort (#1456) 3772faae9
* update weex-vue-render badge (#2995) 42cb930d0
* Update readme 5bdefccde
* [Android] Fix task skipping problem of download. (#2987) 032ede37e
* [iOS] update WeexSDK.podspec (#2990) d28bf4350
* Update website and license 189ccba68
* Update readme (#2984) 6a06d1391
* [iOS] Shield oc lint. d2a035157
* Merge Release/0.28 into master (#2968) 8922b1e05
* [iOS] Fix crash that creating timer with NAN interval. 2045413e3
* [data-render] fix interactive time record is not correct in wlasm mode (#2977) e5d78b635
* Fix text component's property multi-thread crash. 5177d07f4
* update HOW-TO-BUILD.md (#2969) cadd403fc
* [fix] fix npe in core_side_platform refresh an eagle instance 70b2e9cd0
* 修复当数据改变时index无效 3b61758cf
