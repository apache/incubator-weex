---
title: 使用 Weexpack
type: guide
order: 5.2
version: 2.1
---

# 使用 Weexpack

## weexpack 介绍

weexpack 是 weex 新一代的工程开发套件，是基于 weex 快速搭建应用原型的利器。它能够帮助开发者通过命令行创建 Weex 工程，添加相应平台的 Weex app 模版，并基于模版从本地、GitHub 或者 Weex 应用市场安装插件，快速打包 Weex 应用并安装到手机运行，对于具有分享精神的开发者而言还能够创建 Weex 插件模版并发布插件到 Weex 应用市场。

**注意：** weexpack 目前暂不支持搭建 Vue 2.0 版本的 Weex。

weexpack 所提供的命令大致可分为三组，分别是：

* 打包命令
 * **weexpack create** — 创建 Weex 工程项目。
 * **weexpack platform add/remove** — 安装／移除 Weex 应用模版，默认模版支持 Weex bundle 调试和插件机制。
 * **weexpack platform list** — 查看已安装的平台模版及版本。
 * **weexpack platform run** - 打包应用并安装到设备运行。


* 插件使用者命令
 * **weexpack plugin add/remove** — 安装／移除 Weex 插件，支持从本地、GitHub 或者  Weex 应用市场安装插件。

 * **weexpack plugin list** — 查看已安装的插件及版本。


* 插件开发者命令

 * **weexpack plugin create** - 生成 Weex 插件模版，主要是配置文件和必需的目录。
 * **weexpack plugin publish** - 发布插件到 Weex 插件市场。


## 安装

### 环境要求

 - 目前支持 Mac、windows、linux 平台 ( windows 下仅能打包 android )。
 - 配置 [Node.js][1] 环境，并且安装 [npm][2] 包管理器 (`需要 node6.0+`)。
 - 如果要支持 iOS 平台则需要配置 iOS 开发环境：
     - 安装 [Xcode IDE][3] ，启动一次 Xcode ，使 Xcode 自动安装开发者工具和确认使用协议。
     - 安装 cocoaPods。
 - 如果要支持 Android 平台则需要配置 Android 开发环境：
    - 安装 [Android Studio][4]（推荐）或者 [Android SDK][7]。打开 [AVD Manager][5] ，新建 Android 模拟器并启动（如果有安装 [Docker][6] ，请关闭 Docker Server ）。
    - 保证 Android build-tool 的版本为 23.0.2。

#### 安装命令

首先，全局安装 weex-pack 命令：

```bash
$ npm install -g weexpack
```

或者 在 clone 的 weexpack 根目录下执行

```bash
$ npm install
```

## 打包及插件使用

打包主要过程如下图所示，其中插件操作是打包过程的可选项，不是必须的。

![weexpack1](https://img.alicdn.com/tps/TB1.bpVPXXXXXarapXXXXXXXXXX-462-310.png)

### 详细步骤

#### 1. 创建 weexpack 工程

```bash
$ weexpack create appName
```

生成工程的目录如下：

```
WeexProject
├── README.md
├── android.config.json
├── config.xml
├── hooks
│   └── README.md
├── ios.config.json
├── package.json
├── platforms     // 平台模版目录
├── plugins       // 插件下载目录
│   └── README.md
├── src           // 业务代码（we文件）目录
│   └── index.we
├── start
├── start.bat
├── tools
│   └── webpack.config.plugin.js
├── web
│   ├── index.html
│   ├── index.js
│   └── js
│       └── init.js
└── webpack.config.js
```

通过 create 命令创建的工程默认不包含 iOS 和 Android 工程模版，创建完成之后就可以切换到 appName 目录下并安装依赖。

```bash
$ cd appName && npm install
```

#### 2. 安装 Weex 应用模版

添加应用模版，官方提供的模版默认支持 Weex bundle 调试和插件机制，注意模版名称均为小写，模版被安装到 platforms 目录下。

* android 模版

  ```bash
  $ weexpack platform add android
  ```

* iOS 模版

  ```bash
  $ weexpack platform add ios
  ```

android 平台，安装后 platforms 目录如下

```
platforms
└── android
    ├── LICENSE
    ├── NOTICE
    ├── README.md
    ├── app
    │   ├── build
    │   ├── build.gradle
    │   ├── proguard-rules.pro
    │   └── src
    ├── appframework
    │   ├── build
    │   ├── build.gradle
    │   ├── proguard-rules.pro
    │   └── src
    ├── build
    │   └── generated
    ├── build.gradle
    ├── codeStyleSettings.xml
    ├── gradle
    │   └── wrapper
    ├── gradle.properties
    ├── gradlew
    ├── gradlew.bat
    ├── settings.gradle
    └── weexplugin
        ├── build.gradle
        ├── proguard-rules.pro
        └── src 
                              
```

ios平台，安装后 platforms 目录如下
  
```
platforms
   └── ios
       ├── LICENSE
       ├── Podfile
       ├── README.md
       ├── WeexDemo
       │   ├── AppDelegate.h
       │   ├── AppDelegate.m
       │   ├── Assets.xcassets
       │   ├── DemoDefine.h
       │   ├── Images.xcassets
       │   ├── WeexConfig
       │   ├── WeexDemo-Info.plist
       │   ├── WeexScanner
       │   ├── config.xml
       │   ├── main.m
       │   └── weex-icon.png
       ├── WeexDemo.xcodeproj
       │   ├── project.pbxproj
       │   ├── project.xcworkspace
       │   ├── xcshareddata
       │   └── xcuserdata
       ├── WeexDemo.xcworkspace
       │   ├── contents.xcworkspacedata
       │   ├── xcshareddata
       │   └── xcuserdata
       ├── WeexDemoTests
       │   ├── Info.plist
       │   └── WeexDemoTests.m
       ├── WeexUITestDemo-Info.plist
       ├── WeexUITestDemoUITests
       │   ├── Info.plist
       │   └── WeexUITestDemoUITests.m
       ├── Weexplugin
       │   ├── Podfile
       │   ├── Weexplugin
       │   ├── Weexplugin.podspec
       │   ├── Weexplugin.xcodeproj
       │   └── WeexpluginTests
       ├── bundlejs
  
  ```

对于已安装的模版可以使用 weexpack platform list 命令查看。

* 查看已安装模版

  ```bash
  $ weexpack platform list
  ```

  示例结果：

  ```
  Installed platforms:
    android
  Available platforms:
    android ^6.2.1
  ```
    
如果想要删除某个不需要的平台可以使用 weexpack platform remove 移除，比如 Windows 平台用户是不需要 iOS 模版的，可用如下命令移除。


* 移除相应平台模版

  ```bash
  $ weexpack platform remove ios
  ```

#### 3. 安装 Weex 插件（可选项）

添加想要使用的插件，支持从本地或者 Weex 应用市场安装插件。

* 从本地添加插件，在开发插件时会经常用到

  ```bash
  $ weexpack plugin add path/to/plugin
  ```

* 从插件市场添加插件，例如 weex-chart。

  ```bash
  $ weexpack plugin add weex-chart
  ```

模版的 weexplugin 是插件安装的目标工程。以 Android 为例，其目录如下：

```
weexplugin
├── build.gradle  //  插件编译脚本，工具自动维护
├── libs
├── proguard-rules.pro
├── src
│   └── main
│       ├── AndroidManifest.xml // 插件android manifest配置文件
│       ├── java
│       │   ├── // 插件src安装目录
│       │   └── com
│       │       └── alibaba
│       │           └── weex
│       │               └── plugin
│       │                   ├── ConfigXmlParser.java // 配置文件解析器
│       │                   ├── PluginConfig.java    // 外部接口
│       │                   ├── PluginEntry.java     // 插件描述
│       │                   └── PluginManager.java   // 插件管理器
│       └── res // 插件资源安装目录
│           ├── drawable
│           ├── values
│           │   └── strings.xml
│           └── xml
│               └── config.xml // 插件配置文件，PluginManager 解析配置文件加载插件
```

对于已安装的插件可以使用 weexpack plugin list 命令查看。

* 查看已安装插件

  ```bash
  $ weexpack plugin list
  ```

如果想要删除某个不需要的插件可以使用 weexpack plugin remove 移除，比如 weex-chart

* 移除插件，例如 weex-chart

  ```bash
  $ weexpack plugin remove weex-chart
  ```



#### 4. 打包应用并安装运行

完成以上步骤并 Weex 代码放在 src 目录下，就可以打包运行了，打包过程中可能要下载依赖和编译工具，这会是个较为耗时的过程，安装运行需要打开模拟器或者连接设备。

* 打包运行 android 应用

  ```bash
  $ weexpack run android
  ```

  你可以更改项目目录下的 android.config.json
    * AppName: 应用名
    * AppId: application_id 包名
    * SplashText: 欢迎页上面的文字
    * WeexBundle: 指定的 Weex bundle 文件（支持文件名和 url 的形式）

  文件名则以本地文件的方式加载 bundle, url 则以远程的方式加载 bundle
  如果以本地方式指定 bundle, `.we` 文件请放到 src 目录。

* 打包运行 iOS 应用
  * 模拟器运行

  ```bash
  $ weexpack run ios
  ```

  * 构建 ipa 包

  ```bash
  $ weexpack build ios
  ```

  构建包的过程中，将会提示让您输入 `CodeSign（证书）`，`Profile(provisioning profile)`,`AppId`，只有输入真实的这些信息才能成功打包。
  其余如 AppName , 和入口 Weex bundle 文件可以编辑项目目录下的 ios.config.json 配置。
  打完包成功之后，可以在 `/playground/build/ipa_build/` 目录下获取 ipa 文件

  **注：证书需要预先安装到 keychain 中，在 keychain 中点击右键获取证书 `id`（证书名称），`provisioning profile` 文件（*mobileprovision）需要获取 UUID，进入[目录](https://github.com/weexteam/weex-pack/tree/dev/generator/platforms/templates) 可以看到 `mobileprovision_UUID.sh` 文件，此文件可以获取到 UUID**

  `mobileprovision_UUID.sh` 用法如下：
  
  ```bash
  $  ./mobileprovision_UUID.sh   \*mobileprovision
  ```

  参数（\*mobileprovision）为 `provisioning profile` 文件路径

* 打包 Web 平台：

  ```bash
  $ weexpack build web
  ```
     
  这样你可以把打包后的资源上传到 CDN 服务器，然后上线你的 Web 项目。
  
* 在 Web 平台运行：

  ```bash
  $ weexpack run web
  ```
     
#### 5.现有应用集成插件 —— 组件容器

* 组件容器作用

  插件可以通过组件容器的方式集成到客户端（app），组件容器可以方便生成二进制包，现有应用可以通过各种形式把二进制包或源码集成进去，比如 iOS 可以通过 pod 依赖   的形式等。在 weexpack 创建的根目录下，运行 weexpack plugin add/remove 命令直接安装/删除 Weex plugin 到组件容器中。

* 组件容器获取
	* 方式1：通过第二步<sup>[[link](#weex-plugin)]</sup>生成的 Android/iOS 目录中包含 Weexplugin 目录，此目录是组件容器
	* 方式2：通过 repo 地址 clone 代码 [iOS地址](https://github.com/weexteam/weexpluginContainer-iOS) [Android 地址](https://github.com/weexteam/weexplugin-android) 

## 插件发布

对于插件开发者来说，也有一组用于创建和发布插件的命令，[前往](./README_plugin.md) 查看详细文档

![weexpack2](https://img.alicdn.com/tps/TB18hxjPXXXXXXgapXXXXXXXXXX-248-258.png)

## 插件开发文档

+ [开发 web 的插件教程](https://github.com/weexteam/weex-pack/blob/master/doc/plugin-devloping-web.md)


## 示例
[使用 Weexpack 打包出一个 Weex 版的 《One App》](https://github.com/weexteam/weex-pack/wiki/Create-Weex-One-App-with-Weexpack)

[1]: https://nodejs.org/
[2]: https://www.npmjs.com/
[3]: https://itunes.apple.com/us/app/xcode/id497799835?mt=12
[4]: https://developer.android.com/studio/install.html
[5]: https://developer.android.com/studio/run/managing-avds.html
[6]: https://www.docker.com/
[7]: https://developer.android.com/studio/releases/sdk-tools.html
[8]: https://developer.android.com/studio/run/managing-avds.html
