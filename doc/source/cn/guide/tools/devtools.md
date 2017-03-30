---
title: 使用 Devtools
type: guide
order: 5.1
version: 2.1
has_chapter_content: true
---

# Devtools

**weex devtools** 是专门为 Weex 定制的一款实现了 [**Chrome Debugging Protocol**](https://developer.chrome.com/devtools/docs/debugger-protocol) 的 inspect/debug 工具，能够帮助你快速查看 app 运行状态和调试 weex 中的 Javascript 代码，当前支持 **iOS** 和 **Android** 两个平台。

1. 支持 Android 和 iOS 调试
2. 支持真机和模拟器
3. 可以检查 native Elements, vdom, network, Console, Resource
4. 可以调试 js/wx/Rx 等前端源码文件
5. 支持同时在前端文件和 native (Android／iOS) 文件上断点调试
6. 支持远程调试，不需要真机 USB 连接电脑
7. 支持 screencast 和 select element 功能
8. 支持同时调试多个应用和客户端

## 安装

```
$ npm install  -g  weex-toolkit
```

## 用法

 weex debug [options] [we_file|bundles_dir]

  选项:

```
-h, --help           显示帮助
-V, --verbose        显示 debug 服务器运行时的各种 log
-v, --version        显示版本
-p, --port [port]    设置 debug 服务器端口号 默认为 8088
-e, --entry [entry]  debug 一个目录时,这个参数指定整个目录的入口 bundle 文件,这个 bundle 文件的地址会显示在debug主页上(作为二维码)
-m, --mode [mode]    设置构建we文件的方式, transformer 最基础的风格适合单文件, loader:wepack 风格 适合模块化的多文件。默认为 transformer
```

#### 开启调试

```
$ weex debug
```

单纯启动一个调试服务器,并同时唤起chrome浏览器打开`调试主页`.
这个`调试主页`上会有一个二维码,使用Playground App扫这个二维码可以开启Playground调试.
开启调试后,设备列表中会出现您的设备,根据提示进行后续的调试操作
#### 调试 we文件

```
$weex debug your_weex.we
```

这个命令会将your_weex.we编译成bundlejs文件 部署到debug服务器
并启动debug服务器如上述命令那样.打开的`调试主页`会多显示一个二维码,使用playground app
扫这个码可以加载your_weex.we.(注意要先扫描开启调试的那个码)
这个命令会自动检测your_weex.we文件变动,如果发现内容被修改则立即重新编译部署,并刷新debugger页面
.
#### 调试整个bundle/we文件夹

```
$weex debug your/we/path  -e index.we
```

这个命令会编译你指定目录下的所有的we文件,并把编译好的bundlejs部署到debug服务器,他们的地址会映射到 http://lcoalhost:8088/weex/ 下
比如 your/we/path/`index.we` 可以通过http://lcoalhost:8088/weex/index.js访问  
your/we/path/`demo/test.we` 可以通过http://lcoalhost:8088/weex/demo/index.js  

-e参数可以指定一个入口的we文件,这个文件的地址会显示在`调试主页`上(作为二维码)
## 特性
### 连接设备

![devtools-main](https://img.alicdn.com/tps/TB13fwSKFXXXXXDaXXXXXXXXXXX-887-828.png)
### Inspector

 Inspector 能够用来查看 `Element` \ `NetWork` \ `Console log` \ `ScreenCast` \ `BoxModel` \ `Native View` 等。

![devtools-inspector](https://img.alicdn.com/tps/TB1O.nwKFXXXXX8XpXXXXXXXXXX-1436-811.png)
#### Element

![inspector-element](https://img.alicdn.com/tps/TB1.02bKFXXXXXwaXXXXXXXXXXX-2880-1800.png)
#### NetWork
##### 查看网络请求的总耗时和延时

![inspector-network](https://img.alicdn.com/tps/TB1NjO_KFXXXXcaaXXXXXXXXXXX-2880-1800.png)
##### 查看网络请求的header和response

![inspector-network](https://img.alicdn.com/tps/TB1ck6lKFXXXXbZXFXXXXXXXXXX-2880-1800.png)
#### 控制台

![inspector-console](https://img.alicdn.com/tps/TB1a7HqKFXXXXXMXFXXXXXXXXXX-2880-1800.png)
#### 资源

![inspector-resource](https://img.alicdn.com/tps/TB1oY6cKFXXXXXQaXXXXXXXXXXX-2880-1800.png)
### Debugger

 调试器用来调试weex中的js代码，能够设置断点、查看调用栈。 

![devtools-debugger](https://img.alicdn.com/tps/TB1aPTEKFXXXXXaXXXXXXXXXXXX-1436-813.png)
##### Breakpoint and CallStack

![debugger-breakpoint](https://img.alicdn.com/tps/TB1_trbKFXXXXc0XVXXXXXXXXXX-2880-1800.png)
#### 集成devtools
- Android
  - 请参考文档 [Weex devtools (Android)](../../references/advanced/integrate-devtool-to-android.html), 其中有详细说明。
- IOS
  - 请参考文档 [Weex devtools (iOS)](../../references/advanced/integrate-devtool-to-ios.html), 其中有详细说明。
