title: Weex ❤️ iOS 11 系列之二：ARKit初探
type: blog
date: 2017-06-27 14:32:10

---

## ARKit
苹果在wwdc 2017公布了第一款具有AR功能的操作系统iOS11，并提供了AR的SDK，ARKit，同时宣称苹果是世界上最大的AR平台，Weex也积极拥抱AR，让Weex用户也可以方便开发AR功能

AR体验就是创建和追踪用户所在的现实世界和可建立模型的空间的对应关系的能力，让用户感觉产生虚拟内容在现实世界的错觉，这就是所谓的增强现实。ARKit通过识别图像场景中的显著特征，追踪这些特征的差异位置，最后与传感器数据比较，得到设备位置和运动状态的高精度模型

## 原理
ARkit 在ARSCNView，ARSession,ARFrame的基础上实现

### ARSCNView和ARSession
![x](http://upload-images.jianshu.io/upload_images/6271687-d5b0e534ef0d7aaa.png?imageMogr2/auto-orient/strip%7CimageView2/2)
ARKit提供了两种增强现实的视图，他们分别是ARSCNView(3D)和ARSKView(2D),这两种视图都采用ARCamera作为视图背景的，而ARSCNView等视图和相机没有直接关系，他们是通过ARSession这个桥梁和camera交互的，运行一个session，需要指定会话追踪配置的对象：ARSessionConfiguration，ARSessionConfiguration负责追踪相机在3D世界中的位置和平面探测

### ARWorldTrackingSessionConfiguration与ARFrame
![x](https://github.com/kfeagle/firstdemo/blob/master/arkit.png?raw=true)
ARWorldTrackingSessionConfiguration通过一系列的复杂计算和调用传感器来跟踪设备的方向和位置，当ARWorldTrackingSessionConfiguration计算出位置，会交给ARSession管理,ARSession就会维护一个相机的位置ARFame，用户可以通过这个frame将4x4矩阵世界坐标转换为3D系统相机坐标

## weex-ar
ARKit在wwdc上带来的震撼效果，我通过weex插件的方式将ARKit能力透传到Weex，让用户在weex上也第一时间体验到Native的AR效果
所有的demo以及源码我已经开源，同步到[github](https://github.com/kfeagle/weex-ar)，用户在自己工程依赖即可使用

### 在现实世界渲染一个3D的虚拟物体
```
this.$refs['scene'].addNode({
        name:'color',
        width:0.1,
        height:0.1,
        length:0.1,
        chamferRadius:0,
        vector:{
          x:0.1,
          y:0.1,
          z:-0.5
        },
        contents:{
          type:'color',
          name:'red'
        }
      });
```
Weex在compnonent中增加addNode方法，用户可以设置好节点的参数，然后就会构建一个虚拟物体
- 效果
   - ![x](http://gitlab.alibaba-inc.com/Jerry/amapdemo/raw/master/ar/1.gif)

### 交互支持
当native触发了点击事件，会向jsfm发送点击位置信息

```
-(void)tapped:(UITapGestureRecognizer *)recognizer
{
    SCNView *sceneView = (SCNView *)recognizer.view ;
    CGPoint touchLocation =  [recognizer locationInView:sceneView];
    [self fireEvent:@"tap" params:@{@"touchLocation":@{@"x":@(touchLocation.x),@"y":@(touchLocation.y)}}];
}
```
当VUE接收到相关信息，会处理位置信息，然后更新节点
```
tap:function (event) {
        this.index = this.index+1;
        if(this.index>3){
          this.index = 0;
        }
        var color = 'red';
        if(this.index == 1){
          color = 'blue';
        }
        if(this.index == 2){
          color = 'green';
        }
        if(this.index == 3){
          color = 'yellow';
        }
        this.$refs['scene'].updateNode({
          name:'color',
          x:event.touchLocation.x,
          y:event.touchLocation.y,
          color:color
        })
      }
```
- 效果
   - ![x](http://gitlab.alibaba-inc.com/Jerry/amapdemo/raw/master/ar/tap.gif)

### 碰撞游戏
设置节点的mask，通过arkit的代理就可以检测碰撞了
```
node.physicsBody.categoryBitMask= [self getMask:[WXConvert NSInteger:[options objectForKey:@"categoryBitMask"]]];
    node.physicsBody.contactTestBitMask= [self getMask:[WXConvert NSInteger:[options objectForKey:@"contactTestBitMask"]]];
```
当碰撞发生时候，会通过physicsWorld:(SCNPhysicsWorld *)world didBeginContact:(SCNPhysicsContact *)contact代理获取到节点的信息，weex将相关信息发送到jsfm
vue获取先关节点可以删除节点，增减节点，从而完成碰撞的处理
```
contact:function (event) {
                if(event.nodes.nodeA.mask == 0 || event.nodes.nodeB.mask == 0 ){

                    this.isAdd = false;
                    this.$refs['scene'].removeNode(event.nodes.nodeA.name);
                    this.$refs['scene'].removeNode(event.nodes.nodeB.name);

                }

            }
```

- 效果

  - ![x](http://gitlab.alibaba-inc.com/Jerry/amapdemo/raw/master/ar/game.gif)