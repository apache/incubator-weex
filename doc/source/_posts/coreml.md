title: Weex ❤️ iOS 11 系列之一：Core ML 初探
type: blog
date: 2017-06-21 15:46:44

---

## 背景

WWDC 2017 有一个非常明确的信号：**端上机器学习将会是未来苹果的一个重要发力点，同时开发者的使用门槛在不断降低**。

Core ML 是今年 WWDC 上苹果发布的机器学习框架。它允许开发者在自己开发的 app 上使用机器学习，同时不需要收集用户数据。 有了 Core ML，你可以将自己训练得到的模型，借助苹果提供的转换工具，转化成 Core ML 类型的模型文件，你可以非常轻松地直接实现模型和 app 数据的互通。

WWDC 上展示了一张简单的架构图：

![屏幕快照 2017-06-20 下午8.50.21](https://gw.alicdn.com/tfs/TB1YT5dRVXXXXc1XFXXXXXXXXXX-498-392.png)

最底层的 Accelerate 和 MPS，其实是苹果去年推出的接口，前者可以直接调用 CPU，后者则直接调用 GPU。 Core ML 会自行决定到底使用 CPU 还是 GPU 来进行运算，可以最大程度地利用硬件资源来处理模型数据。

而 Core ML 往上一层则是应用层。Vision 主要负责图像相关的人工智能处理，如人脸识别、面部追踪、文字识别等等；NLP 则提供语义分析相关 API。这部分内容在以前版本的 SDK 中也存在，这次被集成到了新框架当中。好处在于，开发者拥有了更大的自由度。在以前，开发者的 app 调用这些高级接口，对底层的实现细节的掌控力度非常小。现在，你可以通过自定义底层所使用的机器学习模型，从而实现更个性化的需求。

## Weex-ML

Weex 交互视觉能力小结中提到，**Weex 最大的特别之处在于其 native 的能力， 我们要最大化利用 native 的特性， 而不是去模仿它**。

作为本次 iOS 11 的一个重要 native 能力，我也第一时间尝试将 Core ML 透传到了 Weex 上。所有代码和 Demo 都已开源到 [Github](https://github.com/cxfeng1/Weex-ML)，后续也会将机器学习相关的Weex 模块统一放到 Weex Market 上。

得益于 Core ML API 的简洁，在 Weex 上使用 Core ML 相关能力变得非常简单:

```javascript
let imageRecognition = weex.requireModule('imageRecognition')
imageRecognition.predictWithImage(this.$refs.image.ref, (results)=>{
  this.results = results.slice(0,5);
})
```

通过调用透传的 module `imageRecognition` , 并传入 image 组件的 ref 作为参数，就可以实现图片识别的功能，最后提取出识别结果中 confidence 最高的 5 个结果。

## Demo

### imageRecognition

![](https://gw.alicdn.com/tfs/TB1AmqgRVXXXXcmXFXXXXXXXXXX-240-427.gif)

使用 [Inceptionv3](https://github.com/fchollet/keras/blob/0bb4e0fad5b4bb3743c8a7d03c260b62a35e7045/keras/applications/inception_v3.py) 作为模型，能够高效识别， 该模型已经在 [keras](https://github.com/fchollet/keras) 中被训练好并通过苹果提供的工具转换成 **mlmodel** 文件， 只需拖拽的方式就可以集成到 iOS 开发环境中， 上述 demo 输出了对一张图片分类最大可能的5个预测结果， 对于 Weex 来说，几行代码就能搞定。



### digitDetect

![](https://gw.alicdn.com/tfs/TB1nZ8_RVXXXXczXVXXXXXXXXXX-240-427.gif)

上述 demo 首先使用 [weex-gcanvas](http://market.dotwe.org/ext/list.htm#6) 插件做了一个简单的绘制板，然后使用 MNIST model 完成了对手写数字的识别。

### faceDetect

![](https://gw.alicdn.com/tfs/TB1lN0_RVXXXXa_XVXXXXXXXXXX-240-427.gif)

这个 demo 没有加载任何 model，使用了 iOS 11 内置的 Vision Framework，顾名思义，Vision 可以让开发者运行计算机视觉相关的任务，它相当于 Core ML 的 High-Level Solution， 它可以做很多事情:

- 脸部识别，会返回一个矩形数组来表示每一个脸部
- 脸部更精细识别，比如眼睛和鼻子的具体位置、头部的形状等，上述 demo 就是拿到这些信息以后使用 gcanvas 绘制出了面部精细轮廓
- 图片中的矩形探测
- 图片中的文字探测
- 二维码/条形码探测
- 视频中跟踪移动的物体

## 总结

这次 Core ML 的推出，同时得益于苹果对软硬件的全面掌控，使得从系统级到第三方开发者，都具备了在端上就可以进行机器学习模型处理的能力。所有代码和 Demo 都已上传到 [Github](https://github.com/cxfeng1/Weex-ML)，欢迎贡献和交流。

当然在实际研究过程中，也发现了 Core ML 的一些限制：

- 无法在端上进行模型的训练，所有模型必须提前训练好再转换成 Core ML 的格式
- 无法动态下载和更新模型，对于需要经常重新训练的模型不太友好
- 目前看只支持监督学习的模型，不支持非监督学习。

Weex 未来在提供更多功能透传的同时，也会继续探索更深层次的结合。

最后，本文是 Weex 和 iOS 11 结合系列的第一篇文章，后续将会有更多有意思的iOS 11 feature (如 ARKit、 drag & drop 等) 和大家一起探讨。