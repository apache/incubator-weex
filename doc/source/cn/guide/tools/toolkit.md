---
title: 使用 weex-toolkit
type: guide
order: 5.1
version: 2.1
---

# weex-toolkit

[weex-toolkit](https://github.com/weexteam/weex-toolkit) 是官方提供的一个脚手架命令行工具，你可以使用它进行Weex项目的创建，调试以及打包等功能。

### 安装

使用npm安装:

``` bash
$ npm install -g weex-toolkit
```
安装成功后，你输入 `weex`应该可以看到下面的提示效果;

![weex-commands](https://img.alicdn.com/tfs/TB1NBhdQXXXXXXzXFXXXXXXXXXX-712-343.png)

如果你本地没有安装node.js你可以前往[官网](https://nodejs.org/en/)下载安装。

*请确保你的node 版本是>=6,你可以使用 [n](https://github.com/tj/n) 来进行node的版本管理*。

中国用户如果npm遭遇网络问题，可以使用淘宝的 `cnpm` 镜像， [cnpm](https://npm.taobao.org/)：

``` bash
$ npm install -g cnpm --registry=https://registry.npm.taobao.org
$ cnpm install -g weex-toolkit
```

如果你安装的过程中遇到了问题，比如*permission error*你可以去[weex-toolkit issues](https://github.com/weexteam/weex-toolkit/issues)找到解决方法。


### 初始化 weex 项目


```bash
$ weex init awesome-project
```

执行完命令后，在 `awesome-project` 目录中就创建了一个使用 `Weex` 和 `Vue` 的模板项目。


然后我们进入项目所在路径，`weex-toolkit` 已经为我们生成了标准项目结构。

在 `package.json` 中，已经配置好了几个常用的 npm script，分别是：

- `build`: 源码打包，生成 JS Bundle
- `dev`: webpack watch 模式，方便开发
- `serve`: 开启静态服务器
- `debug`: 调试模式

我们先通过 `npm install` 安装项目依赖。之后运行 `npm run dev` 和 `npm run serve` 开启`watch`模式和静态服务器。

然后我们打开浏览器，进入 `http://localhost:8080/index.html` 即可看到 Weex h5 页面。


### 实时预览

`weex-toolkit`支持预览你当前开发的weex页面(`.we`或者`.vue`)，你只需要指定预览的文件路径即可:

``` bash
$ weex src/foo.vue 
```

浏览器会自动弹出页面，这个时候你可以看到你所编辑的weex页面的具体效果和页面布局。如果你使用[playground](https://weex.apache.org/cn/playground.html)扫描右边的二维码，就能够看到Weex在 Android/IOS 设备上的效果了。

如果你需要预览整个项目目录，你可以输入这样的命令:

``` bash
$ weex src --entry src/foo.vue
```
你需要在传入的参数指定预览的目录和入口文件。

### 打包weex项目

如果开发完成后，你可以使用`weex compile`通过命令行工具进行单个文件或者整个项目的打包。

``` bash
weex compile src/foo.vue dist
```
命令行需要两个参数，你的源码文件或者目录， 以及你生成打包后的目录地址。



### 调试 Weex 页面

weex-toolkit支持调试工具。**[weex devtools](https://github.com/weexteam/weex-devtool)**，它是专门为Weex定制的一款实现了[**Chrome Debugging Protocol**](https://developer.chrome.com/devtools/docs/debugger-protocol)的inspect/debug工具，能够帮助你快速查看app运行状态和调试weex中的Javascript代码，当前支持**IOS**和**Android**两个平台。

#### 用法

``` bash
 weex debug [options] [we_file|bundles_dir]
 ```

  选项:

```
-h, --help           显示帮助
-V, --verbose        显示debug服务器运行时的各种log
-v, --version        显示版本
-p, --port [port]    设置debug服务器端口号 默认为8088
-e, --entry [entry]  debug一个目录时,这个参数指定整个目录的入口bundle文件,这个bundle文件的地址会显示在debug主页上(作为二维码)
-m, --mode [mode]    设置构建we文件的方式,transformer 最基础的风格适合单文件,loader:wepack风格 适合模块化的多文件.默认为transformer
```
#### 开启调试

```
$ weex debug
```

单纯启动一个调试服务器,并同时唤起Chrome浏览器打开`调试主页`.
这个`调试主页`上会有一个二维码,使用Playground App扫这个二维码可以开启Playground调试.
开启调试后,设备列表中会出现您的设备,根据提示进行后续的调试操作

#### 调试 we|vue文件

```
$ weex debug your_weex.vue
```

这个命令会将`your_weex.vue`编译成`bundlejs`文件 部署到debug服务器
并启动debug服务器如上述命令那样.打开的`调试vue主页`会多显示一个二维码,使用Playground App
扫这个码可以加载`your_weex.we`.(注意要先扫描开启调试的那个码)
这个命令会自动检测`your_weex.we`文件变动,如果发现内容被修改则立即重新编译部署,并刷新`debugger`页面
.
#### 调试整个bundle/we文件夹

同样你也可以调试整个目录的文件，你只需要传入目录的路径和 入口文件即可

```
$weex debug your/we/path  -e index.we
```

这个命令会编译你指定目录下的所有的we文件,并把编译好的bundlejs部署到debug服务器,他们的地址会映射到 http://lcoalhost:8088/weex/ 下
比如 `your/we/path/index.we` 可以通过http://lcoalhost:8088/weex/index.js访问  
`your/we/path/demo/test.we` 可以通过http://lcoalhost:8088/weex/demo/index.js  

-e参数可以指定一个入口的we文件,这个文件的地址会显示在`调试主页`上(作为二维码)

#### 特性
##### 连接设备

![devtools-main](https://img.alicdn.com/tps/TB13fwSKFXXXXXDaXXXXXXXXXXX-887-828.png)
##### Inspector

 Inspector 能够用来查看 `Element` \ `NetWork` \ `Console log` \ `ScreenCast` \ `BoxModel` \ `Native View` 等。

![devtools-inspector](https://img.alicdn.com/tps/TB1O.nwKFXXXXX8XpXXXXXXXXXX-1436-811.png)
##### Element

![inspector-element](https://img.alicdn.com/tps/TB1.02bKFXXXXXwaXXXXXXXXXXX-2880-1800.png)
##### NetWork
##### 查看网络请求的总耗时和延时

![inspector-network](https://img.alicdn.com/tps/TB1NjO_KFXXXXcaaXXXXXXXXXXX-2880-1800.png)
##### 查看网络请求的header和response

![inspector-network](https://img.alicdn.com/tps/TB1ck6lKFXXXXbZXFXXXXXXXXXX-2880-1800.png)
##### 控制台

![inspector-console](https://img.alicdn.com/tps/TB1a7HqKFXXXXXMXFXXXXXXXXXX-2880-1800.png)
##### 资源

![inspector-resource](https://img.alicdn.com/tps/TB1oY6cKFXXXXXQaXXXXXXXXXXX-2880-1800.png)
#### Debugger

 调试器用来调试weex中的js代码，能够设置断点、查看调用栈。 

![devtools-debugger](https://img.alicdn.com/tps/TB1aPTEKFXXXXXaXXXXXXXXXXXX-1436-813.png)
##### Breakpoint and CallStack

![debugger-breakpoint](https://img.alicdn.com/tps/TB1_trbKFXXXXc0XVXXXXXXXXXX-2880-1800.png)
#### 集成devtools
- Android
  - 请参考文档 [Weex devtools (Android)](../../references/advanced/integrate-devtool-to-android.html), 其中有详细说明。
- IOS
  - 请参考文档 [Weex devtools (iOS)](../../references/advanced/integrate-devtool-to-ios.html), 其中有详细说明。
  
  
### weex-toolkit 对 weexpack的水平扩展

[weexpack](https://github.com/weexteam/weex-pack) 是基于 Weex 快速搭建应用原型的利器。它能够帮助开发者通过命令行创建 Weex 工程，添加相应平台的 Weex app 模版，并基于模版从本地、GitHub 或者 Weex 应用市场安装插件，快速打包 Weex 应用并安装到手机运行，对于具有分享精神的开发者而言还能够创建 Weex 插件模版并发布插件到 Weex 应用市场。

现在使用`weex-toolkit`同样支持对`weexpack`的命令调用,如果你当前的项目与`weexpack`生成的项目目录一致，那么你可以直接实现对于platform的操作，从而构建具体的Android/IOS app。

### weex platform 以及 run 命令

如果我们希望在模拟器或者真机上查看Weex运行的效果，我们可以使用 `platform` 添加或者删除 Weex 应用模板。

``` bash
$ weex platform add ios 
```
在第一次使用platform/plugin命令的时候，可能会遇到下面的界面，你只需要输入Y或者直接enter键即可。
![install weexpack](https://gw.alicdn.com/tfs/TB19n4AQXXXXXawXVXXXXXXXXXX-577-70.png)

添加ios平台，然后这个时候只要输入:

``` bash
$ weex run ios
```

就能看到启动的模拟器运行的效果了。


### weex plugin 命令

如果你想使用[插件市场](https://market.weex-project.io/)的插件，你可以使用

```bash
$ weex plugin add plugin_name
```

你只需要输入插件的名称就可以从远程添加插件到你本地的项目，比如添加weex-chart,我们可以输入命令:

``` bash
$ weex plugin add weex-chart
```


我们可以使用`plugin remove`移除插件,比如移除安装好的weex-cahrt:

``` bash
$ weex plugin remove weex-chart
```

关于weexpack 更加详细的介绍，你可以阅读 [官方文档](https://github.com/weexteam/weex-pack)。



