---
title: Weex语法支持插件
type: guide
order: 5.1
version: 2.1
---

# Weex语法支持插件 

[Weex Language Support](https://plugins.jetbrains.com/plugin/9189-weex-language-support) 插件是官方提供的一个工具，你可以使用它在IntelliJ IDEA，WebStorm等一系列IDE上对Weex DSL进行语法高亮，自动补全和错误检查等操作。

### 支持的IDE
你可以在任何操作系统上的下列IDE上安装和使用Weex Language Support插件：    
**IntelliJ IDEA Ultimate, PhpStorm,  WebStorm,  PyCharm,  RubyMine,  AppCode,  CLion,  Gogland,  Rider**

### 安装
在IDE的插件仓库中搜索`Weex Language Support`来安装该插件，安装完毕后重启IDE即可激活插件相关功能    
![install plugin](https://img.alicdn.com/tfs/TB1y6nrXwvGK1Jjy0FdXXaxzVXa-1316-462.png)

### 配置
打开`Preferences -> Other Settings -> Weex language support`可配置插件的相关功能    
![plugin settings](https://img.alicdn.com/tfs/TB1FonrXwvGK1Jjy0FgXXX9hFXa-559-244.png)    
- Target Weex Version: 配置插件以哪一个版本的语法规则来对DSL进行提示及检查，默认值`LATEST`表示总是应用最行新版本weex的语法规则
- Vue Support: 配置插件是否支持Weex 2.0版本的DSL(.vue文件)，开启后重启生效（注意：如果IDE内有其他支持Vue语法的插件，则需要关闭相应的插件后Weex插件才能生效）
- Custom Rules: 引入自定义的Weex DSL规则，如果你在native中定义了自己的Module或Component，可通过自定义规则引入插件中来提供相应的提示和补全支持，自定义规则的格式将在后文列出
- Global Weex Components: 默认地，插件会解析当前工程及npm root路径下的`node_modules`目录，解析其中包含的Weex Components并对其提供补全支持。如果你的项目中引用了这两个路径以外的Components，可以在此处将其添加到搜索路径中，插件将会将其中的Components载入，并在编写DSL时为相应的标签提供补全支持

### 自定义规则格式
自定义规则包含在一个json文件中，json文件的根节点为数组类型，数组中的每一个元素对应DSL中的一个标签。
我们以`<loading>`标签的规则来举例：
```js
{
    "tag": "loading", //标签名，不可为空
    "attrs": [ //标签属性列表，可为空
      {
        "name": "display", //属性名，不可为空
        "valuePattern": null, //属性值的正则表达式，用于检测值是否合法，可为空
        "valueEnum": [ //属性值枚举，可为空
          "show",
          "hide"
        ],
        "valueType": "var", //属性值类型，必须是var或function，决定该从数据列表还是函数列表中查找属性值补全的候选值，不可为空
        "since": 0, //该属性何时被添加到sdk中，例如0.11，默认为0
        "weexOnly": false //该属性是否仅在1.0语法中可用，默认为false
      }
    ],
    "events": [ //事件列表。可为空
      {
        "name": "loading", //事件名称，不可为空
        "since": 0 //该事件何时被添加到sdk中
      }
    ],
    "parents": [ //该标签允许被作为哪些标签的子元素，空表示可以作为任意元素的子元素
      "list",
      "scroller"
    ],
    "childes": [ //该标签允许哪些元素作为自己的子元素，空表示任意元素都可作为子元素
      "text",
      "image",
      "loading-indicator"
    ],
    "document": "/references/components/loading.html" //文档地址，配置该属性之后可在编辑界面中对应的标签上直接打开文档
  }
```    

### 使用
插件的绝大部分功能被集成到编辑器上下文中，会随用户输入在需要补全，提示或Lint时被触发，无需特殊干预。下列功能需要用户手动触发：
#### 文档搜索
打开IDE右侧工具栏的`Weex Documents`即可对文档进行搜索，搜索结果与官网保持同步，勾选 `EN` 可切换搜索结果为英文内容    
![doc search](https://img.alicdn.com/tfs/TB1ihvDXE6FK1Jjy0FoXXXHqVXa-360-430.png)

#### 打开标签对应的文档
将光标定位到标签上，并通过`Show Intention Actions`操作（OSX上默认键为 option + enter，可通过Keymap查看）打开Intenion菜单，选择`Open Document`可打开标签对应的文档    
![open doc](https://img.alicdn.com/tfs/TB1LeLDXDzGK1JjSspjXXcHWXXa-416-86.png)

### 参与插件建设
请将Issues及Pull Requests提交到[weex-language-support](https://github.com/misakuo/weex-language-support)项目中